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
#include "fims/defer.hpp"

#include "../../include/gcom_dnp3_server.h"

bool first_test_dnp3_server = true;

std::vector<TMWSIM_POINT *> setupParseBodyServerTest(GcomSystem &sys)
{
    if(first_test_dnp3_server){
        printf("Testing dnp3_server.cpp...\n");
        first_test_dnp3_server = false;
    }
    sys.protocol_dependencies->who = DNP3_OUTSTATION;
    sys.protocol_dependencies->ip_address = strdup("0.0.0.0");
    sys.id = strdup("test_dnp3_server");
    sys.protocol_dependencies->port = 20000;
    sys.protocol_dependencies->dnp3.point_status_info = new PointStatusInfo();
    sys.protocol_dependencies->dnp3.openTMWChannel = openTMWServerChannel;
    sys.protocol_dependencies->dnp3.openTMWSession = openTMWServerSession;
    init_tmw(sys);
    sys.fims_dependencies->fims_gateway.Connect("dnp3_test");
    std::vector<TMWSIM_POINT *> points = addPoints(sys);
    return points;
}

TEST_SUITE("dnp3_server")
{
    TEST_CASE("parseBody - analog inputs - multi")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "{\"%s\":%d}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value);
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":%d}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value);
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint *)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"some_process") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"some_username") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(dbPoint->data.analog.value == static_cast<double>(value * ((FlexPoint *)dbPoint->flexPointHandle)->scale));
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - counters - multi")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryCounters); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binCntrGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "{\"%s\":%d}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value);
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":%d}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value);
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint *)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"some_process") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"some_username") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(dbPoint->data.counter.value == static_cast<uint32_t>(value * ((FlexPoint *)dbPoint->flexPointHandle)->scale));
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - analog outputs - multi")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "{\"%s\":%d}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value);
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":%d}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value);
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint *)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"some_process") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"some_username") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_RESTART);
                CHECK(dbPoint->data.analog.value == static_cast<double>(value * ((FlexPoint *)dbPoint->flexPointHandle)->scale));
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - binary inputs - multi")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "{\"%s\":%s}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":%s}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value ? "true" : "false");
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint *)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"some_process") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"some_username") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(dbPoint->data.binary.value == value);
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - binary outputs - multi")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "{\"%s\":%s}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":%s}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value ? "true" : "false");
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint *)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"some_process") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"some_username") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_RESTART);
                CHECK(dbPoint->data.binary.value == value);
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - test multi")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);
        sprintf(sys.fims_dependencies->data_buf, "{");
        std::string combinedJSON = "{"; // Initialize the combined JSON object
        bool isFirstPoint = true;       // To keep track of the first point
        int value = 55;
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                // Append a comma if it's not the first point
                if (!isFirstPoint)
                {
                    strcat(sys.fims_dependencies->data_buf, ",");
                    combinedJSON += ",";
                }
                isFirstPoint = false;
                char *pointJSON;
                asprintf(&pointJSON, "\"%s\":%d", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value);

                strcat(sys.fims_dependencies->data_buf, pointJSON); // Append to the data_buf
                combinedJSON += pointJSON;                          // Append to the combined JSON
                free(pointJSON);
            }
        }

        combinedJSON += "}"; // Close the combined JSON object
        strcat(sys.fims_dependencies->data_buf, "}");

        meta_data.data_len = combinedJSON.length();
        sys.fims_dependencies->uri_view = std::string_view(((FlexPoint *)(dbPoint->flexPointHandle))->uri);
        sys.fims_dependencies->process_name_view = std::string_view("some_process");
        sys.fims_dependencies->username_view = std::string_view("some_username");
        CHECK(parseBodyServer(sys, meta_data));
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(dbPoint->data.analog.value == static_cast<double>(value * ((FlexPoint *)dbPoint->flexPointHandle)->scale));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"some_process") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"some_username") == 0);
            }
        }
        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - analog inputs - multi with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "{\"%s\":{\"value\":%d}}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value);
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":{\"value\":%d}}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value);
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint *)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"some_process") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"some_username") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(dbPoint->data.analog.value == static_cast<double>(value * ((FlexPoint *)dbPoint->flexPointHandle)->scale));
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - counters - multi with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryCounters); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binCntrGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "{\"%s\":{\"value\":%d}}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value);
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":{\"value\":%d}}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value);
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint *)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"some_process") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"some_username") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(dbPoint->data.counter.value == static_cast<uint32_t>(value * ((FlexPoint *)dbPoint->flexPointHandle)->scale));
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - analog inputs - multi without 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "{\"%s\":{\"blah\":%d}}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value);
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":{\"blah\":%d}}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value);
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint *)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_RESTART);
                CHECK(dbPoint->data.analog.value == static_cast<double>(0));
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - counters - multi without 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryCounters); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binCntrGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "{\"%s\":{\"blah\":%d}}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value);
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":{\"blah\":%d}}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value);
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint *)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_RESTART);
                CHECK(dbPoint->data.counter.value == static_cast<uint32_t>(0));
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - analog outputs - multi with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "{\"%s\":{\"value\":%d}}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value);
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":{\"value\":%d}}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value);
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint *)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"some_process") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"some_username") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_RESTART);
                CHECK(dbPoint->data.analog.value == static_cast<double>(value * ((FlexPoint *)dbPoint->flexPointHandle)->scale));
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - binary inputs - multi with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "{\"%s\":{\"value\":%s}}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":{\"value\":%s}}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value ? "true" : "false");
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint *)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"some_process") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"some_username") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(dbPoint->data.binary.value == value);
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - binary inputs - multi with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "{\"%s\":{\"blah\":%s}}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":{\"blah\":%s}}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value ? "true" : "false");
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint *)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_RESTART);
                CHECK(dbPoint->data.binary.value == false);
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - binary outputs - multi with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "{\"%s\":{\"value\":%s}}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "{\"%s\":{\"value\":%s}}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value ? "true" : "false");
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(((FlexPoint *)(dbPoint->flexPointHandle))->uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"some_process") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"some_username") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_RESTART);
                CHECK(dbPoint->data.binary.value == value);
                free(fims_message);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - test multi with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);
        sprintf(sys.fims_dependencies->data_buf, "{");
        std::string combinedJSON = "{"; // Initialize the combined JSON object
        bool isFirstPoint = true;       // To keep track of the first point
        int value = 55;
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                // Append a comma if it's not the first point
                if (!isFirstPoint)
                {
                    strcat(sys.fims_dependencies->data_buf, ",");
                    combinedJSON += ",";
                }
                isFirstPoint = false;
                char *pointJSON;
                asprintf(&pointJSON, "\"%s\":{\"value\":%d}", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value);

                strcat(sys.fims_dependencies->data_buf, pointJSON); // Append to the data_buf
                combinedJSON += pointJSON;                          // Append to the combined JSON
                free(pointJSON);
            }
        }
        combinedJSON += "}"; // Close the combined JSON object
        strcat(sys.fims_dependencies->data_buf, "}");

        meta_data.data_len = combinedJSON.length();
        sys.fims_dependencies->uri_view = std::string_view(((FlexPoint *)(dbPoint->flexPointHandle))->uri);
        sys.fims_dependencies->process_name_view = std::string_view("some_process");
        sys.fims_dependencies->username_view = std::string_view("some_username");
        CHECK(parseBodyServer(sys, meta_data));
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(dbPoint->data.analog.value == static_cast<double>(value*((FlexPoint *)dbPoint->flexPointHandle)->scale));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"some_process") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"some_username") == 0);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - analog inputs - single")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        char *uri;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "%d", value);
                sprintf(sys.fims_dependencies->data_buf, "%d", value);
                asprintf(&uri, "%s/%s", ((FlexPoint *)(dbPoint->flexPointHandle))->uri, ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"some_process") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"some_username") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(dbPoint->data.analog.value == static_cast<double>(value * ((FlexPoint *)dbPoint->flexPointHandle)->scale));
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - analog outputs - single")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        char *uri;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "%d", value);
                sprintf(sys.fims_dependencies->data_buf, "%d", value);
                asprintf(&uri, "%s/%s", ((FlexPoint *)(dbPoint->flexPointHandle))->uri, ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"some_process") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"some_username") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_RESTART);
                CHECK(dbPoint->data.analog.value == static_cast<double>(value * ((FlexPoint *)dbPoint->flexPointHandle)->scale));
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - binary inputs - single")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        char *uri;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "%s", value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "%s", value ? "true" : "false");
                asprintf(&uri, "%s/%s", ((FlexPoint *)(dbPoint->flexPointHandle))->uri, ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"some_process") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"some_username") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(dbPoint->data.binary.value == value);
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - binary outputs - single")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        char *uri;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "%s", value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "%s", value ? "true" : "false");
                asprintf(&uri, "%s/%s", ((FlexPoint *)(dbPoint->flexPointHandle))->uri, ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"some_process") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"some_username") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_RESTART);
                CHECK(dbPoint->data.binary.value == value);
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - analog inputs - single with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        char *uri;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "{\"value\":%d}", value);
                sprintf(sys.fims_dependencies->data_buf, "{\"value\":%d}", value);
                asprintf(&uri, "%s/%s", ((FlexPoint *)(dbPoint->flexPointHandle))->uri, ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"some_process") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"some_username") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(dbPoint->data.analog.value == static_cast<double>(value * ((FlexPoint *)dbPoint->flexPointHandle)->scale));
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - counters - single with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        char *uri;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryCounters); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binCntrGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "{\"value\":%d}", value);
                sprintf(sys.fims_dependencies->data_buf, "{\"value\":%d}", value);
                asprintf(&uri, "%s/%s", ((FlexPoint *)(dbPoint->flexPointHandle))->uri, ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"some_process") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"some_username") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(dbPoint->data.counter.value == static_cast<uint32_t>(value * ((FlexPoint *)dbPoint->flexPointHandle)->scale));
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - analog inputs - single without 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        char *uri;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "{\"blah\":%d}", value);
                sprintf(sys.fims_dependencies->data_buf, "{\"blah\":%d}", value);
                asprintf(&uri, "%s/%s", ((FlexPoint *)(dbPoint->flexPointHandle))->uri, ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_RESTART);
                CHECK(dbPoint->data.analog.value == static_cast<double>(0));
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - counters - single without 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        char *uri;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryCounters); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binCntrGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "{\"blah\":%d}", value);
                sprintf(sys.fims_dependencies->data_buf, "{\"blah\":%d}", value);
                asprintf(&uri, "%s/%s", ((FlexPoint *)(dbPoint->flexPointHandle))->uri, ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_RESTART);
                CHECK(dbPoint->data.counter.value == static_cast<uint32_t>(0));
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - analog outputs - single with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        char *uri;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                int value = rand() % 100;
                asprintf(&fims_message, "{\"value\":%d}", value);
                sprintf(sys.fims_dependencies->data_buf, "{\"value\":%d}", value);
                asprintf(&uri, "%s/%s", ((FlexPoint *)(dbPoint->flexPointHandle))->uri, ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"some_process") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"some_username") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_RESTART);
                CHECK(dbPoint->data.analog.value == value * ((FlexPoint *)dbPoint->flexPointHandle)->scale);
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - binary inputs - single with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        char *uri;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "{\"value\":%s}", value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "{\"value\":%s}", value ? "true" : "false");
                asprintf(&uri, "%s/%s", ((FlexPoint *)(dbPoint->flexPointHandle))->uri, ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"some_process") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"some_username") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(dbPoint->data.binary.value == value);
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - binary inputs - single with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        char *uri;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "{\"blah\":%s}", value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "{\"blah\":%s}", value ? "true" : "false");
                asprintf(&uri, "%s/%s", ((FlexPoint *)(dbPoint->flexPointHandle))->uri, ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_RESTART);
                CHECK(dbPoint->data.binary.value == false);
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - binary outputs - single with 'value'")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        char *uri;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "{\"value\":%s}", value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "{\"value\":%s}", value ? "true" : "false");
                asprintf(&uri, "%s/%s", ((FlexPoint *)(dbPoint->flexPointHandle))->uri, ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"some_process") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"some_username") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_RESTART);
                CHECK(dbPoint->data.binary.value == value);
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - invalid uri")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        char *uri;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "{\"value\":%s}", value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "{\"value\":%s}", value ? "true" : "false");
                asprintf(&uri, "/some/non_existent/uri%s/%s", ((FlexPoint *)(dbPoint->flexPointHandle))->uri, ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str());
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_RESTART);
                CHECK(dbPoint->data.binary.value == false);
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - invalid point")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        char *fims_message;
        char *uri;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                bool value = rand() % 2 == 0;
                asprintf(&fims_message, "{\"value\":%s}", value ? "true" : "false");
                sprintf(sys.fims_dependencies->data_buf, "{\"value\":%s}", value ? "true" : "false");
                asprintf(&uri, "%s", ((FlexPoint *)(dbPoint->flexPointHandle))->uri);
                meta_data.data_len = strlen(fims_message);
                sys.fims_dependencies->uri_view = std::string_view(uri);
                sys.fims_dependencies->process_name_view = std::string_view("some_process");
                sys.fims_dependencies->username_view = std::string_view("some_username");
                CHECK(!parseBodyServer(sys, meta_data));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"") == 0);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_RESTART);
                CHECK(dbPoint->data.binary.value == false);
                free(fims_message);
                free(uri);
            }
        }

        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("parseBody - invalid json message")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        Meta_Data_Info meta_data;
        sys.fims_dependencies->data_buf = (char *)malloc(1000);
        sys.fims_dependencies->data_buf[0] = '\0';
        std::string combinedJSON = ""; // Initialize the combined JSON object
        int value = 55;
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                char *pointJSON;
                asprintf(&pointJSON, "\"%s\":%d", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), value);
                strcat(sys.fims_dependencies->data_buf, pointJSON); // Append to the data_buf
                combinedJSON += pointJSON;                          // Append to the combined JSON
                free(pointJSON);
            }
        }

        meta_data.data_len = combinedJSON.length();
        sys.fims_dependencies->uri_view = std::string_view(((FlexPoint *)(dbPoint->flexPointHandle))->uri);
        sys.fims_dependencies->process_name_view = std::string_view("some_process");
        sys.fims_dependencies->username_view = std::string_view("some_username");
        CHECK(!parseBodyServer(sys, meta_data));
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_RESTART);
                CHECK(dbPoint->data.analog.value == static_cast<double>(0));
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_process,"") == 0);
                CHECK(strcmp(((FlexPoint *)(dbPoint->flexPointHandle))->last_update_username,"") == 0);
            }
        }
        free(sys.fims_dependencies->data_buf);
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("received_command_callback - analog outputs"){
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);
        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->reason = TMWDEFS_CHANGE_REMOTE_OP;
                dbPoint->data.analog.value = rand() % 100;
                received_command_callback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_40_ANA_OUT_STATUSES, dbPoint->pointNumber);
                CHECK(!((FlexPoint *)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value == dbPoint->data.analog.value);
            }
        }

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("received_command_callback - binary outputs"){
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);
        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->reason = TMWDEFS_CHANGE_REMOTE_OP;
                dbPoint->data.binary.value = rand() % 2 == 0;
                received_command_callback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_10_BIN_OUT_STATUSES, dbPoint->pointNumber);
                CHECK(!((FlexPoint *)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(dbPoint->data.binary.value));
            }
        }

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("received_command_callback - analog inputs, binary inputs, and counters"){
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);
        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->reason = TMWDEFS_CHANGE_REMOTE_OP;
                dbPoint->data.analog.value = rand() % 100;
                received_command_callback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_30_ANA_INPUTS, dbPoint->pointNumber);
                CHECK(!((FlexPoint *)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value == 0.0);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryCounters); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binCntrGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->reason = TMWDEFS_CHANGE_REMOTE_OP;
                dbPoint->data.binary.value = rand() % 100;
                received_command_callback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_20_RUNNING_CNTRS, dbPoint->pointNumber);
                CHECK(!((FlexPoint *)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value == 0.0);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->reason = TMWDEFS_CHANGE_REMOTE_OP;
                dbPoint->data.binary.value = rand() % 2 == 0;
                received_command_callback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_1_BIN_INPUTS, dbPoint->pointNumber);
                CHECK(!((FlexPoint *)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value == 0.0);
            }
        }

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("received_command_callback - analog outputs - interval set"){
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT *> points = setupParseBodyServerTest(sys);
        setIntervalSet(points);
        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->reason = TMWDEFS_CHANGE_REMOTE_OP;
                dbPoint->data.analog.value = rand() % 100;
                received_command_callback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_40_ANA_OUT_STATUSES, dbPoint->pointNumber);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value == dbPoint->data.analog.value);
                dbPoint->data.analog.value = rand() % 100;
                received_command_callback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_40_ANA_OUT_STATUSES, dbPoint->pointNumber);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value == dbPoint->data.analog.value);
            }
        }

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("received_command_callback - binary outputs - interval set"){
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT *> points = setupParseBodyServerTest(sys);
        setIntervalSet(points);
        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->reason = TMWDEFS_CHANGE_REMOTE_OP;
                dbPoint->data.binary.value = rand() % 2 == 0;
                received_command_callback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_10_BIN_OUT_STATUSES, dbPoint->pointNumber);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(dbPoint->data.binary.value));
                dbPoint->data.binary.value = rand() % 2 == 0;
                received_command_callback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_10_BIN_OUT_STATUSES, dbPoint->pointNumber);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(dbPoint->data.binary.value));
            }
        }

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("received_command_callback - analog inputs, binary inputs, and counters - interval set"){
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT *> points = setupParseBodyServerTest(sys);
        setIntervalSet(points);
        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.analog.value = rand() % 100;
                received_command_callback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_30_ANA_INPUTS, dbPoint->pointNumber);
                CHECK(!((FlexPoint *)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value == 0.0);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryCounters); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binCntrGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.counter.value = rand() % 100;
                received_command_callback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_20_RUNNING_CNTRS, dbPoint->pointNumber);
                CHECK(!((FlexPoint *)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value == 0.0);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.binary.value = rand() % 2 == 0;
                received_command_callback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_1_BIN_INPUTS, dbPoint->pointNumber);
                CHECK(!((FlexPoint *)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value == 0.0);
            }
        }

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("received_command_callback - analog outputs - batch set"){
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT *> points = setupParseBodyServerTest(sys);
        setBatchSet(points);
        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->reason = TMWDEFS_CHANGE_REMOTE_OP;
                dbPoint->data.analog.value = rand() % 100;
                received_command_callback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_40_ANA_OUT_STATUSES, dbPoint->pointNumber);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value == dbPoint->data.analog.value);
                dbPoint->data.analog.value = rand() % 100;
                received_command_callback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_40_ANA_OUT_STATUSES, dbPoint->pointNumber);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value == dbPoint->data.analog.value);
            }
        }

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("received_command_callback - binary outputs - batch set"){
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT *> points = setupParseBodyServerTest(sys);
        setBatchSet(points);
        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->reason = TMWDEFS_CHANGE_REMOTE_OP;
                dbPoint->data.binary.value = rand() % 2 == 0;
                received_command_callback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_10_BIN_OUT_STATUSES, dbPoint->pointNumber);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(dbPoint->data.binary.value));
                dbPoint->data.binary.value = rand() % 2 == 0;
                received_command_callback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_10_BIN_OUT_STATUSES, dbPoint->pointNumber);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value == static_cast<double>(dbPoint->data.binary.value));
            }
        }

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("received_command_callback - aanalog inputs, binary inputs, and counters - batch set"){
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT *> points = setupParseBodyServerTest(sys);
        setBatchSet(points);
        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.analog.value = rand() % 100;
                received_command_callback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_30_ANA_INPUTS, dbPoint->pointNumber);
                CHECK(!((FlexPoint *)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value == 0.0);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryCounters); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binCntrGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.counter.value = rand() % 100;
                received_command_callback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_20_RUNNING_CNTRS, dbPoint->pointNumber);
                CHECK(!((FlexPoint *)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value == 0.0);
            }
        }
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->data.binary.value = rand() % 2 == 0;
                received_command_callback(sys.protocol_dependencies->dnp3.dbHandle, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_1_BIN_INPUTS, dbPoint->pointNumber);
                CHECK(!((FlexPoint *)(dbPoint->flexPointHandle))->set_timer.active);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value == 0.0);
            }
        }

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("fimsSendSetCallback - analog outputs"){
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);
        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        fmt::memory_buffer send_buf;
        
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value = rand() % 100;
                FORMAT_TO_BUF(send_buf, R"({:.{}g})", ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value/((FlexPoint *)(dbPoint->flexPointHandle))->scale, std::numeric_limits<double>::max_digits10 - 1);
                fimsSendSetCallback(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work);
                CHECK(strncmp(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.send_buf.data(),send_buf.data(), ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.send_buf.size()) == 0);
                send_buf.clear();
            }
        }

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("fimsSendSetCallback - binary outputs"){
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);
        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        char *fims_message;
        
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value = rand() % 2 == 0;
                asprintf(&fims_message, "%s", ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value?"true":"false");
                fimsSendSetCallback(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work);
                CHECK(strncmp(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.send_buf.data(),fims_message, ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.send_buf.size()) == 0);
                free(fims_message);
            }
        }

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("fimsSendSetCallback - binary outputs - crob_int"){
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);
        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        char *fims_message;
        
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->crob_int = true;
                ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value = rand() % 2 == 0;
                asprintf(&fims_message, "%d", ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value?1:0);
                fimsSendSetCallback(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work);
                CHECK(strncmp(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.send_buf.data(),fims_message, ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.send_buf.size()) == 0);
                free(fims_message);
            }
        }

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("fimsSendSetCallback - binary outputs - crob_string"){
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);
        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        char *fims_message;
        
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->crob_string = true;
                ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value = rand() % 2 == 0;
                asprintf(&fims_message, "%s", ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value?"\"LATCH_ON\"":"\"LATCH_OFF\"");
                fimsSendSetCallback(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work);
                CHECK(strncmp(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.send_buf.data(),fims_message, ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.send_buf.size()) == 0);
                free(fims_message);
            }
        }

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("fimsSendSetCallback - binary outputs - scale negative"){
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);
        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        char *fims_message;
        
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->scale = -1;
                ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value = rand() % 2 == 0;
                asprintf(&fims_message, "%s", (!((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value)?"true":"false");
                fimsSendSetCallback(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work);
                CHECK(strncmp(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.send_buf.data(),fims_message, ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.send_buf.size()) == 0);
                free(fims_message);
            }
        }

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("fimsSendSetCallback - binary outputs - crob_int - scale negative"){
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);
        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        char *fims_message;
        
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->scale = -1;
                ((FlexPoint *)(dbPoint->flexPointHandle))->crob_int = true;
                ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value = rand() % 2 == 0;
                asprintf(&fims_message, "%d", (!((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value)?1:0);
                fimsSendSetCallback(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work);
                CHECK(strncmp(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.send_buf.data(),fims_message, ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.send_buf.size()) == 0);
                free(fims_message);
            }
        }

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("fimsSendSetCallback - binary outputs - crob_string - scale negative"){
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupParseBodyServerTest(sys);
        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        char *fims_message;
        
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->scale = -1;
                ((FlexPoint *)(dbPoint->flexPointHandle))->crob_string = true;
                ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value = rand() % 2 == 0;
                asprintf(&fims_message, "%s", (!((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value)?"\"LATCH_ON\"":"\"LATCH_OFF\"");
                fimsSendSetCallback(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work);
                CHECK(strncmp(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.send_buf.data(),fims_message, ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.send_buf.size()) == 0);
                free(fims_message);
            }
        }

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("fimsSendIntervalSetCallback - analog and binary outputs"){
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::vector<TMWSIM_POINT *> points = setupParseBodyServerTest(sys);
        setIntervalSet(points);
        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        fmt::memory_buffer send_buf;
        send_buf.clear();
        
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value = rand() % 100;
                FORMAT_TO_BUF(send_buf, R"({:.{}g})", ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value/((FlexPoint *)(dbPoint->flexPointHandle))->scale, std::numeric_limits<double>::max_digits10 - 1);
                fimsSendIntervalSetCallback(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work);
                CHECK(strncmp(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.send_buf.data(),send_buf.data(), ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.send_buf.size()) == 0);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_timer.active);
                send_buf.clear();
            }
        }
        
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value = rand() % 2 == 0;
                if (((FlexPoint *)(dbPoint->flexPointHandle))->scale < 0)
                {
                    FORMAT_TO_BUF(send_buf, R"({})", ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value?"false":"true");
                } else {
                    FORMAT_TO_BUF(send_buf, R"({})", ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value?"true":"false");
                }
                fimsSendIntervalSetCallback(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work);
                CHECK(strncmp(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.send_buf.data(),send_buf.data(), ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.send_buf.size()) == 0);
                CHECK(((FlexPoint *)(dbPoint->flexPointHandle))->set_timer.active);
                send_buf.clear();
            }
        }

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    
}
