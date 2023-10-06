#ifndef CONFIG_VALIDATION_RESULT_H_
#define CONFIG_VALIDATION_RESULT_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
#include <string>
#include <vector>
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */

// Contains the detailed results of validating a config, whether the config was valid or not and any
// additional details as strings
struct Config_Validation_Result {
    bool is_valid_config;  // true if the config is valid
    std::string brief;     // general description of the result which can be used as a simpler alternative to details

    std::vector<std::string> INFO_details;     // info level details
    std::vector<std::string> WARNING_details;  // warning level details
    std::vector<std::string> ERROR_details;    // error level details

    Config_Validation_Result() = default;
    Config_Validation_Result(bool is_valid_config);
    Config_Validation_Result(bool is_valid_config, std::string brief);

    void absorb(const Config_Validation_Result& other);

    void log_details_helper(const char* file, const char* func, int line) const;
};

// Log details in order of severity (most severe appearing last).
// This macro captures the location in which it is called so that the log can note the correct file, function and line number.
#define LOG_DETAILS_OF_CONFIG_VALIDATION_RESULT(result) (((Config_Validation_Result)result).log_details_helper(__FILE__, __FUNCTION__, __LINE__))

#endif /* CONFIG_VALIDATION_RESULT_H_ */