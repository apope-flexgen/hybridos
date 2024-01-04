#define DNP3_CLIENT
#include "gcom_dnp3_client.h"
#include <future>
#include <float.h>
#include <functional>
#include <shared_mutex>
#include "shared_utils.hpp"
#include "gcom_dnp3_system_structs.h"
#include "gcom_dnp3_fims.h"
#include "Jval_buif.hpp"
#include "rigtorp/MPMCQueue.h"
#include "logger/gcom_dnp3_logger.h"
#include "gcom_dnp3_watchdog.h"
#include "gcom_dnp3_heartbeat.h"
#include "gcom_dnp3_flags.h"
#include "gcom_dnp3_io_filter.h"

extern "C"
{
#include "tmwscl/utils/tmwpltmr.h"
#include "tmwscl/dnp/mdnpo042.h"
#include "tmwscl/dnp/mdnpo032.h"
#include "tmwscl/dnp/mdnpsim.h"
#include "tmwscl/dnp/mdnpbrm.h"
}

#include "gcom_dnp3_stats.h"
#include "gcom_dnp3_utils.h"
#include "gcom_dnp3_tmw_utils.h"
#include "version.h"

using namespace std;
GcomSystem clientSys = GcomSystem(Protocol::DNP3);
GcomSystem *clientSysp = &clientSys;

#ifndef DNP3_TEST_MODE
GcomSystem *serverSysp = nullptr;
#endif

int64_t num_configs_client = 0;

#define MAX_CONFIGS 16


void signal_handler(int sig)
{
    clientSys.keep_running = false;
    FPS_ERROR_LOG("signal of type %s caught.", strsignal(sig));
    signal(sig, SIG_DFL);
};

/**
 * @brief Add a dbPoint to a queue for publishing.
 * 
 * "Freeze" the value, flags, and timestamp of a DNP3 point into a PubPoint structure. Add
 * the PubPoint structure to a queue to be published. This function is called immediately
 * for points that are configured as direct pubs and immediately for integrity/class scans.
 * For batch and interval pubs, this function is called at the expiration of the callback
 * timer.
 * 
 * @param pDbPoint void * pointer to the TMWSIM_POINT * to be added to the publish work queue.
 * 
 * @pre pDbPoint is a valid pointer to a TMWSIM_POINT struct
*/
void addPointToPubWork(void *pDbPoint)
{
    TMWSIM_POINT *dbPoint = (TMWSIM_POINT *)pDbPoint;
    GcomSystem *sys = ((FlexPoint *)(dbPoint->flexPointHandle))->sys;
    ((FlexPoint *)(dbPoint->flexPointHandle))->last_pub = std::chrono::system_clock::now();
    if (sys->protocol_dependencies->dnp3.pub_all && ((FlexPoint *)(dbPoint->flexPointHandle))->batch_pubs && tmwtimer_isActive(&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer))
    {
        tmwtimer_cancel(&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer);
    }
    bool needs_lock = !sys->protocol_dependencies->dnp3.pub_all && !isDirectPub(dbPoint);
    while(needs_lock &&!sys->db_mutex.try_lock_shared()){};
    PubWork *pub_work;
    PubPoint *point = new PubPoint();
    point->dbPoint = dbPoint;
    if (dbPoint->type == TMWSIM_TYPE_ANALOG)
    {
        point->value = dbPoint->data.analog.value;
    }
    else
    {
        point->value = static_cast<double>(dbPoint->data.binary.value);
    }
    point->flags = dbPoint->flags;
    point->changeTime = dbPoint->timeStamp;
    if(needs_lock){
        sys->db_mutex.unlock_shared();
    }
    // the section below 1) checks if a uri currently exists with pub work for that point
    // 2) if not, it creates a vector to store this pub work and any future pub work for the uri. It also adds new pub work to the pub work queue.
    // 3) if the uri already exists with pub work, check to see if that point is contained within that uri's set of pub work
    // 4) if it is, keep looking until we find pub work for that uri that doesn't contain that point, then add the point
    // 5) if it isn't, add that point to the pub_work
    sys->fims_dependencies->uris_with_data_mutex.lock();
    auto pub_work_it = sys->fims_dependencies->uris_with_data.find(((FlexPoint *)(dbPoint->flexPointHandle))->uri);
    if (pub_work_it == sys->fims_dependencies->uris_with_data.end())
    {
        pub_work = new PubWork();
        std::vector<PubWork *> pub_work_uri_vector;
        pub_work->pub_uri = ((FlexPoint *)(dbPoint->flexPointHandle))->uri;
        sys->fims_dependencies->pub_q.try_push(pub_work);
        pub_work_uri_vector.push_back(pub_work);
        sys->fims_dependencies->uris_with_data[((FlexPoint *)(dbPoint->flexPointHandle))->uri] = pub_work_uri_vector;
        pub_work_it = sys->fims_dependencies->uris_with_data.find(((FlexPoint *)(dbPoint->flexPointHandle))->uri);
        if (pub_work_it == sys->fims_dependencies->uris_with_data.end())
        {
            if(!spam_limit(sys, sys->point_errors))
            {
                FPS_ERROR_LOG("Someting went wrong when attempting to add point [%s] to pub work.", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str());
            }
            if(point){
                delete point;
            }
            return;
        }
    }

    bool inserted_into_pub_work = false;
    std::vector<PubWork *> &pub_work_uri_vector = pub_work_it->second;
    // check all sets of pub work to see where we need to insert the value
    for (size_t i = 0; i < pub_work_uri_vector.size(); i++)
    {
        if (pub_work_uri_vector.at(i)->pub_vals.find(((FlexPoint *)(dbPoint->flexPointHandle))->name) != pub_work_uri_vector[i]->pub_vals.end())
        {
            continue; // the point is already in this set of pub vals
        }
        else
        { // the point is not already in this set of pub vals so add it

            pub_work_uri_vector.at(i)->pub_vals[((FlexPoint *)(dbPoint->flexPointHandle))->name] = point;
            inserted_into_pub_work = true;
            break;
        }
    }
    if (!inserted_into_pub_work && (pub_work_uri_vector.size() == 0 || ((FlexPoint *)(dbPoint->flexPointHandle))->event_pub)) // if we get through all of them and haven't found a pub_work with space for this point
    {
        pub_work = new PubWork();
        pub_work->pub_uri = ((FlexPoint *)(dbPoint->flexPointHandle))->uri;
        sys->fims_dependencies->pub_q.try_push(pub_work);
        pub_work->pub_vals[((FlexPoint *)(dbPoint->flexPointHandle))->name] = point;
        sys->fims_dependencies->uris_with_data[((FlexPoint *)(dbPoint->flexPointHandle))->uri].push_back(pub_work);
    }
    sys->fims_dependencies->uris_with_data_mutex.unlock();
    // if it's not a direct pub, then we've moved on from the original message's call to queuePubs
    // so we need to call it again to make sure the point is pubbed
    if (needs_lock)
    {
        queuePubs(sys);
    }
}

/**
 * @brief Call addPointToPubWork and then restart the interval timer for the point to call
 * this function again after the interval period.
 * 
 * @param pDbPoint void * pointer to the TMWSIM_POINT * to be added to the publish work queue.
 * 
 * @pre pDbPoint is a valid pointer to a TMWSIM_POINT struct
*/
void addPointToIntervalPubWork(void *pDbPoint)
{
    TMWSIM_POINT *dbPoint = (TMWSIM_POINT *)pDbPoint;
    addPointToPubWork(pDbPoint);
    ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.unlock_shared();
    tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer),
                   ((FlexPoint *)(dbPoint->flexPointHandle))->interval_pub_rate,
                   (((FlexPoint *)(dbPoint->flexPointHandle))->sys)->protocol_dependencies->dnp3.pChannel,
                   addPointToIntervalPubWork,
                   pDbPoint);
}

