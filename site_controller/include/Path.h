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
#include <Site_Manager.h>
#include <macros.h>

class Path
{
public:
    Site_Manager* pSite;

    char* path_name;
    char* return_id;
    states return_state;
    int steps_size;
    int timeout;  //each sequence will have independent timeout to fault if a step does not complete in time
    Step* steps;

    // The first item in each pair is the fault name, and the second item in each pair is the fault value
    std::vector<std::pair<std::string,uint64_t>> faults;
    int num_active_faults;

    // The first item in each pair is the alarm name, and the second item in each pair is the alarm value
    std::vector<std::pair<std::string,uint64_t>> alarms;
    int num_active_alarms;

    Path(Site_Manager* siteref);
    virtual ~Path();

    const char* get_name();
    Step* get_step(int n);
    int get_steps_size();
    bool configure_path(cJSON* object, int path_index);
    bool check_alerts(UI_Type type);
    states get_return_state();
};



#endif /* INCLUDE_PATH_H_ */
