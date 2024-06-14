// gcom_stats.cpp
// p. wilshire
// s .reynolds
// 11_08_2023
// self review 11_22_2023

#include <sstream>

#include <unistd.h>

#include "version.h"
#include "gcom_config.h"
#include "gcom_iothread.h"
#include "gcom_fims.h"
#include "gcom_timer.h"
void getTimerObjects(std::stringstream &ss);



void get_stats(std::stringstream &ss, struct cfg &myCfg) {
    ss << "{";
    ss << "\"config_filename\":\"" << myCfg.filename << "\",";
    ss << "\"pid\":" << getpid() << ",";
    ss << "\"git_build\":" << myCfg.git_version_info.get_build() << ",";
    ss << "\"git_commit\":" << myCfg.git_version_info.get_commit() << ",";
    ss << "\"git_tag\":" << myCfg.git_version_info.get_tag() << ",";
    formatThreadConnectionInfo(ss, true);
    ss << ",";

    //ss << "\"multi_pub_message_prep_times\":";
    //myCfg.performance_timers.pub_timer.showNum(ss);
    //ss << ",";
    ss << "\"pub_time\":{";
    bool first = true;
    for (std::pair<const std::string, std::shared_ptr<cfg::pub_struct>> pub_pair : myCfg.pubs){
        if(first){
            first = false;
        } else {
            ss << ",";
        }
        ss << "\"" << pub_pair.first << "\":";
        pub_pair.second->pubStats.showNum(pub_pair.second->pmtx, ss);
    }
    ss << "},";
    // TODO possibly lock here
    ss << "\"fims_message_stats\":";
    myCfg.fims_message_stats.showNum(ss);
    ss << ",";
    ss << "\"fims_get_errors\":";
    ss << myCfg.fims_get_errors;
    ss << ",";
    ss << "\"fims_set_errors\":";
    ss << myCfg.fims_set_errors;
    ss << ",";
    ss << "\"fims_pub_errors\":";
    ss << myCfg.fims_pub_errors;
    ss << ",";
    getTimerObjects(ss); 
    ss << ",";
    ss << "\"heartbeat_values\":{";
    first = true;
    for (std::pair<const std::string, std::shared_ptr<cfg::heartbeat_struct>> heartbeat_pair : myCfg.heartbeat_points){
        if(first){
            first = false;
        } else {
            ss << ",";
        }
        ss << "\"" << heartbeat_pair.first << "\":";
        if(heartbeat_pair.second->enabled){
        ss << heartbeat_pair.second->value;
        } else{
            ss << "\"DISABLED\"";
        }
    }
    // ss << "},";
    // ss << "\"heartbeat_stats\":{";
    // first = true;
    // for (std::pair<const std::string, std::shared_ptr<cfg::heartbeat_struct>> heartbeat_pair : myCfg.heartbeat_points){
    //     if(first){
    //         first = false;
    //     } else {
    //         ss << ",";
    //     }
    //     ss << "\"" << heartbeat_pair.first << "\":";
    //     heartbeat_pair.second->heartbeatStats.showNum(ss);
    // }
    ss << "}";
    // ss << "\"watchdog_states\":{";
    // first = true;
    // for (std::pair<const std::string, std::shared_ptr<cfg::watchdog_struct>> watchdog_pair : myCfg.watchdog_points){
    //     if(first){
    //         first = false;
    //     } else {
    //         ss << ",";
    //     }
    //     ss << "\"" << watchdog_pair.first << "\":";
    //     if (!watchdog_pair.second->enabled){
    //         ss << "\"DISABLED\"";
    //     }
    //     else {
    //         ss << "\"" << watchdog_pair.second->state_str << "\"";
    //     }
    // }
    // ss << "},";
    // ss << "\"watchdog_stats\":{";
    // first = true;
    // for (std::pair<const std::string, std::shared_ptr<cfg::watchdog_struct>> watchdog_pair : myCfg.watchdog_points){
    //     if(first){
    //         first = false;
    //     } else {
    //         ss << ",";
    //     }
    //     ss << "\"" << watchdog_pair.first << "\":";
    //     watchdog_pair.second->watchdogStats.showNum(ss);
    // }
    // ss << "}";
    ss << "}";
}

void pubStatsCallback(std::shared_ptr <TimeObject>t, void *p){
    cfg *myCfg = (cfg *)p;
    if(myCfg->pub_stats)
    {
        std::stringstream ss;

        get_stats(ss, *myCfg);
        send_pub(myCfg->fims_gateway, std::string_view(myCfg->connection.stats_uri), std::string_view(ss.str()));
    }
}

void gcom_setup_stats_pubs(struct cfg &myCfg, double init_time)
{
    if (myCfg.connection.stats_frequency_ms > 0)
    {
        std::shared_ptr<TimeObject> obj1 = createTimeObject("stats_pubs", // name
                                                            init_time, // start time (initial startup time)
                                                            0,          // stop time - 0 = don't stop
                                                            myCfg.connection.stats_frequency_ms/1000.0, // how often to repeat
                                                            0,            // count - 0 = don't stop
                                                            pubStatsCallback,  // callback
                                                            &myCfg); // callback params
        // TimeObject obj1(mypub.first, 5.0,   0,    mypub.second->frequency/1000.0,  mypub.second->offset_time/1000.0,   pubCallback, (void *)mypub.second);
        addTimeObject(obj1, 1, false);
    }
}