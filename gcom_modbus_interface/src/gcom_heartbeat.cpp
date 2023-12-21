// gcom_heartbeat.cpp
// p. wilshire
// 11_08_2023
// self review 11_11_2023

#include <iostream>
#include "gcom_config.h"
#include "gcom_iothread.h"
#include "gcom_timer.h"
#include "logger/logger.h"
#include "gcom_modbus_utils.h"


void hbCallback(std::shared_ptr<TimeObject> t, void *p)
{
    bool debug = false;

    cfg::heartbeat_struct *myhb = static_cast<cfg::heartbeat_struct *>(p);//, [](cfg::heartbeat_struct *) {});
    struct cfg* myCfg = myhb->cfg;
    
    const char *connection_name = myCfg->connection.name.c_str();
    debug = myCfg->debug_hb;
    double tNow = get_time_double();
    // create an io_work object 
    auto io_write_point = myhb->heartbeat_write_point;
    auto io_read_point = myhb->heartbeat_read_point;
    // if the read value has not changed within the component_heartbeat_timeout period we have a problem
    if (!io_read_point)
    {
        if(debug)
            std::cout << " heartbeat callback  [" << myhb->id << "] time :"<< tNow << " no read_point defined" 
                        << std::endl;
        //myhb->enabled =  false; 
    }
    if(debug)
        std::cout 
            << " heartbeat callback name  [" << myhb->id 
            << "] heartbeat init [" << (myhb->init?"true":"false")
            << "] time :"<< tNow 
            << std::endl;

    std::string smode("set");
    std::string gmode("get");
    std::string rep("");
    if(io_read_point)
    {

        // so if the update time moved but the value did not the component is connectd but stuck
        // so if the update time did not move we may not be connected
        std::unique_lock<std::mutex> lock(io_read_point->mtx); 

        auto num_changes = myhb->num_changes;
        auto last_val = myhb->last_val;
        auto raw_val = io_read_point->raw_val;
        auto tUpdate = io_read_point->tUpdate;
        if(debug)std::cout << " heartbeat value check  read_point  [" << io_read_point->id 
                        << "] data_error  [" << (io_read_point->data_error?"true":"false")
                        << "] first_val  [" << (myhb->first_val?"true":"false")
                        << "]  with value : [" << raw_val 
                        << "]  last_val: [" << last_val 
                        << "]  num_changes: [" << num_changes 
                        << "]  tUpdate : [" << tUpdate 
                        << "] at time :"<< tNow <<  std::endl; 

        if(myhb->first_val)
        {
            myhb->last_val = raw_val;
            myhb->last_time = tNow;
            myhb->state_str = "INIT";
            // stay in this state until after the first pub time
            myhb->first_val = false;
            myhb->state_frozen = false;
            myhb->state_fault = false;
            myhb->state_normal = false;
            if(debug)std::cout << " heartbeat value started  [" << io_read_point->id 
                    << "]  with value : [" << raw_val 
                    << "]  tUpdate : [" << tUpdate 
                    << "] at time :"<< tNow <<  std::endl; 

        }
        else
        {
            if(!myhb->init && tUpdate > 0.0)
                myhb->init = true;
            // we have to pick up the first change before we monitor
            if (raw_val != myhb->last_val)
            {
                if(debug)std::cout << " heartbeat value changed  [" << io_read_point->id 
                            << "]  from value : [" << myhb->last_val 
                            << "]  to value : [" << raw_val 
                            << "]  tUpdate : [" << tUpdate 
                            << "]  init : [" << myhb->init 
                            << "] at time :"<< tNow <<  std::endl; 
                myhb->num_changes++;
                myhb->value_changed = true;
                myhb->last_val = raw_val;
                myhb->last_time = tNow;
            }
        }
        bool normal =  false;
        if(myhb->init)
        {
            normal = true;
            if ((tNow - myhb->last_time) > (myhb->timeout/1000.0))
            {
                if (!myhb->state_frozen)
                {
                    myhb->state_str = "FROZEN";
                    if(debug)std::cout << " heartbeat value frozen [" << io_read_point->id 
                        << "]  value : [" << raw_val 
                        << "] at time :"<< tNow <<  std::endl; 
                    myhb->state_frozen = true;
                    std::string message = fmt::format("Heartbeat [{}] [{}] has entered the [{}] state.", 
                            myhb->id, io_read_point->id, myhb->state_str);
                    FPS_ERROR_LOG(message);
                    emit_event(&myCfg->fims_gateway, connection_name, message.c_str(), 3);

                }
                if (!myhb->state_fault)
                {
                    if ((tNow - myhb->last_time) > (3*(myhb->timeout/1000.0)))
                    {
                        myhb->state_str = "FAULT";

                        if(debug)std::cout << " heartbeat value frozen [" << io_read_point->id 
                            << "]  value : [" << raw_val 
                            << "] at time :"<< tNow <<  std::endl; 
                        myhb->state_fault = true;
                        std::string message = fmt::format("Heartbeat [{}] [{}] has entered the [{}] state.", 
                            myhb->id, io_read_point->id, myhb->state_str);
                        FPS_ERROR_LOG(message);
                        emit_event(&myCfg->fims_gateway, connection_name, message.c_str(), 3);
                    }
                }
                normal = false;

            }
            if(normal)
            {
                if (!myhb->state_normal)
                {
                    if (myhb->state_frozen)
                    {
                        if(debug)std::cout << " heartbeat unfrozen [" << myhb->id << "] at time :"<< tNow <<  std::endl; 
                        myhb->state_frozen = false;
                    }

                    if (myhb->state_fault)
                    {
                        if(debug)std::cout << " heartbeat unfaulted [" << myhb->id << "] at time :"<< tNow <<  std::endl; 
                        myhb->state_fault = false;
                    }

                    myhb->state_normal = true;
                    myhb->state_str = "NORMAL";

                    std::string message = fmt::format("Heartbeat [{}] id [{}] has entered the [normal] state.", myhb->id, io_read_point->id);
                    FPS_INFO_LOG(message);
                    if(debug)std::cout << message <<" at time :"<< tNow <<  std::endl; 
                    emit_event(&myCfg->fims_gateway, connection_name, message.c_str(), 3);
                }
            }
            else
            {
                if(debug)std::cout << " heartbeat not now normal [" << myhb->id << "] at time :"<< tNow <<  std::endl; 
                myhb->state_normal = false;
            }
        }
        //if tNow > timeout set error state
        // if was not in error state then notify

        myhb->value = raw_val + 1;
        if(debug)std::cout << " >>>>heartbeat callback  #1 [" << myhb->id << "] time :"<< tNow 
                     << " value :" << myhb->value 
                     << " enabled :" << myhb->enabled 
                     << " write io_point :" << io_write_point 
                     << " read io_point :" << io_read_point 
                     << " time read  update:" << tUpdate
                     << std::endl; 

    }
    else  // no read point just source a value
    {
        myhb->value += 1;
    }

    if (myhb->max_value > 0)
        if(myhb->value > myhb->max_value)
            myhb->value = 0;
    
    if(debug)std::cout << " >>>>>>heartbeat callback  #2 [" << myhb->id << "] time :"<< tNow  
                    << " max_value " << myhb->max_value
                    << " value " << myhb->value
                    << " tlastUpdate " << (io_read_point?io_read_point->tlastUpdate:0.0)
                    << std::endl;
    if(myhb->enabled && io_read_point && io_read_point->tlastUpdate > 0.0)
    {
        // if ((io_read_point->register_type == cfg::Register_Types::Holding && !myCfg->pub_holding) 
        //     || (io_read_point->register_type == cfg::Register_Types::Input && !myCfg->pub_input)) 
        // {
        //     if(debug)std::cout << " heartbeat callback  #3 [" << myhb->id << "] time :"<< tNow  
        //             << std::endl;

        //     // we'll have to do a special read
        //     auto io_work = make_work(io_read_point->register_type, io_read_point->device_id, io_read_point->offset, io_read_point->size,
        //             io_read_point->reg16, io_read_point->reg8, strToWorkType(gmode, false));
        //     io_work->wtype = WorkTypes::Get;

        //     io_work->io_points.emplace_back(io_read_point);
        //     io_work->tNow = tNow;
        //     io_work->work_name = std::string("");
        //     io_work->erase_group = true;
        //     io_work->work_group = 1;
        //     io_work->work_id = 1;
        //     pollWork(io_work);

        // }
        if(io_write_point)
        {
            if(debug)std::cout   << " heartbeat callback  #3 [" 
                                    << "] value [" <<myhb->value << "] "
                                    << myhb->id 
                                    << " ] size [ "
                                    << io_write_point->size
                                    << "] time :"<< tNow  
                                    << std::endl;

            auto io_work = make_work(io_write_point->register_type, io_write_point->device_id, io_write_point->offset, io_write_point->size,
                    io_write_point->reg16, io_write_point->reg8, strToWorkType(smode, false));
            io_work->wtype = WorkTypes::Set;

            set_reg16_from_uint64(io_write_point, myhb->value, io_work->buf16);

            io_work->io_points.emplace_back(io_write_point);
            io_work->tNow = tNow;
            io_work->work_name = std::string("");
            io_work->erase_group = true;
            io_work->work_group = 1;
            io_work->work_id = 1;

            setWork(io_work);
        }
    }
}

