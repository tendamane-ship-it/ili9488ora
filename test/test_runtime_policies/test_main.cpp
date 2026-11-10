#include <unity.h>
#include "RuntimePolicies.h"

using runtime_policies::RamProtection;
using runtime_policies::ResetHistory;

void testRamProtectionUsesStrictThresholds()
{
    RamProtection protection;

    TEST_ASSERT_FALSE(protection.update(100UL * 1024UL));
    TEST_ASSERT_TRUE(protection.update(100UL * 1024UL - 1));
    TEST_ASSERT_TRUE(protection.update(100UL * 1024UL));
    TEST_ASSERT_TRUE(protection.update(130UL * 1024UL));
    TEST_ASSERT_FALSE(protection.update(130UL * 1024UL + 1));
}

void testRamProtectionCanActivateAgainAfterRecovery()
{
    RamProtection protection;

    TEST_ASSERT_TRUE(protection.update(99UL * 1024UL));
    TEST_ASSERT_FALSE(protection.update(131UL * 1024UL));
    TEST_ASSERT_TRUE(protection.update(99UL * 1024UL));
}

void testResetHistoryPreservesInsertionOrderBeforeWrapping()
{
    ResetHistory<8> history;
    history.push(1);
    history.push(2);
    history.push(3);

    uint8_t reason = 0;
    TEST_ASSERT_EQUAL_UINT32(3, history.size());
    TEST_ASSERT_TRUE(history.atOldestFirst(0, reason));
    TEST_ASSERT_EQUAL_UINT8(1, reason);
    TEST_ASSERT_TRUE(history.atOldestFirst(2, reason));
    TEST_ASSERT_EQUAL_UINT8(3, reason);
    TEST_ASSERT_FALSE(history.atOldestFirst(3, reason));
}

void testResetHistoryKeepsOnlyNewestEntriesAfterWrapping()
{
    ResetHistory<8> history;
    for (uint8_t reason = 1; reason <= 10; ++reason)
        history.push(reason);

    uint8_t reason = 0;
    TEST_ASSERT_EQUAL_UINT32(8, history.size());
    for (std::size_t i = 0; i < history.size(); ++i)
    {
        TEST_ASSERT_TRUE(history.atOldestFirst(i, reason));
        TEST_ASSERT_EQUAL_UINT8(i + 3, reason);
    }
}

void testResetHistoryCanRestorePersistedState()
{
    ResetHistory<8> history;
    for (uint8_t reason = 1; reason <= 10; ++reason)
        history.push(reason);

    ResetHistory<8> restored;
    TEST_ASSERT_TRUE(
        restored.restore(history.data(), history.nextIndex(), history.size())
    );
    restored.push(11);

    uint8_t reason = 0;
    TEST_ASSERT_EQUAL_UINT32(8, restored.size());
    TEST_ASSERT_TRUE(restored.atOldestFirst(0, reason));
    TEST_ASSERT_EQUAL_UINT8(4, reason);
    TEST_ASSERT_TRUE(restored.atOldestFirst(7, reason));
    TEST_ASSERT_EQUAL_UINT8(11, reason);
}

void testResetHistoryRejectsInvalidPersistedState()
{
    ResetHistory<8> history;
    const uint8_t entries[8] = {};

    TEST_ASSERT_FALSE(history.restore(nullptr, 0, 0));
    TEST_ASSERT_FALSE(history.restore(entries, 8, 0));
    TEST_ASSERT_FALSE(history.restore(entries, 0, 9));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(testRamProtectionUsesStrictThresholds);
    RUN_TEST(testRamProtectionCanActivateAgainAfterRecovery);
    RUN_TEST(testResetHistoryPreservesInsertionOrderBeforeWrapping);
    RUN_TEST(testResetHistoryKeepsOnlyNewestEntriesAfterWrapping);
    RUN_TEST(testResetHistoryCanRestorePersistedState);
    RUN_TEST(testResetHistoryRejectsInvalidPersistedState);
    return UNITY_END();
}
