#include "doctest/doctest.h"
#include "../../include/gcom_dnp3_stats.h"
#include "../../include/gcom_dnp3_system_structs.h"

TEST_SUITE("dnp3_stats"){
    TEST_CASE("initStatsMonitor - client"){
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        sys.protocol_dependencies->who = DNP3_MASTER;
        DNP3Dependencies &dnp3_sys = sys.protocol_dependencies->dnp3;
        initStatsMonitor(sys);
        REQUIRE(dnp3_sys.channel_stats != nullptr);
        REQUIRE(dnp3_sys.session_stats != nullptr);
        REQUIRE(dnp3_sys.timings != nullptr);
        CHECK(dnp3_sys.channel_stats->who == DNP3_MASTER);
        CHECK(dnp3_sys.session_stats->who == DNP3_MASTER);
        CHECK(dnp3_sys.timings->who == DNP3_MASTER);
    } 
    TEST_CASE("initStatsMonitor - server"){
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        sys.protocol_dependencies->who = DNP3_OUTSTATION;
        DNP3Dependencies &dnp3_sys = sys.protocol_dependencies->dnp3;
        initStatsMonitor(sys);
        REQUIRE(dnp3_sys.channel_stats != nullptr);
        REQUIRE(dnp3_sys.session_stats != nullptr);
        REQUIRE(dnp3_sys.timings != nullptr);
        CHECK(dnp3_sys.channel_stats->who == DNP3_OUTSTATION);
        CHECK(dnp3_sys.session_stats->who == DNP3_OUTSTATION);
        CHECK(dnp3_sys.timings->who == DNP3_OUTSTATION);
    }       
}