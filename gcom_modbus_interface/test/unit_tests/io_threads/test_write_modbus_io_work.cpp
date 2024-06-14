#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <cstring>
#include "gcom_iothread.h"

extern int modbus_write_registers_ret;
extern int modbus_write_bit_ret;
extern int modbus_write_bits_ret;

TEST(WriteModbusIOWorkTest, WritesHoldingRegisters)
{
    cfg myCfg;
    int io_tries = 0;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    io_work->register_type = cfg::Register_Types::Holding;
    io_work->offset = 10;
    io_work->num_registers = 5;
    io_work->off_by_one = false;

    modbus_write_registers_ret = 5;

    int result = write_modbus_io_work(myCfg, io_tries, io_thread, io_work, false);

    EXPECT_EQ(result, 5);
    EXPECT_EQ(io_work->func, "write_registers");
    EXPECT_EQ(io_work->good, 1);
    EXPECT_EQ(io_work->errors, 0);
}

TEST(WriteModbusIOWorkTest, WritesSingleCoilRegister)
{
    cfg myCfg;
    int io_tries = 0;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    io_work->register_type = cfg::Register_Types::Coil;
    io_work->offset = 10;
    io_work->num_registers = 1;
    io_work->off_by_one = false;

    modbus_write_bit_ret = 1;

    int result = write_modbus_io_work(myCfg, io_tries, io_thread, io_work, false);

    EXPECT_EQ(result, 1);
    EXPECT_EQ(io_work->func, "write_bit");
    EXPECT_EQ(io_work->good, 1);
    EXPECT_EQ(io_work->errors, 0);
}

TEST(WriteModbusIOWorkTest, WritesMultipleCoilRegisters)
{
    cfg myCfg;
    int io_tries = 0;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    io_work->register_type = cfg::Register_Types::Coil;
    io_work->offset = 10;
    io_work->num_registers = 5;
    io_work->off_by_one = false;
    myCfg.force_multi_sets = true;

    modbus_write_bits_ret = 5;

    int result = write_modbus_io_work(myCfg, io_tries, io_thread, io_work, false);

    EXPECT_EQ(result, 5);
    EXPECT_EQ(io_work->func, "write_bits");
    EXPECT_EQ(io_work->good, 1);
    EXPECT_EQ(io_work->errors, 0);
}

TEST(WriteModbusIOWorkTest, HandlesUnknownRegisterType)
{
    cfg myCfg;
    int io_tries = 0;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    io_work->register_type = cfg::Register_Types::Undefined;  // Unknown register type
    io_work->offset = 10;
    io_work->num_registers = 5;
    io_work->off_by_one = false;

    int result = write_modbus_io_work(myCfg, io_tries, io_thread, io_work, false);

    EXPECT_EQ(result, 0);
    EXPECT_EQ(io_work->good, 1);
    EXPECT_EQ(io_work->errors, 0);
}

TEST(WriteModbusIOWorkTest, HandlesWriteError)
{
    cfg myCfg;
    int io_tries = 0;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    io_work->register_type = cfg::Register_Types::Holding;
    io_work->offset = 10;
    io_work->num_registers = 5;
    io_work->off_by_one = false;

    modbus_write_registers_ret = -1;

    int result = write_modbus_io_work(myCfg, io_tries, io_thread, io_work, false);

    EXPECT_EQ(result, -1);
    EXPECT_EQ(io_work->good, 0);
    EXPECT_EQ(io_work->errors, 5);
}