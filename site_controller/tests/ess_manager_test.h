#ifndef ESS_TEST_H_
#define ESS_TEST_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Logger.h>
#include "ESS_Manager.h"
#include <Configurator.h>
#include <Site_Controller_Utils.h>

class ESS_Manager_Mock : public ESS_Manager
{
	public:
	ESS_Manager_Mock(){}
	
	// Configuration generation functions
	cJSON* generate_calculatePower_essRoot(int numParse, float ratedActivePower, float ratedReactivePower);
    cJSON* generate_controllable_essRoot(int numParse);
    cJSON* generate_energy_essRoot(int numParse, float rated_capacity, float min_raw_soc, float max_raw_soc, float soc, float soh);
    cJSON* generate_power_essRoot(int numParse, float soc);
    cJSON* generate_running_mask_root(uint64_t mask, int numParse);
	// Variable endpoint/helper functions
	void set_controllable_avg_soc(float soc);
	void set_soc_status(SOC_State status);
	SOC_State get_soc_status();
	void set_charge_control_divisor(float divisor);
	void set_entering_soc_range(float entering);
	void set_leaving_soc_range(float leaving);
	void set_far_soc_range(float far);
	void set_ess_total_dischargeable_power(float power);
	void set_ess_total_chargeable_power(float power);
	void set_ess_total_rated_reactive_power(float target);
	void set_ess_potential_reactive_power(float,int);
    void set_ess_soh(float _soh, int num_parse);
    void set_ess_all_socs(std::vector<float> * socs);
    void set_ess_all_dischargeable_powers(float _dp);
    void set_ess_all_chargeable_powers(float _cp);
	void set_ess_all_max_limited_active_powers(float max_ap);
	void set_ess_all_min_limited_active_powers(float min_ap);
    void set_ess_limits_override(bool flag, int num_parse);
    void set_chargeable_dischargeable_power_raw(float chargeable, float dischargable, int num_parse);
    void set_soc_balancing_factor(float bf);
	void run_asset_instances(int numRunning);
	void set_demand_modes(void);
	void configure_pEss();
};

class ess_manager_test : public testing::Test
{
public:
	// Only print messages to log if a test fails
	bool failure;
	std::stringstream errorLog;
	ESS_Manager_Mock* essMgr;

    bool* primary_controller;
	
	virtual void SetUp()
	{
		failure = false;
		essMgr = new ESS_Manager_Mock();
        primary_controller = new bool(true);
	}
 
    // TearDown override removed - results in double free
    // 
	// virtual void TearDown()
	// {
	// 	delete essMgr;
	// }
};

