#ifndef BATTERYBALANCINGUTILITY_HPP
#define BATTERYBALANCINGUTILITY_HPP

#include "asset.h"
#include "formatters.hpp"
#include <iostream>
#include <vector>
#include "asset.h"
#include "assetVar.h"
#include "channel.h"
#include "ESSLogger.hpp"
#include "chrono_utils.hpp"
#include "ess_utils.hpp"
#include "varMapUtils.h"
#include "formatters.hpp"
#include "scheduler.h"

namespace BatteryBalancingUtility
{
class RackInfo
{
public:
    struct ExtU  // inputs
    {
        double voltage = -1.0;
        double soc = -1.0;
        bool contactorStatus;
        bool ignoreExternal;  // ignoreExternal comes in as an input from the amap because something external is telling
                              // us to ignore this rack
        bool enableFeedback;
    };

    struct ExtY  // output struct
    {
        bool enableCmd;
        bool ignoreInternal;  // ignoreInternal is something that we set internally that we output to the amap so other
                              // things can see that we are ignoring this rack
    };

    // Copy Constructor
    RackInfo(RackInfo const&) = delete;

    // Assignment Operator
    RackInfo& operator=(RackInfo const&) & = delete;

    // Move Constructor
    RackInfo(RackInfo&&) = delete;

    // Move Assignment Operator
    RackInfo& operator=(RackInfo&&) = delete;

    // Constructor
    RackInfo(ExtU* inputs, ExtY* outputs);

    static void initialize();
    void step();

    ExtU* RackInputRef;
    ExtY* RackOutputRef;

    int rackNum;
};

// enumeration for state variable
enum States
{
    INIT,
    VOLTAGE_ARBITRATION,
    ACTIVE_POWER_BALANCING,
    CONTACTOR_CONTROL,
    FINE_BALANCE,
    END,
    ERR,
    DEFAULT  // so we can initialize the state variable to a "non working" value
};

enum ContactorRequestState
{
    RETURN_SUCCESS = 0,
    RETURN_IN_PROGRESS = 1,
    RETURN_FAIL = -1,
    DEFAULT_REQUEST_STATE = 9
};

enum ContactorControlStates
{
    BATTERY_RELAX,
    OPEN_CONTACTORS,
    RACK_FEEDBACK,
    CLOSE_CONTACTORS,
};

enum SiteControllerReporting
{
    Inactive = 1,
    In_Progress = 2,
    Failed = 4,
    Aborted = 8,
    Completed = 16,
    Exiting = 32
};

class BatteryBalancing final
{
public:
    // inputs
    struct ExtU
    {
        bool StartCmd;
        bool StopCmd;
        bool FineBalanceEnabled;
        bool debug;
        double MaxBalancePower;
        double PActl;             // current power being delivered through pcs
        int OpenContactorResult;  // Results default to 1 because of Contactor functions
        int CloseContactorResult;
        bool reset;

        ExtU()
            : StartCmd(false),
              StopCmd(false),
              FineBalanceEnabled(false),
              debug(false),
              MaxBalancePower(0.0),
              PActl(0.0),
              OpenContactorResult(1),
              CloseContactorResult(1),
              reset(false)
        {
        }
    };

    // outputs
    struct ExtY
    {
        int StateVariable;
        double PCmd;
        bool OpenContactorReq;
        bool CloseContactorReq;
        bool PcsStartReq;
        bool PcsStopReq;
        std::string errStr;

        ExtY()
            : StateVariable(States::DEFAULT),
              PCmd(0.0),
              OpenContactorReq(false),
              CloseContactorReq(false),
              PcsStartReq(false),
              PcsStopReq(false),
              errStr()
        {
        }
    };

    // constants are gotten from aV on setup. this structs constructor is not called during init so values will persist
    struct Constants
    {
        // this is used for ContactorControl timers
        double repTime;

        // ramp delta voltages
        double RampStartDeltaVoltage;
        double RampEndDeltaVoltage;
        double RackCloseDeltaVoltage;

        // timer limits
        double BatteryRelaxTime;
        double MinimumFeedbackDelayTime;
        double ActivePowerUpdateTimeMinimum;
        double ActionDelayTimeout;

        // attempt limits
        int MaxOpenContactorAttempts;
        int MaxCloseContactorAttempts;

        // this flag informs us if we have rack level contactor control or not
        bool RackLevelContactorControl;

        // this flag is used to determine if we are using the functionalized contactor control or not
        bool UseFunctionalizedContCls;

        // used for ramping up to MaxBalancePower
        double ActivePowerRampRatekWps;

        double MinRackBalancePower;

        // cutoff frequencies for inputs
        double VoltageFilterFC;

        Constants()
            : repTime(0.001),
              RampStartDeltaVoltage(2.5),
              RampEndDeltaVoltage(0.25),
              RackCloseDeltaVoltage(5),
              BatteryRelaxTime(60),
              MinimumFeedbackDelayTime(10),
              ActivePowerUpdateTimeMinimum(10),
              ActionDelayTimeout(20),
              MaxOpenContactorAttempts(2),
              MaxCloseContactorAttempts(2),
              RackLevelContactorControl(false),
              UseFunctionalizedContCls(false),
              ActivePowerRampRatekWps(10),
              MinRackBalancePower(1.5),
              VoltageFilterFC(0.05)
        {
        }
    };

