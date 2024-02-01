#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>


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

// TODO why can't  we use the function defined in 7_Utils?
    bool value_between(uint16_t value, uint16_t min, uint16_t max) {
    return ((value > min) && (value < max));
}


// TODO Add parameter bool if frame[] values consumed should be decreased (pulses[i] = pulses[i] - pulseDuration)
// TODO change return type to int? --> remaining bits (if we didn't consume all bits in the inner loop), return -1 on ERR
bool decode_bits(uint8_t frame[], const uint16_t* pulses, const int pulsesCount, const int pulseIndex, uint16_t pulseDuration, size_t bitsToRead) {
    size_t bitsRead = 0;

    for (size_t i = 0; i < pulsesCount && bitsRead < bitsToRead; i++) {
        size_t bits = (size_t)((pulses[pulseIndex + i] + (pulseDuration / 2)) / pulseDuration);

        for (size_t j = 0; j < bits && bitsRead < bitsToRead; j++) {
            frame[bitsRead / 8] <<= 1;
            frame[bitsRead / 8] |= i % 2 == 0;
            bitsRead++;
        }
    }

    // Check if there are enough bits read
    return bitsRead >= bitsToRead;
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

bool foo(const uint16_t pulses[], size_t pulseCount) {
    int pulseIndex = 1;
    
    const int syncWordSize = 8;
    if (pulseCount <= (int)(2 * AVTK_SyncPairsCount + syncWordSize)) {
        return 1;
    }

    u_short preamblePairsFound = 0;
    while (pulseIndex <= 2 * AVTK_SyncPairsCount - 1) {
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
        return 1;
    }   
    printf("Preamble found\n");

    unsigned char pattern[] = {0xCA, 0xCA, 0x53, 0x53};
    size_t patternLength = sizeof(pattern) / sizeof(pattern[0]);

    uint8_t synword[patternLength];
    if (!decode_bits(synword, pulses, pulseCount, pulseIndex, AVTK_PULSE_DURATION_MID_D, 8 * patternLength)) {
        printf("Error on syncword decode\n");
        return 1;
    }
    if (!checkSyncWord(synword, pattern, patternLength)) {
        printf("0xCACA5353 syncword not found\n");
        return 1;
    }    
    printf("0xCACA5353 of syncword found\n");    

    return 0;
}
