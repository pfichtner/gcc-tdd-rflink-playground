#include "Unity/src/unity.h"
#include "../src/rflink_util.h"

void ok() {
        uint16_t pulses[] = {4242, 695, 303, 585, 419, 582, 424, 548, 448, 534, 470, 524, 474, 521, 478, 518, 483, 1009, 989, 505, 494, 503, 493, 1002, 998, 499, 498, 498, 999, 498, 498, 496, 1001, 996, 501, 497, 500, 495, 1003, 1995, 503, 494, 1004, 493, 1002, 495, 1000, 996, 502, 995, 504, 995, 503, 492, 1004, 493, 1004, 493, 1004, 492, 1004, 494, 1004, 993, 505, 991, 507, 990, 508, 490, 1007, 989, 506, 991, 507, 991, 505, 494, 1002, 995, 502, 996, 500, 997, 498, 997, 500, 996, 501, 994, 505, 990, 508, 988, 510, 984, 516, 977, 522, 968, 534, 958, 550, 444, 1057, 934, 576, 922, 574, 924, 2066, 433, 1066, 931, 2064, 434, 567, 431, 562, 435, 563, 436, 559, 437, 559, 438, 559, 439, 556, 440, 556, 942, 1052, 446, 553, 445, 549, 946, 1047, 451, 548, 449, 1049, 448, 549, 447, 1045, 953, 545, 452, 545, 452, 1042, 1956, 540, 457, 1039, 458, 1039, 458, 1037, 959, 538, 961, 535, 961, 537, 460, 1034, 463, 1035, 463, 1033, 464, 1033, 467, 1030, 968, 532, 965, 530, 967, 528, 468, 1028, 969, 531, 967, 529, 969, 527, 470, 1027, 972, 525, 970, 526, 972, 524, 972, 527, 971, 524, 973, 524, 972, 524, 973, 524, 974, 524, 973, 522, 974, 522, 975, 522, 474, 1023, 976, 518, 980, 519, 976, 2017, 481, 1018, 979, 2017, 482, 516, 481, 515, 481, 516, 481, 516, 481, 516, 480, 516, 481, 516, 481, 517, 980, 1015, 482, 514, 482, 514, 983, 1011, 485, 514, 482, 1013, 484, 515, 483, 1014, 983, 513, 484, 513, 484, 1013, 1985, 510, 487, 1012, 485, 1010, 485, 1010, 988, 511, 985, 511, 986, 509, 487, 1009, 489, 1011, 486, 1011, 486, 1009, 488, 1007, 989 };

    TEST_ASSERT_TRUE(decode(pulses, sizeof(pulses) / sizeof(pulses[0])));
}

void ok_because_two_first_preamble_pairs_gets_ignored() {
        uint16_t pulses[] = {4242, 9991, 9992, 9993, 9994, 582, 424, 548, 448, 534, 470, 524, 474, 521, 478, 518, 483, 1009, 989, 505, 494, 503, 493, 1002, 998, 499, 498, 498, 999, 498, 498, 496, 1001, 996, 501, 497, 500, 495, 1003, 1995, 503, 494, 1004, 493, 1002, 495, 1000, 996, 502, 995, 504, 995, 503, 492, 1004, 493, 1004, 493, 1004, 492, 1004, 494, 1004, 993, 505, 991, 507, 990, 508, 490, 1007, 989, 506, 991, 507, 991, 505, 494, 1002, 995, 502, 996, 500, 997, 498, 997, 500, 996, 501, 994, 505, 990, 508, 988, 510, 984, 516, 977, 522, 968, 534, 958, 550, 444, 1057, 934, 576, 922, 574, 924, 2066, 433, 1066, 931, 2064, 434, 567, 431, 562, 435, 563, 436, 559, 437, 559, 438, 559, 439, 556, 440, 556, 942, 1052, 446, 553, 445, 549, 946, 1047, 451, 548, 449, 1049, 448, 549, 447, 1045, 953, 545, 452, 545, 452, 1042, 1956, 540, 457, 1039, 458, 1039, 458, 1037, 959, 538, 961, 535, 961, 537, 460, 1034, 463, 1035, 463, 1033, 464, 1033, 467, 1030, 968, 532, 965, 530, 967, 528, 468, 1028, 969, 531, 967, 529, 969, 527, 470, 1027, 972, 525, 970, 526, 972, 524, 972, 527, 971, 524, 973, 524, 972, 524, 973, 524, 974, 524, 973, 522, 974, 522, 975, 522, 474, 1023, 976, 518, 980, 519, 976, 2017, 481, 1018, 979, 2017, 482, 516, 481, 515, 481, 516, 481, 516, 481, 516, 480, 516, 481, 516, 481, 517, 980, 1015, 482, 514, 482, 514, 983, 1011, 485, 514, 482, 1013, 484, 515, 483, 1014, 983, 513, 484, 513, 484, 1013, 1985, 510, 487, 1012, 485, 1010, 485, 1010, 988, 511, 985, 511, 986, 509, 487, 1009, 489, 1011, 486, 1011, 486, 1009, 488, 1007, 989 };

    TEST_ASSERT_TRUE(decode(pulses, sizeof(pulses) / sizeof(pulses[0])));
}