/**
 * @brief In response to an incoming DNP3 message, prepare to send a 'pub' over fims based
 * on whether the point is a batch pub, interval pub, or direct pub.
 * 
 * In response to an incoming DNP3 message, update the value that will be sent out
 * as a pub over fims. If batched or interval pubs are configured, start the pub timer with
 * the appropriate callback (addPointToPubWork or addPointToIntervalPubWork). If the
 * point is configured for direct pub, call addPointToPubWork immediately. Also update the
 * point status (COMM_LOST or ONLINE).
 * 
 * This function should be configured at startup as the mdnpsim callback function with the
 * mdnpsim database pointer as the callback parameter.
 * 
 * @param pDbHandle void * pointer to the database handle
 * @param type TMWSIM_EVENT_TYPE, which will likely only ever be TMWSIM_POINT_UPDATE (though
 * it could potentially also be TMWSIM_POINT_ADD, TMWSIM_POINT_DELETE, or TMWSIM_CLEAR_DATABASE.)
 * @param objectGroup DNPDEFS_OBJ_GROUP_ID corresponds to any valid DNP3 group number. This
 * function currently only handles Group 30/32, Group 1/2, Group 40 (if watchdog point), and
 * Group 10 (if watchdog point).
 * @param pointNumber TMWTYPES_UINT the point number for the current point being handled
*/
void updatePointCallback(void *pDbHandle, TMWSIM_EVENT_TYPE type, DNPDEFS_OBJ_GROUP_ID objectGroup, TMWTYPES_USHORT pointNumber)
{
    if (type == TMWSIM_POINT_UPDATE)
    {
        if (objectGroup == DNPDEFS_OBJ_30_ANA_INPUTS || objectGroup == DNPDEFS_OBJ_32_ANA_CHNG_EVENTS)
        {
            TMWSIM_POINT *dbPoint = (TMWSIM_POINT *)mdnpsim_analogInputLookupPoint(pDbHandle, pointNumber);
            if (dbPoint != nullptr && dbPoint->flexPointHandle != nullptr)
            {
                GcomSystem *sys = ((FlexPoint *)(dbPoint->flexPointHandle))->sys;
                checkPointCommLost(dbPoint);
                if (sys->protocol_dependencies->dnp3.pub_all || isDirectPub(dbPoint))
                {
                    addPointToPubWork(dbPoint);
                }
                else if (((FlexPoint *)(dbPoint->flexPointHandle))->batch_pubs && ((FlexPoint *)(dbPoint->flexPointHandle))->event_pub)
                {
                    if (!tmwtimer_isActive(&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer))
                    {
                        tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer),
                                       ((FlexPoint *)(dbPoint->flexPointHandle))->batch_pub_rate,
                                       sys->protocol_dependencies->dnp3.pChannel,
                                       addPointToPubWork,
                                       dbPoint);
                    }
                }
                else if (((FlexPoint *)(dbPoint->flexPointHandle))->interval_pubs && ((FlexPoint *)(dbPoint->flexPointHandle))->event_pub)
                {
                    if (!tmwtimer_isActive(&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer))
                    {
                        tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer),
                                       ((FlexPoint *)(dbPoint->flexPointHandle))->interval_pub_rate,
                                       sys->protocol_dependencies->dnp3.pChannel,
                                       addPointToIntervalPubWork,
                                       dbPoint);
                    }
                }
            }
            else
            {
                if(!spam_limit(&clientSys, clientSys.point_errors))
                {
                    FPS_ERROR_LOG("unable to find analog input point number %d ", pointNumber);
                    FPS_LOG_IT("could_not_find_point");
                }
            }
        }
        else if (objectGroup == DNPDEFS_OBJ_1_BIN_INPUTS || objectGroup == DNPDEFS_OBJ_2_BIN_CHNG_EVENTS)
        {
            TMWSIM_POINT *dbPoint = (TMWSIM_POINT *)mdnpsim_binaryInputLookupPoint(pDbHandle, pointNumber);
            if (dbPoint != nullptr && dbPoint->flexPointHandle != nullptr)
            {
                GcomSystem *sys = ((FlexPoint *)(dbPoint->flexPointHandle))->sys;
                checkPointCommLost(dbPoint);
                if (sys->protocol_dependencies->dnp3.pub_all || isDirectPub(dbPoint))
                {
                    addPointToPubWork(dbPoint);
                }
                else if (((FlexPoint *)(dbPoint->flexPointHandle))->batch_pubs && ((FlexPoint *)(dbPoint->flexPointHandle))->event_pub)
                {
                    if (!tmwtimer_isActive(&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer))
                    {
                        tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer),
                                       ((FlexPoint *)(dbPoint->flexPointHandle))->batch_pub_rate,
                                       sys->protocol_dependencies->dnp3.pChannel,
                                       addPointToPubWork,
                                       dbPoint);
                    }
                }
                else if (((FlexPoint *)(dbPoint->flexPointHandle))->interval_pubs && ((FlexPoint *)(dbPoint->flexPointHandle))->event_pub)
                {
                    if (!tmwtimer_isActive(&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer))
                    {
                        tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer),
                                       ((FlexPoint *)(dbPoint->flexPointHandle))->interval_pub_rate,
                                       sys->protocol_dependencies->dnp3.pChannel,
                                       addPointToIntervalPubWork,
                                       dbPoint);
                    }
                }
            }
            else
            {
                if(!spam_limit(&clientSys, clientSys.point_errors))  
                {
                    FPS_ERROR_LOG("unable to find binary input point number %d ", pointNumber);
                    FPS_LOG_IT("could_not_find_point");
                }
            }
        }
        else if (objectGroup == DNPDEFS_OBJ_40_ANA_OUT_STATUSES)
        {
            // TMWSIM_POINT *dbPoint = (TMWSIM_POINT *)mdnpsim_analogOutputLookupPoint(pDbHandle, pointNumber);
            // if (dbPoint != nullptr && dbPoint->flexPointHandle != nullptr)
            // {
            //     GcomSystem *sys = ((FlexPoint *)(dbPoint->flexPointHandle))->sys;
            // }
        }
        else if (objectGroup == DNPDEFS_OBJ_10_BIN_OUT_STATUSES)
        {
            // TMWSIM_POINT *dbPoint = (TMWSIM_POINT *)mdnpsim_binaryOutputLookupPoint(pDbHandle, pointNumber);
            // if (dbPoint != nullptr && dbPoint->flexPointHandle != nullptr)
            // {
            //     GcomSystem *sys = ((FlexPoint *)(dbPoint->flexPointHandle))->sys;
            // }
        }
    }
}

/**
 * @brief Call tmwdb_storeEntry for all entries on the tmwdb queue.
 * 
 * Typically called directly from DNP3 response callbacks when a message is received.
 * 
 * @param sys pointer to fully initialized GcomSystem for client
*/
void storeData(GcomSystem *sys)
{
    // store data into db and get timing info
    const auto start_store_data_in_db = std::chrono::system_clock::now();
    sys->db_mutex.lock();
    while (tmwdb_storeEntry(TMWDEFS_TRUE))
        ; // this stores all of the values into the database, which calls the updatePointCallback above
    sys->db_mutex.unlock();
    uint64_t time_to_store_data_in_db = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_store_data_in_db).count();
    dnp3stats_updateTiming(sys->protocol_dependencies->dnp3.timings->store_data, time_to_store_data_in_db);
}

