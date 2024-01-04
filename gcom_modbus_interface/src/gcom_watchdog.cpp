// gcom_heartbeat.cpp
// p. wilshire
// 11_08_2023
// reviewed PSW 11_11_2023

#include <iostream>
#include "spdlog/fmt/fmt.h"
#include "gcom_config.h"
#include "gcom_iothread.h"
#include "gcom_timer.h"
#include "logger/logger.h"
#include "gcom_modbus_utils.h"

void cfg::watchdog_struct::setAlarmState()
{
    state_normal = false;
    state_fault = false;
    state_alarm = true;
    state_recovery = false;
    state_str = "ALARM";
    timeOfAlarm = get_time_double();

    std::string message = fmt::format("Watchdog [{}] has entered the [ALARM] state.", id);
    FPS_WARNING_LOG(message);
    emit_event(&cfg->fims_gateway, cfg->client_name.c_str(), message.c_str(), 3);
}

void cfg::watchdog_struct::setFaultState()
{
    state_normal = false;
    state_fault = true;
    state_alarm = false;
    state_recovery = false;
    state_str = "FAULT";
    timeOfFault = get_time_double();

    std::string message = fmt::format("Watchdog [{}] has entered the [FAULT] state.", id);
    FPS_ERROR_LOG(message);
    emit_event(&cfg->fims_gateway, cfg->client_name.c_str(), message.c_str(), 4);
}

void cfg::watchdog_struct::setRecoveryState()
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
    FPS_INFO_LOG(message);
    emit_event(&cfg->fims_gateway, cfg->client_name.c_str(), message.c_str(), 2);
}

void cfg::watchdog_struct::setNormalState()
{
    state_normal = true;
    state_fault = false;
    state_alarm = false;
    state_recovery = false;
    state_str = "NORMAL";

    std::string message = fmt::format("Watchdog [{}] has entered the [NORMAL] state.", id);
    FPS_INFO_LOG(message);
    emit_event(&cfg->fims_gateway, cfg->client_name.c_str(), message.c_str(), 2);
}

void cfg::watchdog_struct::disable()
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

void cfg::watchdog_struct::enable()
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

void cfg::watchdog_struct::touchWatchdog()
{
    // TODO we'll need to lock this
    auto raw_val = io_point->raw_val;

    if (value != raw_val)
    {
        watchdogStats.snap();
        watchdogStats.start();
        last_value_changed = value_changed;
        value_changed = get_time_double();
        value = raw_val;
    }
}

void watchdogCallback(std::shared_ptr<TimeObject>t, void *p)
{
    cfg::watchdog_struct *watchdog = static_cast<cfg::watchdog_struct *>(p);
    if (!watchdog->enabled)
    {
        return;
    }
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
    else if ((watchdog->state_fault || watchdog->state_alarm) && timeSinceLastChanged <= watchdog->alarmTimeout && timeBetweenChanges <= watchdog->alarmTimeout && watchdog->watchdogStats.count > 1)
    {
        watchdog->setRecoveryState();
    }
    else if (watchdog->state_recovery && watchdog->timeInRecovery >= watchdog->timeRequiredToRecover)
    {
        watchdog->setNormalState();
    }
}

void watchdogInitCallback(std::shared_ptr<TimeObject> t, void *p)
{
    cfg::watchdog_struct *watchdog = static_cast<cfg::watchdog_struct *>(p);
    if (watchdog->value_changed > 0)
    {
        watchdog->state_normal = true;
        watchdog->state_init = false;
        watchdog->state_str = "NORMAL";
        t->setCallback(watchdogCallback, p);
    }
}

void cfg::watchdog_struct::setupWatchdogTimer(struct cfg &myCfg, const std::string &name, double watchdog_frequency, double watchdog_offset)
{
    int init_time = 5;
    watchdogStats.start();
    std::shared_ptr<TimeObject> obj1 = createTimeObject(name,                 // name
                                                        init_time,            // start time (initial startup time)
                                                        0,                    // stop time - 0 = don't stop
                                                        watchdog_frequency,   // how often to repeat
                                                        0,                    // count - 0 = don't stop
                                                        watchdogInitCallback, // callback
                                                        (void *)this);        // callback params
    addTimeObject(obj1, watchdog_offset, false);
}