    // local variables and other state for main state machine
    struct Balancing_DW
    {
        bool initialized;
        double avgVoltage;  // average rack voltage
        int numClosedRacks;
        bool configWrapperReset;
        int actionDelayTimer;

        // timer limits in iterations
        int batteryRelax_i;
        int delay_i;
        int actvpwrdelay_i;
        int actiondelay_i;

        // Minimum overall balance power based on number of closed racks
        double MinBalancePower;

        Balancing_DW()
            : initialized(false),
              avgVoltage(0.0),
              numClosedRacks(0),
              configWrapperReset(false),
              actionDelayTimer(0),
              batteryRelax_i(0),
              delay_i(0),
              actvpwrdelay_i(0),
              MinBalancePower(0.0)
        {
        }
    };

    // state variables for voltage arbitration state
    struct VoltageArbitration_DW
    {
        double targetVoltage;
        bool balancingNeeded;
        bool closingNeeded;
        std::vector<BatteryBalancingUtility::RackInfo*> targetRacks;

        VoltageArbitration_DW() : targetVoltage(0.0), balancingNeeded(false), closingNeeded(false), targetRacks() {}
    };

    struct ActivePowerBalancing_DW
    {
        double targetPower;
        bool stopBalancing;
        double deltaVoltageLast;
        bool rampStarted;
        double PCmdLast;
        double MaxPwrReached;
        int delayIter;
        bool reachedRamp;
        bool PcsOnCmdSent;
        bool timeoutDone;

        ActivePowerBalancing_DW()
            : targetPower(0.0),
              stopBalancing(false),
              deltaVoltageLast(0.0),
              rampStarted(false),
              PCmdLast(0.0),
              MaxPwrReached(0.0),
              delayIter(0),
              reachedRamp(false),
              PcsOnCmdSent(false),
              timeoutDone(false)
        {
        }
    };

    struct ContactorControl_DW
    {
        // transitioners
        int state;
        bool closingDone;

        // counters
        int batteryRelaxCounter;
        int feedbackTimer;
        int delayTimer;

        // contactor request flag
        bool reqSent;

        // failure trackers
        int failedOpens;
        int failedCloses;

        bool allTrgtClosed;
        bool useFunctionalizedContCls;
        bool pcsStopDone;
        bool openCmdSent;
        bool closeCmdSent;

        ContactorControl_DW()
            : state(ContactorControlStates::BATTERY_RELAX),
              closingDone(false),
              batteryRelaxCounter(0.0),
              feedbackTimer(0.0),
              delayTimer(0.0),
              reqSent(false),
              failedOpens(0),
              failedCloses(0),
              allTrgtClosed(false),
              useFunctionalizedContCls(false),
              pcsStopDone(false),
              openCmdSent(false),
              closeCmdSent(false)
        {
        }
    };

    struct FineBalance_DW
    {
        bool balanceDone;

        FineBalance_DW() : balanceDone(false) {}
    };

    struct End_DW
    {
        bool pcsStartDone;

        End_DW() : pcsStartDone(false) {}
    };

    // vector containing references to rack info objects.
    // this is how the battery balancing class gets access to the rack data without hard coding a number of racks
    std::vector<std::unique_ptr<BatteryBalancingUtility::RackInfo>> racks;

    // Copy Constructor
    BatteryBalancing(BatteryBalancing const&) = delete;

    // Assignment Operator
    BatteryBalancing& operator=(BatteryBalancing const&) & = delete;

    // Move Constructor
    BatteryBalancing(BatteryBalancing&&) = delete;

    // Move Assignment Operator
    BatteryBalancing& operator=(BatteryBalancing&&) = delete;

    // Constructor
    BatteryBalancing(ExtU* inputs_i, ExtY* outputs_o);

    // input and output reference
    ExtU* InputRef;
    ExtY* OutputRef;

    // this struct needs to be public so it can be accessed in interface file
    Constants Configs;
    Balancing_DW Dw;

    void initialize();
    void step();
    void VoltageArbitrationDu();
    void ActivePowerBalancingDu();
    void GetRackInfo();
    void InitEn();
    void InitDu();
    void InitEx();
    void VoltageArbitrationEn();
    void VoltageArbitrationEx();
    void ActivePowerBalancingEn();
    void ActivePowerBalancingEx();
    void ContactorControlEn();
    void ContactorControlDu();
    void ContactorControlEx();
    void FineBalanceEn();
    void FineBalanceDu();
    void FineBalanceEx();
    void EndEn();
    void EndDu();
    void EndEx();
    void ErrorDu();
    double ActivePowerRamp(double PTrgt);
    bool ActionDelayTimeout();

private:
    VoltageArbitration_DW VltArbDw;
    ActivePowerBalancing_DW ActvPwrDw;
    ContactorControl_DW ContCtrlDw;
    FineBalance_DW FineBalDw;
    End_DW EndDw;
};
}

// added global access to this map bc config wrapper needs access to it
extern std::unordered_map<std::string,
                          std::unordered_map<int, std::unique_ptr<BatteryBalancingUtility::BatteryBalancing>>>
    BatteryBalancingObjects;

#endif