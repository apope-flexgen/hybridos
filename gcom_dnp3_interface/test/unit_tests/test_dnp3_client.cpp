#include "doctest/doctest.h"
#include "add_points.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <cstdlib>

#include "../../include/gcom_dnp3_system_structs.h"
#include "../../include/gcom_dnp3_flags.h"
#include "../../include/gcom_dnp3_utils.h"
#include "../../include/gcom_dnp3_tmw_utils.h"
#include "../../include/gcom_dnp3_stats.h"
#include "fims/defer.hpp"

#include "../../include/gcom_dnp3_client.h"

bool first_test_dnp3_client = true;

std::vector<TMWSIM_POINT*> setupParseBodyClientTest(GcomSystem& sys, bool same_uri = false)
{
    if (first_test_dnp3_client)
    {
        printf("Testing dnp3_client.cpp...\n");
        first_test_dnp3_client = false;
    }
    sys.protocol_dependencies->who = DNP3_MASTER;
    sys.protocol_dependencies->ip_address = strdup("0.0.0.0");
    sys.id = strdup("test_dnp3_client");
    sys.protocol_dependencies->port = 20000;
    sys.protocol_dependencies->dnp3.point_status_info = new PointStatusInfo();
    sys.protocol_dependencies->dnp3.openTMWChannel = openTMWClientChannel;
    sys.protocol_dependencies->dnp3.openTMWSession = openTMWClientSession;
    init_tmw(sys);
    initStatsMonitor(sys);
    std::vector<TMWSIM_POINT*> points;
    if (same_uri)
    {
        points = addPointsSameUri(sys);
    }
    else
    {
        points = addPoints(sys);
    }
    sys.fims_dependencies->fims_gateway.Connect("dnp3_test");

    DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
    TMWTYPES_UINT num_analog_outputs = tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs);
    TMWTYPES_UINT num_binary_outputs = tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs);
    dnp3_sys->analogOutputVar1Values = new MDNPBRM_ANALOG_INFO[num_analog_outputs];
    dnp3_sys->analogOutputVar2Values = new MDNPBRM_ANALOG_INFO[num_analog_outputs];
    dnp3_sys->analogOutputVar3Values = new MDNPBRM_ANALOG_INFO[num_analog_outputs];
    dnp3_sys->analogOutputVar4Values = new MDNPBRM_ANALOG_INFO[num_analog_outputs];
    dnp3_sys->CROBInfo = new MDNPBRM_CROB_INFO[num_binary_outputs];
    return points;
}

void checkRequestCounts(GcomSystem& sys, int a, int b, int c, int d)
{
    CHECK(sys.protocol_dependencies->dnp3.count_var1_requests == a);
    CHECK(sys.protocol_dependencies->dnp3.count_var2_requests == b);
    CHECK(sys.protocol_dependencies->dnp3.count_var3_requests == c);
    CHECK(sys.protocol_dependencies->dnp3.count_var4_requests == d);
}

