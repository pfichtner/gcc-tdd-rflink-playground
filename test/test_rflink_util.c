#include "Unity/src/unity.h"
#include "../src/rflink_util.h"

void test_your_function() {
    TEST_ASSERT_EQUAL_INT(0, mainX());
}

void setUp(void) {
}

void tearDown(void) {
}

int main() {
    UNITY_BEGIN();  // Initialize Unity

    // Run your tests
    RUN_TEST(test_your_function);

    return UNITY_END();  // Cleanup and report results
}

