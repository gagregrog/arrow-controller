#include <unity.h>
#include "ReconnectTimer.h"

void setUp() {}
void tearDown() {}

// While connected, no attempt is ever requested.
static void test_connected_never_attempts() {
    ReconnectTimer t(10000);
    TEST_ASSERT_FALSE(t.shouldAttempt(1000, true));
    TEST_ASSERT_FALSE(t.shouldAttempt(50000, true));
}

// The first loop after a drop attempts immediately.
static void test_first_drop_attempts_immediately() {
    ReconnectTimer t(10000);
    TEST_ASSERT_TRUE(t.shouldAttempt(1000, false));
}

// After an attempt, retries are paced by the interval.
static void test_retries_are_paced() {
    ReconnectTimer t(10000);
    TEST_ASSERT_TRUE(t.shouldAttempt(1000, false));   // attempt at 1000
    TEST_ASSERT_FALSE(t.shouldAttempt(5000, false));  // too soon
    TEST_ASSERT_FALSE(t.shouldAttempt(10999, false)); // still too soon
    TEST_ASSERT_TRUE(t.shouldAttempt(11000, false));  // interval elapsed
    TEST_ASSERT_FALSE(t.shouldAttempt(15000, false)); // paced again
}

// Reconnecting resets state so a later drop retries immediately.
static void test_reconnect_resets() {
    ReconnectTimer t(10000);
    TEST_ASSERT_TRUE(t.shouldAttempt(1000, false));   // attempt
    TEST_ASSERT_FALSE(t.shouldAttempt(2000, true));   // link back — reset
    TEST_ASSERT_TRUE(t.shouldAttempt(3000, false));   // new drop retries now
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_connected_never_attempts);
    RUN_TEST(test_first_drop_attempts_immediately);
    RUN_TEST(test_retries_are_paced);
    RUN_TEST(test_reconnect_resets);
    return UNITY_END();
}
