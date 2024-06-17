#ifndef BATTERYBALANCINGUTILITY_CPP
#define BATTERYBALANCINGUTILITY_CPP

#include "asset.h"
#include "formatters.hpp"
#include <iostream>
#include "BatteryBalancingUtility.hpp"
#include <cstdlib>
#include <ctime>

using namespace BatteryBalancingUtility;

void BatteryBalancing::VoltageArbitrationDu()
{
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);

    // ==================== Rack Closure Status Check ====================
    bool openRacks = false;
    bool closeableRacks = false;

    for (const auto& uqRackptr : racks)
    {
        RackInfo* rackptr = uqRackptr.get();
        if (rackptr->RackInputRef->contactorStatus == false &&
            !(rackptr->RackOutputRef->ignoreInternal || rackptr->RackInputRef->ignoreExternal))
        {
            // Amongst open racks, find those with voltages outside of the contactor closure voltage
            // and record those voltages for later.
            // VltArbDw.closingNeeded = true;
            double rackV = rackptr->RackInputRef->voltage;
            if (InputRef->debug)
                FPS_PRINT_INFO("[{}] found a rack needing adjustment. Voltage is {}", __func__, rackV);
            if (std::abs(rackV - Dw.avgVoltage) >= Configs.RackCloseDeltaVoltage)
            {
                VltArbDw.targetRacks.push_back(
                    rackptr);  // these are all the possible targetRacks, this vector is shortened later in code
                VltArbDw.balancingNeeded = true;
            }
            else
            {
                closeableRacks = true;
            }
            openRacks = true;
        }
        else if (rackptr->RackInputRef->contactorStatus &&
                 !(rackptr->RackOutputRef->ignoreInternal || rackptr->RackInputRef->ignoreExternal))
        {
            VltArbDw.targetRacks.push_back(rackptr);
        }
    }
    if (!openRacks)
    {
        VltArbDw.balancingNeeded = false;
        VltArbDw.closingNeeded = false;
        return;
    }
    else if (closeableRacks)
    {
        VltArbDw.closingNeeded = true;
        return;  // if any racks are within the rack close delta voltage then try to close them first before we go
                 // moving power around.
    }

    if (Dw.numClosedRacks == 0)
    {
        // We attempt to close contactors in the beginning of the state machine if all are open
        // So if we get here and no contactors are closed then we've got an error and need to abort.
        OutputRef->StateVariable = States::ERR;
        OutputRef->errStr = "Voltage Arbitration: No racks with closed contactors";
        return;
    }

    // find the minimum voltage of our target racks
    double minVoltage = std::numeric_limits<double>::max();   // Initialize minVoltage to maximum possible value
    double maxVoltage = -std::numeric_limits<double>::max();  // Initialize maxVoltage to minimum possible value

    for (const RackInfo* rackPtr : VltArbDw.targetRacks)
    {
        if (rackPtr->RackInputRef->voltage < minVoltage && !rackPtr->RackInputRef->contactorStatus)
        {
            minVoltage = rackPtr->RackInputRef->voltage;
        }

        if (rackPtr->RackInputRef->voltage > maxVoltage && !rackPtr->RackInputRef->contactorStatus)
        {
            maxVoltage = rackPtr->RackInputRef->voltage;
        }
    }

    // determine if our avg voltage is closer to the max voltage or min voltage
    double minVoltageDiffFromClosed = std::abs(Dw.avgVoltage - minVoltage);
    double maxVoltageDiffFromClosed = std::abs(Dw.avgVoltage - maxVoltage);
    double min = std::min(minVoltageDiffFromClosed, maxVoltageDiffFromClosed);

    // determine if we want to use min voltage or max voltage as our target voltage
    double targV = (min == minVoltageDiffFromClosed) ? minVoltage : maxVoltage;
    VltArbDw.targetVoltage = targV;

    // Remove RackInfo objects with voltage not equal to targetVoltage using the erase-remove idiom
    VltArbDw.targetRacks.erase(std::remove_if(VltArbDw.targetRacks.begin(), VltArbDw.targetRacks.end(),
                                              // this is a lambda function that if returns true removes the rack it is
                                              // iterating on from the targetRacks vector
                                              [targV](const RackInfo* rackPtr) {
                                                  return rackPtr->RackInputRef->voltage != targV &&
                                                         !rackPtr->RackInputRef->contactorStatus;

                                                  // TODO: later, allow for a range of voltages to acceptable for our
                                                  // targetRacks
                                              }),
                               VltArbDw.targetRacks.end());

    // now our targetRacks vector only has racks that have our target voltage

    if (InputRef->debug)
    {
        // print all the variables we used
        FPS_PRINT_INFO("closedVoltage {}", Dw.avgVoltage);
        FPS_PRINT_INFO("minVoltage {}", minVoltage);
        FPS_PRINT_INFO("minVoltage {}", maxVoltage);
        FPS_PRINT_INFO("minVoltageDiffFromClosed {}", minVoltageDiffFromClosed);
        FPS_PRINT_INFO("maxVoltageDiffFromClosed {}", maxVoltageDiffFromClosed);
        FPS_PRINT_INFO("min {}", min);
        FPS_PRINT_INFO("targetVoltage {}", VltArbDw.targetVoltage);

        FPS_PRINT_INFO("Closing needed: {}", VltArbDw.closingNeeded);
        FPS_PRINT_INFO("Balancing needed: {}", VltArbDw.balancingNeeded);
        FPS_PRINT_INFO("        ---TargetRacks---", NULL);
        for (auto rack : VltArbDw.targetRacks)
        {
            FPS_PRINT_INFO(" | Rack_{}: voltage: {} | soc: {} | contactorStatus: {} |", rack->rackNum,
                           rack->RackInputRef->voltage, rack->RackInputRef->soc, rack->RackInputRef->contactorStatus);
        }
    }
}

