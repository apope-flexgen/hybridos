#ifndef BATTERYBALANCINGUTILITY_CPP
#define BATTERYBALANCINGUTILITY_CPP

#include "asset.h"
#include "formatters.hpp"
#include <iostream>
#include "FunctionUtility.hpp"
#include "InfoMessageUtility.hpp"
#include "OutputHandler.hpp"
#include "BatteryBalancingUtility.hpp"


namespace BatteryBalancingUtility
{


    /**
    * @brief 
    * 
    * @param racks 
    */
    VoltageArbitrationResult VoltageArbitration(double deadband, const std::vector<RackInfoObject> racks)
    {

// ==================== Closed Rack Check ====================
        std::vector<RackInfoObject> closedRacks;
        for (const auto& rack : racks) {
            if(rack.contactorStatus == ContactorStatus::CLOSED) {
                closedRacks.push_back(rack);
            }
        } 
        //FAILURE CASE
        if(closedRacks.size() == 0){
            // TODO: add logic for failure
        }

        VoltageArbitrationResult result;


// ==================== Open Rack Check ====================
        std::vector<RackInfoObject> openRacks;
        for (const auto& rack : racks) {
            if(rack.contactorStatus == ContactorStatus::OPEN) {
                openRacks.push_back(rack);
            }
        } 
        //NO BALANCING NEEDED
        if(openRacks.size() == 0){
            result.voltage = -1;
            result.balancingNeeded = false;
            return result;
        }

        double closedVoltage = closedRacks[0].voltage;


// ==================== Reduction to Open Racks Outside Deadband ====================
        std::vector<RackInfoObject> openRacksOutsideDeadband;
        for (const auto& rack : openRacks) {
            if(std::abs(rack.voltage - closedVoltage) >= deadband){
                openRacksOutsideDeadband.push_back(rack);
            }
        }
        //NO BALANCING NEEDED
        if(openRacksOutsideDeadband.size() == 0){
            result.voltage = -1;
            result.balancingNeeded = false;
            return result;
        }



// ==================== Find Least Extreme from Min and Max ====================
        std::vector<double> voltages;
        for (const auto& rack : openRacksOutsideDeadband) {
            voltages.push_back(rack.voltage);
        }

        auto minVoltage = std::min_element(voltages.begin(), voltages.end());

        auto maxVoltage = std::max_element(voltages.begin(), voltages.end());

        double bestVoltage = 0.0;

        double minVoltageDiffFromClosed = std::abs(closedVoltage - *minVoltage);
        double maxVoltageDiffFromClosed = std::abs(closedVoltage - *maxVoltage);

        int min = std::min(minVoltageDiffFromClosed, maxVoltageDiffFromClosed);
        if(min == minVoltageDiffFromClosed) {
            bestVoltage = *minVoltage;
        } else {
            bestVoltage = *maxVoltage;
        }


// ==================== Average Voltage of Group from Optimal Choice and Return ====================
        double sum = 0.0;
        int count = 0;
        for (const auto& voltage : voltages) {
            if(std::abs(voltage - bestVoltage) <= deadband){
                sum += voltage;
                count++;
            }
        }

        if(sum == 0) {
            result.voltage = -1;
            result.balancingNeeded = false;
            return result;
        }

        double average = sum / count;


        result.voltage = average;
        result.balancingNeeded = true;
        return result;
        

    }



}


#endif
