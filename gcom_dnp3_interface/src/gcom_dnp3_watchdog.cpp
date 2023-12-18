#include <iostream>
#include "spdlog/fmt/fmt.h"

extern "C"
{
#include "tmwscl/utils/tmwtimer.h"

#include "tmwscl/dnp/dnpchnl.h"
#include "tmwscl/dnp/mdnpsesn.h"
#include "tmwscl/dnp/mdnpbrm.h"
#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpsim.h"
#include "tmwscl/dnp/sdnputil.h"
#include "tmwtargio.h"
}
#include "gcom_dnp3_system_structs.h"
#include "logger/gcom_dnp3_logger.h"
#include "gcom_dnp3_utils.h"
/// @brief 
void Watchdog::setAlarmState()
{
    state_normal = false;
    state_fault = false;
    state_alarm = true;
    state_recovery = false;
    state_str = "ALARM";
    timeOfAlarm = get_time_double();

    std::string message = fmt::format("Watchdog [{}] has entered the [ALARM] state.", id);
    if(!spam_limit(sys, sys->watchdog_errors))
    {
        FPS_WARNING_LOG(message);
    }
    emit_event(&sys->fims_dependencies->fims_gateway, sys->fims_dependencies->name.c_str(), message.c_str(), 3);
}
/// @brief 
void Watchdog::setFaultState()
{
    state_normal = false;
    state_fault = true;
    state_alarm = false;
    state_recovery = false;
    state_str = "FAULT";
    timeOfFault = get_time_double();
    std::string message = fmt::format("Watchdog [{}] has entered the [FAULT] state.", id);
    if(!spam_limit(sys, sys->watchdog_errors))
    {
        FPS_ERROR_LOG(message);
    }
    emit_event(&sys->fims_dependencies->fims_gateway, sys->fims_dependencies->name.c_str(), message.c_str(), 3);
}

void Watchdog::setRecoveryState()
{
    double tNow = get_time_double();
    double faultToRecoveryTime = tNow - timeOfFault;
    if (state_fault && faultToRecoveryTime > maxFaultToRecoveryTime)
    {
        maxFaultToRecoveryTime = faultToRecoveryTime;
    }
    double alarmToRecoveryTime = tNow - timeOfAlarm;
    if (state_alarm && alarmToRecoveryTime > maxAlarmToRecoveryTime)
    {
        maxAlarmToRecoveryTime = alarmToRecoveryTime;
    }
    timeOfRecoveryPeriodStart = tNow;
    state_normal = false;
    state_fault = false;
    state_alarm = false;
    state_recovery = true;
    state_str = "RECOVERY";

    std::string message = fmt::format("Watchdog [{}] has entered the [RECOVERY] state.", id);
    if(!spam_limit(sys, sys->watchdog_errors))
    {
        FPS_INFO_LOG(message);
    }
    emit_event(&sys->fims_dependencies->fims_gateway, sys->fims_dependencies->name.c_str(), message.c_str(), 3);
}

void Watchdog::setNormalState()
{
    state_init = false;
    state_normal = true;
    state_fault = false;
    state_alarm = false;
    state_recovery = false;
    state_str = "NORMAL";

    std::string message = fmt::format("Watchdog [{}] has entered the [NORMAL] state.", id);
    if(!spam_limit(sys, sys->watchdog_errors))
    {
        FPS_INFO_LOG(message);
    }
    emit_event(&sys->fims_dependencies->fims_gateway, sys->fims_dependencies->name.c_str(), message.c_str(), 3);

}
/// @brief 
void Watchdog::disable()
{
    enabled = false;
    timeOfFault = 0;
    timeOfAlarm = 0;
    timeOfRecoveryPeriodStart = 0;

    if (!state_init)
    {
        state_normal = true;
        state_fault = false;
        state_alarm = false;
        state_recovery = false;
    }

    state_str = "DISABLED";

    value = 0;
    value_changed = 0;
    last_value_changed = 0;
}
/// @brief 
void Watchdog::enable()
{
    if (state_init)
    {
        state_str = "INIT";
    }
    else
    {
        state_str = "NORMAL";
    }
    enabled = true;
}
/// @brief 
void Watchdog::touchWatchdog()
{
    sys->db_mutex.lock_shared();
    uint64_t raw_val = static_cast<uint64_t>(io_point->data.analog.value);

    if (value != raw_val)
    {
        count += 1;
        last_value_changed = value_changed;
        value_changed = get_time_double(io_point->timeStamp);
        value = raw_val;
    }
    sys->db_mutex.unlock_shared();
}
/// @brief 
/// @param pWatchdog 
void watchdogCallback(void *pWatchdog)
{
    Watchdog *watchdog = static_cast<Watchdog *>(pWatchdog);
    watchdog->mtx.lock();
    if (!watchdog->enabled)
    {
        tmwtimer_start(&watchdog->watchdog_timer, watchdog->frequency, watchdog->pChannel, watchdogCallback, pWatchdog);
    }
    watchdog->touchWatchdog();
    double tNow = get_time_double();
    double timeSinceLastChanged = tNow - watchdog->value_changed;
    double timeBetweenChanges = watchdog->value_changed - watchdog->last_value_changed;
    watchdog->timeInRecovery = tNow - watchdog->timeOfRecoveryPeriodStart;

    if (watchdog->state_normal && timeSinceLastChanged >= watchdog->alarmTimeout)
    {
        watchdog->setAlarmState();
    }
    else if ((watchdog->state_alarm && timeSinceLastChanged >= watchdog->faultTimeout) || (watchdog->state_recovery && timeSinceLastChanged >= watchdog->recoveryTimeout))
    {
        watchdog->setFaultState();
    }
    else if ((watchdog->state_fault || watchdog->state_alarm) && timeSinceLastChanged <= watchdog->alarmTimeout && timeBetweenChanges <= watchdog->alarmTimeout && watchdog->count > 1)
    {
        watchdog->setRecoveryState();
    }
    else if (watchdog->state_recovery && watchdog->timeInRecovery >= watchdog->timeRequiredToRecover)
    {
        watchdog->setNormalState();
    }
    watchdog->mtx.unlock();
    tmwtimer_start(&watchdog->watchdog_timer, watchdog->frequency, watchdog->pChannel, watchdogCallback, pWatchdog);  
}

/// @brief 
/// @param sys 
/// @return 
bool setupWatchdogTimer(GcomSystem &sys)
{
    if (sys.watchdog == nullptr){
        return true;
    }

    FPS_INFO_LOG("Setting up watchdog for point [%s] every %f seconds", sys.watchdog->id, sys.watchdog->frequency/1000.0);
    {
        std::unique_lock<std::mutex> lk{sys.main_mutex};
        sys.main_cond.wait(lk, [&]()
                           { return sys.start_signal; });
    }

    sys.watchdog->enable();
    sys.watchdog->state_init = false;
    sys.watchdog->state_normal = true;
    sys.watchdog->state_fault = false;
    sys.watchdog->state_alarm = false;
    sys.watchdog->state_recovery = false;
    sys.watchdog->state_str = "INIT";
    sys.watchdog->pChannel = sys.protocol_dependencies->dnp3.pChannel;
    sys.watchdog->pSession = sys.protocol_dependencies->dnp3.pSession;
    tmwtimer_start(&sys.watchdog->watchdog_timer, sys.watchdog->frequency, sys.protocol_dependencies->dnp3.pChannel, watchdogCallback, sys.watchdog);
    return true;
}