#include "doctest/doctest.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <cstdlib>

#include "../../include/gcom_dnp3_system_structs.h"
#include "../../include/gcom_dnp3_utils.h"

TEST_CASE("Test addBits with empty bits array") {
    printf("Testing dnp3_utils.cpp...\n");
    GcomSystem sys;
    TMWSIM_POINT dbPoint;
    FlexPoint flexpoint(nullptr, "test_point", "/components/test");
    dbPoint.flexPointHandle = &flexpoint;
    cJSON* bits = cJSON_CreateArray(); // Create an empty bits array
    int result = addBits(sys, &dbPoint, bits);
    CHECK_EQ(result, 0); // Expect 0 bits added
    CHECK_EQ(flexpoint.dbBits.size(), 0); // Expect dbBits to remain empty
    cJSON_Delete(bits); // Clean up cJSON object
}

TEST_CASE("Test addBits with non-empty bits array") {
    GcomSystem sys;
    TMWSIM_POINT dbPoint;
    FlexPoint flexpoint(nullptr, "test_point", "/components/test");
    dbPoint.flexPointHandle = &flexpoint;
    cJSON* bits = cJSON_CreateArray();
    cJSON_AddItemToArray(bits, cJSON_CreateString("Bit0"));
    cJSON_AddItemToArray(bits, cJSON_CreateString("Bit1"));
    cJSON_AddItemToArray(bits, cJSON_CreateString("Bit2"));
    int result = addBits(sys, &dbPoint, bits);
    CHECK_EQ(result, 3); // Expect all bits to be added
    CHECK_EQ(flexpoint.dbBits.size(), 3); // Expect dbBits to have 3 elements
    CHECK_EQ(flexpoint.dbBits[0].first, "Bit0");
    CHECK_EQ(flexpoint.dbBits[1].first, "Bit1");
    CHECK_EQ(flexpoint.dbBits[2].first, "Bit2");
    cJSON_Delete(bits); // Clean up cJSON object
    }

    TEST_CASE("Test addBits with IGNORE bitstring") {
        GcomSystem sys;
        TMWSIM_POINT dbPoint;
        FlexPoint flexpoint(nullptr, "test_point", "/components/test");
        dbPoint.flexPointHandle = &flexpoint;
        cJSON* bits = cJSON_CreateArray();
        cJSON_AddItemToArray(bits, cJSON_CreateString("Bit0"));
        cJSON_AddItemToArray(bits, cJSON_CreateString("IGNORE"));
        cJSON_AddItemToArray(bits, cJSON_CreateString("Bit2"));
        int result = addBits(sys, &dbPoint, bits);
        CHECK_EQ(result, 3); // Expect all bits to be added
        CHECK_EQ(flexpoint.dbBits.size(), 3); // Expect dbBits to have 3 elements
        // Ensure IGNORE bitstring is replaced with "Unknown"
        CHECK_EQ(flexpoint.dbBits[1].first, "Unknown");
        cJSON_Delete(bits); // Clean up cJSON object
    }

    TEST_CASE("Test addEnum with empty bits array") {
    GcomSystem sys;
    TMWSIM_POINT dbPoint;
    FlexPoint flexpoint(nullptr, "test_point", "/components/test");
    dbPoint.flexPointHandle = &flexpoint;
    cJSON* bits = cJSON_CreateArray(); // Create an empty bits array
    int result = addEnum(sys, &dbPoint, bits);
    CHECK_EQ(result, 0); // Expect 0 bits added
    CHECK_EQ(flexpoint.dbBits.size(), 0); // Expect dbBits to remain empty
    cJSON_Delete(bits); // Clean up cJSON object
}