// calculate_ess_active_power
TEST_F(ess_manager_test, calculate_ess_active_power)
{
    // struct that has variables to configure for each test case
    struct testCase
    {
        int numEss;
        float ratedActivePower;
        float targetActivePower;
        std::vector<float> essSOCs;
        std::vector<float> resultActivePowerSetpoints;
        float soc_balancing_factor;
    };

    std::vector<testCase> tests; // an array with an element for each test case

    // Configure variables for each test case
    //              numEss          ratedActivePower    targetActivePower                 essSOCs             resultActivePowerSetpoints    socBalancingFactor
    tests.push_back({    4,                     100,                 100,           {50,50,50,50},                       {25,25,25,25},     3}); // equal discharge
    tests.push_back({    4,                     100,                -100,           {50,50,50,50},                   {-25,-25,-25,-25},     3}); // equal charge
    tests.push_back({    4,                     100,                 100,           {10,45,55,90},         {0.101,9.228,16.848,73.823},     3}); // wide spread discharge
    tests.push_back({    4,                     100,                -100,           {10,45,55,90},     {-73.823,-16.848,-9.228,-0.101},     3}); // wide spread charge
    tests.push_back({    4,                     100,                 100,       {49,49.5,50.5,51},       {23.512,24.239,25.738,26.510},     3}); // narrow spread discharge
    tests.push_back({    4,                     100,                -100,       {49,49.5,50.5,51},   {-26.510,-25.738,-24.239,-23.512},     3}); // narrow spread charge
    tests.push_back({    4,                     100,                 300,           {10,45,55,90},              {1.085,98.915,100,100},     3}); // wide spread high power discharge
    tests.push_back({    4,                     100,                -300,           {10,45,55,90},          {-100,-100,-98.915,-1.085},     3}); // wide spread high power charge
    tests.push_back({    4,                     100,                -400,       {100,100,100,100},               {-100,-100,-100,-100},     3}); // attempt to charge fully-charged batteries
    tests.push_back({    4,                     100,                 400,               {0,0,0,0},                   {100,100,100,100},     3}); // attempt to discharge fully-discharged batteries

    for (auto it = tests.begin() ; it != tests.end() ; ++it)
    {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
		// Capture any prints within site controller that might be present in debug mode
		capture_stdout();

        errorLog << "calculate_ess_active_power test " << it-tests.begin()+1 << " of " << tests.size() << " (test array index " << it-tests.begin() << ")\n";
        SetUp();

		// Configure ESS Manager with the correct number of ESS instances for this test case
		cJSON* essRoot = NULL;
		std::map <std::string, std::vector<Fims_Object*>> * const component_var_map = new std::map<std::string, std::vector<Fims_Object*>>;
		std::map <std::string, Fims_Object*> * const asset_var_map = new std::map<std::string, Fims_Object*>;
		essRoot = essMgr->generate_calculatePower_essRoot(it->numEss, it->ratedActivePower / it->numEss, 0);
        Type_Configurator* ess_configurator = new Type_Configurator(essMgr, component_var_map, asset_var_map, primary_controller);
        ess_configurator->assetTypeRoot = essRoot;
        ess_configurator->config_validation = false;
		bool configure_success = ess_configurator->create_assets();
		ASSERT_TRUE(configure_success);

        // Set test state
		essMgr->set_ess_all_dischargeable_powers(it->ratedActivePower);
        essMgr->set_ess_all_chargeable_powers(it->ratedActivePower);
		essMgr->set_ess_all_max_limited_active_powers(it->ratedActivePower);		
		essMgr->set_ess_all_min_limited_active_powers((it->ratedActivePower) * -1.0);
		essMgr->set_ess_target_active_power(it->targetActivePower);
        essMgr->set_ess_all_socs(&it->essSOCs);
		essMgr->run_asset_instances(it->numEss);
		essMgr->set_demand_modes();
        essMgr->set_soc_balancing_factor(it->soc_balancing_factor);

		// Calculate test results
		essMgr->calculate_ess_active_power();

		// Check test results against expected results
		for (int j = 0; j < it->numEss; ++j)
		{
			EXPECT_NEAR(essMgr->get_asset_active_power_setpoint(j), it->resultActivePowerSetpoints[j], 0.1);
            if (essMgr->get_asset_active_power_setpoint(j) > it->resultActivePowerSetpoints[j]+0.1 || essMgr->get_asset_active_power_setpoint(j) < it->resultActivePowerSetpoints[j]-0.1 || std::isnan(essMgr->get_asset_active_power_setpoint(j)) )
            {
                failure = true;
            }
		}

        cJSON_Delete(essRoot);
        delete component_var_map;
        delete asset_var_map;

		// Release stdout so we can write again
		release_stdout(failure);
        if (failure)
            std::cout << errorLog.str() << '\n';
    }
}

