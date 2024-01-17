#ifndef BATTERYBALANCINGUTILITY_HPP
#define BATTERYBALANCINGUTILITY_HPP

#include "asset.h"
#include "formatters.hpp"
#include <iostream>
#include <vector>


namespace BatteryBalancingUtility
{

    enum class ContactorStatus { OPEN, CLOSED };

    struct RackInfoObject {
        int rackNum = 0;
        double voltage = 0.0;
        ContactorStatus contactorStatus = ContactorStatus::OPEN;

        RackInfoObject() = default;

        RackInfoObject(int a, double b, ContactorStatus c) 
            : rackNum(a), voltage(b), contactorStatus(c) {}


    };

    struct VoltageArbitrationResult {
        double targetVoltage = 0.0;
        double closedRackAverageVoltage = 0.0;
        bool balancingNeeded = false;

        VoltageArbitrationResult() = default;

        VoltageArbitrationResult(double a, double b, bool c) 
            : targetVoltage(a), closedRackAverageVoltage(b), balancingNeeded(c) {}


    };
    
    struct ActivePowerBalancingInput {
        double targetVoltage = 0.0;
        double closedRackAverageVoltage = 0.0;
        double targetVoltageDeadband = 0.0;
        double rampStartDeadband = 0.0;
        double powerVsDeltaVoltageSlope = 0.0;
        double currentActivePower = 0.0;
        double maxPowerForBalancing = 0.0;

        ActivePowerBalancingInput() = default;

        ActivePowerBalancingInput(double a, double b, double c, double d, double e, double f, double g) 
            : targetVoltage(a), closedRackAverageVoltage(b), targetVoltageDeadband(c),
            rampStartDeadband(d), powerVsDeltaVoltageSlope(e), currentActivePower(f), maxPowerForBalancing(g) {}
    };

    VoltageArbitrationResult VoltageArbitration(double deadband, const std::vector<RackInfoObject> racks);

    double ActivePowerBalancing(ActivePowerBalancingInput input);

    ActivePowerBalancingInput GetActivePowerBalancingInfo(varmap& amap, VoltageArbitrationResult result);

    std::vector<RackInfoObject> GetRackInfoList(varmap& amap);



}
 
#endif