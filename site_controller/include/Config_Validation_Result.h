#ifndef Config_Validation_Result_H_
#define Config_Validation_Result_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Types.h>

// Holds the detail message as well as the file, function, and line number of the calling function
struct Result_Details {
    std::string details;
    char file[SHORT_MSG_LEN];
    char function[SHORT_MSG_LEN];
    int line;

    Result_Details(std::string details, const char* file, const char* function, const int line) {
        strncpy(this->file, file, SHORT_MSG_LEN_COPY);
        strncpy(this->function, function, SHORT_MSG_LEN_COPY);
        this->line = line;
        this->details = details;
    }

    // Equality check. Only used for testing currently
    // Returns true if the details of this result contains the string passed as a parameter
    bool operator==(const std::string& other) const { return (details.find(other) != std::string::npos); }
};
#define Result_Details(details) (Result_Details((std::string)details, __FILE__, __FUNCTION__, __LINE__))

// Contains the detailed results of validating a config, whether the config was valid or not and any
// additional details as strings
struct Config_Validation_Result {
    bool is_valid_config;  // true if the config is valid
    std::string brief;     // general description of the result which can be used as a simpler alternative to details

    std::vector<Result_Details> INFO_details;     // info level details
    std::vector<Result_Details> WARNING_details;  // warning level details
    std::vector<Result_Details> ERROR_details;    // error level details

    Config_Validation_Result() = default;
    Config_Validation_Result(bool is_valid_config);
    Config_Validation_Result(bool is_valid_config, std::string brief);

    void absorb(const Config_Validation_Result& other);

    void log_details() const;
};

#endif /* Config_Validation_Result_H_ */
