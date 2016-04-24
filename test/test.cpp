#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include <chrono>
#include <thread>

#include "timerwheels.h"

static std::chrono::milliseconds elapsed(0);

struct MockClock {
    using duration = std::chrono::milliseconds;
    using time_point = std::chrono::time_point<MockClock>;

    static time_point now() {
        return time_point(elapsed);
    }
};

class TimerWheelsTest final : public CppUnit::TestFixture {
    using millis = std::chrono::milliseconds;

  public:
    void testParameterDefaults() {
        timerwheels::FixedRangeTimerWheel<> it;
    }

    void testMockClock() {
        elapsed = millis(0);
        CPPUNIT_ASSERT(MockClock::now() == MockClock::time_point(millis(0)));

        elapsed = millis(20);
        CPPUNIT_ASSERT(MockClock::now() == MockClock::time_point(millis(20)));
    }

    void testSimpleCase() {
        elapsed = millis(0);

        timerwheels::FixedRangeTimerWheel<millis, 12, 2, MockClock> it;
        bool set = false;

        it.tick();
        CPPUNIT_ASSERT(!set);

        it.schedule([&set]() { set = true; }, millis(1));
        CPPUNIT_ASSERT(!set);

        it.tick();
        CPPUNIT_ASSERT(!set);

        elapsed = millis(4);
        it.tick();
        CPPUNIT_ASSERT(set);
    }

    void testDistantFuture() {
        elapsed = millis(0);

        timerwheels::FixedRangeTimerWheel<millis, 12, 2, MockClock> it;
        bool set = false;

        it.tick();
        CPPUNIT_ASSERT(!set);

        it.schedule([&set]() { set = true; }, millis(1000));
        CPPUNIT_ASSERT(!set);

        it.tick();
        CPPUNIT_ASSERT(!set);

        elapsed = millis(4);
        it.tick();
        CPPUNIT_ASSERT(!set);

        // max is 12 ms, one whole cycle of the clock
        elapsed = millis(12);
        it.tick();
        CPPUNIT_ASSERT(set);
    }

    void testSchedule2SameTime() {
        elapsed = millis(0);

        timerwheels::FixedRangeTimerWheel<millis, 12, 2, MockClock> it;
        bool set1 = false;
        bool set2 = false;

        it.tick();
        CPPUNIT_ASSERT(!set1);
        CPPUNIT_ASSERT(!set2);

        it.schedule([&set1]() { set1 = true; }, millis(1));
        it.schedule([&set2]() { set2 = true; }, millis(1));
        CPPUNIT_ASSERT(!set1);
        CPPUNIT_ASSERT(!set2);

        it.tick();
        CPPUNIT_ASSERT(!set1);
        CPPUNIT_ASSERT(!set2);

        elapsed = millis(4);
        it.tick();
        CPPUNIT_ASSERT(set1);
        CPPUNIT_ASSERT(set2);
    }

    void testSchedule2DifferentTimes() {
        elapsed = millis(0);

        timerwheels::FixedRangeTimerWheel<millis, 12, 2, MockClock> it;
        bool set1 = false;
        bool set2 = false;

        it.tick();
        CPPUNIT_ASSERT(!set1);
        CPPUNIT_ASSERT(!set2);

        it.schedule([&set1]() { set1 = true; }, millis(1));
        it.schedule([&set2]() { set2 = true; }, millis(3));
        CPPUNIT_ASSERT(!set1);
        CPPUNIT_ASSERT(!set2);

        it.tick();
        CPPUNIT_ASSERT(!set1);
        CPPUNIT_ASSERT(!set2);

        elapsed = millis(4);
        it.tick();
        CPPUNIT_ASSERT(set1);
        CPPUNIT_ASSERT(set2);
    }

    void testScheduleRepeated() {
        elapsed = millis(0);

        timerwheels::FixedRangeTimerWheel<millis, 12, 2, MockClock> it;
        int set = 0;

        it.tick();
        CPPUNIT_ASSERT(set == 0);

        it.schedule([&set]() { ++set; }, millis(3), millis(3));
        CPPUNIT_ASSERT(set == 0);

        it.tick();
        CPPUNIT_ASSERT(set == 0);

        elapsed = millis(4);
        it.tick();
        CPPUNIT_ASSERT(set == 1);

        // rescheduled in next bucket
        it.tick();
        CPPUNIT_ASSERT(set == 1);

        // still same bucket
        elapsed = millis(5);
        it.tick();
        CPPUNIT_ASSERT(set == 1);

        // not in the next bucket (skip 3)
        elapsed = millis(6);
        it.tick();
        CPPUNIT_ASSERT(set == 1);

        // (same bucket)
        elapsed = millis(7);
        it.tick();
        CPPUNIT_ASSERT(set == 1);

        // and now rescheduled
        elapsed = millis(8);
        it.tick();
        CPPUNIT_ASSERT(set == 2);
    }

    CPPUNIT_TEST_SUITE(TimerWheelsTest);
    CPPUNIT_TEST(testMockClock);
    CPPUNIT_TEST(testParameterDefaults);
    CPPUNIT_TEST(testSimpleCase);
    CPPUNIT_TEST(testSchedule2SameTime);
    CPPUNIT_TEST(testSchedule2DifferentTimes);
    CPPUNIT_TEST(testScheduleRepeated);
    CPPUNIT_TEST(testDistantFuture);
    CPPUNIT_TEST_SUITE_END();
};

int main() {
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(TimerWheelsTest::suite());
    return !runner.run("", false);
}
