#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <vector>
#include <cstring>
#include <mutex>

#include "gcom_config.h"
#include "gcom_iothread.h"

TEST(LocalWriteBitsTest, WritesDataCorrectly)
{
    cfg myCfg;
    auto io_work = std::make_shared<IO_Work>();

    // Create and initialize cfg::io_point_structs
    auto io_point1 = std::make_shared<cfg::io_point_struct>();
    memset(io_point1->reg8, 0, 4);
    io_point1->size = 2;

    auto io_point2 = std::make_shared<cfg::io_point_struct>();
    memset(io_point2->reg8, 0, 4);
    io_point2->size = 3;

    io_work->io_points.push_back(io_point1);
    io_work->io_points.push_back(io_point2);

    // Initialize buffer
    uint8_t buf[5] = { 1, 2, 3, 4, 5 };
    memcpy(io_work->buf8, buf, 5);
    io_work->errors = 0;

    local_write_bits(myCfg, io_work, false);

    EXPECT_EQ(io_point1->reg8[0], 1);
    EXPECT_EQ(io_point1->reg8[1], 2);
    EXPECT_EQ(io_point2->reg8[0], 3);
    EXPECT_EQ(io_point2->reg8[1], 4);
    EXPECT_EQ(io_point2->reg8[2], 5);
    EXPECT_EQ(io_work->errors, 5);
}

TEST(LocalWriteBitsTest, HandlesEmptyIOPoints)
{
    cfg myCfg;
    auto io_work = std::make_shared<IO_Work>();

    // No cfg::io_point_structs added
    uint8_t buf[5] = { 1, 2, 3, 4, 5 };
    memcpy(io_work->buf8, buf, 5);
    io_work->errors = 0;

    local_write_bits(myCfg, io_work, false);

    EXPECT_EQ(io_work->errors, 0);
}

TEST(LocalWriteBitsTest, HandlesEmptyBuffer)
{
    cfg myCfg;
    auto io_work = std::make_shared<IO_Work>();

    // Create and initialize cfg::io_point_structs
    auto io_point1 = std::make_shared<cfg::io_point_struct>();
    memset(io_point1->reg8, 0, 4);
    io_point1->size = 2;

    io_work->io_points.push_back(io_point1);

    // Empty buffer
    memset(io_work->buf8, 0, 256);
    io_work->errors = 0;

    local_write_bits(myCfg, io_work, false);

    EXPECT_EQ(io_point1->reg8[0], 0);
    EXPECT_EQ(io_point1->reg8[1], 0);
    EXPECT_EQ(io_work->errors, 2);
}

TEST(LocalWriteBitsTest, HandlesNullIOPoints)
{
    cfg myCfg;
    auto io_work = std::make_shared<IO_Work>();

    // Null cfg::io_point_structs
    io_work->io_points.push_back(nullptr);

    uint8_t buf[5] = { 1, 2, 3, 4, 5 };
    memcpy(io_work->buf8, buf, 5);
    io_work->errors = 0;

    local_write_bits(myCfg, io_work, false);

    EXPECT_EQ(io_work->errors, 0);  // No data should be written
}

TEST(LocalWriteBitsTest, ThreadSafety)
{
    cfg myCfg;
    auto io_work = std::make_shared<IO_Work>();

    auto io_point1 = std::make_shared<cfg::io_point_struct>();
    memset(io_point1->reg8, 0, 4);
    io_point1->size = 2;

    io_work->io_points.push_back(io_point1);

    uint8_t buf[2] = { 1, 2 };
    memcpy(io_work->buf8, buf, 2);
    io_work->errors = 0;

    // Lock the mutex manually to simulate concurrent access
    io_point1->mtx.lock();

    std::thread t([&]() { local_write_bits(myCfg, io_work, false); });

    // Ensure the mutex is locked by the main thread
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    io_point1->mtx.unlock();
    t.join();

    EXPECT_EQ(io_point1->reg8[0], 1);
    EXPECT_EQ(io_point1->reg8[1], 2);
    EXPECT_EQ(io_work->errors, 2);
}