TEST_CASE("Test addEnum with non-empty bits array") {
    GcomSystem sys;
    TMWSIM_POINT dbPoint;
    FlexPoint flexpoint(nullptr, "test_point", "/components/test");
    dbPoint.flexPointHandle = &flexpoint;
    cJSON* bits = cJSON_CreateArray();
    cJSON_AddItemToArray(bits, cJSON_CreateString("Bit0"));
    cJSON* bit2 = cJSON_CreateObject();
    cJSON_AddItemToObject(bit2, "value", cJSON_CreateNumber(5));
    cJSON_AddItemToObject(bit2, "string", cJSON_CreateString("Bit5"));
    cJSON_AddItemToArray(bits, bit2);
    cJSON_AddItemToArray(bits, cJSON_CreateString("Bit2"));
    int result = addEnum(sys, &dbPoint, bits);
    CHECK_EQ(result, 3); // Expect all bits to be added
    CHECK_EQ(flexpoint.dbBits.size(), 3); // Expect dbBits to have 3 elements
    // Ensure IGNORE bitstring is replaced with "Unknown"
    CHECK_EQ(flexpoint.dbBits[0].first, "Bit0");
    CHECK_EQ(flexpoint.dbBits[0].second, 0);
    CHECK_EQ(flexpoint.dbBits[1].first, "Bit5");
    CHECK_EQ(flexpoint.dbBits[1].second, 5);
    CHECK_EQ(flexpoint.dbBits[2].first, "Bit2");
    CHECK_EQ(flexpoint.dbBits[2].second, 6);
    cJSON_Delete(bits); // Clean up cJSON object
}

TEST_CASE("Test addEnum with bit objects containing value and string fields") {
    GcomSystem sys;
    TMWSIM_POINT dbPoint;
    FlexPoint flexpoint(nullptr, "test_point", "/components/test");
    dbPoint.flexPointHandle = &flexpoint;
    cJSON* bits = cJSON_CreateArray();
    cJSON* bit1 = cJSON_CreateObject();
    cJSON_AddItemToObject(bit1, "value", cJSON_CreateNumber(0));
    cJSON_AddItemToObject(bit1, "string", cJSON_CreateString("Bit0"));
    cJSON_AddItemToArray(bits, bit1);
    cJSON* bit2 = cJSON_CreateObject();
    cJSON_AddItemToObject(bit2, "value", cJSON_CreateNumber(1));
    cJSON_AddItemToObject(bit2, "string", cJSON_CreateString("Bit1"));
    cJSON_AddItemToArray(bits, bit2);
    cJSON* bit3 = cJSON_CreateObject();
    cJSON_AddItemToObject(bit3, "value", cJSON_CreateNumber(55));
    cJSON_AddItemToObject(bit3, "string", cJSON_CreateString("Bit55"));
    cJSON_AddItemToArray(bits, bit3);
    int result = addEnum(sys, &dbPoint, bits);
    CHECK_EQ(result, 3); // Expect both bits to be added
    CHECK_EQ(flexpoint.dbBits.size(), 3); // Expect dbBits to have 3 elements
    // Ensure bits are added correctly
    CHECK_EQ(flexpoint.dbBits[0].first, "Bit0");
    CHECK_EQ(flexpoint.dbBits[0].second, 0);
    CHECK_EQ(flexpoint.dbBits[1].first, "Bit1");
    CHECK_EQ(flexpoint.dbBits[1].second, 1);
    CHECK_EQ(flexpoint.dbBits[2].first, "Bit55");
    CHECK_EQ(flexpoint.dbBits[2].second, 55);
    cJSON_Delete(bits); // Clean up cJSON object
}

TEST_CASE("Test addEnum with IGNORE bitstring") {
    GcomSystem sys;
    TMWSIM_POINT dbPoint;
    FlexPoint flexpoint(nullptr, "test_point", "/components/test");
    dbPoint.flexPointHandle = &flexpoint;
    cJSON* bits = cJSON_CreateArray();
    cJSON_AddItemToArray(bits, cJSON_CreateString("Bit0"));
    cJSON_AddItemToArray(bits, cJSON_CreateString("IGNORE"));
    cJSON_AddItemToArray(bits, cJSON_CreateString("Bit2"));
    int result = addEnum(sys, &dbPoint, bits);
    CHECK_EQ(result, 3); // Expect all bits to be added
    CHECK_EQ(flexpoint.dbBits.size(), 3); // Expect dbBits to have 3 elements
    // Ensure IGNORE bitstring is replaced with "Unknown"
    CHECK_EQ(flexpoint.dbBits[1].first, "Unknown");
    cJSON_Delete(bits); // Clean up cJSON object
}