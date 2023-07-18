#ifndef ESS_TEST_H_
#define ESS_TEST_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Logger.h>
#include "ESS_Manager.h"
#include <Configurator.h>
#include <Site_Controller_Utils.h>
#include <test_tools.h>
#include <fims/defer.hpp>

class ESS_Manager_Mock : public ESS_Manager
{
public:
	// Configuration generation functions
	cJSON* generate_calculatePower_essRoot(int numParse, float ratedActivePower, float ratedReactivePower);
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
    void set_ess_all_socs(const std::vector<float> &socs);
    void set_ess_all_dischargeable_powers(float _dp);
    void set_ess_all_chargeable_powers(float _cp);
	void set_ess_all_max_limited_active_powers(float max_ap);
	void set_ess_all_min_limited_active_powers(float min_ap);
	void set_ess_all_reactive_power_setpoints(const std::vector<float> &reactive_setpoints);
	void set_ess_all_rated_apparent_power(float power);
	void set_ess_all_reactive_power_priority(bool is_reactive_power_priority);
	void set_ess_all_potential_reactive_powers(float rated_rp);
    void set_soc_balancing_factor(float bf);
	void run_asset_instances(int numRunning);
	void set_demand_modes(void);
	void process_all_ess_potential_active_power();
};

class ess_manager_test : public testing::Test
{
public:
    bool primary_controller = true;
};

TEST_F(ess_manager_test, calculate_ess_active_power) {
    struct test_case {
		int id;
        int num_ess;
        float rated_kw;
        float target_kw;
        std::vector<float> socs;
        std::vector<float> result_kw_setpoints;
        float soc_balancing_factor;
    };

    std::vector<test_case> tests = {
	//   id   	num_ess rated_kw   			target_kw  		  socs                   result_kw_setpoints    		    soc_balancing_factor
		{1,		4,		100,                100,           	  {50,50,50,50},         {25,25,25,25},     				3}, // equal discharge
		{2,		4,		100,               -100,           	  {50,50,50,50},         {-25,-25,-25,-25},     			3}, // equal charge
		{3,		4,		100,                100,           	  {10,45,55,90},         {0.101,9.228,16.848,73.823},     	3}, // wide spread discharge
		{4,		4,		100,               -100,           	  {10,45,55,90},     	 {-73.823,-16.848,-9.228,-0.101},   3}, // wide spread charge
		{5,		4,		100,                100,       	 	  {49,49.5,50.5,51},     {23.512,24.239,25.738,26.510},     3}, // narrow spread discharge
		{6,		4,		100,               -100,       	 	  {49,49.5,50.5,51},     {-26.510,-25.738,-24.239,-23.512}, 3}, // narrow spread charge
		{7,		4,		100,                300,           	  {10,45,55,90},         {1.085,98.915,100,100},     		3}, // wide spread high power discharge
		{8,		4,		100,               -300,           	  {10,45,55,90},         {-100,-100,-98.915,-1.085},     	3}, // wide spread high power charge
		{9,		4,		100,               -400,       	 	  {100,100,100,100},     {-100,-100,-100,-100},     		3}, // attempt to charge fully-charged batteries
		{10,	4,		100,                400,           	  {0,0,0,0},             {100,100,100,100},     			3} 	// attempt to discharge fully-discharged batteries
	};

    for (auto &test : tests) {
		test_logger t_log("calculate_ess_active_power", test.id, tests.size());
		ESS_Manager_Mock ess_mgr;

		// build a JSON with this test case's number of ESS and power rating
		cJSON* ess_config = ess_mgr.generate_calculatePower_essRoot(test.num_ess, test.rated_kw / test.num_ess, 0);
		defer { cJSON_Delete(ess_config); };

		// configure ESS Manager
		std::map <std::string, std::vector<Fims_Object*>> component_var_map;
		std::map <std::string, Fims_Object*> asset_var_map;
        Type_Configurator ess_configurator(&ess_mgr, &component_var_map, &asset_var_map, &primary_controller);
        ess_configurator.assetTypeRoot = ess_config;
        ess_configurator.config_validation = false;
		bool configure_success = ess_configurator.create_assets();
		ASSERT_TRUE(configure_success);

        // set test state
		ess_mgr.set_ess_all_dischargeable_powers(test.rated_kw);
        ess_mgr.set_ess_all_chargeable_powers(test.rated_kw);
		ess_mgr.set_ess_all_max_limited_active_powers(test.rated_kw);		
		ess_mgr.set_ess_all_min_limited_active_powers((test.rated_kw) * -1.0);
		ess_mgr.set_ess_target_active_power(test.target_kw);
        ess_mgr.set_ess_all_socs(test.socs);
		ess_mgr.run_asset_instances(test.num_ess);
		ess_mgr.set_demand_modes();
        ess_mgr.set_soc_balancing_factor(test.soc_balancing_factor);

		// calculate test results
		ess_mgr.calculate_ess_active_power();

		// check test results
		for (int i = 0; i < test.num_ess; ++i) {
			t_log.range_results.push_back({
				test.result_kw_setpoints[i], // expected
				0.01F, // % tolerance
				ess_mgr.get_asset_active_power_setpoint(i), // actual
				"ess_" + std::to_string(i) // result name
			});
		}
		t_log.check_solution();
    }
}

