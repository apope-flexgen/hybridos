/*
 * Path.h
 *
 *  Created on: Nov 5, 2019
 *      Author: kbrezina
 */

#ifndef INCLUDE_PATH_H_
#define INCLUDE_PATH_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
#include <vector>
#include <string>
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Step.h>
#include <macros.h>
#include <Types.h>

// predeclaration of some classes to use below
// need this to compile
class Site_Manager;
class Asset;
class Asset_ESS;
class Asset_Solar;
class Asset_Generator;
class Asset_Feeder;

class Path {
public:
    // using a union that can be either a pointer to a site or a pointer to an asset
    // site = sequences.json
    // others = actions.json
    union {
        Site_Manager* site;
        Asset_ESS* asset_ess;
        Asset_Solar* asset_solar;
        Asset_Generator* asset_generator;
        Asset_Feeder* asset_feeder;
    };

    std::string path_name;
    std::string return_id;
    states return_state;
    int timeout;  // each sequence will have independent timeout to fault if a step does not complete in time
    std::vector<Step> steps;

    // The first item in each pair is the fault name, and the second item in each pair is the fault value
    std::vector<std::pair<std::string, uint64_t>> faults;
    int num_active_faults;

    // The first item in each pair is the alarm name, and the second item in each pair is the alarm value
    std::vector<std::pair<std::string, uint64_t>> alarms;
    int num_active_alarms;

    // constructors 
    Path();  
    Path(Site_Manager* siteref);

    // funcs
    bool configure_path(cJSON* object, bool must_have_return_id = true);
    bool check_alerts(alert_type type, Sequence_Type sequence_type);
    void handle_site_check_alerts(alert_type type_of_alert);
    void handle_ess_check_alerts(alert_type type_of_alert);
    void handle_solar_check_alerts(alert_type type_of_alert);
    void handle_gen_check_alerts(alert_type type_of_alert);
    void handle_feeder_check_alerts(alert_type type_of_alert);
    bool handle_return_id(cJSON* JSON_return_id);
    bool handle_faults(cJSON* JSON_active_faults);
    bool handle_alarms(cJSON* JSON_active_alarms);
    int collect_seconds_in_path();
};

#endif /* INCLUDE_PATH_H_ */
