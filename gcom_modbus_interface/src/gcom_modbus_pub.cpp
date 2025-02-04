
// gcom_modbus_pub.cpp
// p. wilshire
// s .reynolds
// 11_08_2023
// self review 11_11_2023

// this is the minimal pub management system.
// we create a list of IO_work objects that we can send to the io_threads
// they will attempt to  communicate with the server and then send the completed io_work object to the response ( aggregator) thread.
// A "pub" is a collection of io_work items identified by the time of creation, we collect the whole list before sending it to the io thread objects
// so we can mark each io_work object with a serial number (7 of 9)
// The response thread will send all the pub io_work objects into something that will collect all the individually completed io_work responses.
// if an io_work response did not complete because of a bad pointer , a one time error message will be produced.
// the system  decodes the valid response data and encode the fims message.
// The system will drop or ingnore pub collections that have been superceded by another pub request for the same pub type.
// all io_work requests will return within a finite response time with good or bad data.
//
// next we'll have to work out how to handle the bad responses
// invalid data is handled using a modbus_flush
// invalid point means we'll have to disable points and reconstruct the io_work items to skip those points.
// the auto_disable option will allow individual points to be disabled and not scanned during the next cycle.
// we could just bypass the point data
// so the thread will have to handle the skipped points
// we can optionally poll coils and holding registers in the pubs if required

#include <mutex>
#include <iostream>
#include <vector>
#include <any>
#include <map>
#include <climits>

#include "gcom_config.h"
#include "gcom_timer.h"
#include "gcom_iothread.h"
#include "gcom_modbus_pub.h"
#include "logger/logger.h"
#include "gcom_utils.h"

// TODO today 09_29_2023...
// 1..  put enough data in the IO_work to allow the response thread to collate the responses.
// we need to know the number of iowork packets get the size of the io_point_map vector for that.
// then we need to annotate the io_work object with the pub name and then id each packet.
//      done
//
//  work_time 5.40045 work_name pub_components_comp_alt_2440 work_id 1 in  2

// 2.. route the  poll chan now through the io_simulator to populate the responseChannel
// run the response thread and we should get pubs.
// 3.. spit out the basic pubs

// std::cout <<"  Pubs found :" <<std::endl;
// // 2. extract poll groups   -> into myCfg
// //  getPolls(myCfg)
// for (auto &mypub : myCfg.pubs)
// {
//     auto pubs = mypub.second;

//     printf(" pubs %p\n", (void*)pubs.get());
//     std::cout << "pub: "<< mypub.first
//                 << " freq: "<< pubs->component->frequency/1000.0
//                 << " offset: "<< mypub.second->component->offset_time/1000.0
//                     << std::endl;
//     // we need to make these sync objects
//     // the only fire once and wait for a sync
//      std::shared_ptr<TimeObject> obj1 = createTimeObject(mypub.first,
//                                               5.0,
//                                               0,
//                                               mypub.second->component->frequency/1000.0,
//                                               0,
//                                               pubCallback,
//                                               (void *)pubs.get());
//     //TimeObject obj1(mypub.first, 5.0,   0,    mypub.second->frequency/1000.0,  mypub.second->offset_time/1000.0,   pubCallback, (void *)mypub.second);
//     addTimeObject(obj1, mypub.second->component->offset_time/1000.0, true);

// }    // getSubs(myCfg)

std::mutex cb_output_mutex;

// When a pub completes we need to record the completion time. no point in sending out a new pub idthe last one is not yet done.
// I think we'll just skip the current request.
//


