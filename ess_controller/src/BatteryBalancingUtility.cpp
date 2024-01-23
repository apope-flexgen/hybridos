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

        double closedRackCount = 0.0;

        //TODO make this configurable
        int contactorCloseAttemptMax = 5;

// ==================== Closed Rack Check ====================
        std::vector<RackInfoObject> closedRacks;
        for (const auto& rack : racks) {
            if(rack.contactorStatus == ContactorStatus::CLOSED) {
                closedRacks.push_back(rack);
                closedRackCount += rack.voltage;
            }
        } 
        //FAILURE CASE
        if(closedRacks.size() == 0){
            // TODO: add logic for failure
            FPS_PRINT_INFO("ERROR: THERE ARE NO CLOSED RACKS");
        }

        VoltageArbitrationResult result;


// ==================== Open Rack Check ====================
        std::vector<RackInfoObject> openRacks;
        for (const auto& rack : racks) {
            if(rack.contactorStatus == ContactorStatus::OPEN && rack.ignore != true) {
                openRacks.push_back(rack);
            }
        } 
        //NO BALANCING NEEDED
        if(openRacks.size() == 0){
            result.targetVoltage = -1;
            result.balancingNeeded = false;
            return result;
        }

        double closedVoltage = closedRackCount / closedRacks.size();
        result.closedRackAverageVoltage = closedVoltage;


