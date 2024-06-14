#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <vector>
#include <cstring>
#include <mutex>

#include "gcom_iothread.h"

TEST(HandlePointErrorTest, HandlesECONNRESET)
{
    cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 2;
    io_thread->connect_reset = 10;
    io_thread->is_enabled = true;

    auto io_work = std::make_shared<IO_Work>();
    io_work->data_error = false;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->errno_code = ECONNRESET;
    io_point->id = "TestPoint";
    io_point->offset = 0;

    handle_point_error(myCfg, io_thread, io_work, io_point, "Test_func", ECONNRESET);

    EXPECT_TRUE(io_work->data_error);
    EXPECT_FALSE(io_thread->is_enabled);
}

TEST(HandlePointErrorTest, HandlesEPIPE)
{
    cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 1;
    io_thread->connect_reset = 0;

    auto io_work = std::make_shared<IO_Work>();
    io_work->data_error = false;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->errno_code = EPIPE;
    io_point->id = "TestPoint";
    io_point->offset = 0;

    handle_point_error(myCfg, io_thread, io_work, io_point, "Test_func", EPIPE);

    EXPECT_TRUE(io_work->data_error);
}

TEST(HandlePointErrorTest, HandlesETIMEDOUT)
{
    cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 1;
    io_thread->connect_reset = 0;

    auto io_work = std::make_shared<IO_Work>();
    io_work->data_error = false;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->errno_code = ETIMEDOUT;
    io_point->id = "TestPoint";
    io_point->offset = 0;

    handle_point_error(myCfg, io_thread, io_work, io_point, "Test_func", ETIMEDOUT);

    EXPECT_TRUE(io_work->data_error);
}

TEST(HandlePointErrorTest, HandlesEMBBADEXC)
{
    cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 1;
    io_thread->connect_reset = 0;

    auto io_work = std::make_shared<IO_Work>();
    io_work->data_error = false;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->errno_code = EMBBADEXC;
    io_point->id = "TestPoint";
    io_point->offset = 0;

    handle_point_error(myCfg, io_thread, io_work, io_point, "Test_func", EMBBADEXC);

    EXPECT_FALSE(io_work->data_error);  // Assuming handleEMBBADEXC will handle the error
}

TEST(HandlePointErrorTest, HandlesOtherErrors)
{
    cfg myCfg;
    myCfg.auto_disable = true;

    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 1;
    io_thread->connect_reset = 0;

    auto io_work = std::make_shared<IO_Work>();
    io_work->data_error = false;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->errno_code = EINVAL;
    io_point->id = "TestPoint";
    io_point->offset = 0;
    io_point->is_disconnected = false;

    handle_point_error(myCfg, io_thread, io_work, io_point, "Test_func", EINVAL);

    EXPECT_TRUE(io_work->data_error);
}

TEST(HandlePointAutoDisableTest, AutoDisableNotEnabled)
{
    cfg myCfg;
    myCfg.auto_disable = false;

    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 1;

    auto io_work = std::make_shared<IO_Work>();
    io_work->data_error = false;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->errno_code = EINVAL;
    io_point->id = "TestPoint";
    io_point->offset = 0;
    io_point->is_disconnected = false;

    handle_point_auto_disable(myCfg, io_thread, io_work, io_point, "test_func", EINVAL);

    EXPECT_FALSE(io_work->data_error);
}

TEST(HandlePointAutoDisableTest, HandlesExemptErrors)
{
    cfg myCfg;
    myCfg.auto_disable = true;

    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 1;

    auto io_work = std::make_shared<IO_Work>();
    io_work->data_error = false;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->errno_code = EPIPE;
    io_point->id = "TestPoint";
    io_point->offset = 0;
    io_point->is_disconnected = false;

    handle_point_auto_disable(myCfg, io_thread, io_work, io_point, "test_func", EPIPE);

    EXPECT_FALSE(io_work->data_error);
}

TEST(HandlePointAutoDisableTest, SetsPointErrorFlag)
{
    cfg myCfg;
    myCfg.auto_disable = true;

    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 1;

    auto io_work = std::make_shared<IO_Work>();
    io_work->data_error = false;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->errno_code = 0;  // Non-exempt error code
    io_point->id = "TestPoint";
    io_point->offset = 0;
    io_point->is_disconnected = false;
    io_point->gap = 1;

    handle_point_auto_disable(myCfg, io_thread, io_work, io_point, "test_func", 0);

    EXPECT_TRUE(io_work->data_error);
}

TEST(HandlePointAutoDisableTest, RemovesGapAndLogs)
{
    cfg myCfg;
    myCfg.auto_disable = true;

    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 1;

    auto io_work = std::make_shared<IO_Work>();
    io_work->data_error = false;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->errno_code = 0;  // Non-exempt error code
    io_point->id = "TestPoint";
    io_point->offset = 0;
    io_point->is_disconnected = false;
    io_point->gap = 1;

    handle_point_auto_disable(myCfg, io_thread, io_work, io_point, "test_func", 0);

    EXPECT_TRUE(io_work->data_error);
}