TEST_F(ess_manager_test, calculate_ess_reactive_power) {
    struct test_case {
		int id;
		int num_ess;
		int num_running;
        float target_kvar;
		float total_potential_kvar;
		float total_rated_kvar;
		float result_asset_kvar_setpoint;
    };

    std::vector<test_case> tests = {
	//	 id	num_ess     num_running     target_kvar             total_potential_kvar   total_rated_kvar     result_asset_kvar_setpoint
		{1,	1,			1,				100,					100,				   100,					100}, // request full rated react pow from a single ESS
		{2,	2,			2,				200,					200,				   200,					100}, // request full rated react pow from 2 ESSs, both running
		{3,	2,			1,				200,					100,				   200,					100}, // request full rated react pow from 2 ESSs, only 1 running
		{4,	3,			3,				600,					300,				   600,					100}, // request full pow from 3 ESSs, potential pow limits response
		{5,	3,			3,				600,					300,				   601,					100}  // request partial pow from 3 ESSs, potential pow limits response
	};

    for(auto &test : tests) {
		test_logger t_log("calculate_ess_reactive_power", test.id, tests.size());
		ESS_Manager_Mock ess_mgr;

		// build a JSON with this test case's number of ESS and power rating
		cJSON* ess_config = ess_mgr.generate_calculatePower_essRoot(test.num_ess, 0, test.total_rated_kvar / test.num_ess);
		defer { cJSON_Delete(ess_config); };

		// configure ESS Manager
		std::map <std::string, std::vector<Fims_Object*>> component_var_map;
		std::map <std::string, Fims_Object*> asset_var_map;
        Type_Configurator ess_configurator(&ess_mgr, &component_var_map, &asset_var_map, &primary_controller);
        ess_configurator.assetTypeRoot = ess_config;
        ess_configurator.config_validation = false;
		bool configure_success = ess_configurator.create_assets();
		ASSERT_TRUE(configure_success);

		// set test state
		ess_mgr.set_ess_total_rated_reactive_power(test.total_rated_kvar);
		ess_mgr.set_ess_target_reactive_power(test.target_kvar);
		ess_mgr.set_ess_potential_reactive_power(test.total_potential_kvar, test.num_running);
		ess_mgr.run_asset_instances(test.num_running);
		ess_mgr.set_demand_modes();

		// calculate test results
		ess_mgr.calculate_ess_reactive_power();

		// check test results
		for (int i = 0; i < test.num_running; ++i) {
			t_log.range_results.push_back({
				test.result_asset_kvar_setpoint, // expected
				0.001F, // % tolerance
				ess_mgr.get_asset_reactive_power_setpoint(i), // actual
				"ess_" + std::to_string(i) // result name
			});
		}
		t_log.check_solution();
    }
}