/**
 * @brief Format fims messages depending on point format and type. Publish all data
 * that has been queued for publishing via fims.
 * 
 * @param sys pointer to fully initialized GcomSystem for client
*/
void queuePubs(GcomSystem *sys)
{
    // process fims_pubs
    fmt::memory_buffer &send_buf = sys->fims_dependencies->send_buf;
    PubWork *pub_work;
    sys->fims_dependencies->uris_with_data_mutex.lock();
    bool has_values = sys->fims_dependencies->pub_q.try_pop(pub_work);
    sys->fims_dependencies->uris_with_data_mutex.unlock();
    bool has_one_point = false;
    while (has_values)
    {
        sys->fims_dependencies->uris_with_data_mutex.lock();
        for (auto pub_work_it = sys->fims_dependencies->uris_with_data[pub_work->pub_uri].begin(); pub_work_it != sys->fims_dependencies->uris_with_data[pub_work->pub_uri].end(); pub_work_it++)
        {
            if ((*pub_work_it) == pub_work)
            {
                sys->fims_dependencies->uris_with_data[pub_work->pub_uri].erase(pub_work_it);
                break;
            }
        }
        sys->fims_dependencies->uris_with_data_mutex.unlock();
        has_one_point = false;
        const auto start_fims_message = std::chrono::system_clock::now();
        send_buf.clear();
        send_buf.push_back('{'); // begin object
        for (auto pair : pub_work->pub_vals)
        {
            PubPoint *point = pair.second;
            auto &dbPoint = point->dbPoint;
            has_one_point = true;
            if (((FlexPoint *)(dbPoint->flexPointHandle))->format == FimsFormat::Naked)
            {
                if (dbPoint->type == TMWSIM_TYPE_ANALOG)
                {
                    if ((((FlexPoint *)(dbPoint->flexPointHandle))->scale) == 0.0)
                    {
                        if (dbPoint->defaultStaticVariation == Group30Var1 || dbPoint->defaultStaticVariation == Group30Var3)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{},)", pair.first, static_cast<int32_t>(point->value));
                        }
                        else if (dbPoint->defaultStaticVariation == Group30Var2 || dbPoint->defaultStaticVariation == Group30Var4)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{},)", pair.first, static_cast<int16_t>(point->value));
                        }
                        else if (dbPoint->defaultStaticVariation == Group30Var5)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{:.{}g},)", pair.first, static_cast<float>(point->value), std::numeric_limits<double>::max_digits10 - 1);
                        }
                        else
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{:.{}g},)", pair.first, point->value, std::numeric_limits<double>::max_digits10 - 1);
                        }
                    }
                    else
                    {
                        if (dbPoint->defaultStaticVariation == Group30Var1 || dbPoint->defaultStaticVariation == Group30Var3)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{:.{}g},)", pair.first, static_cast<int32_t>(point->value) / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1);
                        }
                        else if (dbPoint->defaultStaticVariation == Group30Var2 || dbPoint->defaultStaticVariation == Group30Var4)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{:.{}g},)", pair.first, static_cast<int16_t>(point->value) / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1);
                        }
                        else if (dbPoint->defaultStaticVariation == Group30Var5)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{:.{}g},)", pair.first, static_cast<float>(point->value) / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1);
                        }
                        else
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{:.{}g},)", pair.first, point->value / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1);
                        }
                    }
                }
                else
                {
                    if (((FlexPoint *)(dbPoint->flexPointHandle))->scale < 0)
                    {
                        if (((FlexPoint *)(dbPoint->flexPointHandle))->crob_string)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":"{}",)", pair.first, static_cast<bool>(point->value) ? (((FlexPoint *)(dbPoint->flexPointHandle))->crob_false) : (((FlexPoint *)(dbPoint->flexPointHandle))->crob_true));
                        }
                        else if (((FlexPoint *)(dbPoint->flexPointHandle))->crob_int)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{},)", pair.first, static_cast<bool>(point->value) ? 0 : 1);
                        }
                        else
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{},)", pair.first, static_cast<bool>(point->value) ? "false" : "true");
                        }
                    }
                    else
                    {
                        if (((FlexPoint *)(dbPoint->flexPointHandle))->crob_string)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":"{}",)", pair.first, static_cast<bool>(point->value) ? (((FlexPoint *)(dbPoint->flexPointHandle))->crob_true) : (((FlexPoint *)(dbPoint->flexPointHandle))->crob_false));
                        }
                        else if (((FlexPoint *)(dbPoint->flexPointHandle))->crob_int)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{},)", pair.first, static_cast<bool>(point->value) ? 1 : 0);
                        }
                        else
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{},)", pair.first, static_cast<bool>(point->value) ? "true" : "false");
                        }
                    }
                }
            }
            else if (((FlexPoint *)(dbPoint->flexPointHandle))->format == FimsFormat::Clothed)
            {
                if (dbPoint->type == TMWSIM_TYPE_ANALOG)
                {
                    if ((((FlexPoint *)(dbPoint->flexPointHandle))->scale) == 0.0)
                    {
                        if (dbPoint->defaultStaticVariation == Group30Var1 || dbPoint->defaultStaticVariation == Group30Var3)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{}}},)", pair.first, static_cast<int32_t>(point->value));
                        }
                        else if (dbPoint->defaultStaticVariation == Group30Var2 || dbPoint->defaultStaticVariation == Group30Var4)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{}}},)", pair.first, static_cast<int16_t>(point->value));
                        }
                        else if (dbPoint->defaultStaticVariation == Group30Var5)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{:.{}g}}},)", pair.first, static_cast<float>(point->value), std::numeric_limits<double>::max_digits10 - 1);
                        }
                        else
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{:.{}g}}},)", pair.first, point->value, std::numeric_limits<double>::max_digits10 - 1);
                        }
                    }
                    else
                    {
                        if (dbPoint->defaultStaticVariation == Group30Var1 || dbPoint->defaultStaticVariation == Group30Var3)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{:.{}g}}},)", pair.first, static_cast<int32_t>(point->value) / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1);
                        }
                        else if (dbPoint->defaultStaticVariation == Group30Var2 || dbPoint->defaultStaticVariation == Group30Var4)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{:.{}g}}},)", pair.first, static_cast<int16_t>(point->value) / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1);
                        }
                        else if (dbPoint->defaultStaticVariation == Group30Var5)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{:.{}g}}},)", pair.first, static_cast<float>(point->value) / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1);
                        }
                        else
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{:.{}g}}},)", pair.first, point->value / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1);
                        }
                    }
                }
                else
                {
                    if (((FlexPoint *)(dbPoint->flexPointHandle))->scale < 0)
                    {
                        if (((FlexPoint *)(dbPoint->flexPointHandle))->crob_string)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":"{}"}},)", pair.first, static_cast<bool>(point->value) ? (((FlexPoint *)(dbPoint->flexPointHandle))->crob_false) : (((FlexPoint *)(dbPoint->flexPointHandle))->crob_true));
                        }
                        else if (((FlexPoint *)(dbPoint->flexPointHandle))->crob_int)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{}}},)", pair.first, static_cast<bool>(point->value) ? 0 : 1);
                        }
                        else
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{}}},)", pair.first, static_cast<bool>(point->value) ? "false" : "true");
                        }
                    }
                    else
                    {
                        if (((FlexPoint *)(dbPoint->flexPointHandle))->crob_string)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":"{}"}},)", pair.first, static_cast<bool>(point->value) ? (((FlexPoint *)(dbPoint->flexPointHandle))->crob_true) : (((FlexPoint *)(dbPoint->flexPointHandle))->crob_false));
                        }
                        else if (((FlexPoint *)(dbPoint->flexPointHandle))->crob_int)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{}}},)", pair.first, static_cast<bool>(point->value) ? 1 : 0);
                        }
                        else
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{}}},)", pair.first, static_cast<bool>(point->value) ? "true" : "false");
                        }
                    }
                }
            }
            else // Full
            {
                TMWDTIME *tmpPtr = &(point->changeTime);
                if (dbPoint->type == TMWSIM_TYPE_ANALOG)
                {
                    if ((((FlexPoint *)(dbPoint->flexPointHandle))->scale) == 0.0)
                    {
                        if (dbPoint->defaultStaticVariation == Group30Var1 || dbPoint->defaultStaticVariation == Group30Var3)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{}, "flags":{}, "timestamp":"{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}"}},)", pair.first, static_cast<int32_t>(point->value), DNP_FLAGS{point->flags}, tmpPtr->year, tmpPtr->month, tmpPtr->dayOfMonth, tmpPtr->hour, tmpPtr->minutes, tmpPtr->mSecsAndSecs / 1000, tmpPtr->mSecsAndSecs % 1000);
                        }
                        else if (dbPoint->defaultStaticVariation == Group30Var2 || dbPoint->defaultStaticVariation == Group30Var4)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{}, "flags":{}, "timestamp":"{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}"}},)", pair.first, static_cast<int16_t>(point->value), DNP_FLAGS{point->flags}, tmpPtr->year, tmpPtr->month, tmpPtr->dayOfMonth, tmpPtr->hour, tmpPtr->minutes, tmpPtr->mSecsAndSecs / 1000, tmpPtr->mSecsAndSecs % 1000);
                        }
                        else if (dbPoint->defaultStaticVariation == Group30Var5)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{:.{}g}, "flags":{}, "timestamp":"{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}"}},)", pair.first, static_cast<float>(point->value), std::numeric_limits<double>::max_digits10 - 1, DNP_FLAGS{point->flags}, tmpPtr->year, tmpPtr->month, tmpPtr->dayOfMonth, tmpPtr->hour, tmpPtr->minutes, tmpPtr->mSecsAndSecs / 1000, tmpPtr->mSecsAndSecs % 1000);
                        }
                        else
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{:.{}g}, "flags":{}, "timestamp":"{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}"}},)", pair.first, point->value, DNP_FLAGS{point->flags}, std::numeric_limits<double>::max_digits10 - 1, tmpPtr->year, tmpPtr->month, tmpPtr->dayOfMonth, tmpPtr->hour, tmpPtr->minutes, tmpPtr->mSecsAndSecs / 1000, tmpPtr->mSecsAndSecs % 1000);
                        }
                    }
                    else
                    {
                        if (dbPoint->defaultStaticVariation == Group30Var1 || dbPoint->defaultStaticVariation == Group30Var3)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{:.{}g}, "flags":{}, "timestamp":"{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}"}},)", pair.first, static_cast<int32_t>(point->value) / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1, DNP_FLAGS{point->flags}, tmpPtr->year, tmpPtr->month, tmpPtr->dayOfMonth, tmpPtr->hour, tmpPtr->minutes, tmpPtr->mSecsAndSecs / 1000, tmpPtr->mSecsAndSecs % 1000);
                        }
                        else if (dbPoint->defaultStaticVariation == Group30Var2 || dbPoint->defaultStaticVariation == Group30Var4)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{:.{}g}, "flags":{}, "timestamp":"{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}"}},)", pair.first, static_cast<int16_t>(point->value) / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1, DNP_FLAGS{point->flags}, tmpPtr->year, tmpPtr->month, tmpPtr->dayOfMonth, tmpPtr->hour, tmpPtr->minutes, tmpPtr->mSecsAndSecs / 1000, tmpPtr->mSecsAndSecs % 1000);
                        }
                        else if (dbPoint->defaultStaticVariation == Group30Var5)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{:.{}g}, "flags":{}, "timestamp":"{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}"}},)", pair.first, static_cast<float>(point->value) / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1, DNP_FLAGS{point->flags}, tmpPtr->year, tmpPtr->month, tmpPtr->dayOfMonth, tmpPtr->hour, tmpPtr->minutes, tmpPtr->mSecsAndSecs / 1000, tmpPtr->mSecsAndSecs % 1000);
                        }
                        else
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{:.{}g}, "flags":{}, "timestamp":"{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}"}},)", pair.first, point->value / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1, DNP_FLAGS{point->flags}, tmpPtr->year, tmpPtr->month, tmpPtr->dayOfMonth, tmpPtr->hour, tmpPtr->minutes, tmpPtr->mSecsAndSecs / 1000, tmpPtr->mSecsAndSecs % 1000);
                        }
                    }
                }
                else
                {
                    if (((FlexPoint *)(dbPoint->flexPointHandle))->scale < 0)
                    {
                        if (((FlexPoint *)(dbPoint->flexPointHandle))->crob_string)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{}, "flags":{}, "timestamp":"{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}"}},)", pair.first, (static_cast<bool>(point->value) ? (((FlexPoint *)(dbPoint->flexPointHandle))->crob_false) : (((FlexPoint *)(dbPoint->flexPointHandle))->crob_true)), DNP_FLAGS{point->flags}, tmpPtr->year, tmpPtr->month, tmpPtr->dayOfMonth, tmpPtr->hour, tmpPtr->minutes, tmpPtr->mSecsAndSecs / 1000, tmpPtr->mSecsAndSecs % 1000);
                        }
                        else if (((FlexPoint *)(dbPoint->flexPointHandle))->crob_int)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{}, "flags":{}, "timestamp":"{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}"}},)", pair.first, static_cast<bool>(point->value) ? 0 : 1, DNP_FLAGS{point->flags}, tmpPtr->year, tmpPtr->month, tmpPtr->dayOfMonth, tmpPtr->hour, tmpPtr->minutes, tmpPtr->mSecsAndSecs / 1000, tmpPtr->mSecsAndSecs % 1000);
                        }
                        else
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{}, "flags":{}, "timestamp":"{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}"}},)", pair.first, static_cast<bool>(point->value) ? "false" : "true", DNP_FLAGS{point->flags}, tmpPtr->year, tmpPtr->month, tmpPtr->dayOfMonth, tmpPtr->hour, tmpPtr->minutes, tmpPtr->mSecsAndSecs / 1000, tmpPtr->mSecsAndSecs % 1000);
                        }
                    }
                    else
                    {
                        if (((FlexPoint *)(dbPoint->flexPointHandle))->crob_string)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{}, "flags":{}, "timestamp":"{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}"}},)", pair.first, static_cast<bool>(point->value) ? (((FlexPoint *)(dbPoint->flexPointHandle))->crob_true) : (((FlexPoint *)(dbPoint->flexPointHandle))->crob_false), DNP_FLAGS{point->flags}, tmpPtr->year, tmpPtr->month, tmpPtr->dayOfMonth, tmpPtr->hour, tmpPtr->minutes, tmpPtr->mSecsAndSecs / 1000, tmpPtr->mSecsAndSecs % 1000);
                        }
                        else if (((FlexPoint *)(dbPoint->flexPointHandle))->crob_int)
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{}, "flags":{}, "timestamp":"{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}"}},)", pair.first, static_cast<bool>(point->value) ? 1 : 0, DNP_FLAGS{point->flags}, tmpPtr->year, tmpPtr->month, tmpPtr->dayOfMonth, tmpPtr->hour, tmpPtr->minutes, tmpPtr->mSecsAndSecs / 1000, tmpPtr->mSecsAndSecs % 1000);
                        }
                        else
                        {
                            FORMAT_TO_BUF(send_buf, R"("{}":{{"value":{}, "flags":{}, "timestamp":"{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}"}},)", pair.first, static_cast<bool>(point->value) ? "true" : "false", DNP_FLAGS{point->flags}, tmpPtr->year, tmpPtr->month, tmpPtr->dayOfMonth, tmpPtr->hour, tmpPtr->minutes, tmpPtr->mSecsAndSecs / 1000, tmpPtr->mSecsAndSecs % 1000);
                        }
                    }
                }
            }
            if (pair.second)
            {
                delete pair.second;
            }
        }

        if(sys->heartbeat){
            sys->heartbeat->mtx.lock();
            if(sys->heartbeat->enabled){
                FORMAT_TO_BUF(send_buf, R"("heartbeat_state":"{}",)", sys->heartbeat->state_str);
                FORMAT_TO_BUF(send_buf, R"("heartbeat_value":{},)", sys->heartbeat->last_val);
                FORMAT_TO_BUF(send_buf, R"("component_connected":{},)", (sys->heartbeat->state_str=="NORMAL")? "true":"false");
                has_one_point = true;
            }
            sys->heartbeat->mtx.unlock();
        }
        // "Timestamp" stuff:
        if (has_one_point)
        {
            const auto timestamp = std::chrono::system_clock::now();
            const auto timestamp_micro = time_fraction<std::chrono::microseconds>(timestamp);
            FORMAT_TO_BUF_NO_COMPILE(send_buf, R"("Timestamp":"{:%Y-%m-%d %T}.{:06d}"}})", timestamp, timestamp_micro.count());
            if (!send_pub(sys->fims_dependencies->fims_gateway, pub_work->pub_uri, std::string_view{send_buf.data(), send_buf.size()}))
            {
                if(!spam_limit(sys, sys->fims_errors))
                {
                    FPS_ERROR_LOG("%s cannot publish onto fims, exiting", pub_work->pub_uri);
                    FPS_LOG_IT("fims_send_error");
                }
                if (pub_work)
                {
                    delete pub_work;
                }
                return;
            }
        }
        uint64_t time_to_prep_fims_message = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_fims_message).count();
        dnp3stats_updateTiming(sys->protocol_dependencies->dnp3.timings->fims_pub, time_to_prep_fims_message);
        if (pub_work)
        {
            delete pub_work;
        }
        has_values = sys->fims_dependencies->pub_q.try_pop(pub_work);
    }
}

