#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <cstring>

#include "gcom_iothread.h"

extern int modbus_read_registers_ret;
extern int modbus_read_input_registers_ret;
extern int modbus_read_bits_ret;
extern int modbus_read_input_bits_ret;

TEST(ReadModbusIOWorkTest, ReadsHoldingRegisters)
{
    cfg myCfg;
    int io_tries = 0;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    io_work->register_type = cfg::Register_Types::Holding;
    io_work->offset = 10;
    io_work->num_registers = 5;
    io_work->off_by_one = false;

    modbus_read_registers_ret = 0;

    int result = read_modbus_io_work(myCfg, io_tries, io_thread, io_work, false);

    EXPECT_EQ(result, 0);
    EXPECT_EQ(io_work->func, "read_registers");
    EXPECT_EQ(io_work->good, 1);
    EXPECT_EQ(io_work->errors, 0);
}

TEST(ReadModbusIOWorkTest, ReadsCoilRegisters)
{
    cfg myCfg;
    int io_tries = 0;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    io_work->register_type = cfg::Register_Types::Coil;
    io_work->offset = 10;
    io_work->num_registers = 5;
    io_work->off_by_one = false;

    modbus_read_bits_ret = 0;

    int result = read_modbus_io_work(myCfg, io_tries, io_thread, io_work, false);

    EXPECT_EQ(result, 0);
    EXPECT_EQ(io_work->func, "read_bits");
    EXPECT_EQ(io_work->good, 1);
    EXPECT_EQ(io_work->errors, 0);
}

TEST(ReadModbusIOWorkTest, ReadsInputRegisters)
{
    cfg myCfg;
    int io_tries = 0;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    io_work->register_type = cfg::Register_Types::Input;
    io_work->offset = 10;
    io_work->num_registers = 5;
    io_work->off_by_one = false;

    modbus_read_input_registers_ret = 0;

    int result = read_modbus_io_work(myCfg, io_tries, io_thread, io_work, false);

    EXPECT_EQ(result, 0);
    EXPECT_EQ(io_work->func, "read_input_registers");
    EXPECT_EQ(io_work->good, 1);
    EXPECT_EQ(io_work->errors, 0);
}

TEST(ReadModbusIOWorkTest, ReadsDiscreteInputRegisters)
{
    cfg myCfg;
    int io_tries = 0;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    io_work->register_type = cfg::Register_Types::Discrete_Input;
    io_work->offset = 10;
    io_work->num_registers = 5;
    io_work->off_by_one = false;

    modbus_read_input_bits_ret = 0;

    int result = read_modbus_io_work(myCfg, io_tries, io_thread, io_work, false);

    EXPECT_EQ(result, 0);
    EXPECT_EQ(io_work->func, "read_input_bits");
    EXPECT_EQ(io_work->good, 1);
    EXPECT_EQ(io_work->errors, 0);
}

TEST(ReadModbusIOWorkTest, HandlesUnknownRegisterType)
{
    cfg myCfg;
    int io_tries = 0;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    io_work->register_type = cfg::Register_Types::Undefined;  // Unknown register type
    io_work->offset = 10;
    io_work->num_registers = 5;
    io_work->off_by_one = false;

    int result = read_modbus_io_work(myCfg, io_tries, io_thread, io_work, false);

    EXPECT_EQ(result, 0);
    EXPECT_EQ(io_work->good, 1);
    EXPECT_EQ(io_work->errors, 0);
}

TEST(ReadModbusIOWorkTest, HandlesReadError)
{
    cfg myCfg;
    int io_tries = 0;
    auto io_thread = std::make_shared<IO_Thread>();
    auto io_work = std::make_shared<IO_Work>();
    io_work->register_type = cfg::Register_Types::Holding;
    io_work->offset = 10;
    io_work->num_registers = 5;
    io_work->off_by_one = false;

    modbus_read_registers_ret = -1;

    int result = read_modbus_io_work(myCfg, io_tries, io_thread, io_work, false);

    EXPECT_EQ(result, -1);
    EXPECT_EQ(io_work->good, 0);
    EXPECT_EQ(io_work->errors, 5);
}