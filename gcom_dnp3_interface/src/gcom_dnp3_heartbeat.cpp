#include <iostream>

extern "C"
{
#include "tmwscl/utils/tmwsim.h"
#include "tmwscl/utils/tmwtimer.h"
#include "tmwscl/dnp/mdnpbrm.h"
}
#include "gcom_dnp3_system_structs.h"
#include "gcom_dnp3_heartbeat.h"
#include "gcom_dnp3_utils.h"
#include "logger/gcom_dnp3_logger.h"

/// @brief
/// @param pHeartbeat
void heartbeatCallback(void *pHeartbeat)
{
    Heartbeat *heartbeat = static_cast<Heartbeat *>(pHeartbeat);
    GcomSystem *sys = heartbeat->sys;

    double tNow = get_time_double();
    TMWSIM_POINT *io_write_point = heartbeat->heartbeat_write_point;
    TMWSIM_POINT *io_read_point = heartbeat->heartbeat_read_point;

    heartbeat->mtx.lock();
    if (io_read_point)
    {
        // so if the update time moved but the value did not the component is connectd but stuck
        // so if the update time did not move we may not be connected
        sys->db_mutex.lock_shared();
        uint64_t raw_val = 0;
        if (io_read_point->type == TMWSIM_TYPE_ANALOG)
        {
            raw_val = static_cast<uint64_t>(io_read_point->data.analog.value);
        }
        else if (io_read_point->type == TMWSIM_TYPE_COUNTER)
        {
            raw_val = static_cast<uint64_t>(io_read_point->data.counter.value);
        }
        double tUpdate = get_time_double(io_read_point->timeStamp);
        sys->db_mutex.unlock_shared();

        if (heartbeat->first_val)
        {
            heartbeat->last_val = raw_val;
            heartbeat->last_time = tNow;
            if (tNow > (heartbeat->timeout / 1000.0))
            {
                heartbeat->first_val = false;
                heartbeat->state_frozen = false;
                heartbeat->state_fault = false;
                heartbeat->state_normal = false;
            }
        }

        // we have to pick up the first change before we monitor
        if (raw_val != heartbeat->last_val)
        {
            heartbeat->value_changed = true;
            heartbeat->last_val = raw_val;
            heartbeat->last_time = tNow;
            heartbeat->init = true;
            // set timeout
            // if was timedout then restore
        }
        bool normal = true;
        if (heartbeat->init)
        {
            if ((tNow - heartbeat->last_time) > (heartbeat->timeout / 1000.0))
            {
                if (!heartbeat->state_frozen)
                {

                    heartbeat->state_frozen = true;
                    heartbeat->state_str = "FROZEN";
                    std::string message = fmt::format("Read heartbeat [{}] has entered the [FROZEN] state.)", heartbeat->heartbeat_read_uri);
                    if (!spam_limit(sys, sys->heartbeat_errors))
                    {
                        FPS_ERROR_LOG("%s", message.c_str());
                    }
                    emit_event(&sys->fims_dependencies->fims_gateway, sys->fims_dependencies->name.c_str(), message.c_str(), 3);
                }
                normal = false;
            }
            if ((tNow - tUpdate) > (heartbeat->timeout / 1000.0))
            {
                if (!heartbeat->state_fault)
                {

                    heartbeat->state_fault = true;
                    heartbeat->state_str = "TIMEOUT";
                    std::string message = fmt::format("Read heartbeat [{}] has entered the [TIMEOUT] state.)", heartbeat->heartbeat_read_uri);
                    if (!spam_limit(sys, sys->heartbeat_errors))
                    {
                        FPS_ERROR_LOG("%s", message.c_str());
                    }
                    emit_event(&sys->fims_dependencies->fims_gateway, sys->fims_dependencies->name.c_str(), message.c_str(), 3);
                }
                normal = false;
            }
            if (normal)
            {
                if (!heartbeat->state_normal)
                {
                    if (heartbeat->state_frozen)
                    {
                        heartbeat->state_frozen = false;
                    }

                    if (heartbeat->state_fault)
                    {
                        heartbeat->state_fault = false;
                    }

                    heartbeat->state_normal = true;
                    heartbeat->state_str = "NORMAL";

                    std::string message = fmt::format("Read heartbeat [{}] has entered the [NORMAL] state.)", heartbeat->heartbeat_read_uri);
                    if (!spam_limit(sys, sys->heartbeat_errors))
                    {
                        FPS_INFO_LOG("%s", message.c_str());
                    }
                    emit_event(&sys->fims_dependencies->fims_gateway, sys->fims_dependencies->name.c_str(), message.c_str(), 3);
                }
            }
            else
            {
                heartbeat->state_normal = false;
            }
        }

        heartbeat->value = raw_val + 1;
    }
    else // no read point just source a value
    {
        heartbeat->value += 1;
    }

    if (heartbeat->max_value > 0)
        if (heartbeat->value > heartbeat->max_value)
            heartbeat->value = 0;

    if (heartbeat->enabled && io_read_point)
    {
        if (io_write_point)
        {
            double value = heartbeat->value;
            MDNPBRM_ANALOG_INFO analogValue;
            analogValue.pointNumber = io_write_point->pointNumber;

            analogValue.value.type = TMWTYPES_ANALOG_TYPE_DOUBLE;
            if (((FlexPoint *)(io_write_point->flexPointHandle))->scale == 0.0)
            {
                analogValue.value.value.dval = value;
            }
            else
            {
                analogValue.value.value.dval = value * ((FlexPoint *)(io_write_point->flexPointHandle))->scale;
            }

            mdnpbrm_analogCommand(&(((FlexPoint *)io_write_point->flexPointHandle)->sys)->protocol_dependencies->dnp3.pAnalogCommandRequestDesc, TMWDEFS_NULL, DNPDEFS_FC_DIRECT_OP, MDNPBRM_AUTO_MODE_NONE, 0,
                                  DNPDEFS_QUAL_16BIT_INDEX, io_write_point->defaultStaticVariation, 1, &analogValue);
        }
    }
    heartbeat->mtx.unlock();
    tmwtimer_start(&heartbeat->heartbeat_timer, heartbeat->frequency * 1000, heartbeat->pChannel, heartbeatCallback, heartbeat);
}
/// @brief
/// @param sys
/// @return
bool setupHeartbeatTimer(GcomSystem &sys)
{
    if (sys.heartbeat == nullptr)
    {
        return true;
    }
    {
        std::unique_lock<std::mutex> lk{sys.main_mutex};
        sys.main_cond.wait(lk, [&]()
                           { return sys.start_signal; });
    }

    // frequency is in seconds
    if (sys.heartbeat->heartbeat_write_point)
        FPS_INFO_LOG("Setting up heartbeat_write for [%s] every %f seconds", sys.heartbeat->heartbeat_write_uri, sys.heartbeat->frequency);
    if (sys.heartbeat->heartbeat_read_point)
        FPS_INFO_LOG("Setting up heartbeat_read for [%s] every %f seconds", sys.heartbeat->heartbeat_read_uri, sys.heartbeat->frequency);

    sys.heartbeat->pChannel = sys.protocol_dependencies->dnp3.pChannel;
    sys.heartbeat->pSession = sys.protocol_dependencies->dnp3.pSession;
    // timer reads in milliseconds
    tmwtimer_start(&sys.heartbeat->heartbeat_timer, sys.heartbeat->frequency * 1000, sys.protocol_dependencies->dnp3.pChannel, heartbeatCallback, sys.heartbeat);
    return true;
}