// ==================== Reduction to Open Racks Outside Deadband ====================
        std::vector<RackInfoObject> openRacksOutsideDeadband;
        std::vector<RackInfoObject> openRacksInsideDeadband;
        for (const auto& rack : openRacks) {
            if(std::abs(rack.voltage - closedVoltage) >= deadband){
                openRacksOutsideDeadband.push_back(rack);
            } else {
                openRacksInsideDeadband.push_back(rack);
            }
        }
        //TODO if there are only open racks that are within the deadband then they need to be closed
        // if they can't be closed then exclude them and move forward

        if(openRacksInsideDeadband.size() > 0){
            //TODO close contactors here

            for (auto& rack : openRacksInsideDeadband) {
                if(rack.contactorCloseAttemptCount > contactorCloseAttemptMax){
                    rack.ignore = true;
                }
                rack.contactorCloseAttemptCount++;
            }
            
        }

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

        double min = std::min(minVoltageDiffFromClosed, maxVoltageDiffFromClosed);
        if(min == minVoltageDiffFromClosed) {
            bestVoltage = *minVoltage;
        } else {
            bestVoltage = *maxVoltage;
        }

        // FPS_PRINT_INFO("closedVoltage {}", closedVoltage);
        // FPS_PRINT_INFO("minVoltage {}", *minVoltage);
        // FPS_PRINT_INFO("minVoltage {}", *maxVoltage);
        // FPS_PRINT_INFO("minVoltageDiffFromClosed {}", minVoltageDiffFromClosed);
        // FPS_PRINT_INFO("maxVoltageDiffFromClosed {}", maxVoltageDiffFromClosed);
        // FPS_PRINT_INFO("min {}", min);
        // FPS_PRINT_INFO("bestVoltage {}", bestVoltage);
    

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

        // double targetVoltage = 0.0;
        // double closedRackAverageVoltage = 0.0;
        // double targetVoltageDeadband = 0.0;
        // double rampStartDeadband = 0.0;
        // double powerVsDeltaVoltageSlope = 0.0;
        // double currentActivePower = 0.0;
        // double maxPowerForBalancing = 0.0;

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
            // double m = input.powerVsDeltaVoltageSlope;
            double m = input.maxPowerForBalancing / input.rampStartDeadband;

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

    // void TestVoltageArbitration(varmap& amap){

    //     double voltsPerCellAt0SOC = 2.55;
    //     double voltsPerCellAt100SOC = 3.4;
    //     int cellsPerRack = 380;

    //     double deadband = 3.0;
    //     // double closedVariance = .1;

    //     // double upperBoundVoltage = voltsPerCellAt100SOC * cellsPerRack;
    //     // double lowerBoundVoltage = voltsPerCellAt0SOC * cellsPerRack;



    //     // int numRacks = 5;
    //     // int numClosed = 3;
    //     // int numOpen = 2;


    //     // srand(static_cast<unsigned int>(time(0)));
    //     // // Generate a random number with one decimal place
    //     // double randomNumber = static_cast<double>(rand()) / RAND_MAX;
    //     // // Scale the random number to the range [10.0, 20.0]
    //     // double scaledNumber = lowerBoundVoltage + randomNumber * (upperBoundVoltage - lowerBoundVoltage);

    //     // double randomClosedVoltage = scaledNumber;



    //     const std::vector<RackInfoObject> racks = {
    //         //rackNum, voltage, contactorStatus
    //         BatteryBalancingUtility::RackInfoObject(1, round((2.8 * cellsPerRack) * 10) / 10, ContactorStatus::CLOSED),
    //         BatteryBalancingUtility::RackInfoObject(2, round((2.8 * cellsPerRack) * 10) / 10, ContactorStatus::CLOSED),
    //         BatteryBalancingUtility::RackInfoObject(3, round((voltsPerCellAt0SOC * cellsPerRack) * 10) / 10, ContactorStatus::OPEN),
    //         BatteryBalancingUtility::RackInfoObject(4, round((voltsPerCellAt100SOC * cellsPerRack) * 10) / 10, ContactorStatus::OPEN),
    //         BatteryBalancingUtility::RackInfoObject(5, round((2.8 * cellsPerRack) * 10) / 10, ContactorStatus::CLOSED),
    //         BatteryBalancingUtility::RackInfoObject(6, round((2.8 * cellsPerRack) * 10) / 10, ContactorStatus::CLOSED),
    //         BatteryBalancingUtility::RackInfoObject(7, round((2.8 * cellsPerRack) * 10) / 10, ContactorStatus::CLOSED),
    //         BatteryBalancingUtility::RackInfoObject(8, round((2.9 * cellsPerRack) * 10) / 10, ContactorStatus::OPEN),
    //         BatteryBalancingUtility::RackInfoObject(9, round((3.3 * cellsPerRack) * 10) / 10, ContactorStatus::OPEN),
    //         BatteryBalancingUtility::RackInfoObject(10, round((2.8 * cellsPerRack) * 10) / 10, ContactorStatus::CLOSED)
    //     };

    //     PrintRackInfoList(racks);
    //     VoltageArbitrationResult result = VoltageArbitration(deadband, racks);
    //     PrintVoltageArbitrationResult(result);

    //     ActivePowerBalancingInput input = GetActivePowerBalancingInfo(amap, result);
    //     PrintActivePowerBalancingInfo(input);





    //     // const std::vector<RackInfoObject> racks2 = {
    //     //     //rackNum, voltage, contactorStatus
    //     //     BatteryBalancingUtility::RackInfoObject(1, round((voltsPerCellAt0SOC * cellsPerRack) * 10) / 10, ContactorStatus::CLOSED),
    //     //     BatteryBalancingUtility::RackInfoObject(2, round((voltsPerCellAt0SOC * cellsPerRack) * 10) / 10, ContactorStatus::CLOSED),
    //     //     BatteryBalancingUtility::RackInfoObject(3, round((voltsPerCellAt0SOC * cellsPerRack) * 10) / 10, ContactorStatus::CLOSED),
    //     //     BatteryBalancingUtility::RackInfoObject(4, round((voltsPerCellAt100SOC * cellsPerRack) * 10) / 10, ContactorStatus::OPEN),
    //     //     BatteryBalancingUtility::RackInfoObject(5, round((voltsPerCellAt0SOC * cellsPerRack) * 10) / 10, ContactorStatus::CLOSED),
    //     //     BatteryBalancingUtility::RackInfoObject(6, round((voltsPerCellAt0SOC * cellsPerRack) * 10) / 10, ContactorStatus::CLOSED),
    //     //     BatteryBalancingUtility::RackInfoObject(7, round((voltsPerCellAt0SOC * cellsPerRack) * 10) / 10, ContactorStatus::CLOSED),
    //     //     BatteryBalancingUtility::RackInfoObject(8, round((2.9 * cellsPerRack) * 10) / 10, ContactorStatus::OPEN),
    //     //     BatteryBalancingUtility::RackInfoObject(9, round((3.3 * cellsPerRack) * 10) / 10, ContactorStatus::OPEN),
    //     //     BatteryBalancingUtility::RackInfoObject(10, round((voltsPerCellAt0SOC * cellsPerRack) * 10) / 10, ContactorStatus::CLOSED)
    //     // };

    //     // PrintRackInfoList(racks2);
    //     // VoltageArbitrationResult result2 = VoltageArbitration(deadband, racks2);
    //     // PrintVoltageArbitrationResult(result2);

    //     // const std::vector<RackInfoObject> racks3 = {
    //     //     //rackNum, voltage, contactorStatus
    //     //     BatteryBalancingUtility::RackInfoObject(1, round((2.9 * cellsPerRack) * 10) / 10, ContactorStatus::CLOSED),
    //     //     BatteryBalancingUtility::RackInfoObject(2, round((2.9 * cellsPerRack) * 10) / 10, ContactorStatus::CLOSED),
    //     //     BatteryBalancingUtility::RackInfoObject(3, round((2.9 * cellsPerRack) * 10) / 10, ContactorStatus::CLOSED),
    //     //     BatteryBalancingUtility::RackInfoObject(4, round((voltsPerCellAt100SOC * cellsPerRack) * 10) / 10, ContactorStatus::OPEN),
    //     //     BatteryBalancingUtility::RackInfoObject(5, round((2.9 * cellsPerRack) * 10) / 10, ContactorStatus::CLOSED),
    //     //     BatteryBalancingUtility::RackInfoObject(6, round((2.9 * cellsPerRack) * 10) / 10, ContactorStatus::CLOSED),
    //     //     BatteryBalancingUtility::RackInfoObject(7, round((2.9 * cellsPerRack) * 10) / 10, ContactorStatus::CLOSED),
    //     //     BatteryBalancingUtility::RackInfoObject(8, round((2.9 * cellsPerRack) * 10) / 10, ContactorStatus::CLOSED),
    //     //     BatteryBalancingUtility::RackInfoObject(9, round((3.3 * cellsPerRack) * 10) / 10, ContactorStatus::OPEN),
    //     //     BatteryBalancingUtility::RackInfoObject(10, round((2.9 * cellsPerRack) * 10) / 10, ContactorStatus::CLOSED)
    //     // };

    //     // PrintRackInfoList(racks3);
    //     // VoltageArbitrationResult result3 = VoltageArbitration(deadband, racks3);
    //     // PrintVoltageArbitrationResult(result3);

    // }

    void PrintRackInfoList(const std::vector<RackInfoObject> racks) {
        std::string message = "Rack List: \n";
        for (const auto& rack : racks){
            std::string contactorStatus = "Closed";
            if(rack.contactorStatus == ContactorStatus::OPEN) contactorStatus = "Open";
            message += fmt::format("Rack #{}        Voltage:{}      ContactorStatus:{}      ContactorCloseAttemptCount:{}       Ignore:{}\n", rack.rackNum, rack.voltage, contactorStatus, rack.contactorCloseAttemptCount, rack.ignore);
        }

        FPS_PRINT_INFO("\n\n{}\n\n", message);
    }

    void PrintVoltageArbitrationResult(VoltageArbitrationResult result) {
        std::string message = "Voltage Arbitration Result: \n";
        message += fmt::format("Target Voltage: {}      \nClosed Rack Average Voltage: {}      \nBalancing Needed: {}\n", result.targetVoltage, result.closedRackAverageVoltage, result.balancingNeeded);

        FPS_PRINT_INFO("\n\n{}\n\n", message);
    }

    void PrintActivePowerBalancingInfo(ActivePowerBalancingInput input) {
        std::string message = "Active Power Balancing Info: \n";
        message += fmt::format("targetVoltage: {}\n", input.targetVoltage);
        message += fmt::format("closedRackAverageVoltage: {}\n", input.closedRackAverageVoltage);
        message += fmt::format("targetVoltageDeadband: {}\n", input.targetVoltageDeadband);
        message += fmt::format("rampStartDeadband: {}\n", input.rampStartDeadband);
        message += fmt::format("powerVsDeltaVoltageSlope: {}\n", input.powerVsDeltaVoltageSlope);
        message += fmt::format("currentActivePower: {}\n", input.currentActivePower);
        message += fmt::format("maxPowerForBalancing: {}\n", input.maxPowerForBalancing);

        FPS_PRINT_INFO("\n\n{}\n\n", message);
    }

    std::string GetVoltageArbitrationResultString(VoltageArbitrationResult result){
        std::string message = fmt::format("\nTarget Voltage: {}      \nClosed Rack Average Voltage: {}      \nBalancing Needed: {}\n", result.targetVoltage, result.closedRackAverageVoltage, result.balancingNeeded);

        return message;
    }

    void PrintVoltageArbitrationTestCaseResult(int testNum, VoltageArbitrationTestCase testCase, BatteryBalancingUtility::VoltageArbitrationResult result) {
        if(1)FPS_PRINT_INFO("{}", __func__);

        std::string message = fmt::format("\n\nTest Number #{} \n", testNum);
        
        message += fmt::format("Test Name: [{}] \n\n", testCase.testName);
        message += "Starting Rack List: \n";
        for (const auto& rack : testCase.racks){
            std::string contactorStatus = "Closed";
            if(rack.contactorStatus == BatteryBalancingUtility::ContactorStatus::OPEN) contactorStatus = "Open";
            message += fmt::format("Rack #{}        Voltage:{}      ContactorStatus:{}\n", rack.rackNum, rack.voltage, contactorStatus);
        }
        message += "\n";

        message += fmt::format("Expected Result: {}", BatteryBalancingUtility::GetVoltageArbitrationResultString(testCase.expected));

        message += fmt::format("\nActual Result: {} \n\n", BatteryBalancingUtility::GetVoltageArbitrationResultString(result));

        FPS_PRINT_INFO("{}", message);
    }

    void TestVoltageArbitration(){
        if(1)FPS_PRINT_INFO("{}", __func__);

        int count = 1;
        for (const auto& test : voltageArbitrationTestCases){
            BatteryBalancingUtility::VoltageArbitrationResult result = BatteryBalancingUtility::VoltageArbitration(test.deadband, test.racks);
            PrintVoltageArbitrationTestCaseResult(count, test, result);
            count++;
        }


    }


}


#endif