// calculate_ess_reactive_power
TEST_F(ess_manager_test, calculate_ess_reactive_power)
{
    int const num_tests = 3;  //total number of test cases

    // struct that has variables to configure for each test case
    struct tests 
    {
		int numParse;
		int numRunning;
        float targetReactivePower;
		float totPotReactivePower;
		float totRatedReactivePower;
		float result_asset_kVAR_setpoint;
    };

    tests array[num_tests];  // an array with an element for each test case

    // Configure variables for each test case
	//			numParse	numRunning	targetReactivePower	totPotReactivePower	totRatedReactivePower	result_asset_kVAR_setpoint
    array[0] = {1, 			1, 			100, 				100, 				100,					100}; //request full rated react pow from a single ESS
	array[1] = {2, 			2, 			200, 				200, 				200,					100}; //request full rated react pow from 2 ESSs, both running
	array[2] = {2,			1,			200,				100,				200,					100}; //request full rated react pow from 2 ESSs, only 1 running
	array[3] = {3,			3,			600,				300,				600,					100}; //request full pow from 3 ESSs, potential pow limits response
	array[4] = {3,			3,			600,				300,				601,					100}; //request partial pow from 3 ESSs, potential pow limits response

    //iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
		// Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
		// Capture any prints within site controller that might be present in debug mode
		capture_stdout();

		errorLog << "calculate_ess_reactive_power test " << i+1 << " of " << num_tests << " (test array index " << i << ")" << std::endl;
		SetUp();
		// Configure ESS Manager with the correct number of ESS instances for this test case
		cJSON* essRoot = NULL;
		std::map <std::string, std::vector<Fims_Object*>> * const component_var_map = new std::map<std::string, std::vector<Fims_Object*>>;
		std::map <std::string, Fims_Object*> * const asset_var_map = new std::map<std::string, Fims_Object*>;
		essRoot = essMgr->generate_calculatePower_essRoot(array[i].numParse, 0, array[i].totRatedReactivePower / array[i].numParse);
        Type_Configurator* ess_configurator = new Type_Configurator(essMgr, component_var_map, asset_var_map, primary_controller);
        ess_configurator->assetTypeRoot = essRoot;
        ess_configurator->config_validation = false;
		bool configure_success = ess_configurator->create_assets();
		ASSERT_TRUE(configure_success);
		// Set test state
		essMgr->set_ess_total_rated_reactive_power(array[i].totRatedReactivePower);
		essMgr->set_ess_target_reactive_power(array[i].targetReactivePower);
		essMgr->set_ess_potential_reactive_power(array[i].totPotReactivePower, array[i].numRunning);
		essMgr->run_asset_instances(array[i].numRunning);
		essMgr->set_demand_modes();
		// Calculate test results
		essMgr->calculate_ess_reactive_power();
		// Check test results against expected results
		for (int j = 0; j < array[i].numRunning; ++j)
		{
			failure = essMgr->get_asset_reactive_power_setpoint(j) != array[i].result_asset_kVAR_setpoint;
			EXPECT_EQ(essMgr->get_asset_reactive_power_setpoint(j), array[i].result_asset_kVAR_setpoint);
		}
		// Release stdout so we can write again
		release_stdout(failure);
		if (failure)
            std::cout << errorLog.str() << '\n';
        cJSON_Delete(essRoot);
        delete component_var_map;
        delete asset_var_map;
    }
}