// 
void cfg::heartbeat_struct::setupHb(struct cfg &myCfg, const std::string &name, int device_id
                                        , std::shared_ptr<struct io_point_struct> io_point
                                        , std::shared_ptr<struct io_point_struct> io_ref
                                        , int hbIntTime , int hbIntOffset)
{
    double hbTime = double(hbIntTime / 1000.0);
    double hbOffset = double(hbIntOffset / 1000.0);

    if(heartbeat_write_point)
        FPS_INFO_LOG("Setting up heartbeat %s for [%s] every %f seconds", name, heartbeat_write_point->id, hbTime);
    if(heartbeat_read_point)
        FPS_INFO_LOG("Setting up heartbeat %s for [%s] every %f seconds", name, heartbeat_read_point->id, hbTime);
    if(!heartbeat_read_point && !heartbeat_write_point)
    {
        std::string message = fmt::format("Heartbeat [{}] NO INPUT or OUTPUT POINT SPECIFIED", name);
        FPS_ERROR_LOG(message);
        return;
    }
    int init_time = 1;
    cfg = &myCfg;
    first_val = true;
    init = false;
 
    hbOffset = 0;
    std::shared_ptr<TimeObject> obj1 = createTimeObject(name,                 // name
                                                            init_time,                   // start time (initial startup time)
                                                            0,                           // stop time - 0 = don't stop
                                                            hbTime, // how often to repeat
                                                            0,                           // count - 0 = don't stop
                                                            hbCallback,                 // callback
                                                            (void *)this);         // callback params
   addTimeObject(obj1, hbOffset, false);
}
