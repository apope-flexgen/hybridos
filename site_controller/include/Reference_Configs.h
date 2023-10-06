#ifndef REFERENCE_CONFIGS_H_
#define REFERENCE_CONFIGS_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
#include <string>
#include <stdexcept>
/* External Dependencies */
#include <cjson/cJSON.h>
/* System Internal Dependencies */
/* Local Internal Dependencies */

// Contains default configuration for variables config variables
struct Reference_Configs {
public:
    // Default configuration for variables config variables.
    // The structure is the same as a variables.json, so it needs to be parsed before it can be used.
    cJSON* variables_json_defaults;

    Reference_Configs();
    ~Reference_Configs();

    // Copying is not allowed
    Reference_Configs(const Reference_Configs& other) = delete;
    Reference_Configs& operator=(const Reference_Configs& other) = delete;

private:
    static const std::string variables_json_defaults_string;
};

#endif /* REFERENCE_CONFIGS_H_ */
