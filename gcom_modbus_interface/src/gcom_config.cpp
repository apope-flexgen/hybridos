// gcom_config.cpp
// p. wilshire
// s .reynolds
// 11_08_2023
// self review 11_22_2023

#include <iostream>
#include "gcom_config.h"
#include "logger/logger.h"

cfg::Register_Types cfg::typeFromStr(std::string &register_type_str)
{
    // now we get modbus specific
    if (register_type_str == "Holding" || register_type_str == "holding_registers"|| register_type_str == "Holding Registers")
        return Register_Types::Holding;
    else if (register_type_str == "Coil" || register_type_str == "coils")
        return Register_Types::Coil;
    else if (register_type_str == "Input" || register_type_str == "Input Registers"|| register_type_str == "input_registers")
        return Register_Types::Input;
    else if (register_type_str == "Discrete_Input" 
                                        || register_type_str == "discrete_inputs" 
                                            || register_type_str == "Discrete Inputs" 
                                                || register_type_str == "Discrete Input") 
        return Register_Types::Discrete_Input;
    else
        return Register_Types::Undefined;
}

std::string cfg::typeToServerStr(cfg::Register_Types register_type)
{
    // now we get modbus specific
    if (register_type == Register_Types::Holding)
        return "holding_registers";
    else if (register_type == Register_Types::Coil)
        return "coils";
    else if (register_type == Register_Types::Input)
        return "input_registers";
    else if (register_type == Register_Types::Discrete_Input)
        return "discrete_inputs";
    else
        return "Undefined";
}

std::string cfg::typeToStr(cfg::Register_Types register_type)
{
    // now we get modbus specific
    if (register_type == Register_Types::Holding)
        return "Holding";
    else if (register_type == Register_Types::Coil)
        return "Coil";
    else if (register_type == Register_Types::Input)
        return "Input";
    else if (register_type == Register_Types::Discrete_Input)
        return "Discrete_Input";
    else
        return "Undefined";
}

void cfg::addWatchdog(const std::string &base, const std::string &component_name, std::shared_ptr<component_struct> component, struct cfg *myCfg)
{
    std::string pstr = "wd_" + base + "_" + component_name;
    std::shared_ptr<cfg::io_point_struct> io_point;
    if (component->watchdog_enabled && gcom_findCompVar(io_point, *myCfg, component.get(), component->watchdog_uri))
         return cfg::addWatchdog(io_point, pstr, component, myCfg);
    return;
}


// old system watchdogs were called hearbeats
void cfg::addWatchdog(std::shared_ptr<io_point_struct> io_point, std::string &pstr,std::shared_ptr<component_struct> component, struct cfg *myCfg)
{
    std::shared_ptr<cfg::watchdog_struct> watchdog = std::make_shared<watchdog_struct>();
 
 //   if (component->watchdog_enabled)// && gcom_findCompVar(io_point, *myCfg, component.get(), component->watchdog_uri))
    {
        if (io_point)
        {
            pstr += "_" + io_point->id;
        }
        component->watchdog = watchdog.get();
        component->watchdog_enabled = true;
        watchdog->id = pstr;
        watchdog->watchdogStats.set_label(pstr);
        watchdog->enabled = true;

        watchdog->alarmTimeout = component->watchdog_alarm_timeout/1000.0;
        watchdog->faultTimeout = component->watchdog_fault_timeout/1000.0;
        watchdog->recoveryTimeout = component->watchdog_recovery_timeout/1000.0;
        watchdog->timeRequiredToRecover = component->watchdog_time_to_recover/1000.0;
        watchdog->frequency = component->watchdog_frequency/1000;

        watchdog->io_point = io_point;
        io_point->is_watchdog = true;
        io_point->watchdog_point = watchdog;
        watchdog_points[pstr] = watchdog;
        watchdog->component = component.get();
        watchdog->device_id = component->device_id;
        watchdog->cfg = this;
        watchdog->setupWatchdogTimer(*myCfg, pstr, watchdog->frequency, component->offset_time/1000);
    }
}

// heartbeat_enabled": true,
//             "component_heartbeat_read_uri": "heartbeat",
//             "component_heartbeat_timeout_ms": 3000,
//             "modbus_heartbeat_timeout_ms": 3000,

void cfg::addOldWatchdog(const std::string &base, const std::string &component_name, std::shared_ptr<component_struct> component, struct cfg *myCfg)
{
    std::string pstr = "wd_" + base + "_" + component_name;
    std::shared_ptr<cfg::io_point_struct> io_point;
    if (component->heartbeat_enabled && gcom_findCompVar(io_point, *myCfg, component.get(), component->component_heartbeat_read_uri))
         return cfg::addOldWatchdog(io_point, pstr, component, myCfg);
    return;
}

