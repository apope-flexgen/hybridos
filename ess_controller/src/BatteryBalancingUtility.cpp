#ifndef BATTERYBALANCINGUTILITY_CPP
#define BATTERYBALANCINGUTILITY_CPP

#include "asset.h"
#include "formatters.hpp"
#include <iostream>
#include "FunctionUtility.hpp"
#include "InfoMessageUtility.hpp"
#include "OutputHandler.hpp"
#include "BatteryBalancingUtility.hpp"
#include <cstdlib>  
#include <ctime>



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
            result.targetVoltage = -1;
            result.balancingNeeded = false;
            return result;
        }

        //TODO get the average of all of them
        double closedVoltage = closedRacks[0].voltage;
        result.closedRackAverageVoltage = closedVoltage;


// ==================== Reduction to Open Racks Outside Deadband ====================
        std::vector<RackInfoObject> openRacksOutsideDeadband;
        for (const auto& rack : openRacks) {
            if(std::abs(rack.voltage - closedVoltage) >= deadband){
                openRacksOutsideDeadband.push_back(rack);
            }
        }
        //TODO if there are only open racks that are within the deadband then they need to be closed
        // if they can't be closed then exclude them and move forward
        //NO BALANCING NEEDED
        if(openRacksOutsideDeadband.size() == 0){
            result.targetVoltage = -1;
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
    
    //TODO do some unit tests


// ==================== Average Voltage of Group from Optimal Choice and Return ====================
    //TODO may not need to do this right now
    //future improvement
        // double sum = 0.0;
        // int count = 0;
        // for (const auto& voltage : voltages) {
        //     if(std::abs(voltage - bestVoltage) <= deadband){
        //         sum += voltage;
        //         count++;
        //     }
        // }

        // if(sum == 0) {
        //     result.targetVoltage = -1;
        //     result.balancingNeeded = false;
        //     return result;
        // }

        // double average = sum / count;


        // result.targetVoltage = average;

        result.targetVoltage = bestVoltage;
        result.balancingNeeded = true;
        return result;


        //TODO may need to have some understanding of state of charge
        //TODO an option to include knowledge of soc or not for the layer
        
    }

    double ActivePowerBalancing(ActivePowerBalancingInput input){

        //TODO this works for the discharge case
        // need to consider the charge case

        // double deltaVoltage = input.targetVoltage - input.closedRackAverageVoltage;
        double currentVoltage = input.closedRackAverageVoltage;

        //rampStartDeadband
        if (currentVoltage >= (input.targetVoltage + input.rampStartDeadband) || currentVoltage <= (input.targetVoltage - input.rampStartDeadband)) {
            //charging or discharging changes the sign of this
            //need to figure out which one it is
            return input.maxPowerForBalancing;
        }

        //targetVoltageDeadband
        if (currentVoltage > (input.targetVoltage + input.targetVoltageDeadband) || currentVoltage < (input.targetVoltage - input.targetVoltageDeadband)) {

            //This needs to work for both discharge and charge
            //looks like it only works for discharge rn
            // sign of slope will change if you're charge or discharging
            double x = input.rampStartDeadband - (std::abs(currentVoltage - input.targetVoltage));
            double m = input.powerVsDeltaVoltageSlope;

            //charging or discharging changes the sign of this
            //need to figure out which one it is
            double b = input.maxPowerForBalancing;
            double y = (m*x) + b;
            return y;
        }

        return 0;

    }

    ActivePowerBalancingInput GetActivePowerBalancingInfo(varmap& amap, VoltageArbitrationResult result){
        ActivePowerBalancingInput inputs;
        inputs.targetVoltage = result.targetVoltage;
        inputs.closedRackAverageVoltage = result.closedRackAverageVoltage;
        //TODO shouldn't reuse this cause it changes a lot
        // get the actual average at this point


        inputs.targetVoltageDeadband = amap["battery_rack_balance_coarse"]->getdParam("targetVoltageDeadband");
        inputs.rampStartDeadband = amap["battery_rack_balance_coarse"]->getdParam("rampStartDeadband");
        inputs.powerVsDeltaVoltageSlope = amap["battery_rack_balance_coarse"]->getdParam("powerVsDeltaVoltageSlope");
        inputs.maxPowerForBalancing = amap["battery_rack_balance_coarse"]->getdParam("maxPowerForBalancing");

        inputs.currentActivePower = amap["ActivePowerSetpoint"]->getdVal();

        return inputs;
    }

    std::vector<RackInfoObject> GetRackInfoList(varmap& amap) {
        int numRacks = amap["NumRacks"]->getiVal();
        std::vector<RackInfoObject> racks = {};
        for (int i = 1; i > numRacks; i++) {
            std::string dcClosedName = fmt::format("DCClosed_rack_{}", i);
            std::string dcVoltageName = fmt::format("DCVoltage_rack_{}", i);
            BatteryBalancingUtility::RackInfoObject rackInfo;
            rackInfo.rackNum = i;
            rackInfo.voltage = amap[dcVoltageName.c_str()]->getdVal();
            if(amap[dcClosedName.c_str()]->getbVal()){
                rackInfo.contactorStatus = BatteryBalancingUtility::ContactorStatus::CLOSED;
            } else {
                rackInfo.contactorStatus = BatteryBalancingUtility::ContactorStatus::OPEN;
            }
            racks.push_back(rackInfo);
        } 

        return racks;
    }

    void TestVoltageArbitration(){

        double voltsPerCellAt0SOC = 2.55;
        double voltsPerCellAt100SOC = 3.4;
        int cellsPerRack = 380;

        double deadband = 3.0;
        double closedVariance = .1;

        double upperBoundVoltage = voltsPerCellAt100SOC * cellsPerRack;
        double lowerBoundVoltage = voltsPerCellAt0SOC * cellsPerRack;



        int numRacks = 5;
        int numClosed = 3;
        int numOpen = 2;


        srand(static_cast<unsigned int>(time(0)));
        // Generate a random number with one decimal place
        double randomNumber = static_cast<double>(rand()) / RAND_MAX;
        // Scale the random number to the range [10.0, 20.0]
        double scaledNumber = lowerBoundVoltage + randomNumber * (upperBoundVoltage - lowerBoundVoltage);

        double randomClosedVoltage = scaledNumber;



        const std::vector<RackInfoObject> racks = {
            //rackNum, voltage, contactorStatus
            BatteryBalancingUtility::RackInfoObject(1, 1250.3, ContactorStatus::CLOSED),
            BatteryBalancingUtility::RackInfoObject(2, 1250.9, ContactorStatus::CLOSED),
            BatteryBalancingUtility::RackInfoObject(3, 1201.7, ContactorStatus::OPEN),
            BatteryBalancingUtility::RackInfoObject(4, 1320.3, ContactorStatus::OPEN),
            BatteryBalancingUtility::RackInfoObject(5, 1250.4, ContactorStatus::CLOSED),
            BatteryBalancingUtility::RackInfoObject(6, 1251.1, ContactorStatus::OPEN),
            BatteryBalancingUtility::RackInfoObject(7, 1250.3, ContactorStatus::CLOSED),
            BatteryBalancingUtility::RackInfoObject(8, 1362.0, ContactorStatus::OPEN),
            BatteryBalancingUtility::RackInfoObject(9, 1250.3, ContactorStatus::CLOSED)
        };
    }

}


#endif