/**
 * @brief Update timings for direct operate responses.
 * 
 * Callback function for direct operate responses. Initialized alongside
 * direct operate request header.
 * 
 * @param sys void* pointer to fully initialized GcomSystem for client
 * @param response DNPCHNL_RESPONSE_INFO pointer (filled out by TMW)
*/
void process_analog_direct_operate_response(void *sys, DNPCHNL_RESPONSE_INFO *response)
{
    if (response->last && response->status != DNPCHNL_RESP_STATUS_CANCELED)
    {
        dnp3stats_updateTiming(((GcomSystem *)sys)->protocol_dependencies->dnp3.timings->direct_operates, response->responseTime);
    }
}

/**
 * @brief Update timings for direct operate responses.
 * 
 * Callback function for direct operate responses. Initialized alongside
 * direct operate request header.
 * 
 * @param sys void* pointer to fully initialized GcomSystem for client
 * @param response DNPCHNL_RESPONSE_INFO pointer (filled out by TMW)
*/
void process_binary_direct_operate_response(void *sys, DNPCHNL_RESPONSE_INFO *response)
{
    if (response->last && response->status != DNPCHNL_RESP_STATUS_CANCELED)
    {
        dnp3stats_updateTiming(((GcomSystem *)sys)->protocol_dependencies->dnp3.timings->direct_operates, response->responseTime);
    }
}

/**
 * @brief Update timings for integrity poll. Store incoming data in MDNPSIM_DATABASE
 * and queue fims pubs for all input points.
 * 
 * Callback function for integrity poll responses. Initialized alongside integrity
 * poll request header.
 * 
 * @param sys void* pointer to fully initialized GcomSystem for client
 * @param response DNPCHNL_RESPONSE_INFO pointer (filled out by TMW)
*/
void process_integrity_response(void *sys, DNPCHNL_RESPONSE_INFO *response)
{
    if (response->last && response->status != DNPCHNL_RESP_STATUS_CANCELED)
    {
        dnp3stats_updateTiming(((GcomSystem *)sys)->protocol_dependencies->dnp3.timings->integrity_responses, response->responseTime);
    }
    ((GcomSystem *)sys)->protocol_dependencies->dnp3.pub_all = true;
    storeData((GcomSystem *)sys);
    if (response->last)
    {
        queuePubs((GcomSystem *)sys);
        ((GcomSystem *)sys)->protocol_dependencies->dnp3.pub_all = false;
    }
}

/**
 * @brief Update timings for class 1 scan. Store incoming data in MDNPSIM_DATABASE
 * and queue fims pubs for all input points.
 * 
 * Callback function for Class 1 scan responses. Initialized alongside class 1
 * scan request header.
 * 
 * @param sys void* pointer to fully initialized GcomSystem for client
 * @param response DNPCHNL_RESPONSE_INFO pointer (filled out by TMW)
*/
void process_class1_response(void *sys, DNPCHNL_RESPONSE_INFO *response)
{
    if (response->last && response->status != DNPCHNL_RESP_STATUS_CANCELED)
    {
        dnp3stats_updateTiming(((GcomSystem *)sys)->protocol_dependencies->dnp3.timings->class1_responses, response->responseTime);
    }
    ((GcomSystem *)sys)->protocol_dependencies->dnp3.pub_all = true;
    storeData((GcomSystem *)sys);
    if (response->last)
    {
        queuePubs((GcomSystem *)sys);
        ((GcomSystem *)sys)->protocol_dependencies->dnp3.pub_all = false;
    }
}

/**
 * @brief Update timings for class 2 scan. Store incoming data in MDNPSIM_DATABASE
 * and queue fims pubs for all input points.
 * 
 * Callback function for Class 2 scan responses. Initialized alongside class 2
 * scan request header.
 * 
 * @param sys void* pointer to fully initialized GcomSystem for client
 * @param response DNPCHNL_RESPONSE_INFO pointer (filled out by TMW)
*/
void process_class2_response(void *sys, DNPCHNL_RESPONSE_INFO *response)
{
    if (response->last && response->status != DNPCHNL_RESP_STATUS_CANCELED)
    {
        dnp3stats_updateTiming(((GcomSystem *)sys)->protocol_dependencies->dnp3.timings->class2_responses, response->responseTime);
    }
    ((GcomSystem *)sys)->protocol_dependencies->dnp3.pub_all = true;
    storeData((GcomSystem *)sys);
    if (response->last)
    {
        queuePubs((GcomSystem *)sys);
        ((GcomSystem *)sys)->protocol_dependencies->dnp3.pub_all = false;
    }
}

/**
 * @brief Update timings for class 3 scan. Store incoming data in MDNPSIM_DATABASE
 * and queue fims pubs for all input points.
 * 
 * Callback function for Class 3 scan responses. Initialized alongside class 3
 * scan request header.
 * 
 * @param sys void* pointer to fully initialized GcomSystem for client
 * @param response DNPCHNL_RESPONSE_INFO pointer (filled out by TMW)
*/ 
void process_class3_response(void *sys, DNPCHNL_RESPONSE_INFO *response)
{
    if (response->last && response->status != DNPCHNL_RESP_STATUS_CANCELED)
    {
        dnp3stats_updateTiming(((GcomSystem *)sys)->protocol_dependencies->dnp3.timings->class3_responses, response->responseTime);
    }
    ((GcomSystem *)sys)->protocol_dependencies->dnp3.pub_all = true;
    storeData((GcomSystem *)sys);
    if (response->last)
    {
        queuePubs((GcomSystem *)sys);
        ((GcomSystem *)sys)->protocol_dependencies->dnp3.pub_all = false;
    }
}

/**
 * @brief Store incoming data in MDNPSIM_DATABASE and queue fims pubs for all input points.
 * 
 * Callback function for unsolicited responses. Initialized at the start of a 
 * TMW session using mdnpsesn_setUnsolUserCallback.
 * 
 * @param sys void* pointer to fully initialized GcomSystem for client
 * @param response MDNPSESN_UNSOL_RESP_INFO pointer (filled out by TMW)
*/
void process_unsolicited_response(void *sys, MDNPSESN_UNSOL_RESP_INFO *response)
{
    storeData((GcomSystem *)sys);
    queuePubs((GcomSystem *)sys);
}

