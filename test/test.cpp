#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include <chrono>
#include <thread>

#include "timerwheels.h"

class TimerWheelsTest final : public CppUnit::TestFixture {
  public:

    void testAsserts() {
    }

    CPPUNIT_TEST_SUITE(TimerWheelsTest);
    CPPUNIT_TEST(testAsserts);
    CPPUNIT_TEST_SUITE_END();
};

int main() {
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(TimerWheelsTest::suite());
    return !runner.run("", false);
}
