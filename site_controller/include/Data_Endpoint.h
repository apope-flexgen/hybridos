#ifndef DATA_ENDPOINT_H_
#define DATA_ENDPOINT_H_

/* External Dependencies */
#include <string.h>
#include <cjson/cJSON.h>
/* System Internal Dependencies */
#include <Logger.h>
/* Local Internal Dependencies */
#include <Site_Manager.h>

// Give enough time to send a response by hand
// Will be reduced based on db response time when the storage module is completed
#define DB_RESPONSE_TIMEOUT 1000000  // in uS

extern fims* p_fims;

class Data_Endpoint {
private:
    fmt::memory_buffer send_FIMS_buf;  // Reusable and resizable string buffer used to send FIMS messages

public:
    std::unordered_map<std::string, std::vector<std::string>> opposite_setpoints;

    Data_Endpoint();
    // Send get to storage module
    char* get_from_uri(std::string uris, std::string replyto);
    // Echo sets to storage module
    bool setpoint_writeout(std::string uri, std::string endpoint, cJSON** valueObject);
    // Request and read user setpoints from storage for configuration
    bool setpoint_readin();
    void turn_off_start_cmd(void);
    // Create a list of uris to which the set should be written (DBI)
    std::vector<std::string> construct_writeout_uris(std::string uri, std::string endpoint);
    // Create a fims_message given the paramaters parsed
    fims_message* construct_fims_message(cJSON* variable, std::string method, int nfrags, std::string pfrags_0, std::string pfrags_1, std::string pfrags_2, std::string pfrags_3);
};

#endif /* DATA_ENDPOINT_H_ */
