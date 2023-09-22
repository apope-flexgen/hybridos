/**
 * TYPE_Manager.h
 * Header for TYPE-specific Manager class
 * Refactored from Asset_Manager.h
 *
 * Created on Sep 30th, 2020
 *      Author: Jack Shade (jnshade)
 */

#ifndef TYPE_MANAGER_H_
#define TYPE_MANAGER_H_

/* C Standard Library Dependencies */
#include <string.h>
/* C++ Standard Library Dependencies */
/* External Dependencies */
#include <Logger.h>
/* System Internal Dependencies */
#include <fims/libfims.h>
/* Local Internal Dependencies */
#include <macros.h>
#include <Types.h>
class Type_Configurator;
class Asset;

class Type_Manager {
protected:
    int numRunning;
    int numAvail;
    int numParsed;
    int num_in_local_mode;
    const char* asset_type_id;

    fmt::memory_buffer send_FIMS_buf;  // Reusable and resizable string buffer used to send FIMS messages

    std::vector<Asset*> pAssets;

    // Adds generated asset type summary to buffer and returns true if successful
    virtual void generate_asset_type_summary_json(fmt::memory_buffer& buf, const char* const var = NULL) = 0;

    // Helper function for finding the appropriate index into pType based on the name/uri in asset_var_map
    int map_to_array_translation(std::string name);
    Asset* get_asset_instance(const char* asset_id);
    Asset* get_asset_instance_using_uri_frags(char** pfrags, size_t asset_id_index);

public:
    Type_Manager(const char* _asset_type_id);
    ~Type_Manager();
    const char* get_asset_type_id();

    // configuration functions
    void print_alarm_fault_map();
    virtual void configure_base_class_list(void) = 0;
    virtual bool configure_type_manager(Type_Configurator* configurator) = 0;
    virtual Asset* build_new_asset(void) = 0;
    virtual void append_new_asset(Asset*) = 0;

    // Fims
    bool add_type_data_to_buffer(fmt::memory_buffer& buf);
    bool handle_get(fims_message* pmsg);
    bool handle_summary_get(fims_message* pmsg);
    void handle_set(fims_message& msg);

    int get_num_avail(void);
    int get_num_parsed(void);
    int get_num_running(void);
    int get_num_in_local_mode(void);

    void publish_assets(asset_type type);
    void send_to_components(void);

    void set_clear_faults(void);
    void set_reactive_power_priority(bool priority);

    int get_num_active_alarms() const;
    int get_num_alarmed() const;
    int get_num_active_faults() const;
    int get_num_faulted() const;
    bool check_asset_for_alert(std::string& asset_id, std::string& alert_id, uint64_t& mask);
};

#endif /* TYPE_MANAGER_H_ */