void pubCallback(std::shared_ptr<TimeObject>tObj, void *p)
{
    bool debug = false;
    std::lock_guard<std::mutex> lock2(cb_output_mutex);
    std::shared_ptr<cfg::pub_struct> mypub = std::shared_ptr<cfg::pub_struct>(static_cast<cfg::pub_struct *>(p), [](cfg::pub_struct *) {});
    double tNow = get_time_double();
    int num_threads = GetNumThreads(mypub->cfg);
    debug = mypub->cfg->debug_pub;
    //debug = true;    
    if (debug)
    {
        std::cout << "Callback for :" <<tObj->name
                    << " running ;  num_threads: " << num_threads
                    << " missed " << mypub->num_missed
                    << std::endl;
    }

    if (mypub->num_pubs > 0)
    {

        // extract the time taken to complete this pub.
        double tdiff;
        {
            std::lock_guard<std::mutex> lock(mypub->tmtx);
            tdiff = mypub->comp_time - mypub->start_time;
        }
        // if this is less than zero then last pub request was never completed. 
        // mypub->comp_time == 0.0 also indicates we did not get a reply
        if (tdiff < 0.0)
        {
            mypub->num_missed++;
            if(debug)
                std::cout << "pub :" <<tObj->name
                    << " tdiff " << tdiff
                    << " looks like we missed one " << mypub->num_missed
                    << std::endl;
            if (mypub->num_missed > 5)
            {
                //mypub->num_missed++;
                if(debug)
                    std::cout 
                        << "pub :" <<tObj->name
                        << " not skipping too many missed " << mypub->num_missed
                        << std::endl;
                //return;
            }
            // we've missed 5 pubs have to continue.
        }
        else
        {
            if(debug)
                std::cout << "pub :" <<tObj->name
                    << " reset num_missed " << mypub->num_missed
                    << std::endl;
            mypub->num_missed = 0;
        }

    }

    // we are gathering a new pub
    {
        std::lock_guard<std::mutex> lock(mypub->tmtx);

        mypub->start_time = tNow;
        mypub->comp_time = 0.0; // set to tNow when the pub aborts or completes
        if (mypub->num_pubs < INT_MAX) 
            mypub->num_pubs++;
        //mypub->num_missed = 0;
    }
    // maybe add a mutex pointer to the stats start / snap / showNum
    mypub->pubStats.start(mypub->pmtx);
    // std::string pstr = "pub_" + base + "_" + component_name;



    // Dont request pubs if there are no threads running.
    if(num_threads == 0)
    {
        if (debug)
        {
            std::cout << "Callback for :" <<tObj->name
                      << " Skipped : num_threads == 0 : " << num_threads
                      << std::endl;
        }
        mypub->pub_threads = 0;
        //return;
    }

    if (debug)
        std::cout << "Callback for :" <<tObj->name
                  << " executed at : " << tNow
                  << " with sync: " <<tObj->sync
                  << " and run: " <<tObj->run
                  << " and psid: " << mypub->id
                  << " num_threads: " << num_threads
                  << std::endl;
    double tdiff  = 0.0;
    {
        std::lock_guard<std::mutex> lock(mypub->tmtx);
        tdiff = mypub->comp_time - mypub->start_time;
    }
    if(debug)std::cout << "Callback #1 for :"   << tObj->name
                    << " tnow : "       << tNow
                    << " start_time : " << mypub->start_time
                    << " comp_time : "  << mypub->comp_time
                    << " tdiff : "      << tdiff
                    << " num_pubs: "    << mypub->num_pubs
                    << " num_missed: "    << mypub->num_missed
                    << std::endl;
 
    // if (tdiff < 0.0)
    // {
    //     if (mypub->num_missed < 20)
    //     {
    //         mypub->num_missed++;
    //         // std::cout << " Obj :"   << tObj->name
    //         //             << " num_missed: "    << mypub->num_missed
    //         //             << " missed too many"
    //         //             << std::endl;
    //         // return;
    //     }
    //     if(debug)
    //         std::cout << " Obj :"   << tObj->name
    //                     << " running a new request"
    //                     << std::endl;
    // }

    // set up the pup id == tNow
    // TODO set up the timeout callback
    // we have the componentss so find the register groups
    // we have to create an io_work structure.
    // if the register is enabled.
    // register is permanantly if offtime < 0 , register is forced if offtime < 2
    // register is in use if offtime is zero
    // register is temp disabled if offtime > tnow
    // forced value is in forcedbytes

    // Create and IO_Work object ( or get one)
    // bool queue_work(RegisterTypes register_type, int device_id, int offset, int num_regs, uint16_t* u16bufs, uint8_t* u8bufs, WorkTypes work_type )
    //double tNow = get_time_double();
    int work_id = 1;
    cfg::component_struct *compshr = mypub->component;
    bool sent_something = false;

    int num_local_points  = 0;
    int num_remote_points  = 0;
    std::vector<std::shared_ptr<IO_Work>> io_work_vec;
    std::vector<std::shared_ptr<cfg::io_point_struct>> io_map_vec;
    std::vector<std::shared_ptr<cfg::io_point_struct>> local_map_vec;
    for (std::shared_ptr<cfg::register_group_struct> &register_group : compshr->register_groups)
    {
        if ((register_group->register_type == cfg::Register_Types::Coil) && (mypub->cfg->pub_coil == false))
            continue;
        if ((register_group->register_type == cfg::Register_Types::Holding) && (mypub->cfg->pub_holding == false))
            continue;
        if ((register_group->register_type == cfg::Register_Types::Input) && (mypub->cfg->pub_input == false))
            continue;
        if ((register_group->register_type == cfg::Register_Types::Discrete_Input) && (mypub->cfg->pub_discrete == false))
            continue;

        for (const auto &io_point : register_group->io_point_map)
        {
            bool enabled = true;
            // put disabled but forced io_points into a local_map_vec
            if (!io_point->is_enabled && io_point->is_forced)
            {
                local_map_vec.emplace_back(io_point);
                num_local_points++;
            }

            if (!io_point->is_enabled)
            {
                enabled = false;
            }
            // disconnected flag  can be reset after disconnect time 
            // but we cannot do it here without locking the point
            // well we can just try to enable it.
            if (io_point->is_disconnected)
            {
                if (io_point->reconnect > 0 && tNow >io_point->reconnect)
                {
                     //io_point->is_disconnected = false;
                     // allow enabled to add it we'll see what happens
                }
                else
                {
                    enabled = false;
                    if (io_point->is_forced)
                    {
                        local_map_vec.emplace_back(io_point);
                        num_local_points++;
                    }
                }
            }

            if (enabled)
            {
                // this is the normal io_work vec
                io_map_vec.emplace_back(io_point);
                num_remote_points++;

            }
        }
    }

    if(debug)
        std::cout << "Callback for :" <<tObj->name
                    << " running ;  num remote points : " << num_remote_points
                    << " num local points : " << num_local_points
                    << " missed " << mypub->num_missed
                    << std::endl;

    // check_work_items
    check_work_items(io_work_vec, io_map_vec, *compshr->myCfg, "poll", false, false);
    io_map_vec.clear();

    auto work_group_size = io_work_vec.size();
    if(debug)
        std::cout << "Callback for :" <<tObj->name
                    << " running ; work_group_size : " << work_group_size
                    << std::endl;

    if (local_map_vec.size() > 0)
    {
        std::string oper("poll");
        auto io_point = local_map_vec.at(0);
        if(debug)
            std::cout << __func__ << " local vec point id [" << io_point->id << "]" << std::endl;
        auto io_work = make_work(io_point->component, io_point->register_type, io_point->device_id, io_point->offset, io_point->off_by_one, io_point->size, nullptr, nullptr, strToWorkType(oper, false));
        work_group_size++;
        io_work->tNow = tNow;
        io_work->work_group = work_group_size;
        io_work->work_id = work_id++;
        io_work->work_name =tObj->name;
        // io_work->register_groups = register_group;
        io_work->component = compshr;
        io_work->pub_struct = mypub;

        io_work->local = true;
        io_work->off_by_one = io_point->off_by_one;

        io_work->erase_group = false;
        // erase group if we have no remote requests
        // if(io_work_vec.size() == 0)
        // {
        //     io_work->erase_group = true;
        // }

        for (auto io_point : local_map_vec)
        {
            io_work->io_points.emplace_back(io_point);
        }

        pollWork(io_work);
        sent_something = true;
    }

    if (mypub->pub_threads == 0 && num_threads > 0 )
    {
        mypub->pub_threads = num_threads;
        // reset pub_pending on num_thread transition
         mypub->pending = 0;
    }
    // TODO check the pending operation with serial 
    // I dont think we get into a pending state unless we have 3 pubs missed.

    if (mypub->pending > 3 && num_threads > 0)
    {
        //its not a bug but we need to know
        //if(1)std::cout << " too many pending pubs  aborting poll; name: "<<t->name<<"  num_threads :" << num_threads << std::endl;
        // testThread checks to see if the connection is still true it will force a disconnect if the link is dead
        testThread();
        // we need to have a time out here to allow recovery from the stall 
        // if mypub->pend_timeout == 0 then set it to tnow + 5 seconds perhaps
        // set the pending count to 3 to allow another attempt
        if (mypub->pend_timeout == 0.0 ) {
            FPS_INFO_LOG("Server stall  too many pending pubs [%d],  aborting poll name: %s for 5 seconds, num_threads : %d"
                             , mypub->pending,tObj->name.c_str(), num_threads);
            mypub->pend_timeout = tNow + 5.0;

            if (mypub->kill_timeout == 0.0 ) {
                mypub->kill_timeout = tNow + 10.0;
            }
        }
        if ((mypub->pend_timeout> 0) &&  (mypub->pend_timeout< tNow)) {
            mypub->pend_timeout = 0.0;
            // allow another pub attempt
            mypub->pending--;
        }
        if ((mypub->kill_timeout> 0) &&  (mypub->kill_timeout < tNow)) {
            mypub->kill_timeout = 0.0;
            FPS_INFO_LOG("Server stall  poll name  [%s],  killing thread connection "
                             ,tObj->name.c_str());
            // force a reconnection
            killThread();
            mypub->pending = 0;
        }

    }
    else
    {
        mypub->pending++;
        if(0)std::cout << " pending pubs  ok running poll; name: "<<tObj->name << " size :" << work_group_size<< std::endl;
    }

    for (auto io_work : io_work_vec)
    {
        io_work->tNow = tNow;
        io_work->work_group = work_group_size;
        io_work->work_id = work_id++;
        io_work->work_name =tObj->name;  // this sets up the group name
        // io_work->register_groups = register_group;
        io_work->component = compshr;
        io_work->pub_struct = mypub;

        io_work->local = false;
        io_work->erase_group = false;
        // we need to see how many pending pubs we have
        if (mypub->pending > 3)
        {
            // this throws away pending work.
            stashWork(io_work);
        }
        else
        {
            if(num_threads > 0)
            {
                pollWork(io_work);
                sent_something = true;
            }
            else
            {
                // no need to send it
                stashWork(io_work);
            }
        }
        // the response collator used this lot to
        // work_name pub_components_comp_alt_2440 work_id 2 in  2
        if (debug) //|| (register_group->starting_offset == 200 && io_work->work_id == 1))
            std::cout
                << " component " << compshr->id
                // << " registers " << register_group->register_type_str
                // << " start_offset " << register_group->starting_offset
                // << " number_of_registers " << register_group->number_of_registers
                << " work_time " << io_work->tNow
                << " work_name " << io_work->work_name
                << " work_id " << io_work->work_id
                << " work_offset " << io_work->offset
                // << " in  " << compshr->register_groups.size()
                << std::endl;
    }

    if (!sent_something)
    {
        syncTimeObjectByName(tObj->name, mypub->cfg->syncPct);
    }
}