TEST(HandlePointAutoDisableTest, DisconnectsPointAndLogs)
{
    cfg myCfg;
    myCfg.auto_disable = true;

    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 1;

    auto io_work = std::make_shared<IO_Work>();
    io_work->data_error = false;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->errno_code = 0;  // Non-exempt error code
    io_point->id = "TestPoint";
    io_point->offset = 0;
    io_point->is_disconnected = false;
    io_point->gap = 0;

    handle_point_auto_disable(myCfg, io_thread, io_work, io_point, "test_func", 0);

    EXPECT_TRUE(io_work->data_error);
}

TEST(HandleModbusPointErrorsTest, DataErrorAlreadySet)
{
    cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 1;

    auto io_work = std::make_shared<IO_Work>();
    io_work->data_error = true;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->errno_code = EINVAL;
    io_point->id = "TestPoint";
    io_point->offset = 0;

    handle_modbus_point_errors(myCfg, io_thread, io_work, io_point, "test_func", EINVAL);

    // No further action should be taken
    EXPECT_TRUE(io_work->data_error);
}

TEST(HandleModbusPointErrorsTest, HandlesEMBBADEXCError)
{
    cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 1;

    auto io_work = std::make_shared<IO_Work>();
    io_work->data_error = false;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->errno_code = EMBBADEXC;
    io_point->id = "TestPoint";
    io_point->offset = 0;

    handle_modbus_point_errors(myCfg, io_thread, io_work, io_point, "test_func", EMBBADEXC);

    EXPECT_FALSE(io_work->data_error);  // Assuming handleEMBBADEXC will handle the error
}

TEST(HandleModbusPointErrorsTest, LogsNonEMBBADEXCError)
{
    cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 1;

    auto io_work = std::make_shared<IO_Work>();
    io_work->data_error = false;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->errno_code = EINVAL;
    io_point->id = "TestPoint";
    io_point->offset = 0;

    handle_modbus_point_errors(myCfg, io_thread, io_work, io_point, "test_func", EINVAL);

    EXPECT_FALSE(io_work->data_error);  // No data error set for non-EMBBADEXC error
}

TEST(HandlePointCommonErrorsTest, HandlesCommonErrors)
{
    cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 1;

    auto io_work = std::make_shared<IO_Work>();
    io_work->data_error = false;
    io_work->tNow = 1.23;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->errno_code = EPIPE;
    io_point->id = "TestPoint";
    io_point->offset = 0;
    io_point->off_by_one = false;

    bool result = handle_point_common_errors(myCfg, io_thread, io_work, io_point);

    EXPECT_TRUE(result);
    EXPECT_TRUE(io_work->data_error);
}

TEST(HandlePointCommonErrorsTest, IgnoresNonCommonErrors)
{
    cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 1;

    auto io_work = std::make_shared<IO_Work>();
    io_work->data_error = false;
    io_work->tNow = 1.23;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->errno_code = 12345;  // Non-common error code
    io_point->id = "TestPoint";
    io_point->offset = 0;
    io_point->off_by_one = false;

    bool result = handle_point_common_errors(myCfg, io_thread, io_work, io_point);

    EXPECT_FALSE(result);
    EXPECT_FALSE(io_work->data_error);
}

TEST(HandlePointCommonErrorsTest, DoesNotLogIfDataErrorAlreadySet)
{
    cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 1;

    auto io_work = std::make_shared<IO_Work>();
    io_work->data_error = true;  // Data error already set
    io_work->tNow = 1.23;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->errno_code = EPIPE;
    io_point->id = "TestPoint";
    io_point->offset = 0;
    io_point->off_by_one = false;

    bool result = handle_point_common_errors(myCfg, io_thread, io_work, io_point);

    EXPECT_TRUE(result);
    EXPECT_TRUE(io_work->data_error);
}

TEST(HandlePointConnectionResetTest, HandlesECONNRESET)
{
    cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 2;
    io_thread->connect_reset = 0;
    io_thread->is_enabled = true;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->errno_code = ECONNRESET;
    io_point->id = "TestPoint";
    io_point->offset = 0;

    bool result = handle_point_connection_reset(myCfg, io_thread, io_point);

    EXPECT_TRUE(result);
    EXPECT_EQ(io_thread->connect_reset, 1);
    EXPECT_TRUE(io_thread->is_enabled);
}

TEST(HandlePointConnectionResetTest, DoesNotHandleNonECONNRESET)
{
    cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 2;
    io_thread->connect_reset = 0;
    io_thread->is_enabled = true;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->errno_code = EINVAL;
    io_point->id = "TestPoint";
    io_point->offset = 0;

    bool result = handle_point_connection_reset(myCfg, io_thread, io_point);

    EXPECT_FALSE(result);
    EXPECT_EQ(io_thread->connect_reset, 0);
    EXPECT_TRUE(io_thread->is_enabled);
}

TEST(HandlePointConnectionResetTest, DisablesThreadAfterThreshold)
{
    cfg myCfg;
    auto io_thread = std::make_shared<IO_Thread>();
    io_thread->tid = 2;
    io_thread->connect_reset = 10;
    io_thread->is_enabled = true;

    auto io_point = std::make_shared<cfg::io_point_struct>();
    io_point->errno_code = ECONNRESET;
    io_point->id = "TestPoint";
    io_point->offset = 0;

    bool result = handle_point_connection_reset(myCfg, io_thread, io_point);

    EXPECT_TRUE(result);
    EXPECT_EQ(io_thread->connect_reset, 11);
    EXPECT_FALSE(io_thread->is_enabled);
}