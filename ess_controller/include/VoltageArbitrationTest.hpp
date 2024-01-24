#ifndef VOLTAGEARBITRATIONTEST_HPP
#define VOLTAGEARBITRATIONTEST_HPP

#include "asset.h"
#include "formatters.hpp"
#include <iostream>
#include "FunctionUtility.hpp"
#include "InfoMessageUtility.hpp"
#include "OutputHandler.hpp"
#include "BatteryBalancingUtility.hpp"
#include <cstdlib>  
#include <ctime>



namespace VoltageArbitrationTest
{

    const double voltsPerCellAt0SOC = 2.55;
    const double voltsPerCellAt100SOC = 3.4;
    const int cellsPerRack = 380;
    const double defaultVoltageArbitrationDeadband = 3.0;


    struct VoltageArbitrationTestCase {
        std::string testName = "";
        double deadband = 0.0;
        std::vector<BatteryBalancingUtility::RackInfoObject> racks = {};
        BatteryBalancingUtility::VoltageArbitrationResult expected = {};

        VoltageArbitrationTestCase() = default;

        VoltageArbitrationTestCase(std::string c, double a, std::vector<BatteryBalancingUtility::RackInfoObject> b, BatteryBalancingUtility::VoltageArbitrationResult d) 
            : testName(c), deadband(a), racks(b), expected(d) {}


    };



    const std::vector<VoltageArbitrationTestCase> voltageArbitrationTestCases = {

        {
            "1 Rack Open and Many Closed",
            defaultVoltageArbitrationDeadband/2,
            {
            BatteryBalancingUtility::RackInfoObject(1, 1100.0, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(2, 1100.0, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(3, 1004.5, BatteryBalancingUtility::ContactorStatus::OPEN, 0, false),
            BatteryBalancingUtility::RackInfoObject(4, 1100.1, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(5, 1100.0, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(6, 1099.9, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false)
            },
            BatteryBalancingUtility::VoltageArbitrationResult(1004.5, 1100.0, true)
            
        },
        {
            "2 Rack Open and Both Above Average",
            defaultVoltageArbitrationDeadband/2,
            {
            BatteryBalancingUtility::RackInfoObject(1, 1100.0, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(2, 1100.0, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(3, 1200.0, BatteryBalancingUtility::ContactorStatus::OPEN, 0, false),
            BatteryBalancingUtility::RackInfoObject(4, 1100.1, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(5, 1230.3, BatteryBalancingUtility::ContactorStatus::OPEN, 0, false),
            BatteryBalancingUtility::RackInfoObject(6, 1099.9, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false)
            },
            BatteryBalancingUtility::VoltageArbitrationResult(1200.0, 1100.0, true)
            
        },
        {
            "2 Rack Open and Both Below Average",
            defaultVoltageArbitrationDeadband/2,
            {
            BatteryBalancingUtility::RackInfoObject(1, 1100.0, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(2, 1100.0, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(3, 1000.6, BatteryBalancingUtility::ContactorStatus::OPEN, 0, false),
            BatteryBalancingUtility::RackInfoObject(4, 1100.1, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(5, 1056.2, BatteryBalancingUtility::ContactorStatus::OPEN, 0, false),
            BatteryBalancingUtility::RackInfoObject(6, 1099.9, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false)
            },
            BatteryBalancingUtility::VoltageArbitrationResult(1056.2, 1100.0, true)
            
        },
        {
            "2 Rack Open (1 above and 1 below average) and the above is closer",
            defaultVoltageArbitrationDeadband/2,
            {
            BatteryBalancingUtility::RackInfoObject(1, 1100.0, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(2, 1100.0, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(3, 1111.1, BatteryBalancingUtility::ContactorStatus::OPEN, 0, false),
            BatteryBalancingUtility::RackInfoObject(4, 1100.1, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(5, 1040.8, BatteryBalancingUtility::ContactorStatus::OPEN, 0, false),
            BatteryBalancingUtility::RackInfoObject(6, 1099.9, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false)
            },
            BatteryBalancingUtility::VoltageArbitrationResult(1111.1, 1100.0, true)
            
        },
        {
            "2 Rack Open (1 above and 1 below average) and the below is closer",
            defaultVoltageArbitrationDeadband/2,
            {
            BatteryBalancingUtility::RackInfoObject(1, 1100.0, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(2, 1100.0, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(3, 1220.4, BatteryBalancingUtility::ContactorStatus::OPEN, 0, false),
            BatteryBalancingUtility::RackInfoObject(4, 1100.1, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(5, 1040.8, BatteryBalancingUtility::ContactorStatus::OPEN, 0, false),
            BatteryBalancingUtility::RackInfoObject(6, 1099.9, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false)
            },
            BatteryBalancingUtility::VoltageArbitrationResult(1040.8, 1100.0, true)
            
        }
    };



    void PrintVoltageArbitrationTestCaseResult(int testNum, VoltageArbitrationTestCase testCase, BatteryBalancingUtility::VoltageArbitrationResult result);
    void TestVoltageArbitration();
}


#endif
