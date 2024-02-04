#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>

// #define MANCHESTER_DEBUG

#define AVTK_PULSE_DURATION_MID_D 480
#define AVTK_PULSE_DURATION_MIN_D 370
#define AVTK_PULSE_DURATION_MAX_D 590


const u_int16_t AVTK_PulseDuration = AVTK_PULSE_DURATION_MID_D;
const u_int16_t AVTK_PulseMinDuration = AVTK_PULSE_DURATION_MIN_D;
const u_int16_t AVTK_PulseMaxDuration = AVTK_PULSE_DURATION_MAX_D;

//const u_int16_t AVTK_PulseDuration = AVTK_PULSE_DURATION_MID_D / RawSignal.Multiply;
//const u_int16_t AVTK_PulseMinDuration = AVTK_PULSE_DURATION_MIN_D / RawSignal.Multiply;
//const u_int16_t AVTK_PulseMaxDuration = AVTK_PULSE_DURATION_MAX_D / RawSignal.Multiply;
const u_short AVTK_SyncPairsCount = 8;
const u_short AVTK_MinSyncPairs = 6;


bool value_between_(uint16_t value, uint16_t min, uint16_t max) {
    return (value > min && value < max);
}


bool decode_manchester(uint8_t frame[], uint8_t expectedBitCount, uint16_t const pulses[], const int pulsesCount, int *pulseIndex, uint16_t shortPulseMinDuration, uint16_t shortPulseMaxDuration, uint16_t longPulseMinDuration, uint16_t longPulseMaxDuration, uint8_t bitOffset)
{
    if (*pulseIndex + (expectedBitCount - 1) * 2  > pulsesCount)
    {
        #ifdef PWM_DEBUG
        Serial.print(F("PWM: Not enough pulses: *pulseIndex = "));
        Serial.print(*pulseIndex);
        Serial.print(F(" - expectedBitCount = "));
        Serial.print(expectedBitCount);
        Serial.print(F(" - pulsesCount = "));
        Serial.print(pulsesCount);
        Serial.print(F(" - min required pulses = "));
        Serial.println(*pulseIndex + expectedBitCount * 2);         
        #endif
        return false;
    }

    const uint8_t bitsPerByte = 8;
    const uint8_t endBitCount = expectedBitCount + bitOffset;

    for(uint8_t bitIndex = bitOffset; bitIndex < endBitCount; bitIndex++)
    {
        int currentFrameByteIndex = bitIndex / bitsPerByte;
        uint16_t bitDurationHigh = pulses[*pulseIndex];
        uint16_t bitDurationLow = pulses[*pulseIndex + 1];

        if (value_between_(bitDurationHigh, shortPulseMinDuration, shortPulseMaxDuration) && value_between_(bitDurationLow, longPulseMinDuration, longPulseMaxDuration))
        {
            frame[currentFrameByteIndex] |= 1 << (7 - (bitIndex % 8));
        }
        else if (!value_between_(bitDurationHigh, longPulseMinDuration, longPulseMaxDuration) || !value_between_(bitDurationLow, shortPulseMinDuration, shortPulseMaxDuration))
        {
            #ifdef PWM_DEBUG
            Serial.print(F("PWM: Invalid duration at pulse "));
            Serial.print(*pulseIndex);
            Serial.print(F(" - bit "));
            Serial.print(bitIndex);
            Serial.print(F(": "));
            Serial.println(bitDuration * RFLink::Signal::RawSignal.Multiply);         
            #endif
            return false; // unexpected bit duration, invalid format
        }

        *pulseIndex += 2;
    }

    return true;
}


// TODO why can't  we use the function defined in 7_Utils?
// inline 
bool value_between(uint16_t value, uint16_t min, uint16_t max) {
    return ((value > min) && (value < max));
}

// inline 
bool isLowPulseIndex(const int pulseIndex) {
    return (pulseIndex % 2 == 1);
}


// TODO Add parameter bool if frame[] values consumed should be decreased (pulses[i] = pulses[i] - pulseDuration)
// TODO change return type to int? --> remaining bits (if we didn't consume all bits in the inner loop), return -1 on ERR
uint8_t decode_bits(uint8_t frame[], const uint16_t* pulses, const int pulsesCount, int *pulseIndex, uint16_t pulseDuration, size_t bitsToRead) {
    size_t bitsRead = 0;

    for (size_t i = 0; i < pulsesCount && bitsRead < bitsToRead; i++, (*pulseIndex)++) {
        size_t bits = (size_t)((pulses[*pulseIndex] + (pulseDuration / 2)) / pulseDuration);

        for (size_t j = 0; j < bits; j++) {
            frame[bitsRead / 8] <<= 1;
            frame[bitsRead / 8] |= i % 2 == 0;
            bitsRead++;
            if (bitsRead >= bitsToRead) {
                return j + 1;
            }
        }
    }

    // Check if there are enough bits read
    return bitsRead >= bitsToRead ? 0 : -1;
}