TEST_SUITE("dnp3_client")
{
    TEST_CASE("parseBody - analog inputs - multi")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "{\"%s\":%d}", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":%d}",
                        ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - counters - multi")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryCounters); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryCounterGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "{\"%s\":%d}", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":%d}",
                        ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - analog outputs - multi")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100 + 1;
                asprintf(&fims_message, "{\"%s\":%d}", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":%d}",
                        ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                MDNPBRM_ANALOG_INFO analogValue;
                if (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt32)
                {
                    analogValue = sys.protocol_dependencies->dnp3.analogOutputVar1Values[0];
                    checkRequestCounts(sys, 1, 0, 0, 0);
                }
                else if (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt16)
                {
                    analogValue = sys.protocol_dependencies->dnp3.analogOutputVar2Values[0];
                    checkRequestCounts(sys, 0, 1, 0, 0);
                }
                else if (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPF32)
                {
                    analogValue = sys.protocol_dependencies->dnp3.analogOutputVar3Values[0];
                    checkRequestCounts(sys, 0, 0, 1, 0);
                }
                CHECK(analogValue.pointNumber == dbPoint->pointNumber);
                CHECK(analogValue.value.value.dval == value * ((FlexPoint*)(dbPoint->flexPointHandle))->scale);
                CHECK(analogValue.value.value.dval != 0);  // value and scale should be non-zero, so this should be true
                CHECK(analogValue.value.type == TMWTYPES_ANALOG_TYPE_DOUBLE);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);

                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - binary inputs - multi")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "{\"%s\":%s}", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                         value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":%s}",
                        ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value ? "true" : "false");
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - binary outputs - multi")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "{\"%s\":%s}", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                         value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":%s}",
                        ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value ? "true" : "false");
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(dnp3_sys->CROBInfo[0].pointNumber == dbPoint->pointNumber);
                CHECK(dnp3_sys->CROBInfo[0].control ==
                      ((TMWTYPES_UCHAR)(value ? DNPDEFS_CROB_CTRL_LATCH_ON : DNPDEFS_CROB_CTRL_LATCH_OFF)));
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - test multi")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        sprintf(sys.fims_dependencies->data_buf, "{");
        std::string combinedJSON = "{";  // Initialize the combined JSON object
        bool isFirstPoint = true;        // To keep track of the first point
        int value = 55;
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                // Append a comma if it's not the first point
                if (!isFirstPoint)
                {
                    strcat(sys.fims_dependencies->data_buf, ",");
                    combinedJSON += ",";
                }
                isFirstPoint = false;
                char* pointJSON;
                asprintf(&pointJSON, "\"%s\":%d", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);

                strcat(sys.fims_dependencies->data_buf, pointJSON);  // Append to the data_buf
                combinedJSON += pointJSON;                           // Append to the combined JSON
                free(pointJSON);
            }
        }

        combinedJSON += "}";  // Close the combined JSON object
        strcat(sys.fims_dependencies->data_buf, "}");

        meta_data.data_len = combinedJSON.length();
        sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
        sys.fims_dependencies->process_name_view = std::string_view("some_process");
        sys.fims_dependencies->username_view = std::string_view("some_username");
        CHECK(parseBodyClient(sys, meta_data));
        int count1 = 0;
        int count2 = 0;
        int count3 = 0;
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                MDNPBRM_ANALOG_INFO analogValue;
                if (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt32)
                {
                    analogValue = sys.protocol_dependencies->dnp3.analogOutputVar1Values[count1];
                    count1++;
                }
                else if (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt16)
                {
                    analogValue = sys.protocol_dependencies->dnp3.analogOutputVar2Values[count2];
                    count2++;
                }
                else if (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPF32)
                {
                    analogValue = sys.protocol_dependencies->dnp3.analogOutputVar3Values[count3];
                    count3++;
                }
                CHECK(analogValue.pointNumber == dbPoint->pointNumber);
                CHECK(analogValue.value.value.dval == value * ((FlexPoint*)(dbPoint->flexPointHandle))->scale);
                CHECK(analogValue.value.value.dval != 0);  // value and scale should be non-zero, so this should be true
                CHECK(analogValue.value.type == TMWTYPES_ANALOG_TYPE_DOUBLE);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
            }
        }
        checkRequestCounts(sys, count1, count2, count3, 0);
        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - analog inputs - multi with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "{\"%s\":{\"value\":%d}}",
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":{\"value\":%d}}",
                        ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - counters - multi with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryCounters); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryCounterGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "{\"%s\":{\"value\":%d}}",
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":{\"value\":%d}}",
                        ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - analog outputs - multi with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100 + 1;
                asprintf(&fims_message, "{\"%s\":{\"value\":%d}}",
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":{\"value\":%d}}",
                        ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                MDNPBRM_ANALOG_INFO analogValue;
                if (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt32)
                {
                    analogValue = sys.protocol_dependencies->dnp3.analogOutputVar1Values[0];
                    checkRequestCounts(sys, 1, 0, 0, 0);
                }
                else if (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt16)
                {
                    analogValue = sys.protocol_dependencies->dnp3.analogOutputVar2Values[0];
                    checkRequestCounts(sys, 0, 1, 0, 0);
                }
                else if (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPF32)
                {
                    analogValue = sys.protocol_dependencies->dnp3.analogOutputVar3Values[0];
                    checkRequestCounts(sys, 0, 0, 1, 0);
                }
                CHECK(analogValue.pointNumber == dbPoint->pointNumber);
                CHECK(analogValue.value.value.dval == value * ((FlexPoint*)(dbPoint->flexPointHandle))->scale);
                CHECK(analogValue.value.value.dval != 0);  // value and scale should be non-zero, so this should be true
                CHECK(analogValue.value.type == TMWTYPES_ANALOG_TYPE_DOUBLE);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);

                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - binary inputs - multi with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "{\"%s\":{\"value\":%s}}",
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":{\"value\":%s}}",
                        ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value ? "true" : "false");
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - binary outputs - multi with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "{\"%s\":{\"value\":%s}}",
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":{\"value\":%s}}",
                        ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value ? "true" : "false");
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(dnp3_sys->CROBInfo[0].pointNumber == dbPoint->pointNumber);
                CHECK(dnp3_sys->CROBInfo[0].control ==
                      ((TMWTYPES_UCHAR)(value ? DNPDEFS_CROB_CTRL_LATCH_ON : DNPDEFS_CROB_CTRL_LATCH_OFF)));
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - test multi with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        sprintf(sys.fims_dependencies->data_buf, "{");
        std::string combinedJSON = "{";  // Initialize the combined JSON object
        bool isFirstPoint = true;        // To keep track of the first point
        int value = 55;
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                // Append a comma if it's not the first point
                if (!isFirstPoint)
                {
                    strcat(sys.fims_dependencies->data_buf, ",");
                    combinedJSON += ",";
                }
                isFirstPoint = false;
                char* pointJSON;
                asprintf(&pointJSON, "\"%s\":{\"value\":%d}", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                         value);

                strcat(sys.fims_dependencies->data_buf, pointJSON);  // Append to the data_buf
                combinedJSON += pointJSON;                           // Append to the combined JSON
                free(pointJSON);
            }
        }

        combinedJSON += "}";  // Close the combined JSON object
        strcat(sys.fims_dependencies->data_buf, "}");

        meta_data.data_len = combinedJSON.length();
        sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
        sys.fims_dependencies->process_name_view = std::string_view("some_process");
        sys.fims_dependencies->username_view = std::string_view("some_username");
        CHECK(parseBodyClient(sys, meta_data));
        int count1 = 0;
        int count2 = 0;
        int count3 = 0;
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                MDNPBRM_ANALOG_INFO analogValue;
                if (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt32)
                {
                    analogValue = sys.protocol_dependencies->dnp3.analogOutputVar1Values[count1];
                    count1++;
                }
                else if (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt16)
                {
                    analogValue = sys.protocol_dependencies->dnp3.analogOutputVar2Values[count2];
                    count2++;
                }
                else if (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPF32)
                {
                    analogValue = sys.protocol_dependencies->dnp3.analogOutputVar3Values[count3];
                    count3++;
                }
                CHECK(analogValue.pointNumber == dbPoint->pointNumber);
                CHECK(analogValue.value.value.dval == value * ((FlexPoint*)(dbPoint->flexPointHandle))->scale);
                CHECK(analogValue.value.value.dval != 0);  // value and scale should be non-zero, so this should be true
                CHECK(analogValue.value.type == TMWTYPES_ANALOG_TYPE_DOUBLE);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
            }
        }
        checkRequestCounts(sys, count1, count2, count3, 0);
        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - analog inputs - single")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        char* uri;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "%d", value);
                sprintf(sys.fims_dependencies->data_buf, "%d", value);
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - counters - single")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        char* uri;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryCounters); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryCounterGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "%d", value);
                sprintf(sys.fims_dependencies->data_buf, "%d", value);
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - analog outputs - single")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        char* uri;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100 + 1;
                asprintf(&fims_message, "%d", value);
                sprintf(sys.fims_dependencies->data_buf, "%d", value);
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(value));
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - binary inputs - single")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        char* uri;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "%s", value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "%s", value ? "true" : "false");
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - binary outputs - single")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        char* uri;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "%s", value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "%s", value ? "true" : "false");
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(value));
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - analog inputs - single with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        char* uri;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "{\"value\":%d}", value);
                sprintf(sys.fims_dependencies->data_buf, "{\"value\":%d}", value);
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - counters - single with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        char* uri;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryCounters); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryCounterGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "{\"value\":%d}", value);
                sprintf(sys.fims_dependencies->data_buf, "{\"value\":%d}", value);
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - analog outputs - single with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        char* uri;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100 + 1;
                asprintf(&fims_message, "{\"value\":%d}", value);
                sprintf(sys.fims_dependencies->data_buf, "{\"value\":%d}", value);
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(value));
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - binary inputs - single with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        char* uri;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "{\"value\":%s}", value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "{\"value\":%s}", value ? "true" : "false");
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - binary outputs - single with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        char* uri;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "{\"value\":%s}", value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "{\"value\":%s}", value ? "true" : "false");
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(value));
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - invalid uri")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        char* uri;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100 + 1;
                asprintf(&fims_message, "{\"value\":%s}", value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "{\"value\":%s}", value ? "true" : "false");
                asprintf(&uri, "/some/non_existent/uri%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(0));
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - invalid point")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        char* uri;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100 + 1;
                asprintf(&fims_message, "{\"value\":%d}", value);
                sprintf(sys.fims_dependencies->data_buf, "{\"value\":%d}", value);
                asprintf(&uri, "%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri);
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(0));
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - invalid json message")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);
        sys.fims_dependencies->data_buf[0] = '\0';

        std::string combinedJSON = "";  // Initialize the combined JSON object
        int value = 55;
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                char* pointJSON;
                asprintf(&pointJSON, "\"%s\":%d,", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);

                strcat(sys.fims_dependencies->data_buf, pointJSON);  // Append to the data_buf
                combinedJSON += pointJSON;                           // Append to the combined JSON
                free(pointJSON);
            }
        }

        meta_data.data_len = combinedJSON.length();
        sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
        CHECK(!parseBodyClient(sys, meta_data));
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
            }
        }
        checkRequestCounts(sys, 0, 0, 0, 0);
        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - interval set - multi")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT*> points = setupParseBodyClientTest(sys);
        setIntervalSet(points);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "{\"%s\":%d}", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":%d}",
                        ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                free(fims_message);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100 + 1;
                asprintf(&fims_message, "{\"%s\":%d}", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":%d}",
                        ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(value));
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                free(fims_message);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "{\"%s\":%s}", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                         value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":%s}",
                        ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value ? "true" : "false");
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                free(fims_message);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "{\"%s\":%s}", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                         value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":%s}",
                        ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value ? "true" : "false");
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(value));
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - interval set - multi multi")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT*> points = setupParseBodyClientTest(sys);
        setIntervalSet(points);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        sprintf(sys.fims_dependencies->data_buf, "{");
        std::string combinedJSON = "{";  // Initialize the combined JSON object
        bool isFirstPoint = true;        // To keep track of the first point
        int value = 55;
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                // Append a comma if it's not the first point
                if (!isFirstPoint)
                {
                    strcat(sys.fims_dependencies->data_buf, ",");
                    combinedJSON += ",";
                }
                isFirstPoint = false;
                char* pointJSON;
                asprintf(&pointJSON, "\"%s\":%d", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);

                strcat(sys.fims_dependencies->data_buf, pointJSON);  // Append to the data_buf
                combinedJSON += pointJSON;                           // Append to the combined JSON
                free(pointJSON);
            }
        }

        combinedJSON += "}";  // Close the combined JSON object
        strcat(sys.fims_dependencies->data_buf, "}");

        meta_data.data_len = combinedJSON.length();
        sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
        sys.fims_dependencies->process_name_view = std::string_view("some_process");
        sys.fims_dependencies->username_view = std::string_view("some_username");
        CHECK(parseBodyClient(sys, meta_data));
        checkRequestCounts(sys, 0, 0, 0, 0);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(value));
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
            }
        }
        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - batch set - multi")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT*> points = setupParseBodyClientTest(sys);
        setBatchSet(points);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "{\"%s\":%d}", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":%d}",
                        ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                free(fims_message);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100 + 1;
                asprintf(&fims_message, "{\"%s\":%d}", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":%d}",
                        ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(value));
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                free(fims_message);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "{\"%s\":%s}", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                         value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":%s}",
                        ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value ? "true" : "false");
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                free(fims_message);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "{\"%s\":%s}", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                         value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":%s}",
                        ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value ? "true" : "false");
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(value));
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - batch set - multi multi")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT*> points = setupParseBodyClientTest(sys);
        setBatchSet(points);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        sprintf(sys.fims_dependencies->data_buf, "{");
        std::string combinedJSON = "{";  // Initialize the combined JSON object
        bool isFirstPoint = true;        // To keep track of the first point
        int value = 55;
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                // Append a comma if it's not the first point
                if (!isFirstPoint)
                {
                    strcat(sys.fims_dependencies->data_buf, ",");
                    combinedJSON += ",";
                }
                isFirstPoint = false;
                char* pointJSON;
                asprintf(&pointJSON, "\"%s\":%d", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);

                strcat(sys.fims_dependencies->data_buf, pointJSON);  // Append to the data_buf
                combinedJSON += pointJSON;                           // Append to the combined JSON
                free(pointJSON);
            }
        }

        combinedJSON += "}";  // Close the combined JSON object
        strcat(sys.fims_dependencies->data_buf, "}");

        meta_data.data_len = combinedJSON.length();
        sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
        sys.fims_dependencies->process_name_view = std::string_view("some_process");
        sys.fims_dependencies->username_view = std::string_view("some_username");
        CHECK(parseBodyClient(sys, meta_data));
        checkRequestCounts(sys, 0, 0, 0, 0);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(value));
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
            }
        }
        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - interval set - single")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT*> points = setupParseBodyClientTest(sys);
        setIntervalSet(points);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        char* uri;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "%d", value);
                sprintf(sys.fims_dependencies->data_buf, "%d", value);
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                free(fims_message);
                free(uri);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100 + 1;
                asprintf(&fims_message, "%d", value);
                sprintf(sys.fims_dependencies->data_buf, "%d", value);
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(value));
                free(fims_message);
                free(uri);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "%s", value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "%s", value ? "true" : "false");
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                free(fims_message);
                free(uri);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "%s", value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "%s", value ? "true" : "false");
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(value));
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - batch set - single")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT*> points = setupParseBodyClientTest(sys);
        setBatchSet(points);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        char* uri;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "%d", value);
                sprintf(sys.fims_dependencies->data_buf, "%d", value);
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                free(fims_message);
                free(uri);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100 + 1;
                asprintf(&fims_message, "%d", value);
                sprintf(sys.fims_dependencies->data_buf, "%d", value);
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(value));
                free(fims_message);
                free(uri);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "%s", value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "%s", value ? "true" : "false");
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                free(fims_message);
                free(uri);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "%s", value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "%s", value ? "true" : "false");
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(value));
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - forced - multi - local uri")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT*> points = setupParseBodyClientTest(sys, true);
        setForced(points);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        sprintf(sys.fims_dependencies->data_buf, "{");
        std::string combinedJSON = "{";  // Initialize the combined JSON object
        bool isFirstPoint = true;        // To keep track of the first point
        int value = 55;
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                // Append a comma if it's not the first point
                if (!isFirstPoint)
                {
                    strcat(sys.fims_dependencies->data_buf, ",");
                    combinedJSON += ",";
                }
                isFirstPoint = false;
                char* pointJSON;
                asprintf(&pointJSON, "\"%s\":%d", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);

                strcat(sys.fims_dependencies->data_buf, pointJSON);  // Append to the data_buf
                combinedJSON += pointJSON;                           // Append to the combined JSON
                free(pointJSON);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                strcat(sys.fims_dependencies->data_buf, ",");
                combinedJSON += ",";
                char* pointJSON;
                asprintf(&pointJSON, "\"%s\":%d", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);

                strcat(sys.fims_dependencies->data_buf, pointJSON);  // Append to the data_buf
                combinedJSON += pointJSON;                           // Append to the combined JSON
                free(pointJSON);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                strcat(sys.fims_dependencies->data_buf, ",");
                combinedJSON += ",";
                char* pointJSON;
                asprintf(&pointJSON, "\"%s\":true", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());

                strcat(sys.fims_dependencies->data_buf, pointJSON);  // Append to the data_buf
                combinedJSON += pointJSON;                           // Append to the combined JSON
                free(pointJSON);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                strcat(sys.fims_dependencies->data_buf, ",");
                combinedJSON += ",";
                char* pointJSON;
                asprintf(&pointJSON, "\"%s\":true", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());

                strcat(sys.fims_dependencies->data_buf, pointJSON);  // Append to the data_buf
                combinedJSON += pointJSON;                           // Append to the combined JSON
                free(pointJSON);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryCounters); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryCounterGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                strcat(sys.fims_dependencies->data_buf, ",");
                combinedJSON += ",";
                char* pointJSON;
                asprintf(&pointJSON, "\"%s\":%d", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);

                strcat(sys.fims_dependencies->data_buf, pointJSON);  // Append to the data_buf
                combinedJSON += pointJSON;                           // Append to the combined JSON
                free(pointJSON);
            }
        }

        combinedJSON += "}";  // Close the combined JSON object
        strcat(sys.fims_dependencies->data_buf, "}");

        meta_data.data_len = combinedJSON.length();
        sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
        sys.fims_dependencies->uri_requests.contains_local_uri = true;
        sys.fims_dependencies->process_name_view = std::string_view("some_process");
        sys.fims_dependencies->username_view = std::string_view("some_username");
        CHECK(parseBodyClient(sys, meta_data));
        checkRequestCounts(sys, 5, 5, 5, 0);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->operate_value ==
                      static_cast<double>(value * ((FlexPoint*)(dbPoint->flexPointHandle))->scale));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->standby_value ==
                      static_cast<double>(value * ((FlexPoint*)(dbPoint->flexPointHandle))->scale));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryCounters); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryCounterGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->standby_value ==
                      static_cast<double>(value * ((FlexPoint*)(dbPoint->flexPointHandle))->scale));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->standby_value == static_cast<double>(1));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->operate_value == static_cast<double>(1));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
            }
        }
        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - forced - multi - no local uri")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT*> points = setupParseBodyClientTest(sys, true);
        setForced(points);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);

        sprintf(sys.fims_dependencies->data_buf, "{");
        std::string combinedJSON = "{";  // Initialize the combined JSON object
        bool isFirstPoint = true;        // To keep track of the first point
        int value = 55;
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                // Append a comma if it's not the first point
                if (!isFirstPoint)
                {
                    strcat(sys.fims_dependencies->data_buf, ",");
                    combinedJSON += ",";
                }
                isFirstPoint = false;
                char* pointJSON;
                asprintf(&pointJSON, "\"%s\":%d", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);

                strcat(sys.fims_dependencies->data_buf, pointJSON);  // Append to the data_buf
                combinedJSON += pointJSON;                           // Append to the combined JSON
                free(pointJSON);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                strcat(sys.fims_dependencies->data_buf, ",");
                combinedJSON += ",";
                char* pointJSON;
                asprintf(&pointJSON, "\"%s\":%d", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);

                strcat(sys.fims_dependencies->data_buf, pointJSON);  // Append to the data_buf
                combinedJSON += pointJSON;                           // Append to the combined JSON
                free(pointJSON);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                strcat(sys.fims_dependencies->data_buf, ",");
                combinedJSON += ",";
                char* pointJSON;
                asprintf(&pointJSON, "\"%s\":true", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());

                strcat(sys.fims_dependencies->data_buf, pointJSON);  // Append to the data_buf
                combinedJSON += pointJSON;                           // Append to the combined JSON
                free(pointJSON);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                strcat(sys.fims_dependencies->data_buf, ",");
                combinedJSON += ",";
                char* pointJSON;
                asprintf(&pointJSON, "\"%s\":true", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());

                strcat(sys.fims_dependencies->data_buf, pointJSON);  // Append to the data_buf
                combinedJSON += pointJSON;                           // Append to the combined JSON
                free(pointJSON);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryCounters); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryCounterGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                strcat(sys.fims_dependencies->data_buf, ",");
                combinedJSON += ",";
                char* pointJSON;
                asprintf(&pointJSON, "\"%s\":%d", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), value);

                strcat(sys.fims_dependencies->data_buf, pointJSON);  // Append to the data_buf
                combinedJSON += pointJSON;                           // Append to the combined JSON
                free(pointJSON);
            }
        }

        combinedJSON += "}";  // Close the combined JSON object
        strcat(sys.fims_dependencies->data_buf, "}");

        meta_data.data_len = combinedJSON.length();
        sys.fims_dependencies->uri_view = std::string_view(((FlexPoint*)(dbPoint->flexPointHandle))->uri);
        sys.fims_dependencies->uri_requests.contains_local_uri = false;
        sys.fims_dependencies->process_name_view = std::string_view("some_process");
        sys.fims_dependencies->username_view = std::string_view("some_username");
        CHECK(!parseBodyClient(sys, meta_data));
        checkRequestCounts(sys, 0, 0, 0, 0);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate_before_or_during_force);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->operate_value == 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->standby_value == 55);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate_before_or_during_force);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->operate_value == 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->standby_value == 0);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryCounters); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryCounterGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate_before_or_during_force);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->operate_value == 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->standby_value == 0);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate_before_or_during_force);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->operate_value == 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->standby_value == 0);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate_before_or_during_force);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->operate_value == 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->standby_value == 1);
            }
        }
        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - forced - single - local uri")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT*> points = setupParseBodyClientTest(sys, true);
        setForced(points);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        char* uri;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);
        sys.fims_dependencies->uri_requests.contains_local_uri = true;
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100 + 1;
                asprintf(&fims_message, "%d", value);
                sprintf(sys.fims_dependencies->data_buf, "%d", value);
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate_before_or_during_force);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->operate_value ==
                      static_cast<double>(value * ((FlexPoint*)(dbPoint->flexPointHandle))->scale));
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->standby_value == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(value));
                free(fims_message);
                free(uri);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100 + 1;
                asprintf(&fims_message, "%d", value);
                sprintf(sys.fims_dependencies->data_buf, "%d", value);
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate_before_or_during_force);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->standby_value ==
                      static_cast<double>(value * ((FlexPoint*)(dbPoint->flexPointHandle))->scale));
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->operate_value == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == 0);
                free(fims_message);
                free(uri);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryCounters); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryCounterGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100 + 1;
                asprintf(&fims_message, "%d", value);
                sprintf(sys.fims_dependencies->data_buf, "%d", value);
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate_before_or_during_force);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->standby_value ==
                      static_cast<double>(value * ((FlexPoint*)(dbPoint->flexPointHandle))->scale));
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->operate_value == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == 0);
                free(fims_message);
                free(uri);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100 - 50;
                bool bool_value = value > 0;
                asprintf(&fims_message, "%s", bool_value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "%s", bool_value ? "true" : "false");
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate_before_or_during_force);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->operate_value == static_cast<double>(bool_value));
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->standby_value == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(bool_value));
                free(fims_message);
                free(uri);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100 - 50;
                bool bool_value = value > 0;
                asprintf(&fims_message, "%s", bool_value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "%s", bool_value ? "true" : "false");
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate_before_or_during_force);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->standby_value == static_cast<double>(bool_value));
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->operate_value == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == 0);
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("parseBody - forced - single - no local uri")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT*> points = setupParseBodyClientTest(sys, true);
        setForced(points);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char* fims_message;
        char* uri;
        sys.fims_dependencies->data_buf = (char*)malloc(1000);
        sys.fims_dependencies->uri_requests.contains_local_uri = false;
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100 + 1;
                asprintf(&fims_message, "%d", value);
                sprintf(sys.fims_dependencies->data_buf, "%d", value);
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate_before_or_during_force);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->operate_value == 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->standby_value == static_cast<double>(value));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == 0);
                free(fims_message);
                free(uri);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100 + 1;
                asprintf(&fims_message, "%d", value);
                sprintf(sys.fims_dependencies->data_buf, "%d", value);
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate_before_or_during_force);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->standby_value == 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->operate_value == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == 0);
                free(fims_message);
                free(uri);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryCounters); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryCounterGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100 + 1;
                asprintf(&fims_message, "%d", value);
                sprintf(sys.fims_dependencies->data_buf, "%d", value);
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate_before_or_during_force);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->standby_value == 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->operate_value == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == 0);
                free(fims_message);
                free(uri);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100 - 50;
                bool bool_value = value > 0;
                asprintf(&fims_message, "%s", bool_value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "%s", bool_value ? "true" : "false");
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate_before_or_during_force);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->operate_value == 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->standby_value == static_cast<double>(bool_value));
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_process, "some_process") == 0);
                CHECK(strcmp(((FlexPoint*)(dbPoint->flexPointHandle))->last_update_username, "some_username") == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == 0);
                free(fims_message);
                free(uri);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100 - 50;
                bool bool_value = value > 0;
                asprintf(&fims_message, "%s", bool_value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "%s", bool_value ? "true" : "false");
                asprintf(&uri, "%s/%s", ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                CHECK(!parseBodyClient(sys, meta_data));
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate_before_or_during_force);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->standby_value == 0);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->operate_value == 0);
                checkRequestCounts(sys, 0, 0, 0, 0);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value == 0);
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("updatePointCallback - direct")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT*> points = setupParseBodyClientTest(sys);
        setDirectPub(points);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.analog.value = rand() % 100;
                updatePointCallback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE,
                                    DNPDEFS_OBJ_30_ANA_INPUTS, dbPoint->pointNumber);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer.active);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.binary.value = rand() % 2 == 0;
                updatePointCallback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE,
                                    DNPDEFS_OBJ_2_BIN_CHNG_EVENTS, dbPoint->pointNumber);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer.active);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.analog.value = rand() % 100;
                updatePointCallback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE,
                                    DNPDEFS_OBJ_40_ANA_OUT_STATUSES, dbPoint->pointNumber);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer.active);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.binary.value = rand() % 2 == 0;
                updatePointCallback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE,
                                    DNPDEFS_OBJ_10_BIN_OUT_STATUSES, dbPoint->pointNumber);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer.active);
            }
        }

        queuePubs(&sys);  // clear out any pub work to make valgrind happy

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("updatePointCallback - batch")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT*> points = setupParseBodyClientTest(sys);
        setBatchPub(points);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.analog.value = rand() % 100;
                updatePointCallback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE,
                                    DNPDEFS_OBJ_30_ANA_INPUTS, dbPoint->pointNumber);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer.active);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer.pCallbackParam == dbPoint);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer.pCallback == addPointToPubWork);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.binary.value = rand() % 2 == 0;
                updatePointCallback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE,
                                    DNPDEFS_OBJ_2_BIN_CHNG_EVENTS, dbPoint->pointNumber);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer.active);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer.pCallbackParam == dbPoint);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer.pCallback == addPointToPubWork);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.analog.value = rand() % 100;
                updatePointCallback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE,
                                    DNPDEFS_OBJ_40_ANA_OUT_STATUSES, dbPoint->pointNumber);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer.active);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.binary.value = rand() % 2 == 0;
                updatePointCallback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE,
                                    DNPDEFS_OBJ_10_BIN_OUT_STATUSES, dbPoint->pointNumber);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer.active);
            }
        }

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("updatePointCallback - interval")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT*> points = setupParseBodyClientTest(sys);
        setIntervalPub(points);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.analog.value = rand() % 100;
                updatePointCallback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE,
                                    DNPDEFS_OBJ_30_ANA_INPUTS, dbPoint->pointNumber);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer.active);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer.pCallbackParam == dbPoint);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer.pCallback == addPointToIntervalPubWork);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.binary.value = rand() % 2 == 0;
                updatePointCallback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE,
                                    DNPDEFS_OBJ_2_BIN_CHNG_EVENTS, dbPoint->pointNumber);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer.active);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer.pCallbackParam == dbPoint);
                CHECK(((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer.pCallback == addPointToIntervalPubWork);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.analog.value = rand() % 100;
                updatePointCallback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE,
                                    DNPDEFS_OBJ_40_ANA_OUT_STATUSES, dbPoint->pointNumber);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer.active);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.binary.value = rand() % 2 == 0;
                updatePointCallback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE,
                                    DNPDEFS_OBJ_10_BIN_OUT_STATUSES, dbPoint->pointNumber);
                CHECK(!((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer.active);
            }
        }

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("addPointToPubWork - direct")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT*> points = setupParseBodyClientTest(sys);
        setDirectPub(points);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.analog.value = rand() % 100;
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                tmwtarg_getDateTime(&dbPoint->timeStamp);
                addPointToPubWork(dbPoint);
                auto& pub_work_vector = sys.fims_dependencies
                                            ->uris_with_data[((FlexPoint*)dbPoint->flexPointHandle)->uri];
                auto& pub_vals = pub_work_vector[0]->pub_vals;
                auto& point = pub_vals[((FlexPoint*)(dbPoint->flexPointHandle))->name];
                ASSERT(point != nullptr);
                CHECK(point->dbPoint == dbPoint);
                CHECK(point->value == dbPoint->data.analog.value);
                CHECK(point->flags == dbPoint->flags);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.binary.value = rand() % 2 == 0;
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                tmwtarg_getDateTime(&dbPoint->timeStamp);
                addPointToPubWork(dbPoint);
                auto& pub_work_vector = sys.fims_dependencies
                                            ->uris_with_data[((FlexPoint*)dbPoint->flexPointHandle)->uri];
                auto& pub_vals = pub_work_vector[0]->pub_vals;
                auto& point = pub_vals[((FlexPoint*)(dbPoint->flexPointHandle))->name];
                CHECK(point->dbPoint == dbPoint);
                CHECK(point->value == static_cast<double>(dbPoint->data.binary.value));
                CHECK(point->flags == dbPoint->flags);
            }
        }

        queuePubs(&sys);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("addPointToPubWork - batch")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT*> points = setupParseBodyClientTest(sys);
        setBatchPub(points);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;

        sys.protocol_dependencies->dnp3.pub_all = true;  // don't queue pubs after each point

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.analog.value = rand() % 100;
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                tmwtarg_getDateTime(&dbPoint->timeStamp);
                updatePointCallback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE,
                                    DNPDEFS_OBJ_30_ANA_INPUTS, dbPoint->pointNumber);
                addPointToPubWork(dbPoint);
                auto& pub_work_vector = sys.fims_dependencies
                                            ->uris_with_data[((FlexPoint*)dbPoint->flexPointHandle)->uri];
                auto& pub_vals = pub_work_vector[0]->pub_vals;
                auto& point = pub_vals[((FlexPoint*)(dbPoint->flexPointHandle))->name];
                CHECK(point->dbPoint == dbPoint);
                CHECK(point->value == dbPoint->data.analog.value);
                CHECK(point->flags == dbPoint->flags);
                CHECK(!((FlexPoint*)dbPoint->flexPointHandle)->pub_timer.active);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.binary.value = rand() % 2 == 0;
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                tmwtarg_getDateTime(&dbPoint->timeStamp);
                updatePointCallback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE,
                                    DNPDEFS_OBJ_2_BIN_CHNG_EVENTS, dbPoint->pointNumber);
                addPointToPubWork(dbPoint);
                auto& pub_work_vector = sys.fims_dependencies
                                            ->uris_with_data[((FlexPoint*)dbPoint->flexPointHandle)->uri];
                auto& pub_vals = pub_work_vector[0]->pub_vals;
                auto& point = pub_vals[((FlexPoint*)(dbPoint->flexPointHandle))->name];
                CHECK(point->dbPoint == dbPoint);
                CHECK(point->value == static_cast<double>(dbPoint->data.binary.value));
                CHECK(point->flags == dbPoint->flags);
                CHECK(!((FlexPoint*)dbPoint->flexPointHandle)->pub_timer.active);
            }
        }

        queuePubs(&sys);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("addPointToIntervalPubWork - interval")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT*> points = setupParseBodyClientTest(sys);
        setIntervalPub(points);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;

        sys.protocol_dependencies->dnp3.pub_all = true;  // don't queue pubs after each point

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.analog.value = rand() % 100;
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                tmwtarg_getDateTime(&dbPoint->timeStamp);
                addPointToIntervalPubWork(dbPoint);
                auto& pub_work_vector = sys.fims_dependencies
                                            ->uris_with_data[((FlexPoint*)dbPoint->flexPointHandle)->uri];
                auto& pub_vals = pub_work_vector[0]->pub_vals;
                auto& point = pub_vals[((FlexPoint*)(dbPoint->flexPointHandle))->name];
                CHECK(point->dbPoint == dbPoint);
                CHECK(point->value == dbPoint->data.analog.value);
                CHECK(point->flags == dbPoint->flags);
                CHECK(((FlexPoint*)dbPoint->flexPointHandle)->pub_timer.active);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.binary.value = rand() % 2 == 0;
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                tmwtarg_getDateTime(&dbPoint->timeStamp);
                addPointToIntervalPubWork(dbPoint);
                auto& pub_work_vector = sys.fims_dependencies
                                            ->uris_with_data[((FlexPoint*)dbPoint->flexPointHandle)->uri];
                auto& pub_vals = pub_work_vector[0]->pub_vals;
                auto& point = pub_vals[((FlexPoint*)(dbPoint->flexPointHandle))->name];
                CHECK(point->dbPoint == dbPoint);
                CHECK(point->value == static_cast<double>(dbPoint->data.binary.value));
                CHECK(point->flags == dbPoint->flags);
                CHECK(((FlexPoint*)dbPoint->flexPointHandle)->pub_timer.active);
            }
        }

        queuePubs(&sys);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("assignValueBasedOnType - analog - not forced")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::Analog;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 10.0;
        dbPoint.data.analog.value = 20.0;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 20.0);
        CHECK(status_point.value == 0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - analog - forced")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::Analog;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = true;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 10.0;
        dbPoint.data.analog.value = 20.0;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 10.0);
        CHECK(status_point.value == 0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - binary - not forced")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::Binary;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 1;
        dbPoint.data.binary.value = 1;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 1);
        CHECK(status_point.value == 0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - binary - forced")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::Binary;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = true;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 1;
        dbPoint.data.binary.value = 1;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 1);
        CHECK(status_point.value == 0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - counter - not forced")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::Counter;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 10.0;
        dbPoint.data.counter.value = 20.0;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 20.0);
        CHECK(status_point.value == 0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - counter - forced")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::Counter;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = true;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 10.0;
        dbPoint.data.counter.value = 20.0;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 10.0);
        CHECK(status_point.value == 0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - analogOS - has not sent operate - show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::AnalogOS;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = true;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 10.0;
        dbPoint.data.analog.value = 20.0;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 20.0);
        CHECK(status_point.value == 20.0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - analogOS - has not sent operate - don't show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::AnalogOS;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = false;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 10.0;
        dbPoint.data.analog.value = 20.0;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 20.0);
        CHECK(status_point.value == 0.0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - AnOPInt32 - has not sent operate - show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::AnOPInt32;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = true;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 10.0;
        dbPoint.data.analog.value = 20.0;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 20.0);
        CHECK(status_point.value == 20.0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - AnOPInt32 - has not sent operate - don't show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::AnOPInt32;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = false;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 10.0;
        dbPoint.data.analog.value = 20.0;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 20.0);
        CHECK(status_point.value == 0.0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - AnOPInt16 - has not sent operate - show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::AnOPInt16;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = true;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 10.0;
        dbPoint.data.analog.value = 20.0;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 20.0);
        CHECK(status_point.value == 20.0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - AnOPInt16 - has not sent operate - don't show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::AnOPInt16;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = false;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 10.0;
        dbPoint.data.analog.value = 20.0;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 20.0);
        CHECK(status_point.value == 0.0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - AnOPF32 - has not sent operate - show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::AnOPF32;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = true;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 10.0;
        dbPoint.data.analog.value = 20.0;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 20.0);
        CHECK(status_point.value == 20.0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - AnOPF32 - has not sent operate - don't show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::AnOPF32;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->sent_operate = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = false;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 10.0;
        dbPoint.data.analog.value = 20.0;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 20.0);
        CHECK(status_point.value == 0.0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - analogOS - has sent operate - show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::AnalogOS;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = true;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 10.0;
        ((FlexPoint*)dbPoint.flexPointHandle)->operate_value = 15.0;
        dbPoint.data.analog.value = 20.0;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 15.0);
        CHECK(status_point.value == 20.0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - analogOS - has sent operate - don't show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::AnalogOS;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = false;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 10.0;
        ((FlexPoint*)dbPoint.flexPointHandle)->operate_value = 15.0;
        dbPoint.data.analog.value = 20.0;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 15.0);
        CHECK(status_point.value == 0.0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - AnOPInt32 - has sent operate - show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::AnOPInt32;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = true;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 10.0;
        ((FlexPoint*)dbPoint.flexPointHandle)->operate_value = 15.0;
        dbPoint.data.analog.value = 20.0;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 15.0);
        CHECK(status_point.value == 20.0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - AnOPInt32 - has sent operate - don't show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::AnOPInt32;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = false;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 10.0;
        ((FlexPoint*)dbPoint.flexPointHandle)->operate_value = 15.0;
        dbPoint.data.analog.value = 20.0;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 15.0);
        CHECK(status_point.value == 0.0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - AnOPInt16 - has sent operate - show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::AnOPInt16;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = true;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 10.0;
        ((FlexPoint*)dbPoint.flexPointHandle)->operate_value = 15.0;
        dbPoint.data.analog.value = 20.0;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 15.0);
        CHECK(status_point.value == 20.0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - AnOPInt16 - has sent operate - don't show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::AnOPInt16;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = false;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 10.0;
        ((FlexPoint*)dbPoint.flexPointHandle)->operate_value = 15.0;
        dbPoint.data.analog.value = 20.0;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 15.0);
        CHECK(status_point.value == 0.0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - AnOPF32 - has sent operate - show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::AnOPF32;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = true;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 10.0;
        ((FlexPoint*)dbPoint.flexPointHandle)->operate_value = 15.0;
        dbPoint.data.analog.value = 20.0;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 15.0);
        CHECK(status_point.value == 20.0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - AnOPF32 - has sent operate - don't show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::AnOPF32;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = false;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 10.0;
        ((FlexPoint*)dbPoint.flexPointHandle)->operate_value = 15.0;
        dbPoint.data.analog.value = 20.0;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 15.0);
        CHECK(status_point.value == 0.0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - BinaryOS - has not sent operate - show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::BinaryOS;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->sent_operate = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = true;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 1;
        dbPoint.data.binary.value = 1;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 1);
        CHECK(status_point.value == 1);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - BinaryOS - has not sent operate - don't show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::BinaryOS;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->sent_operate = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = false;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 1;
        dbPoint.data.binary.value = 1;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 1);
        CHECK(status_point.value == 0.0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - CROB - has not sent operate - show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::CROB;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->sent_operate = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = true;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 1;
        dbPoint.data.binary.value = 1;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 1);
        CHECK(status_point.value == 1);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - CROB - has not sent operate - don't show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::CROB;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->sent_operate = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = false;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 1;
        dbPoint.data.binary.value = 1;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 1);
        CHECK(status_point.value == 0.0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - BinaryOS - has sent operate - show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::BinaryOS;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = true;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 1;
        ((FlexPoint*)dbPoint.flexPointHandle)->operate_value = 1;
        dbPoint.data.binary.value = 1;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 1);
        CHECK(status_point.value == 1);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - BinaryOS - has sent operate - don't show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::BinaryOS;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = false;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 1;
        ((FlexPoint*)dbPoint.flexPointHandle)->operate_value = 1;
        dbPoint.data.binary.value = 1;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 1);
        CHECK(status_point.value == 0.0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - CROB - has sent operate - show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::CROB;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = true;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 1;
        ((FlexPoint*)dbPoint.flexPointHandle)->operate_value = 1;
        dbPoint.data.binary.value = 1;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 1);
        CHECK(status_point.value == 1);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }
    TEST_CASE("assignValueBasedOnType - CROB - has sent operate - don't show output status")
    {
        GcomSystem sys;
        // Create a TMWSIM_POINT object
        TMWSIM_POINT dbPoint;
        dbPoint.flexPointHandle = new FlexPoint(&sys, "some_point", "/some/uri");
        ((FlexPoint*)dbPoint.flexPointHandle)->type = Register_Types::CROB;
        ((FlexPoint*)dbPoint.flexPointHandle)->is_forced = false;
        ((FlexPoint*)(dbPoint.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint.flexPointHandle))->show_output_status = false;
        ((FlexPoint*)dbPoint.flexPointHandle)->standby_value = 1;
        ((FlexPoint*)dbPoint.flexPointHandle)->operate_value = 1;
        dbPoint.data.binary.value = 1;

        // Create PubPoint objects for testing
        PubPoint point;
        PubPoint status_point;
        point.value = 0;
        status_point.value = 0;

        // Call the function being tested
        assignValueBasedOnType(&point, &status_point, &dbPoint);

        // Check if the value was assigned correctly
        CHECK(point.value == 1);
        CHECK(status_point.value == 0.0);

        // Clean up
        delete (FlexPoint*)dbPoint.flexPointHandle;
    }

    // TODO: Fix this for formatting
    // TEST_CASE("queuePubs - analog inputs"){
    //     GcomSystem sys = GcomSystem(Protocol::DNP3);
    //     setupParseBodyClientTest(sys);
    //     DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
    //     TMWSIM_POINT *dbPoint = nullptr;
    //     char *fims_message;

    //     sys.protocol_dependencies->dnp3.pub_all = true; // don't queue pubs after each point

    //     for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
    //     {
    //         dbPoint = (TMWSIM_POINT *)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
    //         if (dbPoint)
    //         {
    //             dbPoint->data.analog.value = rand() % 100;
    //             dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
    //             tmwtarg_getDateTime(&dbPoint->timeStamp);
    //             addPointToPubWork(dbPoint);
    //             queuePubs(&sys);
    //             asprintf(&fims_message, "{\"%s\":%f,\"Timestamp\":", ((FlexPoint
    //             *)(dbPoint->flexPointHandle))->name.c_str(), dbPoint->data.analog.value/((FlexPoint
    //             *)(dbPoint->flexPointHandle))->scale);
    //             CHECK(strncmp(sys.fims_dependencies->send_buf.data(),fims_message, strlen(fims_message)) == 0);
    //             free(fims_message);
    //         }
    //     }

    //     fims_message = (char *)malloc(1000);
    //     sprintf(fims_message, "{");
    //     char *append_str = (char *)malloc(100);
    //     for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
    //     {
    //         dbPoint = (TMWSIM_POINT *)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
    //         if (dbPoint)
    //         {
    //             dbPoint->data.analog.value = rand() % 100;
    //             dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
    //             tmwtarg_getDateTime(&dbPoint->timeStamp);
    //             addPointToPubWork(dbPoint);
    //             memset(append_str,'\0',100);
    //             sprintf(append_str, "\"%s\":%f,", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(),
    //             dbPoint->data.analog.value/((FlexPoint *)(dbPoint->flexPointHandle))->scale); strcat(fims_message,
    //             append_str);
    //         }
    //     }
    //     strcat(fims_message, "\"Timestamp\":");
    //     queuePubs(&sys);
    //     CHECK(strncmp(sys.fims_dependencies->send_buf.data(),fims_message, strlen(fims_message)) == 0);
    //     free(fims_message);
    //     free(append_str);

    //     shutdown_tmw(&sys.protocol_dependencies->dnp3);
    // }
    TEST_CASE("queuePubs - binary inputs")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        char* fims_message;

        sys.protocol_dependencies->dnp3.pub_all = true;  // don't queue pubs after each point

        // check individually
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.binary.value = rand() % 2 == 0;
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                tmwtarg_getDateTime(&dbPoint->timeStamp);
                addPointToPubWork(dbPoint);
                queuePubs(&sys);
                asprintf(&fims_message,
                         "{\"%s\":%s,\"Timestamp\":", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                         dbPoint->data.binary.value ? "true" : "false");
                CHECK(strncmp(sys.fims_dependencies->send_buf.data(), fims_message, strlen(fims_message)) == 0);
                free(fims_message);
            }
        }

        // check all at once
        fims_message = (char*)malloc(1000);
        sprintf(fims_message, "{");
        char* append_str = (char*)malloc(100);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.binary.value = rand() % 2 == 0;
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                tmwtarg_getDateTime(&dbPoint->timeStamp);
                addPointToPubWork(dbPoint);
                memset(append_str, '\0', 100);
                sprintf(append_str, "\"%s\":%s,", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                        dbPoint->data.binary.value ? "true" : "false");
                strcat(fims_message, append_str);
            }
        }
        strcat(fims_message, "\"Timestamp\":");
        queuePubs(&sys);
        CHECK(strncmp(sys.fims_dependencies->send_buf.data(), fims_message, strlen(fims_message)) == 0);
        free(fims_message);
        free(append_str);

        // check crob_int
        fims_message = (char*)malloc(1000);
        sprintf(fims_message, "{");
        append_str = (char*)malloc(100);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_int = true;
                dbPoint->data.binary.value = rand() % 2 == 0;
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                tmwtarg_getDateTime(&dbPoint->timeStamp);
                addPointToPubWork(dbPoint);
                memset(append_str, '\0', 100);
                sprintf(append_str, "\"%s\":%d,", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                        dbPoint->data.binary.value ? 1 : 0);
                strcat(fims_message, append_str);
            }
        }
        strcat(fims_message, "\"Timestamp\":");
        queuePubs(&sys);
        CHECK(strncmp(sys.fims_dependencies->send_buf.data(), fims_message, strlen(fims_message)) == 0);
        free(fims_message);
        free(append_str);

        // check crob_string
        fims_message = (char*)malloc(1000);
        sprintf(fims_message, "{");
        append_str = (char*)malloc(100);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_string = true;
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_int = false;
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_true = "ON";
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_false = "OFF";
                dbPoint->data.binary.value = rand() % 2 == 0;
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                tmwtarg_getDateTime(&dbPoint->timeStamp);
                addPointToPubWork(dbPoint);
                memset(append_str, '\0', 100);
                sprintf(append_str, "\"%s\":\"%s\",", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                        dbPoint->data.binary.value ? "ON" : "OFF");
                strcat(fims_message, append_str);
            }
        }
        strcat(fims_message, "\"Timestamp\":");
        queuePubs(&sys);
        CHECK(strncmp(sys.fims_dependencies->send_buf.data(), fims_message, strlen(fims_message)) == 0);
        free(fims_message);
        free(append_str);

        // check scale negative
        fims_message = (char*)malloc(1000);
        sprintf(fims_message, "{");
        append_str = (char*)malloc(100);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->scale = -1;
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_string = false;
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_int = false;
                dbPoint->data.binary.value = rand() % 2 == 0;
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                tmwtarg_getDateTime(&dbPoint->timeStamp);
                addPointToPubWork(dbPoint);
                memset(append_str, '\0', 100);
                sprintf(append_str, "\"%s\":%s,", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                        !dbPoint->data.binary.value ? "true" : "false");
                strcat(fims_message, append_str);
            }
        }
        strcat(fims_message, "\"Timestamp\":");
        queuePubs(&sys);
        CHECK(strncmp(sys.fims_dependencies->send_buf.data(), fims_message, strlen(fims_message)) == 0);
        free(fims_message);
        free(append_str);

        // check scale negative, crob_int
        fims_message = (char*)malloc(1000);
        sprintf(fims_message, "{");
        append_str = (char*)malloc(100);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->scale = -1;
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_int = true;
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_string = false;
                dbPoint->data.binary.value = rand() % 2 == 0;
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                tmwtarg_getDateTime(&dbPoint->timeStamp);
                addPointToPubWork(dbPoint);
                memset(append_str, '\0', 100);
                sprintf(append_str, "\"%s\":%d,", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                        !dbPoint->data.binary.value ? 1 : 0);
                strcat(fims_message, append_str);
            }
        }
        strcat(fims_message, "\"Timestamp\":");
        queuePubs(&sys);
        CHECK(strncmp(sys.fims_dependencies->send_buf.data(), fims_message, strlen(fims_message)) == 0);
        free(fims_message);
        free(append_str);

        // check scale negative, crob_string
        fims_message = (char*)malloc(1000);
        sprintf(fims_message, "{");
        append_str = (char*)malloc(100);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->scale = -1;
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_string = true;
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_int = false;
                dbPoint->data.binary.value = rand() % 2 == 0;
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                tmwtarg_getDateTime(&dbPoint->timeStamp);
                addPointToPubWork(dbPoint);
                memset(append_str, '\0', 100);
                sprintf(append_str, "\"%s\":\"%s\",", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                        !dbPoint->data.binary.value ? "ON" : "OFF");
                strcat(fims_message, append_str);
            }
        }
        strcat(fims_message, "\"Timestamp\":");
        queuePubs(&sys);
        CHECK(strncmp(sys.fims_dependencies->send_buf.data(), fims_message, strlen(fims_message)) == 0);
        free(fims_message);
        free(append_str);

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    // TODO: Fix this for formatting
    // TEST_CASE("queuePubs - analog inputs clothed"){
    //     GcomSystem sys = GcomSystem(Protocol::DNP3);
    //     setupParseBodyClientTest(sys);
    //     DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
    //     TMWSIM_POINT *dbPoint = nullptr;
    //     char *fims_message;

    //     sys.protocol_dependencies->dnp3.pub_all = true; // don't queue pubs after each point

    //     for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
    //     {
    //         dbPoint = (TMWSIM_POINT *)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
    //         if (dbPoint)
    //         {
    //             ((FlexPoint *)(dbPoint->flexPointHandle))->format = FimsFormat::Clothed;
    //             dbPoint->data.analog.value = rand() % 100;
    //             dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
    //             tmwtarg_getDateTime(&dbPoint->timeStamp);
    //             addPointToPubWork(dbPoint);
    //             queuePubs(&sys);
    //             asprintf(&fims_message, "{\"%s\":{\"value\":%g},\"Timestamp\":", ((FlexPoint
    //             *)(dbPoint->flexPointHandle))->name.c_str(), dbPoint->data.analog.value/((FlexPoint
    //             *)(dbPoint->flexPointHandle))->scale);
    //             CHECK(strncmp(sys.fims_dependencies->send_buf.data(),fims_message, strlen(fims_message)) == 0);
    //             free(fims_message);
    //         }
    //     }

    //     fims_message = (char *)malloc(1000);
    //     sprintf(fims_message, "{");
    //     char *append_str = (char *)malloc(100);
    //     for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
    //     {
    //         dbPoint = (TMWSIM_POINT *)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
    //         if (dbPoint)
    //         {
    //             dbPoint->data.analog.value = rand() % 100;
    //             dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
    //             tmwtarg_getDateTime(&dbPoint->timeStamp);
    //             addPointToPubWork(dbPoint);
    //             memset(append_str,'\0',100);
    //             sprintf(append_str, "\"%s\":{\"value\":%g},", ((FlexPoint
    //             *)(dbPoint->flexPointHandle))->name.c_str(), dbPoint->data.analog.value/((FlexPoint
    //             *)(dbPoint->flexPointHandle))->scale); strcat(fims_message, append_str);
    //         }
    //     }
    //     strcat(fims_message, "\"Timestamp\":");
    //     queuePubs(&sys);
    //     CHECK(strncmp(sys.fims_dependencies->send_buf.data(),fims_message, strlen(fims_message)) == 0);
    //     free(fims_message);
    //     free(append_str);

    //     shutdown_tmw(&sys.protocol_dependencies->dnp3);
    // }
    TEST_CASE("queuePubs - binary inputs clothed")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyClientTest(sys);
        DNP3Dependencies* dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT* dbPoint = nullptr;
        char* fims_message;

        sys.protocol_dependencies->dnp3.pub_all = true;  // don't queue pubs after each point

        // check individually
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->format = FimsFormat::Clothed;
                dbPoint->data.binary.value = rand() % 2 == 0;
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                tmwtarg_getDateTime(&dbPoint->timeStamp);
                addPointToPubWork(dbPoint);
                queuePubs(&sys);
                asprintf(&fims_message, "{\"%s\":{\"value\":%s},\"Timestamp\":",
                         ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                         dbPoint->data.binary.value ? "true" : "false");
                CHECK(strncmp(sys.fims_dependencies->send_buf.data(), fims_message, strlen(fims_message)) == 0);
                free(fims_message);
            }
        }

        // check all at once
        fims_message = (char*)malloc(1000);
        sprintf(fims_message, "{");
        char* append_str = (char*)malloc(100);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.binary.value = rand() % 2 == 0;
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                tmwtarg_getDateTime(&dbPoint->timeStamp);
                addPointToPubWork(dbPoint);
                memset(append_str, '\0', 100);
                sprintf(append_str, "\"%s\":{\"value\":%s},", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                        dbPoint->data.binary.value ? "true" : "false");
                strcat(fims_message, append_str);
            }
        }
        strcat(fims_message, "\"Timestamp\":");
        queuePubs(&sys);
        CHECK(strncmp(sys.fims_dependencies->send_buf.data(), fims_message, strlen(fims_message)) == 0);
        free(fims_message);
        free(append_str);

        // check crob_int
        fims_message = (char*)malloc(1000);
        sprintf(fims_message, "{");
        append_str = (char*)malloc(100);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_int = true;
                dbPoint->data.binary.value = rand() % 2 == 0;
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                tmwtarg_getDateTime(&dbPoint->timeStamp);
                addPointToPubWork(dbPoint);
                memset(append_str, '\0', 100);
                sprintf(append_str, "\"%s\":{\"value\":%d},", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                        dbPoint->data.binary.value ? 1 : 0);
                strcat(fims_message, append_str);
            }
        }
        strcat(fims_message, "\"Timestamp\":");
        queuePubs(&sys);
        CHECK(strncmp(sys.fims_dependencies->send_buf.data(), fims_message, strlen(fims_message)) == 0);
        free(fims_message);
        free(append_str);

        // check crob_string
        fims_message = (char*)malloc(1000);
        sprintf(fims_message, "{");
        append_str = (char*)malloc(100);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_string = true;
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_int = false;
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_true = "ON";
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_false = "OFF";
                dbPoint->data.binary.value = rand() % 2 == 0;
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                tmwtarg_getDateTime(&dbPoint->timeStamp);
                addPointToPubWork(dbPoint);
                memset(append_str, '\0', 100);
                sprintf(append_str, "\"%s\":{\"value\":\"%s\"},",
                        ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                        dbPoint->data.binary.value ? "ON" : "OFF");
                strcat(fims_message, append_str);
            }
        }
        strcat(fims_message, "\"Timestamp\":");
        queuePubs(&sys);
        CHECK(strncmp(sys.fims_dependencies->send_buf.data(), fims_message, strlen(fims_message)) == 0);
        free(fims_message);
        free(append_str);

        // check scale negative
        fims_message = (char*)malloc(1000);
        sprintf(fims_message, "{");
        append_str = (char*)malloc(100);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->scale = -1;
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_string = false;
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_int = false;
                dbPoint->data.binary.value = rand() % 2 == 0;
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                tmwtarg_getDateTime(&dbPoint->timeStamp);
                addPointToPubWork(dbPoint);
                memset(append_str, '\0', 100);
                sprintf(append_str, "\"%s\":{\"value\":%s},", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                        !dbPoint->data.binary.value ? "true" : "false");
                strcat(fims_message, append_str);
            }
        }
        strcat(fims_message, "\"Timestamp\":");
        queuePubs(&sys);
        CHECK(strncmp(sys.fims_dependencies->send_buf.data(), fims_message, strlen(fims_message)) == 0);
        free(fims_message);
        free(append_str);

        // check scale negative, crob_int
        fims_message = (char*)malloc(1000);
        sprintf(fims_message, "{");
        append_str = (char*)malloc(100);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->scale = -1;
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_int = true;
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_string = false;
                dbPoint->data.binary.value = rand() % 2 == 0;
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                tmwtarg_getDateTime(&dbPoint->timeStamp);
                addPointToPubWork(dbPoint);
                memset(append_str, '\0', 100);
                sprintf(append_str, "\"%s\":{\"value\":%d},", ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                        !dbPoint->data.binary.value ? 1 : 0);
                strcat(fims_message, append_str);
            }
        }
        strcat(fims_message, "\"Timestamp\":");
        queuePubs(&sys);
        CHECK(strncmp(sys.fims_dependencies->send_buf.data(), fims_message, strlen(fims_message)) == 0);
        free(fims_message);
        free(append_str);

        // check scale negative, crob_string
        fims_message = (char*)malloc(1000);
        sprintf(fims_message, "{");
        append_str = (char*)malloc(100);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->scale = -1;
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_string = true;
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_int = false;
                dbPoint->data.binary.value = rand() % 2 == 0;
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                tmwtarg_getDateTime(&dbPoint->timeStamp);
                addPointToPubWork(dbPoint);
                memset(append_str, '\0', 100);
                sprintf(append_str, "\"%s\":{\"value\":\"%s\"},",
                        ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                        !dbPoint->data.binary.value ? "ON" : "OFF");
                strcat(fims_message, append_str);
            }
        }
        strcat(fims_message, "\"Timestamp\":");
        queuePubs(&sys);
        CHECK(strncmp(sys.fims_dependencies->send_buf.data(), fims_message, strlen(fims_message)) == 0);
        free(fims_message);
        free(append_str);

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
}