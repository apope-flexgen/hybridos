#include <Config_Validation_Result.h>

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Logger.h>

Config_Validation_Result::Config_Validation_Result(bool is_valid_config) {
    this->is_valid_config = is_valid_config;
}

Config_Validation_Result::Config_Validation_Result(bool is_valid_config, std::string brief) {
    this->is_valid_config = is_valid_config;
    this->brief = brief;
}

// Combine another result into this result by anding their values and concatenating their details
// (results in no change to the brief)
void Config_Validation_Result::absorb(const Config_Validation_Result& other) {
    is_valid_config = is_valid_config && other.is_valid_config;
    INFO_details.insert(INFO_details.end(), other.INFO_details.begin(), other.INFO_details.end());
    WARNING_details.insert(WARNING_details.end(), other.WARNING_details.begin(), other.WARNING_details.end());
    ERROR_details.insert(ERROR_details.end(), other.ERROR_details.begin(), other.ERROR_details.end());
}

// Log details in order of severity (most severe appearing last)
void Config_Validation_Result::log_details() const {
    for (Result_Details details : INFO_details) {
        // inlined FPS_INFO_LOG with function call location modified
        MANUAL_FPS_INFO_LOG(details.file, details.function, details.line, details.details);
    }
    for (Result_Details details : WARNING_details) {
        // inlined FPS_WARNING_LOG with function call location modified
        MANUAL_FPS_WARNING_LOG(details.file, details.function, details.line, details.details);
    }
    for (Result_Details details : ERROR_details) {
        // inlined FPS_ERROR_LOG with function call location modified
        MANUAL_FPS_ERROR_LOG(details.file, details.function, details.line, details.details);
    }
}