TEST_F(ess_manager_test, charge_control) {
    struct test_case {
		int id;
		float avg_soc;
		SOC_State status;
		SOC_State expected_status;
		bool charge_disable;
		bool dischg_disable;
		float expected_cmd;
    };

	std::vector<test_case> tests = {
	//   id  avg_soc    status				expected_status		charge_disable	dischg_disable	expected_cmd
		{1,  38.0, 		FarAboveTarget, 	FarBelowTarget,		false, 			false,			-1000.0}, 	// Begin FarAbove test
		{2,  44.9, 		FarAboveTarget, 	BelowTarget,		false, 			false,			-468.468},
		{3,  50.02, 	FarAboveTarget, 	OnTarget,			false, 			false,			0.0},
		{4,  55.1, 		FarAboveTarget, 	AboveTarget,		false, 			false,			468.468},
		{5,  60.02,		FarAboveTarget, 	FarAboveTarget,		false, 			false,			1000.0},
		{6,  38.0, 		AboveTarget, 		FarBelowTarget,		false, 			false,			-1000.0}, 	// Begin Above test
		{7,  44.9, 		AboveTarget, 		BelowTarget,		false, 			false,			-468.468},
		{8,  50.02, 	AboveTarget, 		OnTarget,			false, 			false,			0.0},
		{9,  50.3, 		AboveTarget, 		AboveTarget,		false, 			false,			36.0358},
		{10, 60.02, 	AboveTarget, 		FarAboveTarget,		false, 			false,			1000.0},
		{11, 38.0, 		OnTarget, 			FarBelowTarget,		false, 			false,			-1000.0}, 	// Begin OnTarget test
		{12, 44.9, 		OnTarget, 			BelowTarget,		false, 			false,			-468.468},
		{13, 49.6, 		OnTarget, 			OnTarget,			false, 			false,			0.0},
		{14, 55.1, 		OnTarget, 			AboveTarget,		false, 			false,			468.468},
		{15, 60.02, 	OnTarget, 			FarAboveTarget,		false, 			false,			1000.0},
		{16, 38.0, 		BelowTarget, 		FarBelowTarget,		false, 			false,			-1000.0}, 	// Begin Below test
		{17, 49.7, 		BelowTarget, 		BelowTarget,		false, 			false,			-36.0358},
		{18, 50.02, 	BelowTarget, 		OnTarget,			false, 			false,			0.0},
		{19, 55.1, 		BelowTarget, 		AboveTarget,		false, 			false,			468.468},
		{20, 60.02, 	BelowTarget, 		FarAboveTarget,		false, 			false,			1000.0},
		{21, 38.0, 		FarBelowTarget, 	FarBelowTarget,		false, 			false,			-1000.0}, 	// Begin FarBelow test
		{22, 44.9, 		FarBelowTarget, 	BelowTarget,		false, 			false,			-468.468},
		{23, 50.02, 	FarBelowTarget, 	OnTarget,			false, 			false,			0.0},
		{24, 55.1, 		FarBelowTarget, 	AboveTarget,		false, 			false,			468.468},
		{25, 60.02, 	FarBelowTarget, 	FarAboveTarget,		false, 			false,			1000.0},
		{26, 38.0, 		FarAboveTarget, 	FarBelowTarget,		true, 			false,			0}, 	// Begin FarAbove test
		{27, 44.9, 		FarAboveTarget, 	BelowTarget,		true, 			false,			0},
		{28, 55.1, 		FarAboveTarget, 	AboveTarget,		false, 			true,			0},
		{29, 60.02,		FarAboveTarget, 	FarAboveTarget,		false, 			true,			0},
		{30, 38.0, 		AboveTarget, 		FarBelowTarget,		true, 			false,			0}, 	// Begin Above test
		{31, 44.9, 		AboveTarget, 		BelowTarget,		true, 			false,			0},
		{32, 50.3, 		AboveTarget, 		AboveTarget,		false, 			true,			0},
		{33, 60.02, 	AboveTarget, 		FarAboveTarget,		false, 			true,			0},
		{34, 38.0, 		OnTarget, 			FarBelowTarget,		true, 			false,			0}, 	// Begin OnTarget test
		{35, 44.9, 		OnTarget, 			BelowTarget,		true, 			false,			0},
		{36, 55.1, 		OnTarget, 			AboveTarget,		true, 			true,			0},
		{37, 60.02, 	OnTarget, 			FarAboveTarget,		true, 			true,			0},
		{38, 38.0, 		BelowTarget, 		FarBelowTarget,		true, 			true,			0}, 	// Begin Below test
		{39, 49.7, 		BelowTarget, 		BelowTarget,		true, 			true,			0},
		{40, 55.1, 		BelowTarget, 		AboveTarget,		true, 			true,			0},
		{41, 60.02, 	BelowTarget, 		FarAboveTarget,		true, 			true,			0},
		{42, 38.0, 		FarBelowTarget, 	FarBelowTarget,		true, 			true,			0}, 	// Begin FarBelow test
		{43, 44.9, 		FarBelowTarget, 	BelowTarget,		true, 			true,			0},
		{44, 55.1, 		FarBelowTarget, 	AboveTarget,		true, 			true,			0},
		{45, 60.02, 	FarBelowTarget, 	FarAboveTarget,		true, 			true,			0}
	};

	//iterate through each test case and get results
    for (auto &test : tests) {
		test_logger t_log("charge_control", test.id, tests.size());
		ESS_Manager_Mock ess_mgr;

		// set test state
		ess_mgr.set_controllable_avg_soc(test.avg_soc);
		ess_mgr.set_soc_status(test.status);
		ess_mgr.set_charge_control_divisor(11.1);
		ess_mgr.set_entering_soc_range(0.1);
		ess_mgr.set_leaving_soc_range(0.5);
		ess_mgr.set_far_soc_range(10);
		ess_mgr.set_ess_total_dischargeable_power(1000);
		ess_mgr.set_ess_total_chargeable_power(1000);

		// calculate test results
		float requested_pow = ess_mgr.charge_control(50.0, test.charge_disable, test.dischg_disable);

		// check results
		t_log.range_results.push_back({
			test.expected_cmd, // expected
			0.001F, // % tolerance
			requested_pow, // actual
			"requested_power" // result name
		});
		t_log.int_results.push_back({
			int(test.expected_status), // expected
			int(ess_mgr.get_soc_status()), // actual
			"soc_status" // result name
		});
		t_log.check_solution();
	}
}

