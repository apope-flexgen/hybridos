#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <vector>
#include <cstring>
#include <mutex>

#include "gcom_iothread.h"

TEST(LocalReadBitsTest, ReadsDataCorrectly)
{
    cfg myCfg;
    auto io_work = std::make_shared<IO_Work>();

    // Create and initialize cfg::io_point_structs
    auto io_point1 = std::make_shared<cfg::io_point_struct>();
    memset(io_point1->reg8, 0, 4);
    io_point1->reg8[0] = 1;
    io_point1->reg8[1] = 2;
    io_point1->size = 2;

    auto io_point2 = std::make_shared<cfg::io_point_struct>();
    memset(io_point2->reg8, 0, 4);
    io_point2->reg8[0] = 3;
    io_point2->reg8[1] = 4;
    io_point2->reg8[2] = 5;
    io_point2->size = 3;

    io_work->io_points.push_back(io_point1);
    io_work->io_points.push_back(io_point2);

    // Initialize buffer
    memset(io_work->buf8, 0, 256);
    io_work->errors = 0;

    local_read_bits(myCfg, io_work, false);

    EXPECT_EQ(io_work->buf8[0], 1);
    EXPECT_EQ(io_work->buf8[1], 2);
    EXPECT_EQ(io_work->buf8[2], 3);
    EXPECT_EQ(io_work->buf8[3], 4);
    EXPECT_EQ(io_work->buf8[4], 5);
    EXPECT_EQ(io_work->errors, 5);
}

TEST(LocalReadBitsTest, HandlesEmptyIOPoints)
{
    cfg myCfg;
    auto io_work = std::make_shared<IO_Work>();

    // No cfg::io_point_structs added
    memset(io_work->buf8, 0, 256);
    io_work->errors = 0;

    local_read_bits(myCfg, io_work, false);

    EXPECT_EQ(io_work->errors, 0);
}

TEST(LocalReadBitsTest, HandlesEmptyBuffer)
{
    cfg myCfg;
    auto io_work = std::make_shared<IO_Work>();

    // Create and initialize cfg::io_point_structs
    auto io_point1 = std::make_shared<cfg::io_point_struct>();
    memset(io_point1->reg8, 0, 4);
    io_point1->reg8[0] = 1;
    io_point1->reg8[1] = 2;
    io_point1->size = 2;

    io_work->io_points.push_back(io_point1);

    // Empty buffer
    memset(io_work->buf8, 0, 256);
    io_work->errors = 0;

    local_read_bits(myCfg, io_work, false);

    EXPECT_EQ(io_work->buf8[0], 1);
    EXPECT_EQ(io_work->buf8[1], 2);
    EXPECT_EQ(io_work->errors, 2);
}

TEST(LocalReadBitsTest, HandlesNullIOPoints)
{
    cfg myCfg;
    auto io_work = std::make_shared<IO_Work>();

    // Null cfg::io_point_structs
    io_work->io_points.push_back(nullptr);

    memset(io_work->buf8, 0, 256);
    io_work->errors = 0;

    local_read_bits(myCfg, io_work, false);

    EXPECT_EQ(io_work->errors, 0);  // No data should be read
}

TEST(LocalReadBitsTest, ThreadSafety)
{
    cfg myCfg;
    auto io_work = std::make_shared<IO_Work>();

    auto io_point1 = std::make_shared<cfg::io_point_struct>();
    memset(io_point1->reg8, 0, 4);
    io_point1->reg8[0] = 1;
    io_point1->reg8[1] = 2;
    io_point1->size = 2;

    io_work->io_points.push_back(io_point1);

    memset(io_work->buf8, 0, 256);
    io_work->errors = 0;

    // Lock the mutex manually to simulate concurrent access
    io_point1->mtx.lock();

    std::thread t([&]() { local_read_bits(myCfg, io_work, false); });

    // Ensure the mutex is locked by the main thread
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    io_point1->mtx.unlock();
    t.join();

    EXPECT_EQ(io_work->buf8[0], 1);
    EXPECT_EQ(io_work->buf8[1], 2);
    EXPECT_EQ(io_work->errors, 2);
}