/**
 * @brief Use the settings in dnp3_sys->channelConfig to open a TMW channel.
 * 
 * @param sys a reference to the pre-populated GcomSystem for the client.
*/
bool openTMWClientChannel(GcomSystem &sys)
{
    DNP3Dependencies *dnp3_sys = &(sys.protocol_dependencies->dnp3);
    if (sys.debug)
    {
        dnp3_sys->channelConfig.chnlDiagMask = (TMWDIAG_ID_PHYS | TMWDIAG_ID_LINK | TMWDIAG_ID_TPRT | TMWDIAG_ID_APPL |
                                                TMWDIAG_ID_USER | TMWDIAG_ID_MMI | TMWDIAG_ID_STATIC_DATA |
                                                TMWDIAG_ID_STATIC_HDRS | TMWDIAG_ID_EVENT_DATA | TMWDIAG_ID_EVENT_HDRS |
                                                TMWDIAG_ID_CYCLIC_DATA | TMWDIAG_ID_CYCLIC_HDRS | TMWDIAG_ID_SECURITY_DATA |
                                                TMWDIAG_ID_SECURITY_HDRS | TMWDIAG_ID_TX | TMWDIAG_ID_RX |
                                                TMWDIAG_ID_TIMESTAMP | TMWDIAG_ID_ERROR | TMWDIAG_ID_TARGET);
    }
    else
    {
        dnp3_sys->channelConfig.chnlDiagMask = (TMWDIAG_ID_TIMESTAMP | TMWDIAG_ID_ERROR);
    }
    dnp3_sys->channelConfig.pStatCallback = dnp3stats_chnlEventCallback;
    dnp3_sys->pChannel = dnpchnl_openChannel(dnp3_sys->pApplContext, &(dnp3_sys->channelConfig), &(dnp3_sys->tprtConfig), &(dnp3_sys->linkConfig), &(dnp3_sys->physConfig), &(dnp3_sys->IOConfig), &(dnp3_sys->targConfig));
    if (dnp3_sys->pChannel == TMWDEFS_NULL)
    {
        return false;
    }
    if (!dnp3_sys->unsolUpdate)
    {
        dnp3_sys->pChannel->polledMode = true;
    }
    else
    {
        dnp3_sys->pChannel->polledMode = false;
    }
    return true;
}

/**
 * @brief Use the settings in dnp3_sys->clientSesnConfig to open a TMW session.
 * 
 * Also creates request headers for different types of DNP3 requests and associates 
 * each of them with their particular callback functions.
 * 
 * @param sys a reference to the pre-populated GcomSystem for the client.
*/
// TODO link status period in config perhaps -- let's talk about this. TMW is super configurable and I'm not sure what's worthwhile to configure and what's not. We can configure lots of other things, too...
bool openTMWClientSession(GcomSystem &sys)
{
    DNP3Dependencies *dnp3_sys = &(sys.protocol_dependencies->dnp3);
    mdnpsesn_initConfig(&(dnp3_sys->clientSesnConfig));
    dnp3_sys->clientSesnConfig.defaultResponseTimeout = sys.protocol_dependencies->respTime;
    dnp3_sys->clientSesnConfig.linkStatusPeriod = 30000;
    dnp3_sys->clientSesnConfig.source = sys.protocol_dependencies->master_address;
    dnp3_sys->clientSesnConfig.destination = sys.protocol_dependencies->station_address;
    if (dnp3_sys->unsolUpdate)
    {
        dnp3_sys->clientSesnConfig.autoRequestMask |= MDNPSESN_AUTO_ENABLE_UNSOL;
    }
    if (sys.debug)
    {
        dnp3_sys->clientSesnConfig.sesnDiagMask = (TMWDIAG_ID_PHYS | TMWDIAG_ID_LINK | TMWDIAG_ID_TPRT | TMWDIAG_ID_APPL |
                                                   TMWDIAG_ID_USER | TMWDIAG_ID_MMI | TMWDIAG_ID_STATIC_DATA |
                                                   TMWDIAG_ID_STATIC_HDRS | TMWDIAG_ID_EVENT_DATA | TMWDIAG_ID_EVENT_HDRS |
                                                   TMWDIAG_ID_CYCLIC_DATA | TMWDIAG_ID_CYCLIC_HDRS | TMWDIAG_ID_SECURITY_DATA |
                                                   TMWDIAG_ID_SECURITY_HDRS | TMWDIAG_ID_TX | TMWDIAG_ID_RX |
                                                   TMWDIAG_ID_TIMESTAMP | TMWDIAG_ID_ERROR | TMWDIAG_ID_TARGET);
    }
    else
    {
        dnp3_sys->clientSesnConfig.sesnDiagMask = TMWDIAG_ID_ERROR;
    }
    dnp3_sys->clientSesnConfig.pStatCallback = dnp3stats_sesnStatCallback;
    dnp3_sys->pSession = (TMWSESN *)mdnpsesn_openSession(dnp3_sys->pChannel, &(dnp3_sys->clientSesnConfig), TMWDEFS_NULL);
    if (dnp3_sys->pSession == TMWDEFS_NULL)
    {
        return false;
    }
    dnp3_sys->dbHandle = ((MDNPSESN *)(dnp3_sys->pSession))->pDbHandle;
    mdnpsim_setCallback(dnp3_sys->dbHandle, updatePointCallback, dnp3_sys->dbHandle);
    mdnpsesn_setUnsolUserCallback(dnp3_sys->pSession, process_unsolicited_response, &sys);

    mdnpbrm_initReqDesc(&(dnp3_sys->pBinaryCommandRequestDesc), dnp3_sys->pSession);
    sys.protocol_dependencies->dnp3.pBinaryCommandRequestDesc.pUserCallback = process_binary_direct_operate_response;
    sys.protocol_dependencies->dnp3.pBinaryCommandRequestDesc.pUserCallbackParam = &sys;

    mdnpbrm_initReqDesc(&(dnp3_sys->pAnalogCommandRequestDesc), dnp3_sys->pSession);
    sys.protocol_dependencies->dnp3.pAnalogCommandRequestDesc.pUserCallback = process_analog_direct_operate_response;
    sys.protocol_dependencies->dnp3.pAnalogCommandRequestDesc.pUserCallbackParam = &sys;

    mdnpbrm_initReqDesc(&(dnp3_sys->pIntegrityRequestDesc), dnp3_sys->pSession);
    sys.protocol_dependencies->dnp3.pIntegrityRequestDesc.pUserCallback = process_integrity_response;
    sys.protocol_dependencies->dnp3.pIntegrityRequestDesc.pUserCallbackParam = &sys;

    mdnpbrm_initReqDesc(&(dnp3_sys->pClass1RequestDesc), dnp3_sys->pSession);
    sys.protocol_dependencies->dnp3.pClass1RequestDesc.pUserCallback = process_class1_response;
    sys.protocol_dependencies->dnp3.pClass1RequestDesc.pUserCallbackParam = &sys;

    mdnpbrm_initReqDesc(&(dnp3_sys->pClass2RequestDesc), dnp3_sys->pSession);
    sys.protocol_dependencies->dnp3.pClass2RequestDesc.pUserCallback = process_class2_response;
    sys.protocol_dependencies->dnp3.pClass2RequestDesc.pUserCallbackParam = &sys;

    mdnpbrm_initReqDesc(&(dnp3_sys->pClass3RequestDesc), dnp3_sys->pSession);
    sys.protocol_dependencies->dnp3.pClass3RequestDesc.pUserCallback = process_class3_response;
    sys.protocol_dependencies->dnp3.pClass3RequestDesc.pUserCallbackParam = &sys;

    return true;
}

/**
 * @brief Upon receiving a value via fims, check that the received value is within the dbPoint's
 * numeric limits based on its assigned static variation.
 * 
 * If outside those limits, update the value and provide an appropriate error message.
 * 
 * @param dbPoint the TMWSIM_POINT * currently being updated
 * @param value the value that was passed over fims that will be stored in dbPoint->data.analog.value
*/
void check_limits_client(TMWSIM_POINT *dbPoint, double &value)
{
    GcomSystem *sys = ((FlexPoint *)(dbPoint->flexPointHandle))->sys;

    if (dbPoint->defaultStaticVariation == Group40Var1)
    {
        if (value > std::numeric_limits<int32_t>::max())
        {
            value = std::numeric_limits<int32_t>::max();
            if(!spam_limit(sys, sys->point_errors))
            {
                FPS_ERROR_LOG("Set request to analog output point [%d] (32-bit signed int) exceeded maximum (2,147,483,647). Setting to maximum value instead.", dbPoint->pointNumber);
            }
        }
        else if (value < std::numeric_limits<int32_t>::lowest())
        {
            value = std::numeric_limits<int32_t>::lowest();
            if(!spam_limit(sys, sys->point_errors))
            {
                FPS_ERROR_LOG("Set request to analog output point [%d] (32-bit signed int) exceeded minimum (-2,147,483,647). Setting to minimum value instead.", dbPoint->pointNumber);
            }
        }
    }
    else if (dbPoint->defaultStaticVariation == Group40Var2)
    {

        if (value > std::numeric_limits<int16_t>::max())
        {
            value = std::numeric_limits<int16_t>::max();
            if(!spam_limit(sys, sys->point_errors))
            {
                FPS_ERROR_LOG("Set request to analog output point [%d] (16-bit signed int) exceeded maximum (32,767). Setting to maximum value instead.", dbPoint->pointNumber);
            }
        }
        else if (value < std::numeric_limits<int16_t>::lowest())
        {
            value = std::numeric_limits<int16_t>::lowest();
            if(!spam_limit(sys, sys->point_errors))
            {
                FPS_ERROR_LOG("Set request to analog output point [%d] (16-bit signed int) exceeded minimum (-32,768). Setting to minimum value instead.", dbPoint->pointNumber);
            }
        }
    }
    else if (dbPoint->defaultStaticVariation == Group40Var3)
    {
        if (value > std::numeric_limits<float>::max())
        {
            value = std::numeric_limits<float>::max();
            if(!spam_limit(sys, sys->point_errors))
            {
                FPS_ERROR_LOG("Set request to analog output point [%d] (32-bit float) exceeded maximum (%g). Setting to maximum value instead.", dbPoint->pointNumber, std::numeric_limits<float>::max());
            }
        }
        else if (value < -std::numeric_limits<float>::max())
        {
            value = -std::numeric_limits<float>::max();
            if(!spam_limit(sys, sys->point_errors))
            {
                FPS_ERROR_LOG("Set request to analog output point [%d] (32-bit float) exceeded minimum (-%g). Setting to minimum value instead.", dbPoint->pointNumber, std::numeric_limits<float>::max());
            }
        }
        else if (value != 0.0 && abs(value) < std::numeric_limits<float>::min())
        {
            value = 0.0;
            if(!spam_limit(sys, sys->point_errors))
            {
                FPS_ERROR_LOG("Set request to analog output point [%d] (32-bit float) exceeded minimum exponent value (%g). Setting to 0 instead.", dbPoint->pointNumber, std::numeric_limits<float>::min());
            }
        }
    }
}

