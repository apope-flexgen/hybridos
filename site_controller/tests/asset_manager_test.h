#ifndef ASSET_MANAGER_TEST_H_
#define ASSET_MANAGER_TEST_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Logger.h>
#include "Asset_Manager.h"

/* add specific unit-level test headers here*/
#include "ess_manager_test.h"
#include "feeder_manager_test.h"
#include "generator_manager_test.h"
#include "solar_manager_test.h"

class Asset_Manager_Mock : public Asset_Manager
{
	public:
	Asset_Manager_Mock(){}
};

class asset_manager_test : public testing::Test
{
public:
	Asset_Manager_Mock* assetMgr;
	fims* asset_fims;
	fims* test_fims;
	char* subscriptions[1];
	bool* primary_controller;
	
	virtual void SetUp()
	{
	    assetMgr = new Asset_Manager_Mock();
	}

	virtual void TearDown()
	{
		// No delete as the object is not fully initialized, causing segfault
		// the memory is still recovered on termination
	}
};

#endif /* ASSET_MANAGER_TEST_H_ */