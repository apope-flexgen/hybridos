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
        double voltage = 0.0;
        bool balancingNeeded = false;

        VoltageArbitrationResult() = default;

        VoltageArbitrationResult(double a, bool b) 
            : voltage(a), balancingNeeded(b) {}


    };

    VoltageArbitrationResult VoltageArbitration(const std::vector<RackInfoObject> racks);
}
 
#endif