/// @brief 
/// @param pSetWork 
void send_analog_command_callback(void *pSetWork)
{
    SetWork *set_work = (SetWork *)pSetWork;
    TMWSIM_POINT *dbPoint = set_work->dbPoint;
    double value = set_work->value;
    MDNPBRM_ANALOG_INFO analogValue;
    analogValue.pointNumber = dbPoint->pointNumber;

    analogValue.value.type = TMWTYPES_ANALOG_TYPE_DOUBLE;
    if (((FlexPoint *)(dbPoint->flexPointHandle))->scale == 0.0)
    {
        analogValue.value.value.dval = value;
    }
    else
    {
        analogValue.value.value.dval = value * ((FlexPoint *)(dbPoint->flexPointHandle))->scale;
    }

    check_limits_client(dbPoint, analogValue.value.value.dval);

    mdnpbrm_analogCommand(&(((FlexPoint *)dbPoint->flexPointHandle)->sys)->protocol_dependencies->dnp3.pAnalogCommandRequestDesc, TMWDEFS_NULL, DNPDEFS_FC_DIRECT_OP, MDNPBRM_AUTO_MODE_NONE, 0,
                          DNPDEFS_QUAL_16BIT_INDEX, dbPoint->defaultStaticVariation, 1, &analogValue);
}

/// @brief 
/// @param pSetWork 
void send_binary_command_callback(void *pSetWork)
{
    SetWork *set_work = (SetWork *)pSetWork;
    TMWSIM_POINT *dbPoint = set_work->dbPoint;
    double value = set_work->value;
    bool bool_value = static_cast<bool>(value);
    if (((FlexPoint *)(dbPoint->flexPointHandle))->scale < 0)
    {
        bool_value = !bool_value;
    }
    MDNPBRM_CROB_INFO CROBInfo;
    CROBInfo.pointNumber = dbPoint->pointNumber;
    CROBInfo.control = (TMWTYPES_UCHAR)(bool_value ? DNPDEFS_CROB_CTRL_LATCH_ON : DNPDEFS_CROB_CTRL_LATCH_OFF);
    CROBInfo.onTime = 0;
    CROBInfo.offTime = 0;
    mdnpbrm_binaryCommand(&(((FlexPoint *)dbPoint->flexPointHandle)->sys)->protocol_dependencies->dnp3.pBinaryCommandRequestDesc, TMWDEFS_NULL, DNPDEFS_FC_DIRECT_OP, MDNPBRM_AUTO_MODE_NONE, 0,
                          DNPDEFS_QUAL_16BIT_INDEX, 1, &CROBInfo);
}

/// @brief 
/// @param pSetWork 
void send_interval_analog_command_callback(void *pSetWork)
{
    send_analog_command_callback(pSetWork);
    TMWSIM_POINT *dbPoint = ((SetWork *)pSetWork)->dbPoint;
    tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->set_timer),
                   ((FlexPoint *)(dbPoint->flexPointHandle))->interval_set_rate,
                   (((FlexPoint *)dbPoint->flexPointHandle)->sys)->protocol_dependencies->dnp3.pChannel,
                   send_interval_analog_command_callback,
                   pSetWork);
}

/// @brief 
/// @param pSetWork 
void send_interval_binary_command_callback(void *pSetWork)
{
    send_binary_command_callback(pSetWork);
    TMWSIM_POINT *dbPoint = ((SetWork *)pSetWork)->dbPoint;
    tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->set_timer),
                   ((FlexPoint *)(dbPoint->flexPointHandle))->interval_set_rate,
                   (((FlexPoint *)dbPoint->flexPointHandle)->sys)->protocol_dependencies->dnp3.pChannel,
                   send_interval_binary_command_callback,
                   pSetWork);
}

/// @brief  handle the batching of set points
/// @param dbPoint 
/// @param value 
void handle_batch_sets(TMWSIM_POINT *dbPoint, double value)
{
    if (dbPoint->type == TMWSIM_TYPE_ANALOG)
    {
        ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value = value;
        
        if (!tmwtimer_isActive(&((FlexPoint *)(dbPoint->flexPointHandle))->set_timer))
        {
            if (((FlexPoint *)(dbPoint->flexPointHandle))->interval_sets)
            {
                tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->set_timer),
                               ((FlexPoint *)(dbPoint->flexPointHandle))->interval_set_rate,
                               (((FlexPoint *)dbPoint->flexPointHandle)->sys)->protocol_dependencies->dnp3.pChannel,
                               send_interval_analog_command_callback,
                               (void *)(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work));
            }
            else if (((FlexPoint *)(dbPoint->flexPointHandle))->batch_sets)
            {
                tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->set_timer),
                               ((FlexPoint *)(dbPoint->flexPointHandle))->batch_set_rate,
                               (((FlexPoint *)dbPoint->flexPointHandle)->sys)->protocol_dependencies->dnp3.pChannel,
                               send_analog_command_callback,
                               (void *)(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work));
            }
            else
            {
                send_analog_command_callback((void *)(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work));
            }
        }
    }
    else // TMWSIM_TYPE_BINARY
    {
        ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value = value;
        if (!tmwtimer_isActive(&((FlexPoint *)(dbPoint->flexPointHandle))->set_timer))
        {
            if (((FlexPoint *)(dbPoint->flexPointHandle))->interval_sets)
            {
                tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->set_timer),
                               ((FlexPoint *)(dbPoint->flexPointHandle))->interval_set_rate,
                               (((FlexPoint *)dbPoint->flexPointHandle)->sys)->protocol_dependencies->dnp3.pChannel,
                               send_interval_binary_command_callback,
                               (void *)(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work));
            }
            else if (((FlexPoint *)(dbPoint->flexPointHandle))->batch_sets)
            {
                tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->set_timer),
                               ((FlexPoint *)(dbPoint->flexPointHandle))->batch_set_rate,
                               (((FlexPoint *)dbPoint->flexPointHandle)->sys)->protocol_dependencies->dnp3.pChannel,
                               send_binary_command_callback,
                               (void *)(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work));
            }
            else
            {
                send_binary_command_callback((void *)(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work));
            }
        }
    }
}

