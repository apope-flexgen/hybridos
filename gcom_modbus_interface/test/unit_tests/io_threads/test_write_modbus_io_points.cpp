#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <vector>
#include <cstring>

#include "gcom_iothread.h"

extern int modbus_write_register_ret;
extern int modbus_write_registers_ret;
extern int modbus_write_bit_ret;
extern int modbus_write_bits_ret;

TEST(WriteModbusIOPointsTest, WritesHoldingRegisters)
{
    cfg myCfg;
    int io_tries = 0;
    bool io_done = false;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    io_work->register_type = cfg::Register_Types::Holding;
    io_work->offset = 10;
    io_work->off_by_one = false;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->offset = 10;
    io_point->size = 5;
    io_work->io_points.push_back(io_point);

    modbus_write_registers_ret = 5;

    int result = write_modbus_io_points(myCfg, io_tries, io_thread, io_work, io_done, false);

    EXPECT_EQ(result, 5);
    EXPECT_EQ(io_work->good, 5);
    EXPECT_EQ(io_work->errors, 0);
}

TEST(WriteModbusIOPointsTest, WritesSingleHoldingRegister)
{
    cfg myCfg;
    int io_tries = 0;
    bool io_done = false;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    io_work->register_type = cfg::Register_Types::Holding;
    io_work->offset = 10;
    io_work->off_by_one = false;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->offset = 10;
    io_point->size = 1;
    io_work->io_points.push_back(io_point);

    modbus_write_register_ret = 1;

    int result = write_modbus_io_points(myCfg, io_tries, io_thread, io_work, io_done, false);

    EXPECT_EQ(result, 1);
    EXPECT_EQ(io_work->good, 1);
    EXPECT_EQ(io_work->errors, 0);
}

TEST(WriteModbusIOPointsTest, WritesCoilRegisters)
{
    cfg myCfg;
    int io_tries = 0;
    bool io_done = false;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    io_work->register_type = cfg::Register_Types::Coil;
    io_work->offset = 10;
    io_work->off_by_one = false;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->offset = 10;
    io_point->size = 5;
    io_work->io_points.push_back(io_point);

    modbus_write_bits_ret = 5;

    int result = write_modbus_io_points(myCfg, io_tries, io_thread, io_work, io_done, false);

    EXPECT_EQ(result, 5);
    EXPECT_EQ(io_work->good, 5);
    EXPECT_EQ(io_work->errors, 0);
}

TEST(WriteModbusIOPointsTest, WritesSingleCoilRegister)
{
    cfg myCfg;
    int io_tries = 0;
    bool io_done = false;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    io_work->register_type = cfg::Register_Types::Coil;
    io_work->offset = 10;
    io_work->off_by_one = false;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->offset = 10;
    io_point->size = 1;
    io_work->io_points.push_back(io_point);

    modbus_write_bit_ret = 1;

    int result = write_modbus_io_points(myCfg, io_tries, io_thread, io_work, io_done, false);

    EXPECT_EQ(result, 1);
    EXPECT_EQ(io_work->good, 1);
    EXPECT_EQ(io_work->errors, 0);
}

TEST(WriteModbusIOPointsTest, HandlesUnknownRegisterType)
{
    cfg myCfg;
    int io_tries = 0;
    bool io_done = false;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    io_work->register_type = cfg::Register_Types::Undefined;  // Unknown register type
    io_work->offset = 10;
    io_work->off_by_one = false;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->offset = 10;
    io_point->size = 5;
    io_work->io_points.push_back(io_point);

    int result = write_modbus_io_points(myCfg, io_tries, io_thread, io_work, io_done, false);

    EXPECT_EQ(result, 0);
    EXPECT_EQ(io_work->good, 0);
    EXPECT_EQ(io_work->errors, 0);
}

TEST(WriteModbusIOPointsTest, HandlesWriteError)
{
    cfg myCfg;
    int io_tries = 0;
    bool io_done = false;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    io_work->register_type = cfg::Register_Types::Holding;
    io_work->offset = 10;
    io_work->off_by_one = false;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->offset = 10;
    io_point->size = 5;
    io_work->io_points.push_back(io_point);

    modbus_write_registers_ret = -1;

    int result = write_modbus_io_points(myCfg, io_tries, io_thread, io_work, io_done, false);

    EXPECT_EQ(result, 0);
    EXPECT_EQ(io_work->good, 0);
    EXPECT_EQ(io_work->errors, 1);
}