void BatteryBalancing::ActivePowerBalancingDu()
{
    // ActivePowerBalancingOutput output;
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);
    ActvPwrDw.stopBalancing = false;
    ActvPwrDw.targetPower = 0.0;

    if(!ActvPwrDw.PcsOnCmdSent)
    {
        OutputRef->PcsStartReq = true;
    }
    if(!ActvPwrDw.timeoutDone)
    {
        if(!ActionDelayTimeout())
        {
            return;
        }
    }
    ActvPwrDw.timeoutDone = true;
    OutputRef->PcsStartReq = false;
    //TODO check pcs start feedback

    double sign = 1.0;
    if (Dw.avgVoltage < VltArbDw.targetVoltage)
    {
        // charge case
        sign = -1.0;
    }

    double deltaVoltage = std::abs(Dw.avgVoltage - VltArbDw.targetVoltage);

    //
    // rampStartRackCloseDeltaVoltage
    double PCmdTry = ActvPwrDw.PCmdLast;
    // Only update power command every update seconds, but still let power command ramp every repTime seconds. 
    FPS_PRINT_INFO("delayIter: {}, actvpwrdelay_i: {}, deltaVoltage: {}, reachedRamp: {}, PCmdLast: {}", ActvPwrDw.delayIter, Dw.actvpwrdelay_i, deltaVoltage, ActvPwrDw.reachedRamp, ActvPwrDw.PCmdLast);
    FPS_PRINT_INFO("RampStartDeltaV: {}, RampEndDeltaV: {}", Configs.RampStartDeltaVoltage, Configs.RampEndDeltaVoltage);
    if (ActvPwrDw.delayIter < Dw.actvpwrdelay_i)
    {
        ActvPwrDw.delayIter++;
        if (std::abs(PCmdTry) > std::abs(InputRef->MaxBalancePower))
        {
            PCmdTry = sign * InputRef->MaxBalancePower;
        }
    }
    else
    {
        ActvPwrDw.delayIter = 0;
        if (deltaVoltage > Configs.RampStartDeltaVoltage )
        {
            if (InputRef->debug)
                FPS_PRINT_INFO("RAMP START", nullptr);
            if(!ActvPwrDw.reachedRamp) 
            {
                PCmdTry = sign * InputRef->MaxBalancePower;
            }
        }
        else if (deltaVoltage <= Configs.RampStartDeltaVoltage && deltaVoltage > Configs.RampEndDeltaVoltage)
        {
            ActvPwrDw.reachedRamp = true;
            if (InputRef->debug)
                FPS_PRINT_INFO("TARGET VOLTAGE", nullptr);
            double x = deltaVoltage;
            double divisor = (Configs.RampStartDeltaVoltage - Configs.RampEndDeltaVoltage);
            if (divisor == 0)
            {
                divisor = 0.0000000001;
            }
            double m = (std::abs(ActvPwrDw.MaxPwrReached) / divisor);
            double b = -1 * Configs.RampEndDeltaVoltage * m;
            double y = (m * x) + b;

            PCmdTry = sign * y;

            // MaxBalancePower can change iteration to iteration, so make sure we don't violate that here. 
            // especially if in the ramping regime.
            if(std::abs(PCmdTry) > std::abs(InputRef->MaxBalancePower))
            {
                PCmdTry = sign * InputRef->MaxBalancePower;
            } 
        }
        else
        {
            if (InputRef->debug) 
                FPS_PRINT_INFO("STOP", nullptr);
            ActvPwrDw.stopBalancing = true;
        }
        // Only allow pcmd to change if it is closer to zero than our previous guess or we have gone outside of our hysteresis band. 
        if(std::abs(ActvPwrDw.PCmdLast) < std::abs(PCmdTry) && ActvPwrDw.reachedRamp)
        {
            PCmdTry = ActvPwrDw.PCmdLast;
        }
    }
    if(std::abs(PCmdTry) < Dw.MinBalancePower && ActvPwrDw.reachedRamp)
    {
        PCmdTry = Dw.MinBalancePower < InputRef->MaxBalancePower ? Dw.MinBalancePower : InputRef->MaxBalancePower;
        PCmdTry = sign * PCmdTry;
    }
    FPS_PRINT_INFO("My PCmdTry is [{}]. I tried to update my power command this iteration? [{}]", PCmdTry, ActvPwrDw.delayIter ==0);
    OutputRef->PCmd = ActivePowerRamp(PCmdTry);
    ActvPwrDw.PCmdLast = PCmdTry;
    if(std::abs(OutputRef->PCmd) > std::abs(ActvPwrDw.MaxPwrReached)) 
    {
        ActvPwrDw.MaxPwrReached = OutputRef->PCmd;
    }
}

