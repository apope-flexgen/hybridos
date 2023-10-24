#ifndef GENERATOR_TEST_H_
#define GENERATOR_TEST_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Logger.h>
#include "Generator_Manager.h"
#include <Configurator.h>

class Generator_Manager_Mock : public Generator_Manager {
public:
    Generator_Manager_Mock() {}

    cJSON* generate_genRoot(int numParse);
    cJSON* generate_stop_mask_root(uint64_t mask, int numParse);
};

class generator_manager_test : public testing::Test {
public:
    Generator_Manager_Mock* genMgr;

    bool* primary_controller;

    virtual void SetUp() {
        genMgr = new Generator_Manager_Mock();
        primary_controller = new bool(true);
    }

    // The object is not fully configured causing delete to issue invalid frees
    // The memory will be properly freed on termination
    // virtual void TearDown()
    // {
    // 	delete genMgr;
    // }
};

/*
        The googletest needs actual asset instances to work with. The asset_create function is what gets the asset instance data from
        assets.json and configures the type manager. This function makes a fake version of that assets.json data for the type manager
        to be configured with. This is also where one would add specific initialization data such as variable initial values or how
        many assets to make.
*/
cJSON* Generator_Manager_Mock::generate_genRoot(int numParse) {
    std::stringstream ss;
    // Insert the array header and first generator instance since it doesn't have a comma at the beginning
    ss << "{\"asset_instances\":[{\"id\":\"gen_1\",\"name\":\"Generator Unit 01\",\"modesInit\":[],\"components\":[]}";
    // Insert any additional generator instances
    for (int i = 1; i < numParse; ++i) {
        ss << ",{\"id\":\"gen_" << i + 1 << "\",\"name\":\"Generator Unit 0" << i + 1 << "\",\"modesInit\":[],\"components\":[]}";
    }
    // Close the array
    ss << "]}";
    // Print the final genRoot
    std::string s = ss.str();
    std::cout << "Parsed genRoot:" << s << std::endl;
    return cJSON_Parse(s.c_str());
}

#endif /* Generator_TEST_H_ */
