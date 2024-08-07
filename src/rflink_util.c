#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define PLUGIN_077_DEBUG
//#define MANCHESTER_DEBUG

#define AVTK_PULSE_DURATION_MID_D 480
#define AVTK_PULSE_DURATION_MIN_D 370
#define AVTK_PULSE_DURATION_MAX_D 590

#define BUTTONS_BITS_COUNT 4
// #define SWAP_BIT_ORDER_IF_SIGNAL_HAS_CRC

const u_int16_t AVTK_PulseDuration = AVTK_PULSE_DURATION_MID_D;
const u_int16_t AVTK_PulseMinDuration = AVTK_PULSE_DURATION_MIN_D;
const u_int16_t AVTK_PulseMaxDuration = AVTK_PULSE_DURATION_MAX_D;

// const u_int16_t AVTK_PulseDuration = AVTK_PULSE_DURATION_MID_D /
// RawSignal.Multiply; const u_int16_t AVTK_PulseMinDuration =
// AVTK_PULSE_DURATION_MIN_D / RawSignal.Multiply; const u_int16_t
// AVTK_PulseMaxDuration = AVTK_PULSE_DURATION_MAX_D / RawSignal.Multiply;
const u_short AVTK_SyncPairsCount = 8;
const u_short AVTK_MinSyncPairs = 6;

bool value_between_(uint16_t value, uint16_t min, uint16_t max) {
  return (value > min && value < max);
}

