#include "doctest/doctest.h"
#include "add_points.h"

#include <filesystem>
#include <fstream>
#include <string>

#include "../../include/gcom_dnp3_system_structs.h"
#include "../../include/gcom_dnp3_flags.h"
#include "../../include/gcom_dnp3_utils.h"
#include "../../include/gcom_dnp3_tmw_utils.h"
#include "fims/defer.hpp"

bool first_test_dnp3_flags = true;

void setupServer(GcomSystem &sys){
    if(first_test_dnp3_flags){
        printf("Testing dnp3_flags.cpp...\n");
        first_test_dnp3_flags = false;
    }
    sys.protocol_dependencies->who = DNP3_OUTSTATION;
    sys.protocol_dependencies->ip_address = strdup("0.0.0.0");
    sys.id = strdup("test_dnp3_server");
    sys.protocol_dependencies->port = 20000;
    sys.protocol_dependencies->dnp3.point_status_info = new PointStatusInfo();
    sys.protocol_dependencies->dnp3.openTMWChannel = openTMWServerChannel;
    sys.protocol_dependencies->dnp3.openTMWSession = openTMWServerSession;
    init_tmw(sys);
    addPoints(sys);
}

void setupClient(GcomSystem &sys){
    if(first_test_dnp3_flags){
        printf("Testing dnp3_flags.cpp...\n");
        first_test_dnp3_flags = false;
    }
    sys.protocol_dependencies->ip_address = strdup("0.0.0.0");
    sys.id = strdup("test_dnp3_client");
    sys.protocol_dependencies->who = DNP3_MASTER;
    sys.protocol_dependencies->port = 20000;
    sys.protocol_dependencies->dnp3.point_status_info = new PointStatusInfo();
    sys.protocol_dependencies->dnp3.openTMWChannel = openTMWClientChannel;
    sys.protocol_dependencies->dnp3.openTMWSession = openTMWClientSession;
    init_tmw(sys);
    addPoints(sys);
}