int gcom_setup_pubs(std::map<std::string, std::any> &gcom_map, struct cfg &myCfg, double init_time, bool debug)
{

    // we want to do these things now
    // 1. extract subscriptions -> into myCfg

    FPS_INFO_LOG("Pubs found:");
    // 2. extract poll groups   -> into myCfg
    //  getPolls(myCfg)
    for (auto &mypub : myCfg.pubs)
    {
        auto pubs = mypub.second;
        auto compshr = pubs->component;
        pubs->cfg = &myCfg;

        FPS_INFO_LOG("\t" + mypub.first + "     Frequency: " + std::to_string(compshr->frequency / 1000.0) + " seconds     Offset: " + std::to_string(compshr->offset_time / 1000.0) + " seconds");
        // we need to make these sync objects
        // the only fire once and wait for a sync
        // create a timer object for these pubs
        pubs->pubStats.set_label(mypub.first + " Timer");
        std::shared_ptr<TimeObject> obj1 = createTimeObject(mypub.first,                 // name
                                                            init_time,                   // start time (initial startup time)
                                                            0,                           // stop time - 0 = don't stop
                                                            compshr->frequency / 1000.0, // how often to repeat
                                                            0,                           // count - 0 = don't stop
                                                            pubCallback,                 // callback
                                                            (void *)pubs.get());         // callback params
        // TimeObject obj1(mypub.first, 5.0,   0,    mypub.second->frequency/1000.0,  mypub.second->offset_time/1000.0,   pubCallback, (void *)mypub.second);
        //TODO add sync pubs config item
        if(compshr->use_sync)
        {
            addTimeObject(obj1, compshr->offset_time / 1000.0, true);
        }
        else
        {
            addTimeObject(obj1, compshr->offset_time / 1000.0, false);
        }
    }
    
    return 0;
}