void ok_preamble_only_6_of_8_pairs() {
        uint16_t pulses[] = {4242, 582, 424, 548, 448, 534, 470, 524, 474, 521, 478, 518, 483, 1009, 989, 505, 494, 503, 493, 1002, 998, 499, 498, 498, 999, 498, 498, 496, 1001, 996, 501, 497, 500, 495, 1003, 1995, 503, 494, 1004, 493, 1002, 495, 1000, 996, 502, 995, 504, 995, 503, 492, 1004, 493, 1004, 493, 1004, 492, 1004, 494, 1004, 993, 505, 991, 507, 990, 508, 490, 1007, 989, 506, 991, 507, 991, 505, 494, 1002, 995, 502, 996, 500, 997, 498, 997, 500, 996, 501, 994, 505, 990, 508, 988, 510, 984, 516, 977, 522, 968, 534, 958, 550, 444, 1057, 934, 576, 922, 574, 924, 2066, 433, 1066, 931, 2064, 434, 567, 431, 562, 435, 563, 436, 559, 437, 559, 438, 559, 439, 556, 440, 556, 942, 1052, 446, 553, 445, 549, 946, 1047, 451, 548, 449, 1049, 448, 549, 447, 1045, 953, 545, 452, 545, 452, 1042, 1956, 540, 457, 1039, 458, 1039, 458, 1037, 959, 538, 961, 535, 961, 537, 460, 1034, 463, 1035, 463, 1033, 464, 1033, 467, 1030, 968, 532, 965, 530, 967, 528, 468, 1028, 969, 531, 967, 529, 969, 527, 470, 1027, 972, 525, 970, 526, 972, 524, 972, 527, 971, 524, 973, 524, 972, 524, 973, 524, 974, 524, 973, 522, 974, 522, 975, 522, 474, 1023, 976, 518, 980, 519, 976, 2017, 481, 1018, 979, 2017, 482, 516, 481, 515, 481, 516, 481, 516, 481, 516, 480, 516, 481, 516, 481, 517, 980, 1015, 482, 514, 482, 514, 983, 1011, 485, 514, 482, 1013, 484, 515, 483, 1014, 983, 513, 484, 513, 484, 1013, 1985, 510, 487, 1012, 485, 1010, 485, 1010, 988, 511, 985, 511, 986, 509, 487, 1009, 489, 1011, 486, 1011, 486, 1009, 488, 1007, 989 };

    TEST_ASSERT_TRUE(decode(pulses, sizeof(pulses) / sizeof(pulses[0])));
}


void nok_because_three_first_preamble_pairs_are_wrong() {
        uint16_t pulses[] = {4242, 9991, 9992, 9993, 9994, 9995, 424, 548, 448, 534, 470, 524, 474, 521, 478, 518, 483, 1009, 989, 505, 494, 503, 493, 1002, 998, 499, 498, 498, 999, 498, 498, 496, 1001, 996, 501, 497, 500, 495, 1003, 1995, 503, 494, 1004, 493, 1002, 495, 1000, 996, 502, 995, 504, 995, 503, 492, 1004, 493, 1004, 493, 1004, 492, 1004, 494, 1004, 993, 505, 991, 507, 990, 508, 490, 1007, 989, 506, 991, 507, 991, 505, 494, 1002, 995, 502, 996, 500, 997, 498, 997, 500, 996, 501, 994, 505, 990, 508, 988, 510, 984, 516, 977, 522, 968, 534, 958, 550, 444, 1057, 934, 576, 922, 574, 924, 2066, 433, 1066, 931, 2064, 434, 567, 431, 562, 435, 563, 436, 559, 437, 559, 438, 559, 439, 556, 440, 556, 942, 1052, 446, 553, 445, 549, 946, 1047, 451, 548, 449, 1049, 448, 549, 447, 1045, 953, 545, 452, 545, 452, 1042, 1956, 540, 457, 1039, 458, 1039, 458, 1037, 959, 538, 961, 535, 961, 537, 460, 1034, 463, 1035, 463, 1033, 464, 1033, 467, 1030, 968, 532, 965, 530, 967, 528, 468, 1028, 969, 531, 967, 529, 969, 527, 470, 1027, 972, 525, 970, 526, 972, 524, 972, 527, 971, 524, 973, 524, 972, 524, 973, 524, 974, 524, 973, 522, 974, 522, 975, 522, 474, 1023, 976, 518, 980, 519, 976, 2017, 481, 1018, 979, 2017, 482, 516, 481, 515, 481, 516, 481, 516, 481, 516, 480, 516, 481, 516, 481, 517, 980, 1015, 482, 514, 482, 514, 983, 1011, 485, 514, 482, 1013, 484, 515, 483, 1014, 983, 513, 484, 513, 484, 1013, 1985, 510, 487, 1012, 485, 1010, 485, 1010, 988, 511, 985, 511, 986, 509, 487, 1009, 489, 1011, 486, 1011, 486, 1009, 488, 1007, 989 };

    TEST_ASSERT_FALSE(decode(pulses, sizeof(pulses) / sizeof(pulses[0])));
}

void setUp(void) {
}

void tearDown(void) {
}

int main() {
    UNITY_BEGIN();  // Initialize Unity

    // Run your tests
    RUN_TEST(ok);
    RUN_TEST(ok_because_two_first_preamble_pairs_gets_ignored);
    RUN_TEST(ok_preamble_only_6_of_8_pairs);
    RUN_TEST(nok_because_three_first_preamble_pairs_are_wrong);

    return UNITY_END();  // Cleanup and report results
}

