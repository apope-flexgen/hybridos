#pragma once

#include "gcom_dnp3_fims.h"
#include <map>
#include <chrono>
#include <fstream>
#include "gcom_dnp3_system_structs.h"
extern "C"
{
#include "tmwscl/utils/tmwsim.h"
#include "tmwscl/utils/tmwchnl.h"
#include "tmwscl/utils/tmwtimer.h"
}

struct Heartbeat
{
    char *heartbeat_read_uri = nullptr;
    char *heartbeat_write_uri = nullptr;

    bool enabled = false;
    bool init = false;     // no action until we start getting action
    bool first_val = true; // no action until we start getting action

    double tLate; // when is it late
    double tAvg;
    double tMax;
    double tTotal;

    double last_time = 0.0;

    bool state_normal = true;
    bool state_fault = false;
    bool state_frozen = true;
    std::string state_str = "INIT";

    bool value_changed;
    u64 value = 0;
    u64 last_val = 0;
    int count;
    double frequency = 1000; // seconds (divided by 1000 at config read time)
    int timeout;             // in seconds
    int offset_time = 0;
    int max_value_int = 4096;
    u64 max_value = 4096;

    TMWSIM_POINT *heartbeat_write_point = nullptr;
    TMWSIM_POINT *heartbeat_read_point = nullptr;
    GcomSystem *sys = nullptr;
    TMWSESN *pSession = nullptr;
    TMWCHNL *pChannel = nullptr;
    TMWTIMER heartbeat_timer;
    std::mutex mtx;

    ~Heartbeat()
    {
        if (heartbeat_read_uri)
        {
            free(heartbeat_read_uri);
        }
        if (heartbeat_write_uri)
        {
            free(heartbeat_write_uri);
        }
    }
};

bool setupHeartbeatTimer(GcomSystem &sys);