bool BatteryBalancing::ActionDelayTimeout()
{
    if (Dw.actionDelayTimer < Dw.actiondelay_i)
    {
        Dw.actionDelayTimer++;
        return false;
    }
    Dw.actionDelayTimer = 0;
    return true;
}

double BatteryBalancing::ActivePowerRamp(double PTrgt)
{   
    if(OutputRef->PCmd == PTrgt)
    {
        return PTrgt;
    }
    double PTry = 0.0;
    double sign = OutputRef->PCmd < PTrgt ? 1.0 : -1.0;
    PTry = OutputRef->PCmd + sign * (Configs.ActivePowerRampRatekWps * Configs.repTime);
    if(std::abs(PTry) > std::abs(PTrgt)) 
    {
        PTry = PTrgt;
    }
    return PTry; 
}

void BatteryBalancing::GetRackInfo()
{
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);
    double sum = 0.0;
    int count = 0;
    for (const auto& uqRackptr : racks)
    {
        RackInfo* rackptr = uqRackptr.get();
        if (rackptr->RackInputRef->contactorStatus)
        {
            sum += rackptr->RackInputRef->voltage;
            count++;
        }
    }
    Dw.numClosedRacks = count;
    if (count != 0)
    {
        Dw.avgVoltage = sum / ((double)count);
        Dw.MinBalancePower = sum * Configs.MinRackBalancePower;
    }
    else
    {
        Dw.avgVoltage = -1.0;
        Dw.MinBalancePower = 0;
    }

}

// Resets all local variables through assigning a new struct instance, and set all output variables to initial values
// directly  Do not reset InputRef here as those should only ever change from the amap
void BatteryBalancing::InitEn()
{
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);
    Dw = Balancing_DW();
    VltArbDw = VoltageArbitration_DW();
    ContCtrlDw = ContactorControl_DW();
    FineBalDw = FineBalance_DW();

    // set all racks to disabled and not ignored
    for (const auto& rackptr : racks)
    {
        rackptr->RackOutputRef->enableCmd = false;
        rackptr->RackOutputRef->ignoreInternal = false;
    }

    // dont let repTime be 0 (if our repTime is 0, we likely have other problems)
    if (Configs.repTime == 0)
    {
        OutputRef->StateVariable = States::ERR;
        FPS_PRINT_ERROR("Our repTime is 0 and we cannot continue to run", nullptr);
        OutputRef->errStr = "The repTime of our Battery Rack Balancing function is 0";
        return;
    }
    // set up the configurable timers for contactor control in iterations
    Dw.batteryRelax_i = (int)std::ceil(Configs.BatteryRelaxTime / Configs.repTime);
    Dw.delay_i = (int)std::ceil(Configs.MinimumFeedbackDelayTime / Configs.repTime);
    Dw.actvpwrdelay_i = (int)std::ceil(Configs.ActivePowerUpdateTimeMinimum / Configs.repTime);
    Dw.actiondelay_i = (int)std::ceil(Configs.ActionDelayTimeout / Configs.repTime);

    OutputRef->StateVariable = States::DEFAULT;
    OutputRef->PCmd = 0.0;
    OutputRef->OpenContactorReq = false;
    OutputRef->CloseContactorReq = false;
    return;
}

void BatteryBalancing::InitDu()
{
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);
    return;
}

void BatteryBalancing::InitEx()
{
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);
    return;
}

void BatteryBalancing::VoltageArbitrationEn()
{
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);
    VltArbDw = VoltageArbitration_DW();
    return;
    Dw.actionDelayTimer = 0;
}

void BatteryBalancing::VoltageArbitrationEx()
{
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);
    Dw.actionDelayTimer = 0;
    return;
}

void BatteryBalancing::ActivePowerBalancingEn()
{
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);
    ActvPwrDw = ActivePowerBalancing_DW();
    Dw.actionDelayTimer = 0;
    return;
}

void BatteryBalancing::ActivePowerBalancingEx()
{
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);
    OutputRef->PCmd = 0.0;
    Dw.actionDelayTimer = 0;
    return;
}