TEST_F(ess_manager_test, charge_control_test)
{
	int const num_tests = 45;  //total number of test cases

    // struct that has variables to configure for each test case
    struct tests 
    {
		float avg_soc;
		SOC_State status;
		SOC_State expected_status;
		bool charge_disable;
		bool dischg_disable;
		float expected_cmd;
    };

    tests array[num_tests];  // an array with an element for each test case

    // Configure variables for each test case
	//			avg_soc		status				expected_status		charge_disable	dischg_disable	expected_cmd
    array[0] =  {38.0, 		FarAboveTarget, 	FarBelowTarget,		false, 			false,			-1000.0}; 	// Begin FarAbove test
	array[1] =  {44.9, 		FarAboveTarget, 	BelowTarget,		false, 			false,			-468.468};
	array[2] =  {50.02, 	FarAboveTarget, 	OnTarget,			false, 			false,			0.0};
	array[3] =  {55.1, 		FarAboveTarget, 	AboveTarget,		false, 			false,			468.468};
	array[4] =  {60.02,		FarAboveTarget, 	FarAboveTarget,		false, 			false,			1000.0};
	array[5] =  {38.0, 		AboveTarget, 		FarBelowTarget,		false, 			false,			-1000.0}; 	// Begin Above test
	array[6] =  {44.9, 		AboveTarget, 		BelowTarget,		false, 			false,			-468.468};
	array[7] =  {50.02, 	AboveTarget, 		OnTarget,			false, 			false,			0.0};
	array[8] =  {50.3, 		AboveTarget, 		AboveTarget,		false, 			false,			36.0358};
	array[9] =  {60.02, 	AboveTarget, 		FarAboveTarget,		false, 			false,			1000.0};
	array[10] = {38.0, 		OnTarget, 			FarBelowTarget,		false, 			false,			-1000.0}; 	// Begin OnTarget test
	array[11] = {44.9, 		OnTarget, 			BelowTarget,		false, 			false,			-468.468};
	array[12] = {49.6, 		OnTarget, 			OnTarget,			false, 			false,			0.0};
	array[13] = {55.1, 		OnTarget, 			AboveTarget,		false, 			false,			468.468};
	array[14] = {60.02, 	OnTarget, 			FarAboveTarget,		false, 			false,			1000.0};
	array[15] = {38.0, 		BelowTarget, 		FarBelowTarget,		false, 			false,			-1000.0}; 	// Begin Below test
	array[16] = {49.7, 		BelowTarget, 		BelowTarget,		false, 			false,			-36.0358};
	array[17] = {50.02, 	BelowTarget, 		OnTarget,			false, 			false,			0.0};
	array[18] = {55.1, 		BelowTarget, 		AboveTarget,		false, 			false,			468.468};
	array[19] = {60.02, 	BelowTarget, 		FarAboveTarget,		false, 			false,			1000.0};
	array[20] = {38.0, 		FarBelowTarget, 	FarBelowTarget,		false, 			false,			-1000.0}; 	// Begin FarBelow test
	array[21] = {44.9, 		FarBelowTarget, 	BelowTarget,		false, 			false,			-468.468};
	array[22] = {50.02, 	FarBelowTarget, 	OnTarget,			false, 			false,			0.0};
	array[23] = {55.1, 		FarBelowTarget, 	AboveTarget,		false, 			false,			468.468};
	array[24] = {60.02, 	FarBelowTarget, 	FarAboveTarget,		false, 			false,			1000.0};
																											// Ensure charge/discharge are disabled for all states
	array[25] = {38.0, 		FarAboveTarget, 	FarBelowTarget,		true, 			false,			0}; 	// Begin FarAbove test
	array[26] = {44.9, 		FarAboveTarget, 	BelowTarget,		true, 			false,			0};
	array[27] = {55.1, 		FarAboveTarget, 	AboveTarget,		false, 			true,			0};
	array[28] = {60.02,		FarAboveTarget, 	FarAboveTarget,		false, 			true,			0};
	array[29] = {38.0, 		AboveTarget, 		FarBelowTarget,		true, 			false,			0}; 	// Begin Above test
	array[30] = {44.9, 		AboveTarget, 		BelowTarget,		true, 			false,			0};
	array[31] = {50.3, 		AboveTarget, 		AboveTarget,		false, 			true,			0};
	array[32] = {60.02, 	AboveTarget, 		FarAboveTarget,		false, 			true,			0};
	array[33] = {38.0, 		OnTarget, 			FarBelowTarget,		true, 			false,			0}; 	// Begin OnTarget test
	array[34] = {44.9, 		OnTarget, 			BelowTarget,		true, 			false,			0};
	array[35] = {55.1, 		OnTarget, 			AboveTarget,		true, 			true,			0};
	array[36] = {60.02, 	OnTarget, 			FarAboveTarget,		true, 			true,			0};
	array[37] = {38.0, 		BelowTarget, 		FarBelowTarget,		true, 			true,			0}; 	// Begin Below test
	array[38] = {49.7, 		BelowTarget, 		BelowTarget,		true, 			true,			0};
	array[39] = {55.1, 		BelowTarget, 		AboveTarget,		true, 			true,			0};
	array[40] = {60.02, 	BelowTarget, 		FarAboveTarget,		true, 			true,			0};
	array[41] = {38.0, 		FarBelowTarget, 	FarBelowTarget,		true, 			true,			0}; 	// Begin FarBelow test
	array[42] = {44.9, 		FarBelowTarget, 	BelowTarget,		true, 			true,			0};
	array[43] = {55.1, 		FarBelowTarget, 	AboveTarget,		true, 			true,			0};
	array[44] = {60.02, 	FarBelowTarget, 	FarAboveTarget,		true, 			true,			0};

	//iterate through each test case and get results
    for (int i = 0; i < num_tests; i++)
    {
		// Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
		// Capture any prints within site controller that might be present in debug mode
		capture_stdout();

		errorLog << "charge_control_test " << i+1 << " of " << num_tests << " (test array index " << i << ")" << std::endl;

		essMgr->configure_pEss();
		essMgr->set_controllable_avg_soc(array[i].avg_soc);
		essMgr->set_soc_status(array[i].status);

		// value given in master assets
		essMgr->set_charge_control_divisor(11.1);
		essMgr->set_entering_soc_range(0.1);
		essMgr->set_leaving_soc_range(0.5);
		essMgr->set_far_soc_range(10);
		essMgr->set_ess_total_dischargeable_power(1000);
		essMgr->set_ess_total_chargeable_power(1000);
		float requested_pow = essMgr->charge_control(50.0, array[i].charge_disable, array[i].dischg_disable);

		// failure conditions
		failure = !near(array[i].expected_status, essMgr->get_soc_status(), 0.001) || !near(array[i].expected_cmd, requested_pow, 0.001);
		EXPECT_NEAR(array[i].expected_status, essMgr->get_soc_status(), 0.001);
		EXPECT_NEAR(array[i].expected_cmd, requested_pow, 0.001);

		// Release stdout so we can write again
		release_stdout(failure);
		if (failure)
            std::cout << errorLog.str() << '\n';
	}
}