TEST_SUITE("dnp3_flags")
{
    TEST_CASE("outputPointsGoOnline - server")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupServer(sys);

        outputPointsGoOnline(sys);
        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
            }
        }
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs == 15);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_online == 15);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart == 0);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_comm_lost == 0);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
            }
        }
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs == 10);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_online == 10);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_restart == 0);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_comm_lost == 0);

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("outputPointsGoOnline - server - no points")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        sys.protocol_dependencies->ip_address = strdup("0.0.0.0");
        sys.id = strdup("test_dnp3_server");
        sys.protocol_dependencies->who = DNP3_OUTSTATION;
        sys.protocol_dependencies->port = 20000;
        sys.protocol_dependencies->dnp3.point_status_info = new PointStatusInfo();
        sys.protocol_dependencies->dnp3.openTMWChannel = openTMWServerChannel;
        sys.protocol_dependencies->dnp3.openTMWSession = openTMWServerSession;
        init_tmw(sys);

        outputPointsGoOnline(sys);
        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                FAIL_CHECK("found an enabled analog output point (should have none)");
            }
        }
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs == 0);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_online == 0);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart == 0);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_comm_lost == 0);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                FAIL_CHECK("found an enabled binary output point (should have none)");
            }
        }
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs == 0);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_online == 0);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_restart == 0);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_comm_lost == 0);

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("outputPointsGoOnline - client")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupClient(sys);

        outputPointsGoOnline(sys);
        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_OFF_LINE);
            }
        }
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs == 15);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_online == 0);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart == 15);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_comm_lost == 0);

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_binaryOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_OFF_LINE);
            }
        }
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs == 10);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_online == 0);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_restart == 10);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_comm_lost == 0);

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("pointTimeout - server - all points start online")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupServer(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_online++;
                sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart--;
            }
        }
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_online == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_comm_lost == 0);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_online++;
                sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart--;
            }
        }
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_online == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_comm_lost == 0);

        int count = 0;
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                count++;
                PointTimeoutStruct *pPointTimeoutStruct = new PointTimeoutStruct();
                pPointTimeoutStruct->dbPoint = dbPoint;
                pPointTimeoutStruct->dnp3_sys = dnp3_sys;
                pointTimeout(pPointTimeoutStruct);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_COMM_LOST);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs == 5);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_online == (5-count));
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart == 0);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_comm_lost == count);
                delete pPointTimeoutStruct;
            }
        }

        count = 0;
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                count++;
                PointTimeoutStruct *pPointTimeoutStruct = new PointTimeoutStruct();
                pPointTimeoutStruct->dbPoint = dbPoint;
                pPointTimeoutStruct->dnp3_sys = dnp3_sys;
                pointTimeout(pPointTimeoutStruct);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_COMM_LOST);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs == 5);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_online == (5-count));
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart == 0);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_comm_lost == count);
                delete pPointTimeoutStruct;
            }
        }
        
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("pointTimeout - server - all points in restart")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupServer(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;

        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_online == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_comm_lost == 0);

        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_online == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_comm_lost == 0);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                PointTimeoutStruct *pPointTimeoutStruct = new PointTimeoutStruct();
                pPointTimeoutStruct->dbPoint = dbPoint;
                pPointTimeoutStruct->dnp3_sys = dnp3_sys;
                pointTimeout(pPointTimeoutStruct);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_RESTART);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs == 5);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_online == 0);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart == 5);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_comm_lost == 0);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                PointTimeoutStruct *pPointTimeoutStruct = new PointTimeoutStruct();
                pPointTimeoutStruct->dbPoint = dbPoint;
                pPointTimeoutStruct->dnp3_sys = dnp3_sys;
                pointTimeout(pPointTimeoutStruct);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_RESTART);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs == 5);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_online == 0);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart == 5);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_comm_lost == 0);
            }
        }
        
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("initTimers - server")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupServer(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                REQUIRE(!((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer.active);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                REQUIRE(!((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer.active);
            }
        }

        initTimers(sys);
        
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer.active);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer.active);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(!((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer.active);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(!((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer.active);
            }
        }

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("initTimers - client")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupClient(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                REQUIRE(!((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer.active);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                REQUIRE(!((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer.active);
            }
        }

        initTimers(sys);
        
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(!((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer.active);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(!((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer.active);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(!((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer.active);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_binaryOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                CHECK(!((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer.active);
            }
        }   

        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("setInputPointOnline - server - all points start online")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupServer(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_online++;
                sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart--;
            }
        }
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_online == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_comm_lost == 0);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_online++;
                sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart--;
            }
        }
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_online == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_comm_lost == 0);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                PointTimeoutStruct *pPointTimeoutStruct = new PointTimeoutStruct();
                pPointTimeoutStruct->dbPoint = dbPoint;
                pPointTimeoutStruct->dnp3_sys = dnp3_sys;
                ((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer.pCallbackParam = pPointTimeoutStruct;
                setInputPointOnline(dbPoint);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs == 5);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_online == 5);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart == 0);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_comm_lost == 0);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                PointTimeoutStruct *pPointTimeoutStruct = new PointTimeoutStruct();
                pPointTimeoutStruct->dbPoint = dbPoint;
                pPointTimeoutStruct->dnp3_sys = dnp3_sys;
                ((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer.pCallbackParam = pPointTimeoutStruct;
                setInputPointOnline(dbPoint);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs == 5);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_online == 5);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart == 0);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_comm_lost == 0);
            }
        }
        
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("setInputPointOnline - server - all points in restart")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupServer(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;

        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_online == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_comm_lost == 0);

        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_online == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_comm_lost == 0);

        int count = 0;
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                count++;
                PointTimeoutStruct *pPointTimeoutStruct = new PointTimeoutStruct();
                pPointTimeoutStruct->dbPoint = dbPoint;
                pPointTimeoutStruct->dnp3_sys = dnp3_sys;
                ((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer.pCallbackParam = pPointTimeoutStruct;
                setInputPointOnline(dbPoint);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs == 5);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_online == count);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart == (5-count));
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_comm_lost == 0);
            }
        }

        count = 0;
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                count++;
                PointTimeoutStruct *pPointTimeoutStruct = new PointTimeoutStruct();
                pPointTimeoutStruct->dbPoint = dbPoint;
                pPointTimeoutStruct->dnp3_sys = dnp3_sys;
                ((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer.pCallbackParam = pPointTimeoutStruct;
                setInputPointOnline(dbPoint);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs == 5);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_online == count);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart == (5-count));
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_comm_lost == 0);
            }
        }
        
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("setInputPointOnline - server - all points in comm_lost")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupServer(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->flags = DNPDEFS_DBAS_FLAG_COMM_LOST;
                sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_comm_lost++;
                sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart--;
            }
        }
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_online == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_comm_lost == 5);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->flags = DNPDEFS_DBAS_FLAG_COMM_LOST;
                sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_comm_lost++;
                sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart--;
            }
        }
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_online == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_comm_lost == 5);

        int count = 0;
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                count++;
                PointTimeoutStruct *pPointTimeoutStruct = new PointTimeoutStruct();
                pPointTimeoutStruct->dbPoint = dbPoint;
                pPointTimeoutStruct->dnp3_sys = dnp3_sys;
                ((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer.pCallbackParam = pPointTimeoutStruct;
                setInputPointOnline(dbPoint);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs == 5);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_online == count);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart == 0);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_comm_lost == (5-count));
            }
        }

        count = 0;
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                count++;
                PointTimeoutStruct *pPointTimeoutStruct = new PointTimeoutStruct();
                pPointTimeoutStruct->dbPoint = dbPoint;
                pPointTimeoutStruct->dnp3_sys = dnp3_sys;
                ((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer.pCallbackParam = pPointTimeoutStruct;
                setInputPointOnline(dbPoint);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs == 5);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_online == count);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart == 0);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_comm_lost == (5-count));
            }
        }
        
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("setInputPointOnline - server - don't init point timeout struct")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupServer(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->flags = DNPDEFS_DBAS_FLAG_COMM_LOST;
                sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_comm_lost++;
                sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart--;
            }
        }
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_online == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_comm_lost == 5);

        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->flags = DNPDEFS_DBAS_FLAG_COMM_LOST;
                sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_comm_lost++;
                sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart--;
            }
        }
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_online == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_comm_lost == 5);

        int count = 0;
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                count++;
                setInputPointOnline(dbPoint);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs == 5);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_online == count);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart == 0);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_comm_lost == (5-count));
            }
        }

        count = 0;
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                count++;
                setInputPointOnline(dbPoint);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs == 5);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_online == count);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart == 0);
                CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_comm_lost == (5-count));
            }
        }
        
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
        if (sys.protocol_dependencies->dnp3.point_status_info != nullptr)
        {
            delete sys.protocol_dependencies->dnp3.point_status_info;
        }
    }
    TEST_CASE("checkPointCommLost - client")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupClient(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->flags = DNPDEFS_DBAS_FLAG_COMM_LOST;
                checkPointCommLost(dbPoint);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_COMM_LOST);
                CHECK(((FlexPoint *)dbPoint->flexPointHandle)->lastFlags == DNPDEFS_DBAS_FLAG_COMM_LOST);
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                checkPointCommLost(dbPoint);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(((FlexPoint *)dbPoint->flexPointHandle)->lastFlags == DNPDEFS_DBAS_FLAG_ON_LINE);
                dbPoint->flags = DNPDEFS_DBAS_FLAG_COMM_LOST;
                checkPointCommLost(dbPoint);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_COMM_LOST);
                CHECK(((FlexPoint *)dbPoint->flexPointHandle)->lastFlags == DNPDEFS_DBAS_FLAG_COMM_LOST);
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->flags = DNPDEFS_DBAS_FLAG_COMM_LOST;
                checkPointCommLost(dbPoint);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_COMM_LOST);
                CHECK(((FlexPoint *)dbPoint->flexPointHandle)->lastFlags == DNPDEFS_DBAS_FLAG_COMM_LOST);
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
                checkPointCommLost(dbPoint);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_ON_LINE);
                CHECK(((FlexPoint *)dbPoint->flexPointHandle)->lastFlags == DNPDEFS_DBAS_FLAG_ON_LINE);
                dbPoint->flags = DNPDEFS_DBAS_FLAG_COMM_LOST;
                checkPointCommLost(dbPoint);
                CHECK(dbPoint->flags == DNPDEFS_DBAS_FLAG_COMM_LOST);
                CHECK(((FlexPoint *)dbPoint->flexPointHandle)->lastFlags == DNPDEFS_DBAS_FLAG_COMM_LOST);
            }
        }
        
        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
    TEST_CASE("updatePointStatus - client")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        setupClient(sys);

        DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;
        TMWSIM_POINT *dbPoint = nullptr;

        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs == 15);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs == 10);

        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_online == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_online == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_online == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_online == 0);

        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart == 5);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart == 15);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_restart == 10);

        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_comm_lost == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_comm_lost == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_comm_lost == 0);
        REQUIRE(sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_comm_lost == 0);


        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_analogInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->flags = DNPDEFS_DBAS_FLAG_COMM_LOST;
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_binaryInputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->flags = DNPDEFS_DBAS_FLAG_COMM_LOST;
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_analogOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
            }
        }

        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_binaryOutputGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint)
            {
                dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
            }
        }

        updatePointStatus(sys);
        
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs == 5);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs == 5);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs == 15);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs == 10);

        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_online == 0);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_online == 0);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_online == 15);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_online == 10);

        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart == 0);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart == 0);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart == 0);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_restart == 0);

        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_comm_lost == 5);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_comm_lost == 5);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_comm_lost == 0);
        CHECK(sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_comm_lost == 0);


        shutdown_tmw(&sys.protocol_dependencies->dnp3);
    }
}