void BatteryBalancing::ContactorControlEn()
{
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);

    // set initial and default values on state entry
    ContCtrlDw = ContactorControl_DW();
    Dw.actionDelayTimer = 0;
    if (InputRef->debug)
    {
        FPS_PRINT_INFO("Battery will relax for [{}] seconds, which is [{}] iterations", Configs.BatteryRelaxTime,
                       Dw.batteryRelax_i);
    }

    // enable all of our racks that we arent ignoring
    for (const auto& uqRackptr : racks)
    {
        RackInfo* rackptr = uqRackptr.get();
        if (!(rackptr->RackInputRef->ignoreExternal || rackptr->RackOutputRef->ignoreInternal))
        {
            rackptr->RackOutputRef->enableCmd = true;
            if (InputRef->debug)
                FPS_PRINT_INFO("Enabling Rack_{}", rackptr->rackNum);
            // we check to ensure all racks are enabled in BATTERY_RELAX
        }
    }

    // if our target vector is empty, go to close contactor state
    if (VltArbDw.targetRacks.empty())
    {
        ContCtrlDw.state = ContactorControlStates::CLOSE_CONTACTORS;
    }

    // if useFunctionalizedContCls input happens to change while we are flipping contactors
    // then we should not change our current contactor control path. 
    if (Configs.UseFunctionalizedContCls)
    {
        ContCtrlDw.useFunctionalizedContCls = true;
    }
    return;
}