TEST_F(ess_manager_test, calculate_ess_active_power_with_reactive_power_priority)
{
	struct test_case
    {
		int id;
        int num_ess;
        float rated_kw;
        float target_kw;
        std::vector<float> socs;
        std::vector<float> result_kw_setpoints;
        float soc_balancing_factor;
		bool is_reactive_power_priority;
		std::vector<float> kvar_setpoints;
		float rated_kva;
		float potential_kvar;
    };

    std::vector<test_case> tests = {
    //   ID  num_ess rated_kw  target_kw socs               result_kw_setpoints                 soc_bf is_rpp kvar_setpoints  rated_kva  potential_kvar
        {1,  4,      100,      -100,     {50,50,50,50},     {-25,-25,-25,-25},                  3,     true,  {25,25,25,25},  100,       100           }, // equal charge w/ reactive_power_priority true
        {2,  4,      100,      100,      {10,45,55,90},     {0.101,9.228,16.848,71.41},         3,     true,  {25,25,25,70},  100,       100           }, // wide spread discharge w/ reactive_power_priority true
		{3,  4,      100,      -100,     {10,45,55,90},     {-73.823,-16.848,-9.228,-0.101},    3,     true,  {100,25,25,25}, 100,       100           }, // wide spread charge w/ reactive_power_priority true
		{4,  4,      100,      100,      {49,49.5,50.5,51}, {23.512,24.239,25.738,26.510},      3,     true,  {25,25,25,25},  100,       100           }, // narrow spread discharge w/ reactive_power_priority true
		{5,  4,      100,      -100,     {49,49.5,50.5,51}, {-26.510,-25.738,-24.239,-23.512},  3,     true,  {25,98,25,25},  100,       100           }  // narrow spread charge w/ reactive_power_priority true
    };

	for(auto test : tests) {
		test_logger t_log("calculate_ess_active_power_with_reactive_power_priority", test.id, tests.size());
		ESS_Manager_Mock ess_mgr;

		// build a JSON with this test case's number of ESS and power rating
		cJSON* ess_config = ess_mgr.generate_calculatePower_essRoot(test.num_ess, test.rated_kw / test.num_ess, 0);
		defer { cJSON_Delete(ess_config); };

		// configure ESS Manager
		std::map <std::string, std::vector<Fims_Object*>> component_var_map;
		std::map <std::string, Fims_Object*> asset_var_map;
        Type_Configurator ess_configurator(&ess_mgr, &component_var_map, &asset_var_map, &primary_controller);
        ess_configurator.assetTypeRoot = ess_config;
        ess_configurator.config_validation = false;
		bool configure_success = ess_configurator.create_assets();
		ASSERT_TRUE(configure_success);

        // set test state
		ess_mgr.set_ess_all_dischargeable_powers(test.rated_kw);
        ess_mgr.set_ess_all_chargeable_powers(test.rated_kw);
		ess_mgr.set_ess_all_potential_reactive_powers(test.potential_kvar);
		ess_mgr.set_ess_all_reactive_power_setpoints(test.kvar_setpoints);
		ess_mgr.set_ess_all_reactive_power_priority(test.is_reactive_power_priority);
		ess_mgr.set_ess_target_active_power(test.target_kw);
		ess_mgr.set_ess_all_rated_apparent_power(test.rated_kva);
        ess_mgr.set_ess_all_socs(test.socs);
		ess_mgr.run_asset_instances(test.num_ess);
		ess_mgr.set_demand_modes();
        ess_mgr.set_soc_balancing_factor(test.soc_balancing_factor);		
		ess_mgr.process_all_ess_potential_active_power();

		// calculate test results
		ess_mgr.calculate_ess_active_power();

		// check results
		for(int i = 0; i < test.num_ess; ++i) {
			t_log.range_results.push_back({
				test.result_kw_setpoints[i], // expected
				0.1F, // % tolerance
				ess_mgr.get_asset_active_power_setpoint(i), // actual
				"ess_" + std::to_string(i) // result name
			});
		}
		t_log.check_solution();
	}
}

