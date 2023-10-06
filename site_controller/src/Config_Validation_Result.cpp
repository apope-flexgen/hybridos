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

/**
 * @brief Helper function for log details macro (Do not call this function directly). Logs details in order of
 * severity (most severe appearing last) with logs noting the given function call location.
 * @param file will be filled in by file macro
 * @param func will be filled in by func macro
 * @param line will be filled in by line macro
 */
void Config_Validation_Result::log_details_helper(const char* file, const char* func, const int line) const {
    for (std::string msg : INFO_details) {
        // inlined FPS_INFO_LOG with function call location modified
        ::Logging::log_msg(spdlog::level::info, ::Logging::pre_string(file, func, line), ::Logging::msg_string(msg), ::Logging::post_string());
    }
    for (std::string msg : WARNING_details) {
        // inlined FPS_WARNING_LOG with function call location modified
        ::Logging::log_msg(spdlog::level::warn, ::Logging::pre_string(file, func, line), ::Logging::msg_string(msg), ::Logging::post_string());
    }
    for (std::string msg : ERROR_details) {
        // inlined FPS_ERROR_LOG with function call location modified
        ::Logging::log_msg(spdlog::level::err, ::Logging::pre_string(file, func, line), ::Logging::msg_string(msg), ::Logging::post_string());
    }
}