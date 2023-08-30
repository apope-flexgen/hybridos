#ifndef ASSET_MANAGER_TEST_H_
#define ASSET_MANAGER_TEST_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Logger.h>
#include "Asset_Manager.h"

/* add specific unit-level test headers here*/
#include "asset_test.h"
#include "ess_manager_test.h"
#include "feeder_manager_test.h"
#include "generator_manager_test.h"
#include "solar_manager_test.h"

#include <iostream>
#include <fstream>

/**
 * Parses the config cJSON from storage given the configuration type (Assets/Sequences/Variables)
 */
cJSON* parseJSONConfig(std::string file_path) {
    // open the file
    std::ifstream file;
    try {
        file.open(file_path);
    } catch (std::ifstream::failure e) {
        std::cerr << "Exception opening/reading file" << std::endl;
        return NULL;
    }
    if (!file.is_open()) {
        std::cerr << "Failed to open file" << std::endl;
        return NULL;
    }
    // get number of chars in file
    file.seekg(0, std::ios::end);
    int length = file.tellg();
    file.seekg(0, std::ios::beg);
    // read file into buffer
    char* buffer = new char[length + 1];
    file.read(buffer, length);
    file.close();
    buffer[length] = '\0';
    // parse buffer into cJSON
    cJSON* config = cJSON_Parse(buffer);
    if (config == NULL) {
        std::cerr << "Failed to parse config file" << std::endl;
        return NULL;
    }
    // close file and free buffer
    delete[] buffer;
    return config;
}

class Asset_Manager_Mock : public Asset_Manager {
public:
    Asset_Manager_Mock() {}
};

class asset_manager_test : public testing::Test {
public:
    Asset_Manager_Mock* assetMgr;
    fims* asset_fims;
    fims* test_fims;
    char* subscriptions[1];
    bool* primary_controller;
    // Required to reset the asset manager between tests otherwise get unintended side effects
    void assetMgrReset() {
        // below snippet causes a segfault
        // not sure why
        // so I'm just leaking memory for now it's a unit test it doesn't matter
        /*
        if (assetMgr != NULL)
            delete assetMgr;
        */
        assetMgr = new Asset_Manager_Mock();
    };

    bool test_run(std::string filepath) {
        cJSON* fake_config = parseJSONConfig(filepath);
        bool success = assetMgr->asset_create(fake_config, primary_controller);
        cJSON_Delete(fake_config);
        return success;
    }

    virtual void SetUp() {
        assetMgr = new Asset_Manager_Mock();
        primary_controller = new bool(true);
    }

    virtual void TearDown() {
        // No delete as the object is not fully initialized, causing segfault
        // the memory is still recovered on termination
    }
};

TEST_F(asset_manager_test, asset_create) {
    struct tests {
        std::string filepath;
        bool expected;
    };
    std::vector<tests> test_cases;
    test_cases.push_back({ "unit_tests/unit_test_files/assets/valid_mixed.json", true });
    test_cases.push_back({ "unit_tests/unit_test_files/assets/invalid_ranged.json", false });
    test_cases.push_back({ "unit_tests/unit_test_files/assets/invalid_mixed.json", false });
    test_cases.push_back({ "unit_tests/unit_test_files/assets/invalid_number_of_instances.json", false });

    for (auto test : test_cases) {
        // Only print messages to log if a test fails
        bool failure = false;
        std::stringstream errorLog;
        // Capture any prints within site controller that might be present in debug mode
        capture_stdout();

        errorLog << "Testing: " << test.filepath << std::endl;

        EXPECT_EQ(test_run(test.filepath), test.expected);
        assetMgrReset();  // reset asset manager between tests
        failure = test_run(test.filepath) != test.expected;
        assetMgrReset();  // reset asset manager between tests
        release_stdout(failure);
        if (failure) {
            std::cerr << errorLog.str() << std::endl;
        }
    }
}

#endif /* ASSET_MANAGER_TEST_H_ */