bool checkSyncWord(const unsigned char synword[], const unsigned char pattern[], size_t length) {
    for (size_t i = 0; i < length; i++) {
        if (synword[i] != pattern[i]) {
            return false;
        }
    }
    return true;
}

void reverseAddress(uint8_t *array, size_t size) {
    for (size_t i = 0; i < size / 2; ++i) {
        uint8_t temp = array[i];
        array[i] = array[size - 1 - i];
        array[size - 1 - i] = temp;
    }

    for (size_t i = 0; i < size; ++i) {
        uint8_t value = array[i];
        uint8_t reversedValue = 0;

        for (int j = 0; j < 8; ++j) {
            reversedValue |= ((value >> j) & 1) << (7 - j);
        }

        array[i] = reversedValue;
    }
}

bool decode(uint16_t pulses[], size_t pulseCount) {
    const int syncWordSize = 8;
    unsigned char syncwordChars[] = {0xCA, 0xCA, 0x53, 0x53};
    size_t syncwordLength = sizeof(syncwordChars) / sizeof(syncwordChars[0]);
    uint8_t synword[syncwordLength];


    int pulseIndex = 1;
    bool oneMessageProcessed = false;
    
    while (pulseIndex + (int)(2 * AVTK_SyncPairsCount + syncWordSize) < pulseCount) {
        u_short preamblePairsFound = 0;
        for (size_t i = 0; i < AVTK_SyncPairsCount; i++) {
            if (value_between(pulses[pulseIndex], AVTK_PulseMinDuration, AVTK_PulseMaxDuration)
                && value_between(pulses[pulseIndex + 1], AVTK_PulseMinDuration, AVTK_PulseMaxDuration)) {
                preamblePairsFound++;
            } else if (preamblePairsFound > 0) {
                // if we didn't already had a match, we ignore as mismatch, otherwise we break here
                break;
            }
            pulseIndex += 2;
        }

        if (preamblePairsFound < AVTK_MinSyncPairs) {
        printf("Preamble not found (%i < %i)\n", preamblePairsFound, AVTK_MinSyncPairs);
            return oneMessageProcessed;
        }   
        printf("Preamble found (%i >= %i)\n", preamblePairsFound, AVTK_MinSyncPairs);

        uint8_t bitsProccessed = decode_bits(synword, pulses, pulseCount, &pulseIndex, AVTK_PULSE_DURATION_MID_D, 8 * syncwordLength);
        if (!bitsProccessed) {
            printf("Error on syncword decode\n");
            return oneMessageProcessed;
        }
        
        printf("Syncword 0x");
        for (size_t i = 0; i < syncwordLength; i++) {
            printf("%02X", syncwordChars[i]);
        }

        if (!checkSyncWord(synword, syncwordChars, syncwordLength)) {
            printf(" not found\n");
            return oneMessageProcessed;
        }    
        printf(" found\n");
        
        int alteredIndex = pulseIndex;
        uint16_t alteredValue = pulses[pulseIndex];
         if (isLowPulseIndex(pulseIndex)) {
            // the last pulse "decode_bits" processed was high
            pulses[pulseIndex] = pulses[pulseIndex] - bitsProccessed * AVTK_PulseDuration;
        }

        // byte address[] = { 0, 0, 0, 0 };
        uint8_t address[] = { 0, 0, 0, 0 };

        bool decodeResult = decode_manchester(address, 32, pulses, pulseCount, &pulseIndex, AVTK_PulseMinDuration, AVTK_PulseMaxDuration, 2 * AVTK_PulseMinDuration, 2 * AVTK_PulseMaxDuration, 0);
        pulses[alteredIndex] = alteredValue ;

        if (!decodeResult) {
            printf("Could not decode address manchester data\n");
            return oneMessageProcessed;
        }
printf("Address:          %02x %02x %02x %02x\n", address[0], address[1], address[2], address[3]);
        reverseAddress(address, sizeof(address));
printf("Address reversed: %02x %02x %02x %02x\n", address[0], address[1], address[2], address[3]);
        
printf("pulseIndex is %i\n", pulseIndex);
        
        // byte buttons[] = { 0 };
        uint8_t buttons[] = { 0 };
        if (!decode_manchester(buttons, 2, pulses, pulseCount, &pulseIndex, AVTK_PulseMinDuration, AVTK_PulseMaxDuration, 2 * AVTK_PulseMinDuration, 2 * AVTK_PulseMaxDuration, 0)) {
            printf("Could not decode buttons manchester data\n");
            return oneMessageProcessed;
        }    
        printf("Buttons: %02x\n", buttons[0]);    

        pulseIndex += 7; // CRC
        pulseIndex += 1; // ???

        oneMessageProcessed = true;
    }

    return oneMessageProcessed;
}