void BatteryBalancing::ContactorControlDu()
{
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);

    // if we get an error in ContactorControlEn(), we set the state to error. need to check it here
    if (OutputRef->StateVariable == States::ERR)
        return;
    if(!ContCtrlDw.pcsStopDone)
    {
        OutputRef->PcsStopReq = true;
    }
    if(!ContCtrlDw.pcsStopDone)
    {
        if(!ActionDelayTimeout())
        {
            return;
        }
    }
    ContCtrlDw.pcsStopDone = true;
    OutputRef->PcsStopReq = false;
    ContCtrlDw.allTrgtClosed = true;
    // Check if our target racks have been closed either upon entry or from last iteration
    for (const auto& trgRackptr : VltArbDw.targetRacks)
    {
        if (!trgRackptr->RackInputRef->contactorStatus)
        {
            ContCtrlDw.allTrgtClosed = false;
        }
    }

    switch (ContCtrlDw.state)
    {
        case ContactorControlStates::BATTERY_RELAX:

            if (InputRef->debug)
                FPS_PRINT_INFO("    In Battery Relax Sub State", nullptr);

            // wait for a specific, configurable amount of time while the battery is relaxing
            if (ContCtrlDw.batteryRelaxCounter < Dw.batteryRelax_i)
            {
                // we are not done relaxing, keep waiting
                ContCtrlDw.batteryRelaxCounter++;
                if (InputRef->debug)
                    FPS_PRINT_INFO(
                        "Waiting for the battery to relax. Have waited {} out of {} seconds (iteration {} of {})",
                        ContCtrlDw.batteryRelaxCounter * Configs.repTime, Configs.BatteryRelaxTime,
                        ContCtrlDw.batteryRelaxCounter, Dw.batteryRelax_i);
            }
            else
            {
                // our counter has exceeded our time limit and our battery has cooled
                if (InputRef->debug)
                    FPS_PRINT_INFO("Battery is cooled", nullptr);

                // if we do not have rack level contactor control (default path)
                if (!Configs.RackLevelContactorControl)
                {
                    // we do not need to check rack enable. go directly to close contactor state
                    ContCtrlDw.state = ContactorControlStates::CLOSE_CONTACTORS;
                    Dw.actionDelayTimer = 0;

                    if (InputRef->debug)
                        FPS_PRINT_INFO("We do not have rack level contactor control, setting to close contactors",
                                       nullptr);
                }
                else
                {
                    // if we do have rack level contactor control, we want to open contactors first
                    ContCtrlDw.state = ContactorControlStates::OPEN_CONTACTORS;
                    Dw.actionDelayTimer = 0;

                    // ensure that our racks are enabled
                    for (const auto& uqRackptr : racks)
                    {
                        RackInfo* rackptr = uqRackptr.get();

                        // failing to enable a disabled rack is an ignore
                        if (!rackptr->RackInputRef->enableFeedback &&
                            !(rackptr->RackInputRef->ignoreExternal || rackptr->RackOutputRef->ignoreInternal))
                        {
                            // we set this rack to be enabled but have not recieved feedback from modbus that it is
                            // enabled. ignore this rack
                            rackptr->RackOutputRef->ignoreInternal = true;
                            if (InputRef->debug)
                                FPS_PRINT_WARN(
                                    "Rack_{} is being ignored because its enabledFeedback [{}] is not the same as its enableCmd [{}]",
                                    rackptr->rackNum, rackptr->RackInputRef->enableFeedback,
                                    rackptr->RackOutputRef->enableCmd);
                        }
                    }

                    if (InputRef->debug)
                        FPS_PRINT_INFO(
                            "We do have rack level contactor control, opening contactors before sending close command",
                            nullptr);
                }
            }

            break;

        case ContactorControlStates::OPEN_CONTACTORS:

            if (InputRef->debug)
                FPS_PRINT_INFO("    In Open Contactors Sub State", nullptr);

            // on open contactor sub state entry, we want to send an open contactor request and then wait an iteration
            // before checking the result
            if (!ContCtrlDw.reqSent)
            {
                // if we have not yet sent a request, set output flag and internal flag to true and break out of this
                // iteration
                OutputRef->OpenContactorReq = true;
                ContCtrlDw.reqSent = true;
                break;
            }

            // Open Contactor's function result is remapped to OpenContactorResult's amap input
            if (InputRef->debug)
                FPS_PRINT_INFO("Open Contactor Result = [{}]", InputRef->OpenContactorResult);

            if (ContCtrlDw.useFunctionalizedContCls) // we're flipping contactors with functionalized ess code
            {
                switch (InputRef->OpenContactorResult)
                {
                    case ContactorRequestState::RETURN_IN_PROGRESS:

                        // open request is still in progress
                        if (InputRef->debug)
                            FPS_PRINT_INFO("Contactor Open Request In Progress.", nullptr);
                        break;

                    case ContactorRequestState::RETURN_SUCCESS:

                        // open request successful
                        if (InputRef->debug)
                            FPS_PRINT_INFO("All Contactors are open. Disable racks not in our targetRacks", nullptr);

                        // set our Open Request flag low and reset our internal flag
                        OutputRef->OpenContactorReq = false;
                        ContCtrlDw.reqSent = false;

                        // disable all racks not in target racks
                        for (const auto& uqRackptr : racks)
                        {
                            RackInfo* rackptr = uqRackptr.get();

                            // ignore this rack if an ignore flag is high
                            if ((rackptr->RackInputRef->ignoreExternal || rackptr->RackOutputRef->ignoreInternal))
                                continue;

                            // try to find this rack in our targetRacks vector
                            auto it = std::find(VltArbDw.targetRacks.begin(), VltArbDw.targetRacks.end(), rackptr);
                            if (it == VltArbDw.targetRacks.end())
                            {
                                // this rack does not exist in our targetRacks, disable it before closing contactors
                                rackptr->RackOutputRef->enableCmd = false;

                                if (InputRef->debug)
                                    FPS_PRINT_INFO("    Rack_{} is NOT in our targetRacks, disabling this rack",
                                                rackptr->rackNum);
                            }
                            else
                            {
                                if (InputRef->debug)
                                    FPS_PRINT_INFO("    Rack_{} IS in our targetRacks, it is enabled [{}]",
                                                rackptr->rackNum, rackptr->RackOutputRef->enableCmd);
                            }
                        }

                        // change state variable
                        ContCtrlDw.state = ContactorControlStates::CLOSE_CONTACTORS;
                        ContCtrlDw.pcsStopDone = false;

                        break;

                    case ContactorRequestState::RETURN_FAIL:

                        // open request failed, try again a configurable number of times
                        ContCtrlDw.failedOpens++;

                        // set this request to false here to trigger the ifChanged remap attached to this output
                        OutputRef->OpenContactorReq = false;

                        if (ContCtrlDw.failedOpens >= Configs.MaxOpenContactorAttempts)
                        {
                            // we cannot get our contactors open, send to error state
                            if (InputRef->debug)
                                FPS_PRINT_ERROR(
                                    "Contactor Open Request was tried [{}] times and failed each time. Signaling Error",
                                    ContCtrlDw.failedOpens);

                            OutputRef->StateVariable = States::ERR;
                            OutputRef->errStr = fmt::format(
                                "Contactor Open Request was tried [{}] times and failed each time", ContCtrlDw.failedOpens);
                            break;
                        }
                        else
                        {
                            ContCtrlDw.reqSent = false;
                        }

                        if (InputRef->debug)
                            FPS_PRINT_WARN("Contactor Open Request attempt [{}] failed, trying again",
                                        ContCtrlDw.failedOpens);

                        break;

                    default:
                        FPS_PRINT_ERROR(
                            "[{}] reached an unknown state of value [{}] while in ContactorControlStates::OPEN_CONTACTORS state. Error",
                            __func__, InputRef->OpenContactorResult);
                        OutputRef->StateVariable = States::ERR;
                        OutputRef->errStr = fmt::format(
                            "[{}] reached an unknown state of value [{}] while in ContactorControlStates::OPEN_CONTACTORS sub state",
                            __func__, InputRef->OpenContactorResult);
                        break;
                }  // end OPEN_CONTACTOR switch statement

            } 
            else   // in other words, if(!ContCtrlDw.useFunctionalizedContCls) -> we're flipping contactors with configs
            {
                bool allOpen = true;
                for (const auto& uqRackPtr : racks)
                {
                    RackInfo* rackPtr = uqRackPtr.get();
                    if (rackPtr->RackInputRef->contactorStatus)
                    {
                        allOpen = false;
                    }
                }
                if (allOpen)
                {
                    ContCtrlDw.state = ContactorControlStates::CLOSE_CONTACTORS;
                    ContCtrlDw.pcsStopDone = false;
                } 
                else
                { 
                    if (ContCtrlDw.delayTimer < Dw.delay_i)
                    {
                        ContCtrlDw.delayTimer++;
                        break;
                    } 
                    else
                    {
                        
                        ContCtrlDw.delayTimer = 0;
                        if (ContCtrlDw.failedOpens < Configs.MaxOpenContactorAttempts - 1)
                        {
                            // set up next iteration to retry closure. 
                            OutputRef->OpenContactorReq = false;
                            ContCtrlDw.reqSent = false;
                            ContCtrlDw.failedOpens++;
                        } 
                        else // We give up, this is an error
                        {
                            FPS_PRINT_ERROR("Unable to open contactors. Battery balancing unrecoverable error.",nullptr);
                            OutputRef->StateVariable = States::ERR;
                        }
                    }
                }
                break;
            }
            break;

        case ContactorControlStates::CLOSE_CONTACTORS:

            if (InputRef->debug)
                FPS_PRINT_INFO("    In Close Contactor Sub State", nullptr);

            // on close contactor sub state entry, we want to send a close contactor request and then wait an iteration
            // before checking the result
            // but first open all contactors and wait for enough time for them to actually open. 
            if (!ContCtrlDw.openCmdSent)
            {
                OutputRef->OpenContactorReq = true;
            }
            if (!ContCtrlDw.openCmdSent)
            {
                if(!ActionDelayTimeout())
                {
                    if(InputRef->debug) FPS_PRINT_INFO("   Sent open contactors, waiting for first delay timer: [{}]", Dw.actionDelayTimer);
                    break;
                }

            }
            if (!ContCtrlDw.openCmdSent)
            {
                OutputRef->OpenContactorReq = false;
                ContCtrlDw.openCmdSent = true;
                break;
            }
            if (!ContCtrlDw.reqSent)
            {
                // if we have not yet sent a request, set output flag and internal flag to true and break out of this
                // iteration
                OutputRef->CloseContactorReq = true;
                ContCtrlDw.reqSent = true;
                break;
            }
            if (!ContCtrlDw.closeCmdSent)
            {
                if(!ActionDelayTimeout())
                {
                    if(InputRef->debug) FPS_PRINT_INFO("   Sent close contactors, waiting for first delay timer: [{}]", Dw.actionDelayTimer);
                    break;
                }
            }
            ContCtrlDw.closeCmdSent = true;
            // Close Contactor's function result is remapped to CloseContactorResult's amap input
            if (InputRef->debug)
                FPS_PRINT_INFO("Close Contactor Result = [{}]", InputRef->CloseContactorResult);

            if (ContCtrlDw.useFunctionalizedContCls) // we're flipping contactors with functionalized ess code
            {
                switch (InputRef->CloseContactorResult)
                {
                    case ContactorRequestState::RETURN_IN_PROGRESS:

                        // close request is still in progress
                        FPS_PRINT_INFO("Contactor Close Request In Progress.", nullptr);
                        break;

                    case ContactorRequestState::RETURN_SUCCESS:

                        // close request successful
                        FPS_PRINT_INFO("All enabled Contactors are closed", nullptr);
                        OutputRef->CloseContactorReq = false;
                        ContCtrlDw.reqSent = false;
                        ContCtrlDw.closingDone = true;
                        Dw.actionDelayTimer = 0;

                        break;

                    case ContactorRequestState::RETURN_FAIL:

                        // close request failed, try again a configurable number of times
                        ContCtrlDw.failedCloses++;

                        // set this request to false here to trigger the ifChanged remap attached to this output
                        OutputRef->CloseContactorReq = false;

                        if (ContCtrlDw.failedCloses >= Configs.MaxCloseContactorAttempts)
                        {
                            // setting closing done to true will trigger the Exit actions and set any open racks in
                            // targetRacks to ignore
                            FPS_PRINT_WARN(
                                "Failed to close contactors after [{}] attempts. Continuing while ignoring open racks",
                                ContCtrlDw.failedCloses);

                            // if we have any open racks in our targetRacks, set them to be ignored
                            for (const auto& rack : VltArbDw.targetRacks)
                            {
                                // ignore this rack if an ignore flag is high
                                if ((rack->RackInputRef->ignoreExternal || rack->RackOutputRef->ignoreInternal))
                                    continue;

                                if (!rack->RackInputRef->contactorStatus)  // contactorStatus=true is closed,
                                                                        // contactorStatus=false is open
                                {
                                    // if a rack still isnt closed, ignore that rack
                                    rack->RackOutputRef->ignoreInternal = true;
                                    if (InputRef->debug)
                                        FPS_PRINT_INFO(
                                            "Rack_{} is still open after Contactor Control state. Ignoring this rack",
                                            rack->rackNum);
                                }
                            }

                            ContCtrlDw.closingDone = true;
                            Dw.actionDelayTimer = 0;
                            break;
                        }
                        else
                        {
                            ContCtrlDw.reqSent = false;
                        }

                        if (InputRef->debug)
                            FPS_PRINT_WARN("Contactor Close Request attempt [{}] failed, trying again",
                                        ContCtrlDw.failedCloses);

                        break;

                    default:
                        FPS_PRINT_ERROR(
                            "[{}] reached an unknown state of value [{}] while in ContactorControlStates::CLOSE_CONTACTORS state. Error",
                            __func__, InputRef->CloseContactorResult);
                        OutputRef->StateVariable = States::ERR;
                        OutputRef->errStr = fmt::format(
                            "[{}] reached an unknown state of value [{}] while in ContactorControlStates::CLOSE_CONTACTORS sub state",
                            __func__, InputRef->CloseContactorResult);
                        break;
                }  // end CLOSE_CONTACTOR switch statement

            } 
            else // close through config
            {
                if (InputRef->debug) FPS_PRINT_INFO("============ Closing contactors with config path", nullptr);
                if (ContCtrlDw.allTrgtClosed) 
                {
                        FPS_PRINT_INFO("All enabled Contactors are closed", nullptr);
                        OutputRef->CloseContactorReq = false;
                        ContCtrlDw.reqSent = false;
                        ContCtrlDw.closingDone = true; // main state machine will send us to the next state.
                        Dw.actionDelayTimer = 0;
                } 
                else // Wait for a configurable amount of time. If timeout, retry if able, otherwise ignore open racks. 
                {
                    if (ContCtrlDw.delayTimer < Dw.delay_i)
                    { 
                        ContCtrlDw.delayTimer++;
                        if (InputRef->debug)
                            FPS_PRINT_INFO("============ Waiting for contactor command delay. Iteration {}", ContCtrlDw.delayTimer);
                        break;
                    }
                    else
                    {
                        ContCtrlDw.delayTimer = 0;
                        if (ContCtrlDw.failedCloses < Configs.MaxCloseContactorAttempts - 1)
                        {
                            ContCtrlDw.failedCloses++;
                            if (InputRef->debug)
                                FPS_PRINT_INFO("============ Closure timeout. We did not close our contactors in time. Attempt number {}. We will retry", ContCtrlDw.failedCloses);
                            OutputRef->CloseContactorReq = false;
                            ContCtrlDw.reqSent = false;
                        }
                        else
                        {
                            if (InputRef->debug)
                            {
                                FPS_PRINT_INFO("============ Closure timeout. We did not close our contactors in time. Attempt number {}. We will NOT retry", ContCtrlDw.failedCloses);
                                FPS_PRINT_INFO("============ The following racks are ignored:", nullptr);
                            }
                            int i = 0;
                            for (const auto& trgtRackPtr : VltArbDw.targetRacks)
                            {
                                i++;

                                if (!trgtRackPtr->RackInputRef->contactorStatus)
                                {
                                    trgtRackPtr->RackOutputRef->ignoreInternal = true;
                                    if (InputRef->debug) FPS_PRINT_INFO("============ Ignoring rack number {}", i);
                                }
                            }
                            ContCtrlDw.closingDone = true; // main state machine will send us to the next state.
                            Dw.actionDelayTimer = 0; 
                            break;
                        }
                    }
                }
            }
            break;

        default:  // default case for ContactorControlDu() states
            FPS_PRINT_ERROR("[{}] reached an unknown state of value [{}]. Error", __func__, ContCtrlDw.state);
            OutputRef->StateVariable = States::ERR;
            OutputRef->errStr = fmt::format("[{}] reached an unknown state of value [{}]", __func__, ContCtrlDw.state);
            break;
    }  // end ContactorControlDu switch statement

    return;
}

