
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

#include "gcom_config.h"
#include "gcom_timer.h"
#include "gcom_iothread.h"
#include "gcom_modbus_pub.h"
#include "logger/logger.h"


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

int GetNumThreads(struct cfg *myCfg);

void pubCallback(std::shared_ptr<TimeObject>t, void *p)
{
    bool debug = false;
    std::lock_guard<std::mutex> lock2(cb_output_mutex);
    std::shared_ptr<cfg::pub_struct> mypub = std::shared_ptr<cfg::pub_struct>(static_cast<cfg::pub_struct *>(p), [](cfg::pub_struct *) {});
    double rtime = get_time_double();
    // TODO we need to lock this
    // maybe add a mutes pointer to the stats start / snap / showNum
    mypub->pubStats.start(mypub->pmtx);
    // std::string pstr = "pub_" + base + "_" + component_name;
    int num_threads = GetNumThreads(mypub->cfg);

    // if (1 || mypub->cfg->pub_debug)
    // {
    //     std::cout << "Callback for :" << t->name
    //                 << " running ;  num_threads: " << num_threads
    //                 << std::endl;
    // }


    // Dont request pubs if there are no threads running.
    if(num_threads == 0)
    {
        if (mypub->cfg->pub_debug)
        {
            std::cout << "Callback for :" << t->name
                      << " Skipped : num_threads: " << num_threads
                      << std::endl;
        }
        mypub->pub_threads = 0;
        //return;
    }

    if (mypub->cfg->pub_debug)
        std::cout << "Callback for :" << t->name
                  << " executed at : " << rtime
                  << " with sync: " << t->sync
                  << " and run: " << t->run
                  << " and psid: " << mypub->id
                  << " num_threads: " << num_threads
                  << std::endl;

    // set up the pup id == rtime
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
    double tNow = get_time_double();
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
            if (io_point->is_disconnected)
            {
                if (io_point->reconnect > 0 && tNow >io_point->reconnect)
                {
                    io_point->is_disconnected = false;
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

    // std::cout << "Callback for :" << t->name
    //                 << " running ;  num remote points : " << num_remote_points
    //                 << " num local points : " << num_local_points
    //                 << std::endl;

    // check_work_items
    check_work_items(io_work_vec, io_map_vec, *compshr->myCfg, "poll", false, false);
    io_map_vec.clear();

    auto work_group_size = io_work_vec.size();
    if (local_map_vec.size() > 0)
    {
        std::string oper("poll");
        auto io_point = local_map_vec.at(0);
        if(debug)
            std::cout << __func__ << " local vec point id [" << io_point->id << "]" << std::endl;
        auto io_work = make_work(io_point->register_type, io_point->device_id, io_point->offset, io_point->off_by_one, io_point->size, nullptr, nullptr, strToWorkType(oper, false));
        work_group_size++;
        io_work->tNow = tNow;
        io_work->work_group = work_group_size;
        io_work->work_id = work_id++;
        io_work->work_name = t->name;
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

    if (mypub->pending > 3 && num_threads > 0)
    {
        //its not a bug but we need to know
        //if(1)std::cout << " too many pending pubs  aborting poll; name: "<<t->name<<"  num_threads :" << num_threads << std::endl;
        FPS_INFO_LOG("Server stall  too many pending pubs,  aborting poll name: %s num_threads : %d", t->name.c_str(), num_threads);

    }
    else
    {
        mypub->pending++;
        if(0)std::cout << " pending pubs  ok running poll; name: "<< t->name << " size :" << work_group_size<< std::endl;
    }

    for (auto io_work : io_work_vec)
    {
        io_work->tNow = tNow;
        io_work->work_group = work_group_size;
        io_work->work_id = work_id++;
        io_work->work_name = t->name;
        // io_work->register_groups = register_group;
        io_work->component = compshr;
        io_work->pub_struct = mypub;

        io_work->local = false;
        io_work->erase_group = false;
        // we need to see how many pending pubs we have
        if (mypub->pending > 3)
        {
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
        syncTimeObjectByName(t->name, mypub->cfg->syncPct);
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