// Variable endpoint functions
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

void ESS_Manager_Mock::set_ess_all_socs(const std::vector<float> &socs)
{
	for(size_t i = 0; i < pEss.size(); ++i) {
		pEss[i]->soc->value.set(socs[i]);
	}
}

void ESS_Manager_Mock::set_ess_all_dischargeable_powers(float _dp)
{
	for(auto ess : pEss) {
		ess->dischargeable_power->value.set(_dp);
	}
}

void ESS_Manager_Mock::set_ess_all_chargeable_powers(float _cp)
{
    for(auto ess : pEss) {
		ess->chargeable_power->value.set(_cp);
	}
}

void ESS_Manager_Mock::set_ess_all_max_limited_active_powers(float max_ap) 
{
	for(auto ess : pEss) {
		ess->max_limited_active_power = max_ap;
	}
}

void ESS_Manager_Mock::set_ess_all_min_limited_active_powers(float min_ap) 
{
	for(auto ess : pEss) {
		ess->min_limited_active_power = min_ap;
	}
}

void ESS_Manager_Mock::set_ess_all_reactive_power_setpoints(const std::vector<float> &reactive_setpoints)
{
	for(size_t i = 0; i < pEss.size(); ++i) {
		pEss[i]->set_reactive_power_setpoint(reactive_setpoints[i]);
	}
}

void ESS_Manager_Mock::set_ess_all_rated_apparent_power(float power)
{
	for(auto ess : pEss) {
		ess->rated_apparent_power_kva = power;
	}
}

void ESS_Manager_Mock::set_ess_all_reactive_power_priority(bool is_reactive_power_priority)
{
	for(auto ess : pEss) {
		ess->reactive_power_priority = is_reactive_power_priority;
	}
}

void ESS_Manager_Mock::set_ess_all_potential_reactive_powers(float rated_rp)
{
	for(auto ess : pEss) {
		ess->potential_reactive_power = rated_rp;
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
	for(auto ess : pEss) {
		ess->assetControl = Direct;
	}
}

void ESS_Manager_Mock::process_all_ess_potential_active_power()
{
	for(auto &ess : pEss) {
		// set the ESS's active power slew's clock back 2 seconds so it thinks a lot of time has passed and will allow a large swing of power.
		// this removes the slew from limiting the unit test and thus eliminating any time-based component of the unit test (time-based testing should be in regression tests)
		ess->active_power_slew.reset_slew_target(2);
		ess->process_potential_active_power();
	}
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