// Variable endpoint functions
void ESS_Manager_Mock::configure_pEss()
{
	pEss = std::vector<Asset_ESS*>();
	pEss.push_back(new Asset_ESS);
}

void ESS_Manager_Mock::set_controllable_avg_soc(float soc)
{
	controllableEssAvgSoc = soc;
}

void ESS_Manager_Mock::set_soc_status(SOC_State status)
{
	soc_status = status;
}

SOC_State ESS_Manager_Mock::get_soc_status()
{
	return soc_status;
}

void ESS_Manager_Mock::set_charge_control_divisor(float divisor)
{
	charge_control_divisor = divisor;
}

void ESS_Manager_Mock::set_entering_soc_range(float entering)
{
	entering_soc_range = entering;
}

void ESS_Manager_Mock::set_leaving_soc_range(float leaving)
{
	leaving_soc_range = leaving;
}

void ESS_Manager_Mock::set_far_soc_range(float far)
{
	far_soc_range = far;
}

void ESS_Manager_Mock::set_ess_total_dischargeable_power(float power)
{
	essTotalDischargeablePowerkW = power;
}

void ESS_Manager_Mock::set_ess_total_chargeable_power(float power)
{
	essTotalChargeablePowerkW = power;
}

void ESS_Manager_Mock::set_ess_total_rated_reactive_power(float target)
{
    essTotalRatedReactivePower = target;
}

void ESS_Manager_Mock::set_ess_potential_reactive_power(float target, int numRunning)
{
    essTotalPotentialReactivePower = target;
    for(int i = 0; i < numRunning; ++i)
    {
        pEss[i]->potential_reactive_power = target / numRunning;
    }
}

void ESS_Manager_Mock::set_ess_soh(float _soh, int num_parse)
{
    for (int i = 0; i < num_parse; i++)
    {
        pEss[i]->soh->value.value_float = _soh;
    }
}

void ESS_Manager_Mock::set_ess_all_socs(std::vector<float> * socs)
{
    auto _pSoc = socs->begin();
    for (auto it = pEss.begin() ; it != pEss.end() ; ++it)
        (*it)->soc->value.set(*(_pSoc++));
}

void ESS_Manager_Mock::set_ess_all_dischargeable_powers(float _dp)
{
    for (auto it = pEss.begin() ; it != pEss.end() ; ++it)
        (*it)->dischargeable_power->value.set(_dp);
}

void ESS_Manager_Mock::set_ess_all_chargeable_powers(float _cp)
{
    for (auto it = pEss.begin() ; it != pEss.end() ; ++it)
        (*it)->chargeable_power->value.set(_cp);
}

void ESS_Manager_Mock::set_ess_all_max_limited_active_powers(float max_ap) 
{
	for (auto it = pEss.begin() ; it != pEss.end() ; ++it)
		(*it)->max_limited_active_power = max_ap;
}

void ESS_Manager_Mock::set_ess_all_min_limited_active_powers(float min_ap) 
{
	for (auto it = pEss.begin() ; it != pEss.end() ; ++it)
		(*it)->min_limited_active_power = min_ap;
}

void ESS_Manager_Mock::set_ess_limits_override(bool flag, int num_parse)
{
    for (int i = 0; i < num_parse; i++)
        pEss[i]->limits_override_flag = flag;
}

void ESS_Manager_Mock::set_chargeable_dischargeable_power_raw(float chargeable, float dischargeable, int num_parse)
{
    for (int i = 0; i < num_parse; i++)
    {
        pEss[i]->chargeable_power_raw->value.set(chargeable);
		pEss[i]->dischargeable_power_raw->value.set(dischargeable);
    }
}

void ESS_Manager_Mock::set_soc_balancing_factor(float bf)
{
    socBalancingFactor = bf;
}

void ESS_Manager_Mock::run_asset_instances(int numRunning)
{
    for(int i = 0; i < numRunning; ++i)
    {
        pEss[i]->isAvail = true;
        pEss[i]->isRunning = true;
    }
}