// nah deprecated
void cfg::addOldWatchdog(std::shared_ptr<io_point_struct> io_point, std::string &pstr,std::shared_ptr<component_struct> component, struct cfg *myCfg)
{
    std::shared_ptr<cfg::watchdog_struct> watchdog = std::make_shared<watchdog_struct>();
 
 //   if (component->watchdog_enabled)// && gcom_findCompVar(io_point, *myCfg, component.get(), component->watchdog_uri))
    {
        if (io_point)
        {
            pstr += "_" + io_point->id;
        }

        component->watchdog = watchdog.get();
        component->watchdog_enabled = true;
        watchdog->id = pstr;
        watchdog->watchdogStats.set_label(pstr);
        watchdog->enabled = true;

        watchdog->alarmTimeout = 0.0;//component->watchdog_alarm_timeout/1000.0;
        watchdog->faultTimeout = component->component_heartbeat_timeout_ms/1000.0;
        watchdog->recoveryTimeout = 0.0;//component->watchdog_recovery_timeout/1000.0;
        watchdog->timeRequiredToRecover = 0.0;//component->watchdog_time_to_recover/1000.0;
        watchdog->frequency = component->frequency/1000;

        watchdog->io_point = io_point;
        io_point->is_watchdog = true;
        io_point->watchdog_point = watchdog;
        watchdog_points[pstr] = watchdog;
        watchdog->component = component.get();
        watchdog->device_id = component->device_id;
        watchdog->cfg = this;
        watchdog->setupWatchdogTimer(*myCfg, pstr, watchdog->frequency, component->offset_time/1000);
    }
}


// add heartbeat
// need to process for component_connected 
void cfg::addHeartbeat(const std::string &base, const std::string &component_name, std::shared_ptr<component_struct> component, struct cfg *myCfg)
{

    bool debug = myCfg->debug_hb;
    if (!component->heartbeat_enabled){
        FPS_INFO_LOG("Heartbeat disabled");
        return;
    }

    auto myhb = std::make_shared<heartbeat_struct>();
    std::string pstr = "hb_" + base + "_" + component_name;
    myhb->cfg = myCfg;

    myhb->id = pstr;
    myhb->component = component.get();

    myhb->component->heartbeat = myhb.get();

    std::shared_ptr<cfg::io_point_struct> write_point;
    std::shared_ptr<cfg::io_point_struct> read_point;
    myhb->enabled = component->heartbeat_enabled;
    myhb->component_heartbeat_write_uri = component->component_heartbeat_write_uri;
    myhb->component_heartbeat_read_uri = component->component_heartbeat_read_uri;
    myhb->max_value = component->component_heartbeat_max_value;

    myhb->device_id = component->device_id;
    myhb->frequency = component->modbus_heartbeat_timeout_ms;
    myhb->timeout = component->component_heartbeat_timeout_ms;

    if (gcom_findCompVar(write_point, *myCfg, myhb->component, component->component_heartbeat_write_uri))
        myhb->heartbeat_write_point = write_point;
    if (gcom_findCompVar(read_point, *myCfg, myhb->component, component->component_heartbeat_read_uri))
        myhb->heartbeat_read_point = read_point;

    heartbeat_points[pstr] = myhb;
    myhb->component = component.get();
    //myhb->frequency = component->modbus_heartbeat_timeout_ms;
    //myhb->timeout = component->component_heartbeat_timeout_ms;
    myhb->cfg = myCfg;
    if(debug)std::cout   << ">>>>>>>>>" << __func__ << " adding heartbeat"
                << " freq " << myhb->frequency
                << std::endl;

    myhb->setupHb(*myCfg, pstr, myhb->device_id, myhb->heartbeat_write_point, myhb->heartbeat_read_point, myhb->frequency , component->offset_time);
}

void cfg::addPub(const std::string &base, const std::string &component_name, std::shared_ptr<component_struct> component, struct cfg *myCfg)
{
    auto mypub = std::make_shared<pub_struct>();
    std::string pstr = "pub_" + base + "_" + component_name;
    // std::string bstr = "tmr_" + pstr;
    // std::string lstr = "tlate_" + pstr;
    pubs[pstr] = mypub;
    mypub->id = pstr;
    mypub->component = component.get();
    mypub->cfg = myCfg;
    mypub->syncPct = component->syncPct;
    mypub->use_sync = component->use_sync;

    //mypub->count = 0;
    mypub->pubStats.set_label(pstr);
}

void cfg::addSub(const std::string &base, const std::string &component_name)
{
    std::string bstr = "/" + base + "/" + component_name;
    if (std::find(subs.begin(), subs.end(), bstr) == subs.end())
    {
        subs.emplace_back(bstr);
    }
}

std::vector<std::string> *cfg::getSubs()
{
    return &subs;
}
