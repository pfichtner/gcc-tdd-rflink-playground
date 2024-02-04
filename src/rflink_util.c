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

bool decode_manchester(uint8_t frame[], uint8_t expectedBitCount, uint16_t const pulses[], const int pulsesCount, int pulseIndex, uint8_t nextBit, bool secondPulse, uint16_t halfBitMinDuration, uint16_t halfBitMaxDuration)
{
    int bitIndex = 0;
    const uint8_t bitsPerByte = 8;
    const uint8_t expectedByteCount = expectedBitCount / bitsPerByte;
    const uint16_t fullBitMinDuration = halfBitMinDuration * 2;
    const uint16_t fullBitMaxDuration = halfBitMaxDuration * 2;

    while ((pulseIndex < pulsesCount) && (bitIndex < expectedBitCount))
    {
        uint16_t pulseDuration = pulses[pulseIndex];
        int currentFrameByteIndex = bitIndex / bitsPerByte;
        
        #ifdef MANCHESTER_DEBUG
        printf("Manchester: Processing pulseIndex = %i - bitIndex %i - value = %i - frameIndex %i\n", pulseIndex, bitIndex, pulseDuration, currentFrameByteIndex);
        #endif

        if (value_between_(pulseDuration, fullBitMinDuration, fullBitMaxDuration))
        {
            // TODO we have some inverted logic so we check viceversa
            if (false)
            // if (!secondPulse)
            {
                #ifdef MANCHESTER_DEBUG
                printf("Manchester: Cannot have long pulse as a first pulse: index = %i - value = %i\n", pulseIndex, pulseDuration);
                #endif
                return false;
            }

            frame[currentFrameByteIndex] <<= 1;
            frame[currentFrameByteIndex] |= nextBit;

            nextBit = 1 - nextBit;
            bitIndex++;
        }
        else if (value_between_(pulseDuration, halfBitMinDuration, halfBitMaxDuration))
        {
            // TODO we have some inverted logic so we check viceversa
            // if (secondPulse)
            if (true)
            {
                frame[currentFrameByteIndex] <<= 1;
                frame[currentFrameByteIndex] |= nextBit;

                bitIndex++;
            }

            secondPulse = !secondPulse;
        }
        else
        {
            #ifdef MANCHESTER_DEBUG
            printf("Manchester: Pulse has unexpected duration: index = %i - value = %i\n", pulseIndex, pulseDuration);
            #endif
            return false;
        }

        pulseIndex++;
    }

    // The low part of the down front gets mixed with the interframe silence and is thus too long to be placed in the pulses
    // This means the last bit is never placed in the frame and we must manually add it into the last byte
    if ((pulseIndex == pulsesCount) && (bitIndex == expectedBitCount - 1))
    {
        frame[expectedByteCount - 1] <<= 1;
        frame[expectedByteCount - 1] |= nextBit;
        bitIndex++;
    }

    return (bitIndex == expectedBitCount);
}



// TODO why can't  we use the function defined in 7_Utils?
bool value_between(uint16_t value, uint16_t min, uint16_t max) {
    return ((value > min) && (value < max));
}

    // TODO extract function: this is: isLowPulseIndex
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

// Function to convert an integer to binary and print it
void print_binary(uint8_t value) {
    for (int i = 7; i >= 0; --i) {
        printf("%d", (value >> i) & 1);
    }
}


bool checkSyncWord(const unsigned char synword[], const unsigned char pattern[], size_t length) {
    for (size_t i = 0; i < length; i++) {
        if (synword[i] != pattern[i]) {
            return false;
        }
    }
    return true;
}

bool foo(uint16_t pulses[], size_t pulseCount) {
    int pulseIndex = 1;
    bool oneMessageProcessed = false;
    
    const int syncWordSize = 8;


    while (pulseIndex + (int)(2 * AVTK_SyncPairsCount + syncWordSize) < pulseCount) {
        u_short preamblePairsFound = 0;
        for (size_t i = 0; i < 2 * AVTK_SyncPairsCount - 1; i++) {
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
        printf("Preamble found\n");

        unsigned char pattern[] = {0xCA, 0xCA, 0x53, 0x53};
        size_t patternLength = sizeof(pattern) / sizeof(pattern[0]);

        uint8_t synword[patternLength];
        uint8_t bitsProccessed = decode_bits(synword, pulses, pulseCount, &pulseIndex, AVTK_PULSE_DURATION_MID_D, 8 * patternLength);
        if (!bitsProccessed) {
            printf("Error on syncword decode\n");
            return oneMessageProcessed;
        }
        if (!checkSyncWord(synword, pattern, patternLength)) {
            printf("0xCACA5353 syncword not found\n");
            return oneMessageProcessed;
        }    
        printf("0xCACA5353 syncword found\n");
        
        int alteredIndex = pulseIndex;
        uint16_t alteredValue = pulses[pulseIndex];
         if (isLowPulseIndex(pulseIndex) && bitsProccessed > 0) {
            // the last pulse "decode_bits" processed was high
printf("adjusting pulses at index %i from %i ", pulseIndex, pulses[pulseIndex]);
            pulses[pulseIndex] = pulses[pulseIndex] - bitsProccessed * AVTK_PulseDuration;
printf("to %i\n", pulses[pulseIndex]);
        }

        // TODO this 64 bit (8*8) of binary data!!! 10111001100...
        // byte address[] = { 0, 0, 0, 0 };
        uint8_t address[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        
        bool decodeResult = decode_manchester(address, 64, pulses, pulseCount, pulseIndex, 1, false, AVTK_PulseMinDuration, AVTK_PulseMaxDuration);
printf("restored pulses at index %i from %i ", alteredIndex, pulses[alteredIndex]);
        pulses[alteredIndex] = alteredValue ;
printf("to %i\n", pulses[alteredIndex]);

        if (!decodeResult) {
            printf("Could not decode address manchester data\n");
            return oneMessageProcessed;
        }
        pulseIndex += 64;
printf("pulseIndex is %i\n", pulseIndex);

        // printf("Address : %02x %02x %02x %02x %02x %02x %02x %02x\n", address[0], address[1], address[2], address[3], address[4], address[5], address[6], address[7]);
        printf("Address: %i %i %i %i %i %i %i %i\n", address[0], address[1], address[2], address[3], address[4], address[5], address[6], address[7]);
        
        
        // byte buttons[] = { 0 };
        uint8_t buttons[] = { 0 };
        if (!decode_manchester(buttons, 2, pulses, pulseCount, pulseIndex, 1, false, AVTK_PulseMinDuration, AVTK_PulseMaxDuration)) {
            printf("Could not decode buttons manchester data\n");
            return oneMessageProcessed;
        }    
        pulseIndex += 2;
        printf("Buttons: %02x\n", buttons[0]);    

        pulseIndex += 7; // CRC
        pulseIndex += 3; // ???

        oneMessageProcessed = true;
    }

    return oneMessageProcessed;
}
