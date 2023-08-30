#ifndef Feeder_TEST_H_
#define Feeder_TEST_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Logger.h>
#include "Feeder_Manager.h"

class Feeder_Manager_Mock : public Feeder_Manager {
public:
    Feeder_Manager_Mock() {}

    cJSON* generate_feedRoot(int numParse);
};

class feeder_manager_test : public testing::Test {
public:
    Feeder_Manager_Mock* feedMgr;

    virtual void SetUp() { feedMgr = new Feeder_Manager_Mock(); }

    virtual void TearDown() { delete feedMgr; }
};

/*
        The googletest needs actual asset instances to work with. The asset_create function is what gets the asset instance data from
        assets.json and configures the type manager. This function makes a fake version of that assets.json data for the type manager
        to be configured with. This is also where one would add specific initialization data such as variable initial values or how
        many assets to make.
*/
cJSON* Feeder_Manager_Mock::generate_feedRoot(int numParse) {
    std::stringstream ss;
    // Insert the array header and first feeder instance since it doesn't have a comma at the beginning
    ss << "{\"asset_instances\":[{\"id\":\"feed_1\",\"name\":\"Feeder 01\",\"modesInit\":[],\"components\":[]}";
    // Insert any additional feeder instances
    for (int i = 1; i < numParse; ++i) {
        ss << ",{\"id\":\"feed_" << i + 1 << "\",\"name\":\"Feeder 0" << i + 1 << "\",\"modesInit\":[],\"components\":[]}";
    }
    // Close the array
    ss << "]}";
    // Print the final feedRoot
    std::string s = ss.str();
    std::cout << "Parsed feedRoot:" << s << std::endl;
    return cJSON_Parse(s.c_str());
}

#endif /* Feeder_TEST_H_ */