/**
 * @brief Parse incoming set messages. For each incoming data point, queue a DNP3 command
 * for that point.
 * 
 * Upon receiving a fims message, parse the message (single or multi-point, with or without
 * the keyword "value"). For each valid point, trigger the sending of an analog or binary
 * command to set the value accordingly.
 * 
 * @param sys GcomSystem for DNP3 Client
 * @param meta_data Meta_Data_Info from incoming fims message
 * 
 * @pre the fims message header has been pre-processed such that
 * sys.fims_dependencies->uri_view corresponds to the current message uri.
 * @pre sys.fims_dependencies->data_buf has been properly initialized
*/
bool parseBodyClient(GcomSystem &sys, Meta_Data_Info &meta_data)
{
    auto &parser = sys.fims_dependencies->parser;
    auto &doc = sys.fims_dependencies->doc;

    memset(reinterpret_cast<u8 *>(sys.fims_dependencies->data_buf) + meta_data.data_len, '\0', simdjson::SIMDJSON_PADDING);
    // gets doc
    if (const auto err = parser.iterate(reinterpret_cast<const char *>(sys.fims_dependencies->data_buf),
                                        meta_data.data_len,
                                        meta_data.data_len + simdjson::SIMDJSON_PADDING)
                             .get(doc);
        err)
    {
        if(!spam_limit(&sys, sys.parse_errors))
        {
            FPS_ERROR_LOG("parse error");
            FPS_LOG_IT("parsing_error");
        }
        return false;
    }

    // now set onto channels based on multi or single set uri:
    Jval_buif to_set;
    bool ok = true;
    if (uriIsMultiOrSingle(sys, sys.fims_dependencies->uri_view)) // multi-set
    {
        simdjson::ondemand::object set_obj;
        if (const auto err = doc.get(set_obj); err)
        {
            if(!spam_limit(&sys, sys.fims_errors))
            {
                FPS_ERROR_LOG("doc.get error %s", simdjson::error_message(err));
                FPS_LOG_IT("parsing_error");
            }
            return false;
        }

        int num_analog_requests = 0;
        sys.protocol_dependencies->dnp3.count_var1_requests = 0;
        sys.protocol_dependencies->dnp3.count_var2_requests = 0;
        sys.protocol_dependencies->dnp3.count_var3_requests = 0;
        sys.protocol_dependencies->dnp3.count_var4_requests = 0;
        int num_binary_requests = 0;

        if (sys.debug)
        {
            FPS_DEBUG_LOG("Uri : %s", sys.fims_dependencies->uri_view);
        }
        for (auto pair : set_obj)
        {
            const auto key = pair.unescaped_key();
            if (const auto err = key.error(); err)
            {
                if(!spam_limit(&sys, sys.parse_errors))
                {
                    FPS_ERROR_LOG(" parsing error [%s] on getting a multi-set key, Message dropped", simdjson::error_message(err));
                    FPS_LOG_IT("parsing_error");
                }
                ok = false;
                break;
            }
            const auto key_view = key.value_unsafe();
            auto val = pair.value();
            if (const auto err = val.error(); err)
            {
                if(!spam_limit(&sys, sys.parse_errors))
                {
                    FPS_ERROR_LOG("parsing error on multi-set key  err = [%s] Message dropped", simdjson::error_message(err));
                    FPS_LOG_IT("parsing_error");
                }
                ok = false;
                break;
            }

            auto curr_val = val.value_unsafe();
            auto val_clothed = curr_val.get_object();
            auto success = extractValueMulti(sys, val_clothed, curr_val, to_set);
            if (!success)
            {
                ok = false;
                continue;
            }
            // add event
            TMWSIM_POINT *dbPoint = getDbVar(sys, sys.fims_dependencies->uri_view, key_view);
            // u8 eventVariation = dbPoint->defaultEventVariation;
            if (!dbPoint)
            {
                if (sys.debug > 0)
                {
                    FPS_ERROR_LOG("could not find point %s:%s", sys.fims_dependencies->uri_view, key_view);
                    FPS_LOG_IT("could_not_find_point");
                }
                ok = false;
                continue;
            }
            else if (((FlexPoint *)dbPoint->flexPointHandle)->type != Register_Types::AnOPF32 &&
                     ((FlexPoint *)dbPoint->flexPointHandle)->type != Register_Types::AnOPInt16 &&
                     ((FlexPoint *)dbPoint->flexPointHandle)->type != Register_Types::AnOPInt32 &&
                     ((FlexPoint *)dbPoint->flexPointHandle)->type != Register_Types::AnalogOS &&
                     ((FlexPoint *)dbPoint->flexPointHandle)->type != Register_Types::CROB &&
                     ((FlexPoint *)dbPoint->flexPointHandle)->type != Register_Types::BinaryOS)
            {
                ok = false;
                if (sys.debug > 0)
                {
                    FPS_ERROR_LOG("with single-pub uri: '%s', point is not an OUTPUT type (analog output or binary output)", sys.fims_dependencies->uri_view);
                    FPS_LOG_IT("set_to_wrong_type");
                }
                continue;
            }
            double value = jval_to_double(to_set);
            if (((FlexPoint *)(dbPoint->flexPointHandle))->batch_sets || ((FlexPoint *)(dbPoint->flexPointHandle))->interval_sets)
            {
                handle_batch_sets(dbPoint, value);
            }
            else if (dbPoint->type == TMWSIM_TYPE_ANALOG)
            {
                num_analog_requests++;
                if (dbPoint->defaultStaticVariation == Group40Var1)
                { // 32 bit integer
                    auto &analogValue = sys.protocol_dependencies->dnp3.analogOutputVar1Values[sys.protocol_dependencies->dnp3.count_var1_requests];
                    sys.protocol_dependencies->dnp3.count_var1_requests++;
                    analogValue.pointNumber = dbPoint->pointNumber;
                    analogValue.value.type = TMWTYPES_ANALOG_TYPE_DOUBLE;
                    if (((FlexPoint *)(dbPoint->flexPointHandle))->scale == 0.0)
                    {
                        analogValue.value.value.dval = value;
                    }
                    else
                    {
                        analogValue.value.value.dval = value * ((FlexPoint *)(dbPoint->flexPointHandle))->scale;
                    }

                    check_limits_client(dbPoint, analogValue.value.value.dval);
                }
                else if (dbPoint->defaultStaticVariation == Group40Var2) // 16 bit int
                {
                    auto &analogValue = sys.protocol_dependencies->dnp3.analogOutputVar2Values[sys.protocol_dependencies->dnp3.count_var2_requests];
                    sys.protocol_dependencies->dnp3.count_var2_requests++;
                    analogValue.pointNumber = dbPoint->pointNumber;
                    analogValue.value.type = TMWTYPES_ANALOG_TYPE_DOUBLE;
                    if (((FlexPoint *)(dbPoint->flexPointHandle))->scale == 0.0)
                    {
                        analogValue.value.value.dval = value;
                    }
                    else
                    {
                        analogValue.value.value.dval = value * ((FlexPoint *)(dbPoint->flexPointHandle))->scale;
                    }

                    check_limits_client(dbPoint, analogValue.value.value.dval);
                }
                else if (dbPoint->defaultStaticVariation == Group40Var3)
                { // 32 bit float
                    auto &analogValue = sys.protocol_dependencies->dnp3.analogOutputVar3Values[sys.protocol_dependencies->dnp3.count_var3_requests];
                    sys.protocol_dependencies->dnp3.count_var3_requests++;
                    analogValue.pointNumber = dbPoint->pointNumber;
                    analogValue.value.type = TMWTYPES_ANALOG_TYPE_DOUBLE;
                    if (((FlexPoint *)(dbPoint->flexPointHandle))->scale == 0.0)
                    {
                        analogValue.value.value.dval = value;
                    }
                    else
                    {
                        analogValue.value.value.dval = value * ((FlexPoint *)(dbPoint->flexPointHandle))->scale;
                    }

                    check_limits_client(dbPoint, analogValue.value.value.dval);
                }
                else // 64 bit float
                {
                    auto &analogValue = sys.protocol_dependencies->dnp3.analogOutputVar4Values[sys.protocol_dependencies->dnp3.count_var4_requests];
                    sys.protocol_dependencies->dnp3.count_var4_requests++;
                    analogValue.pointNumber = dbPoint->pointNumber;
                    analogValue.value.type = TMWTYPES_ANALOG_TYPE_DOUBLE;
                    if (((FlexPoint *)(dbPoint->flexPointHandle))->scale == 0.0)
                    {
                        analogValue.value.value.dval = value;
                    }
                    else
                    {
                        analogValue.value.value.dval = value * ((FlexPoint *)(dbPoint->flexPointHandle))->scale;
                    }

                    // if we exceed any limits here, I think it's already taken care of...
                }
            }
            else // TMWSIM_TYPE_BINARY
            {
                bool bool_value = static_cast<bool>(value);
                if (((FlexPoint *)(dbPoint->flexPointHandle))->scale < 0)
                {
                    bool_value = !bool_value;
                }
                MDNPBRM_CROB_INFO &CROBInfo = sys.protocol_dependencies->dnp3.CROBInfo[num_binary_requests];
                num_binary_requests++;
                CROBInfo.pointNumber = dbPoint->pointNumber;
                CROBInfo.control = (TMWTYPES_UCHAR)(bool_value ? DNPDEFS_CROB_CTRL_LATCH_ON : DNPDEFS_CROB_CTRL_LATCH_OFF);
                CROBInfo.onTime = 0;
                CROBInfo.offTime = 0;
            }
        }
        if (num_analog_requests > 0)
        {
            if (sys.protocol_dependencies->dnp3.count_var1_requests > 0)
            {
                mdnpbrm_analogCommand(&sys.protocol_dependencies->dnp3.pAnalogCommandRequestDesc, TMWDEFS_NULL, DNPDEFS_FC_DIRECT_OP, MDNPBRM_AUTO_MODE_NONE, 0,
                                      DNPDEFS_QUAL_16BIT_INDEX, 1, sys.protocol_dependencies->dnp3.count_var1_requests, sys.protocol_dependencies->dnp3.analogOutputVar1Values);
            }
            if (sys.protocol_dependencies->dnp3.count_var2_requests > 0)
            {
                mdnpbrm_analogCommand(&sys.protocol_dependencies->dnp3.pAnalogCommandRequestDesc, TMWDEFS_NULL, DNPDEFS_FC_DIRECT_OP, MDNPBRM_AUTO_MODE_NONE, 0,
                                      DNPDEFS_QUAL_16BIT_INDEX, 2, sys.protocol_dependencies->dnp3.count_var2_requests, sys.protocol_dependencies->dnp3.analogOutputVar2Values);
            }
            if (sys.protocol_dependencies->dnp3.count_var3_requests > 0)
            {
                mdnpbrm_analogCommand(&sys.protocol_dependencies->dnp3.pAnalogCommandRequestDesc, TMWDEFS_NULL, DNPDEFS_FC_DIRECT_OP, MDNPBRM_AUTO_MODE_NONE, 0,
                                      DNPDEFS_QUAL_16BIT_INDEX, 3, sys.protocol_dependencies->dnp3.count_var3_requests, sys.protocol_dependencies->dnp3.analogOutputVar3Values);
            }
            if (sys.protocol_dependencies->dnp3.count_var4_requests > 0)
            {
                mdnpbrm_analogCommand(&sys.protocol_dependencies->dnp3.pAnalogCommandRequestDesc, TMWDEFS_NULL, DNPDEFS_FC_DIRECT_OP, MDNPBRM_AUTO_MODE_NONE, 0,
                                      DNPDEFS_QUAL_16BIT_INDEX, 4, sys.protocol_dependencies->dnp3.count_var4_requests, sys.protocol_dependencies->dnp3.analogOutputVar4Values);
            }
        }
        if (num_binary_requests > 0)
        {
            mdnpbrm_binaryCommand(&sys.protocol_dependencies->dnp3.pBinaryCommandRequestDesc, TMWDEFS_NULL, DNPDEFS_FC_DIRECT_OP, MDNPBRM_AUTO_MODE_NONE, 0,
                                  DNPDEFS_QUAL_16BIT_INDEX, num_binary_requests, sys.protocol_dependencies->dnp3.CROBInfo);
        }
        return ok;
    }
    else // single-set
    {
        if (sys.debug)
        {
            FPS_DEBUG_LOG("Single Uri : %s", sys.fims_dependencies->uri_view);
        }
        simdjson::ondemand::value curr_val;
        auto val_clothed = doc.get_object();

        auto success = extractValueSingle(sys, val_clothed, curr_val, to_set);
        if (!success)
        {
            return false;
        }

        TMWSIM_POINT *dbPoint = getDbVar(sys, sys.fims_dependencies->uri_view, {});
        // u8 eventVariation = dbPoint->defaultEventVariation;
        if (!dbPoint)
        {
            if (sys.debug > 0)
            {
                FPS_ERROR_LOG("could not find point %s", sys.fims_dependencies->uri_view);
                FPS_LOG_IT("could_not_find_point");
            }
            return false;
        }
        else if (((FlexPoint *)dbPoint->flexPointHandle)->type != Register_Types::AnOPF32 &&
                 ((FlexPoint *)dbPoint->flexPointHandle)->type != Register_Types::AnOPInt16 &&
                 ((FlexPoint *)dbPoint->flexPointHandle)->type != Register_Types::AnOPInt32 &&
                 ((FlexPoint *)dbPoint->flexPointHandle)->type != Register_Types::AnalogOS &&
                 ((FlexPoint *)dbPoint->flexPointHandle)->type != Register_Types::CROB &&
                 ((FlexPoint *)dbPoint->flexPointHandle)->type != Register_Types::BinaryOS)
        {
            if (sys.debug > 0)
            {
                FPS_ERROR_LOG("with single-pub uri: '%s', point is not an OUTPUT type (analog output or binary output)", sys.fims_dependencies->uri_view);
                FPS_LOG_IT("set_to_wrong_type");
            }
            return false;
        }

        double value = jval_to_double(to_set);
        handle_batch_sets(dbPoint, value);
    }

    return true;
}

