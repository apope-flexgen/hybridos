#include "doctest/doctest.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <cstdlib>

#include "../../include/gcom_dnp3_system_structs.h"
#include "../../include/gcom_dnp3_flags.h"
#include "../../include/gcom_dnp3_point_utils.h"

// TODO: Test extremes of each type
TEST_SUITE("dnp3_point_utils")
{
    TEST_CASE("format_point_value - analog inputs - Group30Var1")
    {
        printf("Testing dnp3_point_utils.cpp...\n");
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.defaultStaticVariation = Group30Var1;
        dbPoint.flexPointHandle = &flexpoint;
        flexpoint.type = Register_Types::Analog;
        flexpoint.scale = 0.0;
        double value = 42.0;

        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        std::string result = fmt::to_string(send_buf);
        CHECK_EQ(result, "42");

        value = 43.6;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "43");

        value = 42.0;
        flexpoint.scale = 1.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "42");

        value = 43.6;
        flexpoint.scale = 1.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "43");

        value = 42.0;
        flexpoint.scale = 5.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "8.4");

        value = 42.0;
        flexpoint.scale = 6;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "7");
    }
    TEST_CASE("format_point_value - analog inputs - Group30Var2")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.defaultStaticVariation = Group30Var2;
        dbPoint.flexPointHandle = &flexpoint;
        flexpoint.type = Register_Types::Analog;
        flexpoint.scale = 0.0;
        double value = 42.0;

        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        std::string result = fmt::to_string(send_buf);
        CHECK_EQ(result, "42");

        value = 43.6;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "43");

        value = 42.0;
        flexpoint.scale = 1.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "42");

        value = 43.6;
        flexpoint.scale = 1.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "43");

        value = 42.0;
        flexpoint.scale = 5.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "8.4");

        value = 42.0;
        flexpoint.scale = 6;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "7");
    }
    TEST_CASE("format_point_value - analog inputs - Group30Var5")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.defaultStaticVariation = Group30Var5;
        dbPoint.flexPointHandle = &flexpoint;
        flexpoint.type = Register_Types::Analog;
        flexpoint.scale = 0.0;
        double value = 42.0;

        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        std::string result = fmt::to_string(send_buf);
        CHECK_EQ(result, "42");

        value = 43.6;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "43.59999847412109");

        value = 42.0;
        flexpoint.scale = 1.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "42");

        value = 43.6;
        flexpoint.scale = 1.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "43.59999847412109");

        value = 42.0;
        flexpoint.scale = 5.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "8.4");

        value = 42.0;
        flexpoint.scale = 6;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "7");
    }
    TEST_CASE("format_point_value - analog inputs - Group30Var6")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.defaultStaticVariation = Group30Var6;
        dbPoint.flexPointHandle = &flexpoint;
        flexpoint.type = Register_Types::Analog;
        flexpoint.scale = 0.0;
        double value = 42.0;

        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        std::string result = fmt::to_string(send_buf);
        CHECK_EQ(result, "42");

        value = 43.6;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "43.6");

        value = 42.0;
        flexpoint.scale = 1.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "42");

        value = 43.6;
        flexpoint.scale = 1.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "43.6");

        value = 42.0;
        flexpoint.scale = 5.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "8.4");

        value = 42.0;
        flexpoint.scale = 6;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "7");
    }
    TEST_CASE("format_point_value - counters - Group20Var1")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.defaultStaticVariation = Group20Var1;
        dbPoint.flexPointHandle = &flexpoint;
        flexpoint.type = Register_Types::Counter;
        flexpoint.scale = 0.0;
        double value = 42.0;

        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        std::string result = fmt::to_string(send_buf);
        CHECK_EQ(result, "42");

        value = 43.6;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "43");

        value = 42.0;
        flexpoint.scale = 1.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "42");

        value = 43.6;
        flexpoint.scale = 1.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "43");

        value = 42.0;
        flexpoint.scale = 5.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "8.4");

        value = 42.0;
        flexpoint.scale = 6;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "7");
    }
    TEST_CASE("format_point_value - counters - Group20Var2")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.defaultStaticVariation = Group20Var1;
        dbPoint.flexPointHandle = &flexpoint;
        flexpoint.type = Register_Types::Counter;
        flexpoint.scale = 0.0;
        double value = 42.0;

        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        std::string result = fmt::to_string(send_buf);
        CHECK_EQ(result, "42");

        value = 43.6;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "43");

        value = 42.0;
        flexpoint.scale = 1.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "42");

        value = 43.6;
        flexpoint.scale = 1.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "43");

        value = 42.0;
        flexpoint.scale = 5.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "8.4");

        value = 42.0;
        flexpoint.scale = 6;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "7");
    }
    TEST_CASE("format_point_value - analog outputs - Group40Var1")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.defaultStaticVariation = Group40Var1;
        dbPoint.flexPointHandle = &flexpoint;
        flexpoint.type = Register_Types::AnalogOS;
        flexpoint.scale = 0.0;
        double value = 42.0;

        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        std::string result = fmt::to_string(send_buf);
        CHECK_EQ(result, "42");

        value = 43.6;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "43");

        value = 42.0;
        flexpoint.scale = 1.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "42");

        value = 43.6;
        flexpoint.scale = 1.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "43");

        value = 42.0;
        flexpoint.scale = 5.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "8.4");

        value = 42.0;
        flexpoint.scale = 6;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "7");
    }
    TEST_CASE("format_point_value - analog outputs - Group40Var2")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.defaultStaticVariation = Group40Var2;
        dbPoint.flexPointHandle = &flexpoint;
        flexpoint.type = Register_Types::AnalogOS;
        flexpoint.scale = 0.0;
        double value = 42.0;

        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        std::string result = fmt::to_string(send_buf);
        CHECK_EQ(result, "42");

        value = 43.6;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "43");

        value = 42.0;
        flexpoint.scale = 1.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "42");

        value = 43.6;
        flexpoint.scale = 1.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "43");

        value = 42.0;
        flexpoint.scale = 5.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "8.4");

        value = 42.0;
        flexpoint.scale = 6;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "7");
    }
    TEST_CASE("format_point_value - analog outputs - Group40Var3")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.defaultStaticVariation = Group40Var3;
        dbPoint.flexPointHandle = &flexpoint;
        flexpoint.type = Register_Types::AnalogOS;
        flexpoint.scale = 0.0;
        double value = 42.0;

        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        std::string result = fmt::to_string(send_buf);
        CHECK_EQ(result, "42");

        value = 43.6;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "43.59999847412109");

        value = 42.0;
        flexpoint.scale = 1.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "42");

        value = 43.6;
        flexpoint.scale = 1.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "43.59999847412109");

        value = 42.0;
        flexpoint.scale = 5.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "8.4");

        value = 42.0;
        flexpoint.scale = 6;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "7");
    }
    TEST_CASE("format_point_value - analog outputs - Group40Var4")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.defaultStaticVariation = Group40Var4;
        dbPoint.flexPointHandle = &flexpoint;
        flexpoint.type = Register_Types::AnalogOS;
        flexpoint.scale = 0.0;
        double value = 42.0;

        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        std::string result = fmt::to_string(send_buf);
        CHECK_EQ(result, "42");

        value = 43.6;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "43.6");

        value = 42.0;
        flexpoint.scale = 1.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "42");

        value = 43.6;
        flexpoint.scale = 1.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "43.6");

        value = 42.0;
        flexpoint.scale = 5.0;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "8.4");

        value = 42.0;
        flexpoint.scale = 6;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "7");
    }
    TEST_CASE("format_point_value - binary points")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.defaultStaticVariation = Group40Var4;
        dbPoint.flexPointHandle = &flexpoint;
        flexpoint.type = Register_Types::BinaryOS;
        flexpoint.scale = 0.0;
        flexpoint.crob_string = true;
        double value = 42.0;

        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        std::string result = fmt::to_string(send_buf);
        CHECK_EQ(result, "\"LATCH_ON\"");  // Expected result when scale is negative and crob_string is true

        flexpoint.crob_string = false;
        flexpoint.crob_int = true;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "1");

        flexpoint.crob_string = false;
        flexpoint.crob_int = false;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "true");

        value = 0;
        flexpoint.crob_string = true;
        flexpoint.crob_int = false;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "\"LATCH_OFF\"");

        flexpoint.crob_string = false;
        flexpoint.crob_int = true;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "0");

        flexpoint.crob_string = false;
        flexpoint.crob_int = false;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "false");

        flexpoint.scale = -1;
        value = 42.0;
        flexpoint.crob_string = true;
        flexpoint.crob_int = false;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "\"LATCH_OFF\"");  // Expected result when scale is negative and crob_string is true

        flexpoint.crob_string = false;
        flexpoint.crob_int = true;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "0");

        flexpoint.crob_string = false;
        flexpoint.crob_int = false;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "false");

        value = 0;
        flexpoint.crob_string = true;
        flexpoint.crob_int = false;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "\"LATCH_ON\"");

        flexpoint.crob_string = false;
        flexpoint.crob_int = true;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "1");

        flexpoint.crob_string = false;
        flexpoint.crob_int = false;
        send_buf.clear();
        format_point_value(send_buf, &dbPoint, value);
        result = fmt::to_string(send_buf);
        CHECK_EQ(result, "true");
    }
    TEST_CASE("format_bitfield")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.flexPointHandle = &flexpoint;
        send_buf.clear();

        // Example dbBits setup
        flexpoint.dbBits = { { "Bit0", 0 }, { "Bit1", 1 }, { "Unknown", 2 }, { "Bit3", 3 }, { "Bit4", 4 } };

        double value = 24;

        format_bitfield(send_buf, &dbPoint, value);

        std::string result = fmt::to_string(send_buf);
        CHECK_EQ(result, R"([{"value":3,"string":"Bit3"},{"value":4,"string":"Bit4"}])");
    }

    TEST_CASE("Test format_individual_bit with Unknown bit")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.flexPointHandle = &flexpoint;
        send_buf.clear();

        // Set up dbBits
        flexpoint.dbBits = { { "Bit0", 0 }, { "Bit1", 1 }, { "Unknown", 2 }, { "Bit3", 3 }, { "Bit4", 4 } };

        // Set test value
        double value = 20;

        // Call function with Unknown bit
        format_individual_bit(send_buf, &dbPoint, value, DNPDEFS_DBAS_FLAG_ON_LINE, nullptr, "Unknown");

        // Expect no output
        CHECK_EQ(fmt::to_string(send_buf), "");
    }

    TEST_CASE("Test format_individual_bit with Naked format")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.flexPointHandle = &flexpoint;
        send_buf.clear();

        // Set up dbBits
        flexpoint.dbBits = { { "Bit0", 0 }, { "Bit1", 1 }, { "Bit2", 2 }, { "Bit3", 3 }, { "Bit4", 4 } };

        // Set test value
        double value = 20;

        // Set format to Naked
        flexpoint.format = FimsFormat::Naked;

        // Call function with a known bit
        format_individual_bit(send_buf, &dbPoint, value, DNPDEFS_DBAS_FLAG_ON_LINE, nullptr, "Bit2");

        // Expect "true" since Bit2 is set in the value
        CHECK_EQ(fmt::to_string(send_buf), "true");
    }

    TEST_CASE("Test format_individual_bit with Clothed format")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.flexPointHandle = &flexpoint;
        send_buf.clear();

        // Set up dbBits
        flexpoint.dbBits = { { "Bit0", 0 }, { "Bit1", 1 }, { "Bit2", 2 }, { "Bit3", 3 }, { "Bit4", 4 } };

        // Set test value
        double value = 20;

        // Set format to Clothed
        flexpoint.format = FimsFormat::Clothed;

        // Call function with a known bit
        format_individual_bit(send_buf, &dbPoint, value, DNPDEFS_DBAS_FLAG_ON_LINE, nullptr, "Bit3");

        // Expect {"value":false}
        CHECK_EQ(fmt::to_string(send_buf), R"({"value":false})");
    }

    TEST_CASE("Test format_individual_bit with Full format")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.flexPointHandle = &flexpoint;
        send_buf.clear();

        // Set up dbBits
        flexpoint.dbBits = { { "Bit0", 0 }, { "Bit1", 1 }, { "Bit2", 2 }, { "Bit3", 3 }, { "Bit4", 4 } };

        // Set test value
        double value = 20;

        flexpoint.format = FimsFormat::Full;

        // Call function with a known bit
        format_individual_bit(send_buf, &dbPoint, value, DNPDEFS_DBAS_FLAG_ON_LINE, nullptr, "Bit4");

        // Expect {"value":false,"flags":1,"timestamp":null}
        CHECK_EQ(fmt::to_string(send_buf), R"({"value":true,"flags":["ONLINE"],"timestamp":"null"})");
    }
    TEST_CASE("Test format_individual_bits with all bits Unknown")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.flexPointHandle = &flexpoint;
        send_buf.clear();

        // Set up dbBits all as Unknown
        flexpoint.dbBits = { { "Unknown", 0 }, { "Unknown", 1 }, { "Unknown", 2 }, { "Unknown", 3 }, { "Unknown", 4 } };

        // Set test value
        double value = 20;

        // Call function
        format_individual_bits(send_buf, &dbPoint, value, DNPDEFS_DBAS_FLAG_ON_LINE, nullptr);

        // Expect no output
        CHECK_EQ(fmt::to_string(send_buf), "");
    }

    TEST_CASE("Test format_individual_bits with Naked format")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.flexPointHandle = &flexpoint;
        send_buf.clear();

        // Set up dbBits
        flexpoint.dbBits = { { "Bit0", 0 }, { "Bit1", 1 }, { "Bit2", 2 }, { "Bit3", 3 }, { "Bit4", 4 } };

        // Set test value
        double value = 20;

        // Set format to Naked
        flexpoint.format = FimsFormat::Naked;

        // Call function
        format_individual_bits(send_buf, &dbPoint, value, DNPDEFS_DBAS_FLAG_ON_LINE, nullptr);

        CHECK_EQ(fmt::to_string(send_buf), R"("Bit0":false,"Bit1":false,"Bit2":true,"Bit3":false,"Bit4":true)");
    }

    TEST_CASE("Test format_individual_bits with Clothed format")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.flexPointHandle = &flexpoint;
        send_buf.clear();

        // Set up dbBits
        flexpoint.dbBits = { { "Bit0", 0 }, { "Bit1", 1 }, { "Bit2", 2 }, { "Bit3", 3 }, { "Bit4", 4 } };

        // Set test value
        double value = 20;

        // Set format to Clothed
        flexpoint.format = FimsFormat::Clothed;

        // Call function
        format_individual_bits(send_buf, &dbPoint, value, DNPDEFS_DBAS_FLAG_ON_LINE, nullptr);

        CHECK_EQ(
            fmt::to_string(send_buf),
            R"("Bit0":{"value":false},"Bit1":{"value":false},"Bit2":{"value":true},"Bit3":{"value":false},"Bit4":{"value":true})");
    }

    TEST_CASE("Test format_individual_bits with Full format")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.flexPointHandle = &flexpoint;
        send_buf.clear();

        // Set up dbBits
        flexpoint.dbBits = { { "Bit0", 0 }, { "Bit1", 1 }, { "Bit2", 2 }, { "Bit3", 3 }, { "Bit4", 4 } };

        // Set test value
        double value = 20;

        // Set format to Full
        flexpoint.format = FimsFormat::Full;

        // Call function
        format_individual_bits(send_buf, &dbPoint, value, DNPDEFS_DBAS_FLAG_ON_LINE, nullptr);

        CHECK_EQ(
            fmt::to_string(send_buf),
            R"("Bit0":{"value":false,"flags":["ONLINE"],"timestamp":"null"},"Bit1":{"value":false,"flags":["ONLINE"],"timestamp":"null"},"Bit2":{"value":true,"flags":["ONLINE"],"timestamp":"null"},"Bit3":{"value":false,"flags":["ONLINE"],"timestamp":"null"},"Bit4":{"value":true,"flags":["ONLINE"],"timestamp":"null"})");
    }
    TEST_CASE("Test format_timestamp with non-null pointer")
    {
        fmt::memory_buffer send_buf;
        TMWDTIME tmp;
        tmp.year = 2024;
        tmp.month = 3;
        tmp.dayOfMonth = 15;
        tmp.hour = 10;
        tmp.minutes = 30;
        tmp.mSecsAndSecs = 55000;  // 55 seconds
        TMWDTIME* tmpPtr = &tmp;
        send_buf.clear();

        // Call function with non-null pointer
        format_timestamp(send_buf, tmpPtr);

        // Expect formatted timestamp
        CHECK_EQ(fmt::to_string(send_buf), R"("2024-03-15 10:30:55.000")");
    }

    TEST_CASE("Test format_timestamp with null pointer")
    {
        fmt::memory_buffer send_buf;
        TMWDTIME* tmpPtr = nullptr;
        send_buf.clear();

        // Call function with null pointer
        format_timestamp(send_buf, tmpPtr);

        // Expect "null"
        CHECK_EQ(fmt::to_string(send_buf), R"("null")");
    }

    TEST_CASE("Test format_enum with value within range")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.flexPointHandle = &flexpoint;
        send_buf.clear();

        // Set up dbBits
        flexpoint.dbBits = { { "Option0", 0 }, { "Option1", 1 }, { "Option2", 2 }, { "Option3", 3 }, { "Option4", 4 } };

        // Set test value within range
        double value = 1;

        // Call function
        format_enum(send_buf, &dbPoint, value);

        // Expect formatted enum string for Option1
        CHECK_EQ(fmt::to_string(send_buf), R"([{"value":1,"string":"Option1"}])");
    }

    TEST_CASE("Test format_enum with value out of range")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.flexPointHandle = &flexpoint;
        send_buf.clear();

        // Set up dbBits
        flexpoint.dbBits = { { "Option0", 0 }, { "Option1", 1 }, { "Option2", 2 }, { "Option3", 5 }, { "Option4", 7 } };

        // Set test value out of range
        double value = 3;

        // Call function
        format_enum(send_buf, &dbPoint, value);

        // Expect "Unknown" string for out-of-range value
        CHECK_EQ(fmt::to_string(send_buf), R"([{"value":3,"string":"Unknown"}])");
    }

    TEST_CASE("Test format_enum with empty dbBits")
    {
        fmt::memory_buffer send_buf;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.flexPointHandle = &flexpoint;
        send_buf.clear();

        // Set up empty dbBits
        flexpoint.dbBits = {};

        // Set test value
        double value = 0;

        // Call function
        format_enum(send_buf, &dbPoint, value);

        // Expect "Unknown" string for empty dbBits
        CHECK_EQ(fmt::to_string(send_buf), R"([{"value":0,"string":"Unknown"}])");
    }
}