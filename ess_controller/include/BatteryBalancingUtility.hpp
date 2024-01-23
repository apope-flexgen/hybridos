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
        int contactorCloseAttemptCount = 0;
        bool ignore = false;

        RackInfoObject() = default;

        RackInfoObject(int a, double b, ContactorStatus c, int d, bool e) 
            : rackNum(a), voltage(b), contactorStatus(c), contactorCloseAttemptCount(d), ignore(e) {}


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

    // void TestVoltageArbitration(varmap& amap);
    void PrintRackInfoList(const std::vector<RackInfoObject> racks);
    void PrintVoltageArbitrationResult(VoltageArbitrationResult result);
    void PrintActivePowerBalancingInfo(ActivePowerBalancingInput input);
    std::string GetVoltageArbitrationResultString(VoltageArbitrationResult result);






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



    std::vector<VoltageArbitrationTestCase> voltageArbitrationTestCases = {
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
            
        },
        {
            "1 rack open and is within deadband",
            defaultVoltageArbitrationDeadband/2,
            {
            BatteryBalancingUtility::RackInfoObject(1, 1100.0, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(2, 1100.0, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(3, 1101.0, BatteryBalancingUtility::ContactorStatus::OPEN, 0, false),
            BatteryBalancingUtility::RackInfoObject(4, 1100.1, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(5, 1100.0, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(6, 1099.9, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false)
            },
            BatteryBalancingUtility::VoltageArbitrationResult(-1, 1100.0, false)
            
        },
        {
            "2 Rack Open (both above), one is in the deadband and one is out",
            defaultVoltageArbitrationDeadband/2,
            {
            BatteryBalancingUtility::RackInfoObject(1, 1234.5, BatteryBalancingUtility::ContactorStatus::OPEN, 0, false),
            BatteryBalancingUtility::RackInfoObject(2, 1101.3, BatteryBalancingUtility::ContactorStatus::OPEN, 0, false),
            BatteryBalancingUtility::RackInfoObject(3, 1100.0, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(4, 1100.1, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(5, 1100.0, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(6, 1099.9, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false)
            },
            BatteryBalancingUtility::VoltageArbitrationResult(1234.5, 1100.0, true)
            
        },
        {
            "2 Rack Open (both below), one is in the deadband and one is out",
            defaultVoltageArbitrationDeadband/2,
            {
            BatteryBalancingUtility::RackInfoObject(1, 1098.6, BatteryBalancingUtility::ContactorStatus::OPEN, 0, false),
            BatteryBalancingUtility::RackInfoObject(2, 970.3, BatteryBalancingUtility::ContactorStatus::OPEN, 0, false),
            BatteryBalancingUtility::RackInfoObject(3, 1100.0, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(4, 1100.1, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(5, 1100.0, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(6, 1099.9, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false)
            },
            BatteryBalancingUtility::VoltageArbitrationResult(970.3, 1100.0, true)
            
        },
        {
            "2 Rack Open (1 above 1 below), both are within the deadband",
            defaultVoltageArbitrationDeadband/2,
            {
            BatteryBalancingUtility::RackInfoObject(1, 1098.6, BatteryBalancingUtility::ContactorStatus::OPEN, 0, false),
            BatteryBalancingUtility::RackInfoObject(2, 1100.9, BatteryBalancingUtility::ContactorStatus::OPEN, 0, false),
            BatteryBalancingUtility::RackInfoObject(3, 1100.0, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(4, 1100.1, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(5, 1100.0, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(6, 1099.9, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false)
            },
            BatteryBalancingUtility::VoltageArbitrationResult(-1, 1100.0, false)
            
        },
        {
            "Ignore an open rack that has been configured to be so",
            defaultVoltageArbitrationDeadband/2,
            {
            BatteryBalancingUtility::RackInfoObject(1, 1098.6, BatteryBalancingUtility::ContactorStatus::OPEN, 0, false),
            BatteryBalancingUtility::RackInfoObject(2, 1100.9, BatteryBalancingUtility::ContactorStatus::OPEN, 0, false),
            BatteryBalancingUtility::RackInfoObject(3, 1100.0, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(4, 1100.1, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false),
            BatteryBalancingUtility::RackInfoObject(5, 960.0, BatteryBalancingUtility::ContactorStatus::OPEN, 0, true),
            BatteryBalancingUtility::RackInfoObject(6, 1099.9, BatteryBalancingUtility::ContactorStatus::CLOSED, 0, false)
            },
            BatteryBalancingUtility::VoltageArbitrationResult(-1, 1100.0, false)
            
        }
    };



    void PrintVoltageArbitrationTestCaseResult(int testNum, VoltageArbitrationTestCase testCase, BatteryBalancingUtility::VoltageArbitrationResult result);
    void TestVoltageArbitration();



}
 
#endif