#ifndef DNP3_TEST_MODE
/// @brief 
/// @param argc 
/// @param argv 
/// @return 
int main(int argc, char *argv[])
{
    // std::string cmd;
    // if (argc > 1)
    // {
    //     cmd =std::string(argv[1]);
    // }
    // if(cmd == "test_io"){
    //     testIO(argc, argv);
    //     return 0;
    // }
    std::string file_name = getFileName(argc, argv);
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    Logging::Init(file_name, argc, argv);

    clientSys.git_version_info.init();

    FPS_INFO_LOG("build: %s",  clientSys.git_version_info.get_build());
    FPS_INFO_LOG("commit: %s", clientSys.git_version_info.get_commit());
    FPS_INFO_LOG("tag: %s",    clientSys.git_version_info.get_tag());



    // assume 1 for right now
    num_configs_client = getConfigs(argc, argv, clientSys, DNP3_MASTER);
    clientSys.config_file_name = file_name;
    if (num_configs_client > 0)
    // ZoneScoped;
    {
        // These are the basic things that need to happen with TMW
        // regardless of any config settings.
        // Later, we will need to modify IOConfig, initConfig for
        // the channel and the session, openChannel, and openSession.
        DNP3Dependencies *dnp3_sys = &(clientSys.protocol_dependencies->dnp3);
        dnp3_sys->openTMWChannel = openTMWClientChannel;
        dnp3_sys->openTMWSession = openTMWClientSession;
        initStatsMonitor(clientSys);
        
        if (!init_tmw(clientSys))
        {
            shutdown_tmw(dnp3_sys);
            FPS_LOG_IT("tmw_fail_to_initialize");
            return 0;
        }

        clientSys.fims_dependencies->parseBody = parseBodyClient;
        init_fims(clientSys);


        

        init_vars(clientSys, num_configs_client);

        // as far as I can tell, analog command requests need to be sent out by variation
        // thus, we need 4 different lists of request information, one for each variation
        TMWTYPES_UINT num_analog_outputs = tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs);
        TMWTYPES_UINT num_binary_outputs = tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs);
        dnp3_sys->analogOutputVar1Values = new MDNPBRM_ANALOG_INFO[num_analog_outputs];
        dnp3_sys->analogOutputVar2Values = new MDNPBRM_ANALOG_INFO[num_analog_outputs];
        dnp3_sys->analogOutputVar3Values = new MDNPBRM_ANALOG_INFO[num_analog_outputs];
        dnp3_sys->analogOutputVar4Values = new MDNPBRM_ANALOG_INFO[num_analog_outputs];
        dnp3_sys->CROBInfo = new MDNPBRM_CROB_INFO[num_binary_outputs];

        getSysUris(clientSys, DNP3_MASTER, num_configs_client);

        show_fims_subs(clientSys);

        if (!fims_connect(clientSys))
        {
            shutdown_tmw(&clientSys.protocol_dependencies->dnp3);
            FPS_LOG_IT("fims_fail_to_connect");
            return 0;
        }

        std::string message("DNP3 Client running");
        emit_event(&clientSys.fims_dependencies->fims_gateway, clientSys.fims_dependencies->name.c_str(), message.c_str(), 3);

        clientSys.listener_future = std::async(std::launch::async, listener_thread, std::ref(clientSys));
        if (dnp3_sys->stats_pub_frequency > 0)
        {
            clientSys.stats_pub_future = std::async(std::launch::async, pub_stats_thread, std::ref(clientSys));
        }
        clientSys.watchdog_future = std::async(std::launch::async, setupWatchdogTimer, std::ref(clientSys));
        clientSys.heartbeat_future = std::async(std::launch::async, setupHeartbeatTimer, std::ref(clientSys));
        usleep(500); // wait for things to settle before we start
        clientSys.start_signal = true;
        clientSys.keep_running = true;
        clientSys.main_cond.notify_all();
        FPS_INFO_LOG("DNP3 Client Setup complete: Entering main loop.");
        FPS_LOG_IT("startup");

        defer
        {
            clientSys.keep_running = false;
        };

        TMWTYPES_MILLISECONDS myTime = tmwtarg_getMSTime();
        TMWTYPES_MILLISECONDS lastPoll = myTime;//tmwtarg_getMSTime();
        TMWTYPES_MILLISECONDS lastClass1 = myTime;//tmwtarg_getMSTime();
        TMWTYPES_MILLISECONDS lastClass2 = myTime;//tmwtarg_getMSTime();
        TMWTYPES_MILLISECONDS lastClass3 = myTime;//tmwtarg_getMSTime();

        while ((((MDNPSESN *)dnp3_sys->pSession)->unsolRespState == MDNPSESN_UNSOL_STARTUP) && clientSys.keep_running)
        {
            tmwappl_checkForInput(clientSys.protocol_dependencies->dnp3.pApplContext);
            tmwpltmr_checkTimer();
            tmwtarg_sleep(1);
        }

        while (clientSys.keep_running)
        {
            myTime = tmwtarg_getMSTime();
            if (myTime > (lastPoll + clientSys.protocol_dependencies->frequency))
            {
                lastPoll = myTime;
                lastClass1 = myTime;
                lastClass2 = myTime;
                lastClass3 = myTime;
                mdnpbrm_integrityPoll(&clientSys.protocol_dependencies->dnp3.pIntegrityRequestDesc);
            }
            else
            {
                if (clientSys.protocol_dependencies->dnp3.freq1 > 0 && myTime > (lastClass1 + clientSys.protocol_dependencies->dnp3.freq1))
                {
                    lastClass1 = myTime;
                    mdnpbrm_readClass(&clientSys.protocol_dependencies->dnp3.pClass1RequestDesc,
                                      TMWDEFS_NULL, DNPDEFS_QUAL_ALL_POINTS, 0,
                                      TMWDEFS_FALSE, TMWDEFS_TRUE, TMWDEFS_FALSE, TMWDEFS_FALSE);
                }
                if (clientSys.protocol_dependencies->dnp3.freq2 > 0 && myTime > (lastClass2 + clientSys.protocol_dependencies->dnp3.freq2))
                {
                    lastClass2 = myTime;
                    mdnpbrm_readClass(&clientSys.protocol_dependencies->dnp3.pClass2RequestDesc,
                                      TMWDEFS_NULL, DNPDEFS_QUAL_ALL_POINTS, 0,
                                      TMWDEFS_FALSE, TMWDEFS_FALSE, TMWDEFS_TRUE, TMWDEFS_FALSE);
                }
                if (clientSys.protocol_dependencies->dnp3.freq3 > 0 && myTime > (lastClass3 + clientSys.protocol_dependencies->dnp3.freq3))
                {
                    lastClass3 = myTime;
                    mdnpbrm_readClass(&clientSys.protocol_dependencies->dnp3.pClass3RequestDesc,
                                      TMWDEFS_NULL, DNPDEFS_QUAL_ALL_POINTS, 0,
                                      TMWDEFS_FALSE, TMWDEFS_FALSE, TMWDEFS_FALSE, TMWDEFS_TRUE);
                }
            }

            tmwappl_checkForInput(clientSys.protocol_dependencies->dnp3.pApplContext);
            tmwpltmr_checkTimer();
            tmwtarg_sleep(1);
        }

        FPS_INFO_LOG("Cleaning up loose threads before exit.");
        message = "DNP3 Client stopping";
        emit_event(&clientSys.fims_dependencies->fims_gateway, clientSys.fims_dependencies->name.c_str(), message.c_str(), 3);


        auto done_listening = clientSys.listener_future.get();
        bool done_pubbing = false;
        if (dnp3_sys->stats_pub_frequency > 0)
        {
            done_pubbing = clientSys.stats_pub_future.get();
        }
        else
        {
            done_pubbing = true;
        }
        clientSys.watchdog_future.get();
        clientSys.heartbeat_future.get();
        shutdown_tmw(&clientSys.protocol_dependencies->dnp3);

        FPS_INFO_LOG("fims listen thread complete: %s", done_listening ? "true" : "false");
        FPS_INFO_LOG("pub stats thread complete: %s", done_pubbing ? "true" : "false");
        FPS_LOG_IT("shutdown");
        return 0;
    }
}
#endif