bool decode_manchester(uint8_t frame[], uint8_t expectedBitCount,
                       uint16_t const pulses[], const int pulsesCount,
                       int *pulseIndex, uint16_t shortPulseMinDuration,
                       uint16_t shortPulseMaxDuration,
                       uint16_t longPulseMinDuration,
                       uint16_t longPulseMaxDuration, uint8_t bitOffset,
                       const uint8_t bitsPerByte, bool lsb) {
  if (*pulseIndex + (expectedBitCount - 1) * 2 > pulsesCount) {
#ifdef MANCHESTER_DEBUG
    printf(
        "MANCHESTER_DEBUG: Not enough pulses: *pulseIndex = %d - "
        "expectedBitCount = %d - pulsesCount = %d - min required pulses = %d\n",
        *pulseIndex, expectedBitCount, pulsesCount,
        *pulseIndex + expectedBitCount * 2);
#endif
    return false;
  }

  const uint8_t endBitCount = expectedBitCount + bitOffset;

  for (uint8_t bitIndex = bitOffset; bitIndex < endBitCount; bitIndex++) {
    bool isLast = bitIndex + 1 == endBitCount;
    int currentFrameByteIndex = bitIndex / bitsPerByte;
    uint8_t offset = bitIndex % bitsPerByte;
    if (offset == 0) {
        frame[currentFrameByteIndex] = 0;
    }
    uint16_t bitDuration0 = pulses[*pulseIndex];
    uint16_t bitDuration1 = pulses[*pulseIndex + 1];

    // TODO we could add parameter of manchester/inversed manchester
    if (value_between_(bitDuration0, shortPulseMinDuration,
                       shortPulseMaxDuration) &&
        value_between_(bitDuration1, longPulseMinDuration,
                       isLast ? UINT16_MAX : longPulseMaxDuration)) {
      frame[currentFrameByteIndex] |=
          1 << (lsb ? (bitsPerByte - 1 - offset) : offset);
#ifdef MANCHESTER_DEBUG
      printf("MANCHESTER_DEBUG: frame %i, pulseIndex %i: 100 -> 1\n", currentFrameByteIndex, *pulseIndex);
#endif
    } else if (!value_between_(bitDuration0, longPulseMinDuration,
                               longPulseMaxDuration) ||
               !value_between_(bitDuration1, shortPulseMinDuration,
                               isLast ? UINT16_MAX : shortPulseMaxDuration)) {
#ifdef MANCHESTER_DEBUG
      printf("MANCHESTER_DEBUG: Invalid duration at pulse %d - bit %d: %d\n",
             *pulseIndex, bitIndex,
             bitDuration0);
#endif
      return false; // unexpected bit duration, invalid format
    } else {
#ifdef MANCHESTER_DEBUG
      printf("MANCHESTER_DEBUG: frame %i, pulseIndex %i: 110 -> 0\n", currentFrameByteIndex, *pulseIndex);
#endif
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
bool isLowPulseIndex(const int pulseIndex) { return (pulseIndex % 2 == 0); }

uint8_t decode_bits(uint8_t frame[], const uint16_t *pulses,
                    const int pulsesCount, int *pulseIndex,
                    uint16_t pulseDuration, size_t bitsToRead) {
  size_t bitsRead = 0;

  for (; *pulseIndex < pulsesCount && bitsRead < bitsToRead; (*pulseIndex)++) {
    size_t bits =
        (size_t)((pulses[*pulseIndex] + (pulseDuration / 2)) / pulseDuration);
    for (size_t j = 0; j < bits; j++) {
      frame[bitsRead / 8] <<= 1;
      if (!isLowPulseIndex(*pulseIndex)) {
        frame[bitsRead / 8] |= 1;
      }
      bitsRead++;
      if (bitsRead >= bitsToRead) {
        return j + 1;
      }
    }
  }

  // Check if there are enough bits read
  return bitsRead >= bitsToRead ? 0 : -1;
}

bool checkSyncWord(const unsigned char syncword[], const unsigned char pattern[],
                   size_t length) {
  for (size_t i = 0; i < length; i++) {
    if (syncword[i] != pattern[i]) {
      return false;
    }
  }
  return true;
}

u_short countPreamblePairs(const uint16_t pulses[], int *pulseIndex, size_t pulseCount, size_t AVTK_SyncPairsCount, uint16_t AVTK_PulseMinDuration, uint16_t AVTK_PulseMaxDuration) {
    u_short preamblePairsFound = 0;

    for (size_t i = 0; i < AVTK_SyncPairsCount && *pulseIndex < pulseCount - 1; i++, (*pulseIndex) += 2) {
        if (value_between(pulses[*pulseIndex], AVTK_PulseMinDuration, AVTK_PulseMaxDuration) &&
            value_between(pulses[*pulseIndex + 1], AVTK_PulseMinDuration, AVTK_PulseMaxDuration)) {
            preamblePairsFound++;
        } else if (preamblePairsFound > 0) {
            // if we didn't already have a match, we ignore as mismatch, otherwise we break here
            break;
        }
    }

    return preamblePairsFound;
}

bool decode(uint16_t pulses[], size_t pulseCount) {
  const int syncWordSize = 8;
  unsigned char syncwordChars[] = {0xCA, 0xCA, 0x53, 0x53};
  size_t syncwordLength = sizeof(syncwordChars) / sizeof(syncwordChars[0]);
  uint8_t syncword[syncwordLength];

  int pulseIndex = 1;
  while (pulseIndex + (int)(2 * AVTK_SyncPairsCount + syncWordSize) <
         pulseCount) {
    u_short preamblePairsFound = countPreamblePairs(pulses, &pulseIndex, pulseCount, AVTK_SyncPairsCount, AVTK_PulseMinDuration, AVTK_PulseMaxDuration);

    if (preamblePairsFound < AVTK_MinSyncPairs) {
#ifdef PLUGIN_077_DEBUG
      printf("pulseIndex is %i\n", pulseIndex);
      printf("Preamble not found (%i < %i)\n", preamblePairsFound,
             AVTK_MinSyncPairs);
#endif
      continue;
    }
#ifdef PLUGIN_077_DEBUG
    printf("pulseIndex is %i\n", pulseIndex);
    printf("Preamble found (%i >= %i)\n", preamblePairsFound,
           AVTK_MinSyncPairs);
#endif

    for (size_t i = 0; i < syncwordLength; i++) syncword[i] = 0;
    uint8_t bitsProccessed =
        decode_bits(syncword, pulses, pulseCount, &pulseIndex,
                    AVTK_PULSE_DURATION_MID_D, 8 * syncwordLength);
    if (!bitsProccessed) {
#ifdef PLUGIN_077_DEBUG
      printf("Error on syncword decode\n");
#endif
      continue;
    }

#ifdef PLUGIN_077_DEBUG
    printf("Syncword 0x");
    for (size_t i = 0; i < syncwordLength; i++) {
      printf("%02X", syncwordChars[i]);
    }
#endif

    if (!checkSyncWord(syncword, syncwordChars, syncwordLength)) {
#ifdef PLUGIN_077_DEBUG
      printf(" not found\n");
#endif
      continue;
    }
#ifdef PLUGIN_077_DEBUG
    printf(" found\n");
#endif

    int alteredIndex = pulseIndex;
    uint16_t alteredValue = pulses[alteredIndex];
    if (!isLowPulseIndex(pulseIndex)) {
      // the last pulse "decode_bits" processed was high
      pulses[pulseIndex] =
          pulses[pulseIndex] - bitsProccessed * AVTK_PulseDuration;
    }

    // byte address[] = { 0, 0, 0, 0 };
    uint8_t address[] = {0, 0, 0, 0};

    bool decodeResult = decode_manchester(
        address, 32, pulses, pulseCount, &pulseIndex, AVTK_PulseMinDuration,
        AVTK_PulseMaxDuration, 2 * AVTK_PulseMinDuration,
        2 * AVTK_PulseMaxDuration, 0, 8, true);
    pulses[alteredIndex] = alteredValue;

    if (!decodeResult) {
#ifdef PLUGIN_077_DEBUG
      printf("Could not decode address manchester data\n");
#endif
      continue;
    }
#ifdef PLUGIN_077_DEBUG
    printf("Address (lsb): %02x %02x %02x %02x\n", address[0], address[1],
           address[2], address[3]);
    printf("pulseIndex is %i\n", pulseIndex);
#endif

    // byte buttons[] = { 0 };
    uint8_t buttons[] = { 0 };
    if (!decode_manchester(buttons, BUTTONS_BITS_COUNT, pulses, pulseCount, &pulseIndex,
                           AVTK_PulseMinDuration, AVTK_PulseMaxDuration,
                           2 * AVTK_PulseMinDuration, 2 * AVTK_PulseMaxDuration,
                           0, BUTTONS_BITS_COUNT, true)) {
#ifdef PLUGIN_077_DEBUG
      printf("Could not decode buttons manchester data\n");
#endif
      continue;
    }
// TODO we would have to shift back the result because we shifted it too much to
// the left because we think that everything has 8 bits

    bool hasCrc = false;
    if (pulseIndex + 2 * AVTK_SyncPairsCount < pulseCount) {
      short savedPulseIndex = pulseIndex;
      preamblePairsFound = countPreamblePairs(pulses, &pulseIndex, pulseCount, AVTK_SyncPairsCount, AVTK_PulseMinDuration, AVTK_PulseMaxDuration);
      pulseIndex = savedPulseIndex;
      hasCrc = preamblePairsFound < AVTK_SyncPairsCount;
    }

#ifdef SWAP_BIT_ORDER_IF_SIGNAL_HAS_CRC
    if (hasCrc) {
        pulseIndex -= (2 * BUTTONS_BITS_COUNT);
        if (!decode_manchester(buttons, BUTTONS_BITS_COUNT, pulses, pulseCount, &pulseIndex,
                                AVTK_PulseMinDuration, AVTK_PulseMaxDuration,
                                2 * AVTK_PulseMinDuration, 2 * AVTK_PulseMaxDuration,
                                0, BUTTONS_BITS_COUNT, false)) {
#ifdef PLUGIN_077_DEBUG
    printf("Could not (re)decode buttons manchester data\n");
#endif
          continue;
        }
    }
#endif
    #ifdef PLUGIN_077_DEBUG
    printf("Buttons: %x\n", buttons[0]);
    printf("pulseIndex is %i\n", pulseIndex);
    #endif
    
    if (hasCrc) {
      pulseIndex--;

      alteredIndex = pulseIndex;
      alteredValue = pulses[alteredIndex];
      bool bitNr4IsSet = buttons[0] & 0b00010000; // 4th bit to the left, 0=110 (2x 1x), 1=100 (1x 2x)
      pulses[alteredIndex] -= ((bitNr4IsSet ? 2 : 1) * AVTK_PulseDuration);
      
      // byte crc[] = { 0 };
      uint8_t crc[] = { 0 };
      decodeResult = decode_bits(crc, pulses, pulseCount, &pulseIndex, AVTK_PULSE_DURATION_MID_D, 8);
      pulses[alteredIndex] = alteredValue;

      if (!decodeResult) {
#ifdef PLUGIN_077_DEBUG
        printf("Error on crc decode\n");
#endif
        continue;
      }
#ifdef PLUGIN_077_DEBUG
      printf("CRC: 0x%02x\n", crc[0]);
#endif
      pulseIndex += 2;
#ifdef PLUGIN_077_DEBUG
    printf("pulseIndex is %i\n", pulseIndex);
#endif
    }

    return true;
  }

  return false;
}