void ESS_Manager_Mock::set_demand_modes(void)
{
    for (auto it = pAssets.begin(); it != pAssets.end(); ++it)
        (*it)->assetControl = Direct;
}

// The googletest needs actual asset instances to work with. The asset_create function is what gets the asset instance data from
// assets.json and configures the type manager. This function makes a fake version of that assets.json data for the type manager
// to be configured with. This is also where one would add specific initialization data such as variable initial values or how
// many assets to make.
cJSON* ESS_Manager_Mock::generate_calculatePower_essRoot(int numParse, float ratedActivePower, float ratedReactivePower)
{
	std::stringstream ss;
	// Insert the array header and first ESS instance since it doesn't have a comma at the beginning
	ss << "{\"soc_balancing_factor\":4,\"asset_instances\":[{\"id\":\"ess_01\",\"name\":\"BESS Inverter Block 01\",\"slew_rate\":12500,\"rated_active_power_kw\":" << ratedActivePower << ",\"rated_reactive_power_kvar\":" << ratedReactivePower << ",\"demand_control\":\"Direct\",\"components\":[]}";
	// Insert any additional ESS instances
	for (int i = 1; i < numParse; ++i)
	{
		ss << ",{\"id\":\"ess_0" << i+1 << "\",\"name\":\"BESS Inverter Block 0" << i+1 << "\",\"slew_rate\":12500,\"rated_active_power_kw\":" << ratedActivePower << ",\"rated_reactive_power_kvar\":" << ratedReactivePower << ",\"demand_control\":\"Direct\",\"components\":[]}";
	}
	// Close the array
	ss << "]}";
	// Print the final essRoot
	std::string s = ss.str();
	return cJSON_Parse( s.c_str() );
}

cJSON* ESS_Manager_Mock::generate_controllable_essRoot(int numParse)
{
	std::stringstream ss;
	// Insert the array header and first ESS instance since it doesn't have a comma at the beginning
	ss << "{\"soc_balancing_factor\":4,\"asset_instances\":[{\"id\":\"ess_01\",\"name\":\"BESS Inverter Block 01\",\"status_type\":\"random_enum\",\"running_status_mask\":\"FFFE\",\"demand_control\":\"Direct\",\"components\":[]}";
	// Insert any additional ESS instances
	for (int i = 1; i < numParse; ++i)
	{
		ss << ",{\"id\":\"ess_0" << i+1 << "\",\"name\":\"BESS Inverter Block 0" << i+1 << "\",\"status_type\":\"random_enum\",\"running_status_mask\":\"FFFE\",\"demand_control\":\"Direct\",\"components\":[]}";
	}
	// Close the array
	ss << "]}";
	// Return the final essRoot
	std::string s = ss.str();
    // std::cout << "Parsed essRoot:" << s << std::endl;
	return cJSON_Parse( s.c_str() );
}

cJSON* ESS_Manager_Mock::generate_energy_essRoot(int numParse, float rated_capacity, float min_raw_soc, float max_raw_soc, float soc, float soh)
{
	std::stringstream ss;
	// Insert the array header and first ESS instance since it doesn't have a comma at the beginning
	ss << "{\"soc_balancing_factor\":4,\"asset_instances\":[{\"id\":\"ess_01\",\"name\":\"BESS Inverter Block 01\",\"min_raw_soc\":" << min_raw_soc << ",\"max_raw_soc\":" << max_raw_soc << ",\"status_type\":\"random_enum\",\"rated_capacity\":" << rated_capacity << ",\"running_status_mask\":\"FFFE\",\"demand_control\":\"Direct\",\"components\":[{\"component_id\":\"clou_ess_01\",\"variables\":{\"soc\":{\"name\":\"State of Charge\",\"register_id\":\"bms_soc\",\"value\":" << soc << "},\"soh\":{\"name\":\"State of Health\",\"value\":" << soh << "}}}]}";
	// Insert any additional ESS instances
	for (int i = 1; i < numParse; ++i)
	{
		ss << ",{\"id\":\"ess_0" << i+1 << "\",\"name\":\"BESS Inverter Block 0" << i+1 << "\",\"min_raw_soc\":" << min_raw_soc << ",\"max_raw_soc\":" << max_raw_soc << ",\"status_type\":\"random_enum\",\"rated_capacity\":" << rated_capacity << ",\"running_status_mask\":\"FFFE\",\"demand_control\":\"Direct\",\"components\":[{\"component_id\":\"clou_ess_0" << i+1 << "\",\"variables\":{\"soc\":{\"name\":\"State of Charge\",\"register_id\":\"bms_soc\",\"value\":" << soc << "},\"soh\":{\"name\":\"State of Health\",\"value\":" << soh << "}}}]}";
	}
	// Close the array
	ss << "]}";
	// Return the final essRoot
	std::string s = ss.str();
    // std::cout << "Parsed essRoot:" << s << std::endl;
	return cJSON_Parse( s.c_str() );
}

