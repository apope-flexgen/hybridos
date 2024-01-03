#ifndef BATTERYBALANCINGUTILITY_CPP
#define BATTERYBALANCINGUTILITY_CPP

#include "asset.h"
#include "formatters.hpp"
#include <iostream>
#include "FESS/FunctionUtility.hpp"
#include "FESS/InfoMessageUtility.hpp"
#include "FESS/OutputHandler.hpp"
#include "FESS/BatteryBalancingUtility.hpp"


namespace BatteryBalancingUtility
{


    /**
    * @brief 
    * 
    * @param racks 
    */
    VoltageArbitrationResult VoltageArbitration(double deadband, const std::vector<RackInfoObject>& racks)
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
        auto minVoltage = std::min_element(openRacksOutsideDeadband.begin(), openRacksOutsideDeadband.end(), [](const auto& a, const auto& b) {
            return a.voltage < b.voltage;
        });

        auto maxVoltage = std::max_element(openRacksOutsideDeadband.begin(), openRacksOutsideDeadband.end(), [](const auto& a, const auto& b) {
            return a.voltage < b.voltage;
        });

        double bestVoltage = 0.0;

        double minVoltageDiffFromClosed = std::abs(closedVoltage - minVoltage->voltage);
        double maxVoltageDiffFromClosed = std::abs(closedVoltage - maxVoltage->voltage);

        int min = std::min(minVoltageDiffFromClosed, maxVoltageDiffFromClosed);
        if(min == minVoltageDiffFromClosed) {
            bestVoltage = minVoltage->voltage;
        } else {
            bestVoltage = maxVoltage->voltage;
        }


// ==================== Average Voltage of Group from Optimal Choice and Return ====================
        std::vector<RackInfoObject> openRacksWithinTargetArea;
        for (const auto& rack : openRacksOutsideDeadband) {
            if(std::abs(rack.voltage - bestVoltage) <= deadband){
                openRacksWithinTargetArea.push_back(rack);
            }
        }

        double sum = 0.0;
       
       for (const auto& rack : openRacksWithinTargetArea) {
            sum += rack.voltage;
        } 

        double average = openRacksWithinTargetArea.empty() ? 0.0 : sum / openRacksWithinTargetArea.size();

        result.voltage = average;
        result.balancingNeeded = true;
        return;
        

    }



}


#endif