void BatteryBalancing::ContactorControlEx()
{
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);
    Dw.actionDelayTimer = 0;
    return;
}

void BatteryBalancing::FineBalanceEn()
{
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);
    return;
}

void BatteryBalancing::FineBalanceDu()
{
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);
    return;
}

void BatteryBalancing::FineBalanceEx()
{
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);
    return;
}

void BatteryBalancing::EndEn()
{
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);
    EndDw = End_DW();
    return;
}

void BatteryBalancing::EndDu()
{
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);

    if (!EndDw.pcsStartDone)
    {
        if (!ActionDelayTimeout())
        {
            OutputRef->PcsStartReq = true;
            return;
        }
    }
    EndDw.pcsStartDone = true;
    OutputRef->PcsStartReq = false;
    return;
}

void BatteryBalancing::EndEx()
{
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);
    return;
}

void BatteryBalancing::ErrorDu()
{
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);
    OutputRef->PCmd = 0;
    return;
}
// This step function is called directly from the datamp thread state machine in RUN state.
void BatteryBalancing::step()
{
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);
    if (!Dw.initialized)
        initialize();

    // Update balancing class structs with rack information every iteration
    GetRackInfo();
    if (InputRef->reset || Dw.configWrapperReset)
    {  // latch in init while reset is high.
        initialize();
    }

    // Main state machine
    switch (OutputRef->StateVariable)
    {
        case States::INIT:
            if (Dw.numClosedRacks == 0 && InputRef->StartCmd && !InputRef->reset)
            {
                OutputRef->StateVariable = States::CONTACTOR_CONTROL;
                InitEx();
                ContactorControlEn();
                ContactorControlDu();
            }
            else if (InputRef->StartCmd && !InputRef->reset)
            {
                OutputRef->StateVariable =
                    States::VOLTAGE_ARBITRATION;  // for now, go straight in to voltage arbitration function
                InitEx();
                VoltageArbitrationEn();
                VoltageArbitrationDu();
            }
            else
                InitDu();
            break;

        case States::VOLTAGE_ARBITRATION:
            if (VltArbDw.balancingNeeded)  // Voltage arbitration to active power balancing
            {
                OutputRef->StateVariable = States::ACTIVE_POWER_BALANCING;
                VoltageArbitrationEx();
                ActivePowerBalancingEn();
                ActivePowerBalancingDu();
            }
            else if (VltArbDw.closingNeeded)  // We have open racks and all of them should be able to have their
                                              // contactors closed by delta voltage
            {
                OutputRef->StateVariable = States::CONTACTOR_CONTROL;
                VoltageArbitrationEx();
                ContactorControlEn();
                ContactorControlDu();
            }
            else if (InputRef->FineBalanceEnabled)  // no further balancing, go to next step
            {
                OutputRef->StateVariable = States::FINE_BALANCE;
                VoltageArbitrationEx();
                FineBalanceEn();
                FineBalanceDu();
            }
            else
            {
                OutputRef->StateVariable = States::END;
                VoltageArbitrationEx();
                EndEn();
                EndDu();
            }
            break;

        case States::ACTIVE_POWER_BALANCING:
            if (ActvPwrDw.stopBalancing)
            {
                OutputRef->StateVariable = States::CONTACTOR_CONTROL;
                ActivePowerBalancingEx();
                ContactorControlEn();
                ContactorControlDu();
            }
            else
            {
                ActivePowerBalancingDu();
            }
            break;

        case States::CONTACTOR_CONTROL:
            // TODO we can probably go straight to fine balancing here instead of going back to voltage arbitration
            // first if we're done.
            if (ContCtrlDw.closingDone)
            {
                OutputRef->StateVariable = States::VOLTAGE_ARBITRATION;
                ContactorControlEx();
                VoltageArbitrationEn();
                VoltageArbitrationDu();
            }
            else
            {
                ContactorControlDu();
            }
            break;

        case States::FINE_BALANCE:
            if (FineBalDw.balanceDone)
            {
                OutputRef->StateVariable = States::END;
                FineBalanceEx();
                EndEn();
                EndDu();
            }
            break;

        case States::END:
            if (!InputRef->StartCmd)
            {
                OutputRef->StateVariable = States::INIT;
                EndEx();
                InitEn();
                InitDu();
            }
            else
            {
                EndDu();
            }
            break;

        case States::ERR:
            ErrorDu();
            break;

        default:
            FPS_PRINT_INFO(
                "[{}] Reached default state in primary battery balancing state machine. That probably shouldn't happen",
                __func__);
            OutputRef->StateVariable = States::ERR;
            OutputRef->errStr = "Main state machine: Default state reached";
    }
}

// TODO: need a get from amap before calling this.
void BatteryBalancing::initialize()
{
    if (InputRef->debug)
        FPS_PRINT_INFO("[{}] reached", __func__);
    InitEn();
    InitDu();
    Dw.initialized = true;
    OutputRef->StateVariable = States::INIT;
}

BatteryBalancing::BatteryBalancing(ExtU* inputs_i, ExtY* outputs_o)
    : InputRef(), OutputRef(), Configs(), Dw(), VltArbDw(), ActvPwrDw(), ContCtrlDw(), FineBalDw()
{
    InputRef = inputs_i;
    OutputRef = outputs_o;
    Dw.initialized = false;
}

RackInfo::RackInfo(RackInfo::ExtU* inputs, RackInfo::ExtY* outputs)
    :  // rack constructor
      RackInputRef(),
      RackOutputRef()
{
    RackInputRef = inputs;
    RackOutputRef = outputs;
}

#endif