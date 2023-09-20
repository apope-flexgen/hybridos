#ifndef EMPTY_FEATURE_H_
#define EMPTY_FEATURE_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>

// A feature implementation which can be instantiated but doesn't actually provide any additional methods or variables
// on its own. Instead, variables and functionality are defined outside of the Feature class.
// The purpose of this class is to allow the construction of Feature instances for features that haven't been encapsulated
// into their own class yet.
// TODO: once every Feature has its own class, remove this class.
class Empty_Feature : public Feature {
public:
    Empty_Feature(){};

// tell the compiler to ignore warnings about unused parameters for the following functions
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    // Returns true and does nothing else.
    bool parse_json_config(cJSON* JSON_config, bool* p_flag, Input_Source_List* inputs, const Fims_Object& field_defaults, std::vector<Fims_Object*>& multiple_inputs) { return true; };
    // Does nothing.
    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override{};
#pragma GCC diagnostic pop
};

#endif /* EMPTY_FEATURE_H_ */