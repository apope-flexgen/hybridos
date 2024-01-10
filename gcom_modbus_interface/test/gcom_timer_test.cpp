#include <gtest/gtest.h>
#include "gcom_timer.h"  // Replace this with the path to your header
#include <thread>

//g++ --std=c++17 -I include -o gt test/gcom_timer_test.cpp  src/gcom_timer.cpp -lpthread -lgtest -lgmock
namespace {

void TestCallbackFunction(TimeObject* obj, void* param) {
    int* timesCalled = static_cast<int*>(param);
    (*timesCalled)++;
}

class TimerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Called before each test
    }

    void TearDown() override {
        // Called after each test
        stopTimer(); // To ensure the timer stops after each test
    }
};

TEST_F(TimerTest, AddAndRunTimerObject) {
    int timesCalled = 0;
    TimeObject testObj("TestTimer", get_time_double() + 0.5, 0.0, 0.0, 1, TestCallbackFunction, &timesCalled);
    addTimeObject(testObj);
    runTimer();

    // Sleep to ensure the timer gets enough time to execute
    std::this_thread::sleep_for(std::chrono::seconds(1));

    ASSERT_EQ(timesCalled, 1);
}

TEST_F(TimerTest, AddAndRunSyncTimeObject) {
    int timesCalled = 0;
    TimeObject testObj("TestSyncTimer", get_time_double() + 0.5, 0.0, 0.0, 1, TestCallbackFunction, &timesCalled);
    addSyncTimeObject(testObj, 0.2);
    runTimer();

    // Sleep to ensure the timer gets enough time to execute
    std::this_thread::sleep_for(std::chrono::seconds(1));

    ASSERT_EQ(timesCalled, 1);
}

// ... You can add more tests for other functionalities here ...

}  // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