cJSON* ESS_Manager_Mock::generate_power_essRoot(int numParse, float soc)
{
	std::stringstream ss;
	// Insert the array header and first ESS instance since it doesn't have a comma at the beginning
	ss << "{\"soc_balancing_factor\":4,\"asset_instances\":[{\"id\":\"ess_01\",\"name\":\"BESS Inverter Block 01\",\"min_raw_soc\":0,\"max_raw_soc\":100,\"chg_soc_begin\":95.0,\"chg_soc_end\":101.0,\"dischg_soc_begin\":5.0,\"dischg_soc_end\":-1.0,\"status_type\":\"random_enum\",\"rated_active_power_kw\":2750,\"running_status_mask\":\"FFFE\",\"demand_control\":\"Direct\",\"components\":[{\"component_id\":\"clou_ess_01\",\"variables\":{\"soc\":{\"name\":\"State of Charge\",\"register_id\":\"bms_soc\",\"value\":" << soc << "}}}]}";
	// Insert any additional ESS instances
	for (int i = 1; i < numParse; ++i)
	{
		ss << ",{\"id\":\"ess_0" << i+1 << "\",\"name\":\"BESS Inverter Block 0" << i+1 << "\",\"min_raw_soc\":0,\"max_raw_soc\":100,\"chg_soc_begin\":95.0,\"chg_soc_end\":101.0,\"dischg_soc_begin\":5.0,\"dischg_soc_end\":-1.0,\"status_type\":\"random_enum\",\"rated_active_power_kw\":2750,\"running_status_mask\":\"FFFE\",\"demand_control\":\"Direct\",\"components\":[{\"component_id\":\"clou_ess_0" << i+1 << "\",\"variables\":{\"soc\":{\"name\":\"State of Charge\",\"register_id\":\"bms_soc\",\"value\":" << soc << "}}}]}";
	}
	// Close the array
	ss << "]}";
	// Return the final essRoot
	std::string s = ss.str();
    // std::cout << "Parsed essRoot:" << s << std::endl;
	return cJSON_Parse( s.c_str() );
}

cJSON* ESS_Manager_Mock::generate_running_mask_root(uint64_t mask, int numParse)
{
	std::stringstream ss;
	// Insert the array header and first ESS instance since it doesn't have a comma at the beginning
	ss << "{\"soc_balancing_factor\":4,\"asset_instances\":[{\"id\":\"ess_1\",\"name\":\"BESS Inverter Block 01\",\"status_type\":\"random_enum\",\"rated_active_power_kw\":2750,\"running_status_mask\":\"" << std::hex << mask << "\",\"demand_control\":\"Direct\",\"components\":[{\"component_id\":\"clou_ess_1\",\"variables\":{\"soc\":{\"name\":\"State of Charge\",\"register_id\":\"bms_soc\",\"value\":50}}}]}";
	// Insert any additional ESS instances
	for (int i = 1; i < numParse; ++i)
	{
		ss << ",{\"id\":\"ess_" << i+1 << "\",\"name\":\"BESS Inverter Block 0" << i+1 << "\",\"status_type\":\"random_enum\",\"rated_active_power_kw\":2750,\"running_status_mask\":\"" << std::hex << mask << "\",\"demand_control\":\"Direct\",\"components\":[{\"component_id\":\"clou_ess_" << i+1 << "\",\"variables\":{\"soc\":{\"name\":\"State of Charge\",\"register_id\":\"bms_soc\",\"value\":50}}}]}";
	}
	// Close the array
	ss << "]}";
	// Return the final essRoot
	std::string s = ss.str();
    // std::cout << "Parsed essRoot:" << s << std::endl;
	return cJSON_Parse( s.c_str() );
}

#endif /* ESS_TEST_H_ */