#pragma once
#include <string>
#include <cstdint>
extern "C"
{
#include "tmwscl/utils/tmwsim.h"
#include "tmwscl/utils/tmwchnl.h"
#include "tmwscl/utils/tmwtimer.h"
}

struct GcomSystem;

struct Watchdog
{
    char *id;
    bool enabled = false;

    double faultTimeout = DBL_MAX;
    double timeOfFault = 0;
    double maxFaultToRecoveryTime = 0;

    double alarmTimeout = DBL_MAX;
    double timeOfAlarm = 0;
    double maxAlarmToRecoveryTime = 0;

    double recoveryTimeout = DBL_MAX;
    double timeOfRecoveryPeriodStart = 0;
    double timeInRecovery = 0;

    double timeRequiredToRecover = 0;

    bool state_init = true;
    bool state_normal = false;
    bool state_fault = false;
    bool state_alarm = false;
    bool state_recovery = false;
    std::string state_str = "INIT";

    uint64_t value = 0;
    double value_changed = 0;
    double last_value_changed = 0;
    int frequency = 1000;
    int offset_time = 0;
    int count = 0;

    TMWSIM_POINT *io_point = nullptr;
    GcomSystem *sys = nullptr;
    TMWCHNL *pChannel = nullptr;
    TMWSESN *pSession = nullptr;
    TMWTIMER watchdog_timer;
    std::mutex mtx;

    void touchWatchdog();
    void checkWatchdog();
    void setNormalState();
    void setFaultState();
    void setAlarmState();
    void setRecoveryState();
    void disable();
    void enable();

    ~Watchdog(){
        if(id){
            delete id;
        }
    }
};

bool setupWatchdogTimer(GcomSystem &sys);
