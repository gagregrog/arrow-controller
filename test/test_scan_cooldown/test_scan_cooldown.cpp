#include <unity.h>
#include "ScanCooldown.h"

void setUp() {}
void tearDown() {}

// A fresh cooldown accepts the first scan.
static void test_first_scan_accepted() {
    ScanCooldown cd(15000);
    TEST_ASSERT_TRUE(cd.tryScan(1000));
}

// Scans inside the window are rejected, right up to (but not including) expiry.
static void test_scan_within_window_rejected() {
    ScanCooldown cd(15000);
    TEST_ASSERT_TRUE(cd.tryScan(1000));    // window: [1000, 16000)
    TEST_ASSERT_FALSE(cd.tryScan(5000));
    TEST_ASSERT_FALSE(cd.tryScan(15999));
}

// A scan at or after expiry is accepted and starts a new window.
static void test_scan_after_window_accepted() {
    ScanCooldown cd(15000);
    TEST_ASSERT_TRUE(cd.tryScan(1000));
    TEST_ASSERT_TRUE(cd.tryScan(16000));
}

// Breathing arms only once a scan is actually rejected, and clears at expiry.
static void test_breathing_only_after_rejection() {
    ScanCooldown cd(15000);
    cd.tryScan(1000);
    TEST_ASSERT_FALSE(cd.breathing(2000));  // accepted scan alone => no cue
    cd.tryScan(2000);                        // rejected
    TEST_ASSERT_TRUE(cd.breathing(3000));
    TEST_ASSERT_TRUE(cd.breathing(15999));
    TEST_ASSERT_FALSE(cd.breathing(16000)); // window ended
}

// Rejected scans must not push the window out.
static void test_rejection_does_not_extend_window() {
    ScanCooldown cd(15000);
    cd.tryScan(1000);                        // window ends at 16000
    TEST_ASSERT_FALSE(cd.tryScan(10000));    // rejected
    TEST_ASSERT_TRUE(cd.tryScan(16000));     // still expires on schedule
}

// Accepting a new scan clears a prior breathing cue.
static void test_breathing_resets_on_new_scan() {
    ScanCooldown cd(15000);
    cd.tryScan(1000);
    cd.tryScan(2000);                        // rejected -> breathing armed
    TEST_ASSERT_TRUE(cd.breathing(3000));
    TEST_ASSERT_TRUE(cd.tryScan(16000));     // new window
    TEST_ASSERT_FALSE(cd.breathing(17000));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_first_scan_accepted);
    RUN_TEST(test_scan_within_window_rejected);
    RUN_TEST(test_scan_after_window_accepted);
    RUN_TEST(test_breathing_only_after_rejection);
    RUN_TEST(test_rejection_does_not_extend_window);
    RUN_TEST(test_breathing_resets_on_new_scan);
    return UNITY_END();
}
