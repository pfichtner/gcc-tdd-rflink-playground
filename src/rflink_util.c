#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

char* hexToBinaryString(uint8_t* value, size_t size) {
    char* binaryString = (char*)malloc(size * 8 + 1);

    if (binaryString == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    binaryString[size * 8] = '\0';

    for (size_t i = 0; i < size; ++i) {
        for (int j = 7; j >= 0; --j) {
            binaryString[i * 8 + (7 - j)] = ((value[i] >> j) & 1) ? '1' : '0';
        }
    }

    return binaryString;
}

int findRightmostPatternPositionInBinaryString(const char* binaryString, const char* pattern) {
    char* position = strstr(binaryString, pattern);
    int rightmostPosition = -1;

    while (position != NULL) {
        // Calculate the bit position from the right
        rightmostPosition = position - binaryString;
        // Move to the next position
        position = strstr(position + 1, pattern);
    }

    return rightmostPosition;
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

int mainX() {
    // Example input data
//    uint16_t pulses1[] = {960, 480, 480};
//    uint16_t pulses2[] = {695,303,585,419,582,424,548,448,534,470,524,474,521,478,518,483}; // preamble
//    uint16_t pulses3[] = {1009,989,505,494,503,493,1002,998,499,498,498,999,498,498,496,1001,996,501,497,500,495,1003,1995}; // sync-word
    uint16_t pulses4[] = {4242, 695, 303, 585, 419, 582, 424, 548, 448, 534, 470, 524, 474, 521, 478, 518, 483, 1009, 989, 505, 494, 503, 493, 1002, 998, 499, 498, 498, 999, 498, 498, 496, 1001, 996, 501, 497, 500, 495, 1003, 1995, 503, 494, 1004, 493, 1002, 495, 1000, 996, 502, 995, 504, 995, 503, 492, 1004, 493, 1004, 493, 1004, 492, 1004, 494, 1004, 993, 505, 991, 507, 990, 508, 490, 1007, 989, 506, 991, 507, 991, 505, 494, 1002, 995, 502, 996, 500, 997, 498, 997, 500, 996, 501, 994, 505, 990, 508, 988, 510, 984, 516, 977, 522, 968, 534, 958, 550, 444, 1057, 934, 576, 922, 574, 924, 2066, 433, 1066, 931, 2064, 434, 567, 431, 562, 435, 563, 436, 559, 437, 559, 438, 559, 439, 556, 440, 556, 942, 1052, 446, 553, 445, 549, 946, 1047, 451, 548, 449, 1049, 448, 549, 447, 1045, 953, 545, 452, 545, 452, 1042, 1956, 540, 457, 1039, 458, 1039, 458, 1037, 959, 538, 961, 535, 961, 537, 460, 1034, 463, 1035, 463, 1033, 464, 1033, 467, 1030, 968, 532, 965, 530, 967, 528, 468, 1028, 969, 531, 967, 529, 969, 527, 470, 1027, 972, 525, 970, 526, 972, 524, 972, 527, 971, 524, 973, 524, 972, 524, 973, 524, 974, 524, 973, 522, 974, 522, 975, 522, 474, 1023, 976, 518, 980, 519, 976, 2017, 481, 1018, 979, 2017, 482, 516, 481, 515, 481, 516, 481, 516, 481, 516, 480, 516, 481, 516, 481, 517, 980, 1015, 482, 514, 482, 514, 983, 1011, 485, 514, 482, 1013, 484, 515, 483, 1014, 983, 513, 484, 513, 484, 1013, 1985, 510, 487, 1012, 485, 1010, 485, 1010, 988, 511, 985, 511, 986, 509, 487, 1009, 489, 1011, 486, 1011, 486, 1009, 488, 1007, 989 };

    int pulseIndex = 1;

    uint8_t preamble[2] = {0};
    bool decodeOk = decode_bits(preamble, pulses4, sizeof(pulses4) / sizeof(pulses4[0]), pulseIndex, 480, 8 * 2);
    if (!decodeOk) {
        printf("Error on preamble decode\n");
	return 1;
    }

    // debug out of preamble
    for (size_t i = 0; i < sizeof(preamble); i++) {
        print_binary(preamble[i]);
    }
    printf("\n\n");


    // find at least 6 of of 8 pairs 
    const char* preambleToFind = "101010101010";

    // Convert the hexadecimal value to a binary string using sprintf
    char* binaryString = hexToBinaryString(preamble, sizeof(preamble));

    // Print the binary string for debugging
    printf("Binary String: %s\n", binaryString);

    // Find the position where the rightmost occurrence of the pattern starts
    int position = findRightmostPatternPositionInBinaryString(binaryString, preambleToFind);
    // preamble must end in 101010101010
    bool preambleEndsInPatternToFind = position == strlen(binaryString) - strlen(preambleToFind);

    // Free the allocated memory for the binary string
    free(binaryString);
   
    if (!preambleEndsInPatternToFind) {
        printf("Did not find 6 preamble pairs\n");
        return 1;
    }

    printf("Preamble found\n");

    pulseIndex += (position + strlen(preambleToFind));
    printf("PulseIndex: %i\n", pulseIndex);


    decodeOk = decode_bits(preamble, pulses4, sizeof(pulses4) / sizeof(pulses4[0]), pulseIndex, 480, 8 * 2);
    if (!decodeOk) {
        printf("Error on syncword part 1 decode\n");
        return 1;
    }
    if (!preamble[0] == 0xCA || !preamble[1] == 0xCA) {
        printf("0xCACA of syncword not found\n");
        return 1;
    }
    printf("0xCACA of syncword found\n");
    
    decodeOk = decode_bits(preamble, pulses4, sizeof(pulses4) / sizeof(pulses4[0]), pulseIndex, 480, 8 * 2);
    if (!decodeOk) {
        printf("Error on syncword part 2 decode\n");
        return 1;
    }
    if (!preamble[0] == 0x53 || !preamble[1] == 0x53) {
        printf("0x5353 of syncword not found\n");
        return 1;
    }       
    printf("0x5353 of syncword found\n");


    return 0;
}

