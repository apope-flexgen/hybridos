#define DNP3_SERVER
#include <future>
#include <functional>
#include <climits>
#include "shared_utils.hpp"
#include "gcom_dnp3_system_structs.h"
#include "gcom_dnp3_fims.h"
#include "Jval_buif.hpp"
#include "tmwscl/utils/tmwsim.h"

extern "C"
{
#include "tmwscl/utils/tmwpltmr.h"
#include "tmwscl/dnp/sdnpo002.h"
#include "tmwscl/dnp/sdnpo022.h"
#include "tmwscl/dnp/sdnpo032.h"
#include "tmwscl/dnp/sdnpunsl.h"
}

#include "gcom_dnp3_stats.h"
#include "logger/gcom_dnp3_logger.h"
#include "gcom_dnp3_utils.h"
#include "gcom_dnp3_flags.h"
#include "gcom_dnp3_tmw_utils.h"
#include "gcom_dnp3_server.h"
#include "gcom_dnp3_point_utils.h"
#include "version.h"

// #define TRACY_ENABLE
// #include "tracy/Tracy.hpp"

using namespace std;
GcomSystem serverSys = GcomSystem(Protocol::DNP3);

#ifndef DNP3_TEST_MODE
GcomSystem *clientSysp = nullptr; //&Sys;
#endif
GcomSystem *serverSysp = &serverSys;

int64_t num_configs_server = 0;

#ifndef MAX_CONFIGS
#define MAX_CONFIGS 16
#endif

void signal_handler_server(int sig)
{
    serverSys.keep_running = false;
    FPS_ERROR_LOG("signal of type %s caught.", strsignal(sig));
    signal(sig, SIG_DFL);
};

/**
 * @brief Prepare and send a fims set message for an analog or binary output point.
 *
 * This function is called from (or set as a timer callback) from received_command_callback
 * after a DNP3 command message is received.
 *
 * @param pSetWork void * pointer to the SetWork for a particular point
 */
void fimsSendSetCallback(void *pSetWork)
{
    SetWork *set_work = (SetWork *)pSetWork;
    set_work->send_buf.clear();
    TMWSIM_POINT *dbPoint = set_work->dbPoint;
    if(((FlexPoint *)dbPoint->flexPointHandle)->is_individual_bits){
        for(auto bit : ((FlexPoint *)dbPoint->flexPointHandle)->dbBits) {
            set_work->send_buf.clear();
            if(bit.first == "Unknown"){
                continue;
            }
            format_individual_bit(set_work->send_buf, dbPoint, set_work->value, 255, nullptr, bit.first);
            std::string uri = std::string(((FlexPoint *)dbPoint->flexPointHandle)->uri) + "/" + bit.first;
            if (!send_set((((FlexPoint *)dbPoint->flexPointHandle)->sys)->fims_dependencies->fims_gateway, std::string_view{uri.data(), uri.size()}, std::string_view{set_work->send_buf.data(), set_work->send_buf.size()}))
            {
                if (!spam_limit(((FlexPoint *)dbPoint->flexPointHandle)->sys, ((FlexPoint *)dbPoint->flexPointHandle)->sys->fims_errors))
                {
                    FPS_ERROR_LOG("could not send fims_set to %s", std::string_view{uri.data(), uri.size()});
                    FPS_LOG_IT("fims_send_error");
                }
            }
        }
        
    } else {
        format_point(set_work->send_buf, dbPoint, set_work->value, 255, nullptr, false);
        if (!send_set((((FlexPoint *)dbPoint->flexPointHandle)->sys)->fims_dependencies->fims_gateway, std::string_view{set_work->send_uri.data(), set_work->send_uri.size()}, std::string_view{set_work->send_buf.data(), set_work->send_buf.size()}))
        {
            if (!spam_limit(((FlexPoint *)dbPoint->flexPointHandle)->sys, ((FlexPoint *)dbPoint->flexPointHandle)->sys->fims_errors))
            {
                FPS_ERROR_LOG("could not send fims_set to %s", std::string_view{set_work->send_uri.data(), set_work->send_uri.size()});
                FPS_LOG_IT("fims_send_error");
            }
        }
    }
}

/**
 * @brief Call fimsSendSetCallback and then restart the interval timer for the point to call
 * this function again after the interval period.
 *
 * This function is called from (or set as a timer callback) from received_command_callback
 * after a DNP3 command message is received.
 *
 * @param pSetWork void * pointer to the SetWork for a particular point
 */
void fimsSendIntervalSetCallback(void *pSetWork)
{
    fimsSendSetCallback(pSetWork);
    TMWSIM_POINT *dbPoint = ((SetWork *)pSetWork)->dbPoint;
    tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->set_timer),
                   ((FlexPoint *)(dbPoint->flexPointHandle))->interval_set_rate,
                   (((FlexPoint *)dbPoint->flexPointHandle)->sys)->protocol_dependencies->dnp3.pChannel,
                   fimsSendIntervalSetCallback,
                   (void *)(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work));
}

/**
 * @brief In response to an incoming DNP3 command, prepare to send a 'set' over fims based
 * on whether the point is a batch set, interval set, or direct set.
 *
 * In response to an incoming DNP3 command, update the value that will be sent out
 * as a set over fims. If batched or interval sets are configured, start the set timer with
 * the appropriate callback (fimsSendSetCallback or fimsSendIntervalSetCallback). If the
 * point is configured for direct sets, call fimsSendSetCallback immediately.
 *
 * This function should be configured at startup as the sdnpsim callback function with the
 * sdnpsim database pointer as the callback parameter.
 *
 * @param pDbHandle void * pointer to the database handle
 * @param type TMWSIM_EVENT_TYPE, which will likely only ever be TMWSIM_POINT_UPDATE (though
 * it could potentially also be TMWSIM_POINT_ADD, TMWSIM_POINT_DELETE, or TMWSIM_CLEAR_DATABASE.)
 * @param objectGroup DNPDEFS_OBJ_GROUP_ID corresponds to any valid DNP3 group number. This
 * function currently only handles Group 40 and Group 10.
 * @param pointNumber TMWTYPES_UINT the point number for the current point being handled
 */
void received_command_callback(void *pDbHandle, TMWSIM_EVENT_TYPE type, DNPDEFS_OBJ_GROUP_ID objectGroup, TMWTYPES_UINT pointNumber)
{
    if (type == TMWSIM_POINT_UPDATE)
    {
        if (objectGroup == DNPDEFS_OBJ_40_ANA_OUT_STATUSES)
        {
            TMWSIM_POINT *dbPoint = (TMWSIM_POINT *)sdnpsim_anlgOutGetPoint(pDbHandle, pointNumber);
            if (dbPoint != nullptr && dbPoint->flexPointHandle != nullptr && dbPoint->reason == TMWDEFS_CHANGE_REMOTE_OP)
            {
                // checkOutputOverflow(dbPoint); // I don't think this works...something weird with how the over_range flag is set
                ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value = dbPoint->data.analog.value;
                if(((FlexPoint *)dbPoint->flexPointHandle)->sent_operate) { // sent_operate is actually not accurate here...it's more like "received_value_over_fims"
                    dbPoint->data.analog.value = ((FlexPoint *)(dbPoint->flexPointHandle))->operate_value; // this is the last value received over fims
                }
                if (!tmwtimer_isActive(&((FlexPoint *)(dbPoint->flexPointHandle))->set_timer))
                {
                    if (((FlexPoint *)(dbPoint->flexPointHandle))->interval_sets)
                    {
                        tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->set_timer),
                                       ((FlexPoint *)(dbPoint->flexPointHandle))->interval_set_rate,
                                       (((FlexPoint *)dbPoint->flexPointHandle)->sys)->protocol_dependencies->dnp3.pChannel,
                                       fimsSendIntervalSetCallback,
                                       (void *)(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work));
                    }
                    else if (((FlexPoint *)(dbPoint->flexPointHandle))->batch_sets)
                    {
                        tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->set_timer),
                                       ((FlexPoint *)(dbPoint->flexPointHandle))->batch_set_rate,
                                       (((FlexPoint *)dbPoint->flexPointHandle)->sys)->protocol_dependencies->dnp3.pChannel,
                                       fimsSendSetCallback,
                                       (void *)(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work));
                    }
                    else
                    {
                        fimsSendSetCallback((void *)(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work));
                    }
                }
            }
        }
        else if (objectGroup == DNPDEFS_OBJ_10_BIN_OUT_STATUSES)
        {

            TMWSIM_POINT *dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetPoint(pDbHandle, pointNumber);
            if (dbPoint != nullptr && dbPoint->flexPointHandle != nullptr && dbPoint->reason == TMWDEFS_CHANGE_REMOTE_OP)
            {
                // checkOutputOverflow(dbPoint); // I don't think this works...something weird with how the over_range flag is set
                ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value = dbPoint->data.binary.value;
                if(((FlexPoint *)dbPoint->flexPointHandle)->sent_operate) { // sent_operate is actually not accurate here...it's more like "received_value_over_fims"
                    dbPoint->data.binary.value = static_cast<bool>(((FlexPoint *)(dbPoint->flexPointHandle))->operate_value); // this is the last value received over fims
                }
                if (!tmwtimer_isActive(&((FlexPoint *)(dbPoint->flexPointHandle))->set_timer))
                {
                    if (((FlexPoint *)(dbPoint->flexPointHandle))->interval_sets)
                    {
                        tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->set_timer),
                                       ((FlexPoint *)(dbPoint->flexPointHandle))->interval_set_rate,
                                       (((FlexPoint *)dbPoint->flexPointHandle)->sys)->protocol_dependencies->dnp3.pChannel,
                                       fimsSendIntervalSetCallback,
                                       (void *)(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work));
                    }
                    else if (((FlexPoint *)(dbPoint->flexPointHandle))->batch_sets)
                    {
                        tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->set_timer),
                                       ((FlexPoint *)(dbPoint->flexPointHandle))->batch_set_rate,
                                       (((FlexPoint *)dbPoint->flexPointHandle)->sys)->protocol_dependencies->dnp3.pChannel,
                                       fimsSendSetCallback,
                                       (void *)(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work));
                    }
                    else
                    {
                        fimsSendSetCallback((void *)(&((FlexPoint *)(dbPoint->flexPointHandle))->set_work));
                    }
                }
            }
        }
        // else if (objectGroup == DNPDEFS_OBJ_30_ANA_INPUTS)  // I don't think this works...something weird with how the over_range flag is set
        // {
        //     TMWSIM_POINT *dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetPoint(pDbHandle, pointNumber);
        //     if (dbPoint != nullptr && dbPoint->flexPointHandle != nullptr)
        //     {
        //         checkInputOverflow(dbPoint);
        //     }
        // }
        // else if (objectGroup == DNPDEFS_OBJ_1_BIN_INPUTS)
        // {
        //     TMWSIM_POINT *dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetPoint(pDbHandle, pointNumber);
        //     if (dbPoint != nullptr && dbPoint->flexPointHandle != nullptr)
        //     {
        //         checkInputOverflow(dbPoint);
        //     }
        // }
    }
};

/**
 * @brief Upon receiving a value via fims, check that the received value is within the dbPoint's
 * numeric limits based on its assigned static variation.
 *
 * If outside those limits, update the value and provide an appropriate error message.
 *
 * @param dbPoint the TMWSIM_POINT * currently being updated
 * @param value the value that was passed over fims that will be stored in dbPoint->data.analog.value
 */
void add_analog_event_callback(void *pDbPoint)
{
    TMWSIM_POINT *dbPoint = (TMWSIM_POINT *)pDbPoint;
    TMWTYPES_ANALOG_VALUE analogValue;
    analogValue.type = TMWTYPES_ANALOG_TYPE_DOUBLE;
    analogValue.value.dval = tmwsim_getAnalogValue(dbPoint);
    check_limits_server(dbPoint, analogValue.value.dval);
    if (((FlexPoint *)dbPoint->flexPointHandle)->event_pub)
    {
        ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.lock_shared();
        sdnpo032_addEvent(((FlexPoint *)dbPoint->flexPointHandle)->sys->protocol_dependencies->dnp3.pSession, dbPoint->pointNumber, &analogValue, dbPoint->flags, &dbPoint->timeStamp);
        ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.unlock_shared();
    }
}

/**
 * @brief Upon receiving a value via fims, check that the received value is within the dbPoint's
 * numeric limits based on its assigned static variation.
 *
 * If outside those limits, update the value and provide an appropriate error message.
 *
 * @param dbPoint the TMWSIM_POINT * currently being updated
 * @param value the value that was passed over fims that will be stored in dbPoint->data.counter.value
 */
void add_counter_event_callback(void *pDbPoint)
{
    TMWSIM_POINT *dbPoint = (TMWSIM_POINT *)pDbPoint;
    TMWTYPES_ULONG counterValue = tmwsim_getCounterValue(dbPoint);
    double tempCounterValue = static_cast<double>(counterValue);
    check_limits_server(dbPoint, tempCounterValue);
    counterValue = static_cast<uint32_t>(tempCounterValue);
    if (((FlexPoint *)dbPoint->flexPointHandle)->event_pub)
    {
        ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.lock_shared();
        sdnpo022_addEvent(((FlexPoint *)dbPoint->flexPointHandle)->sys->protocol_dependencies->dnp3.pSession, dbPoint->pointNumber, counterValue, dbPoint->flags, &dbPoint->timeStamp);
        ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.unlock_shared();
    }
}

/// @brief
/// @param pDbPoint
void add_binary_event_callback(void *pDbPoint)
{
    TMWSIM_POINT *dbPoint = (TMWSIM_POINT *)pDbPoint;

    ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.lock_shared();
    if (dbPoint->data.binary.value)
    {
        if (((FlexPoint *)dbPoint->flexPointHandle)->event_pub)
        {
            ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.lock_shared();
            sdnpo002_addEvent(((FlexPoint *)dbPoint->flexPointHandle)->sys->protocol_dependencies->dnp3.pSession, dbPoint->pointNumber, DNPDEFS_DBAS_FLAG_BINARY_ON | dbPoint->flags, &dbPoint->timeStamp);
            ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.unlock_shared();
        }
    }
    else
    {
        dbPoint->flags &= ~DNPDEFS_DBAS_FLAG_BINARY_ON; // if it's already on, turn it off
        if (((FlexPoint *)dbPoint->flexPointHandle)->event_pub)
        {
            ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.lock_shared();
            sdnpo002_addEvent(((FlexPoint *)dbPoint->flexPointHandle)->sys->protocol_dependencies->dnp3.pSession, dbPoint->pointNumber, dbPoint->flags, &dbPoint->timeStamp);
            ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.unlock_shared();
        }
    }
    ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.unlock_shared();
}
/// @brief
/// @param pDbPoint
void add_interval_analog_event_callback(void *pDbPoint)
{
    add_analog_event_callback(pDbPoint);
    TMWSIM_POINT *dbPoint = (TMWSIM_POINT *)pDbPoint;
    tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer),
                   ((FlexPoint *)(dbPoint->flexPointHandle))->interval_pub_rate,
                   serverSys.protocol_dependencies->dnp3.pChannel,
                   add_interval_analog_event_callback,
                   pDbPoint);
}

void add_interval_counter_event_callback(void *pDbPoint)
{
    add_counter_event_callback(pDbPoint);
    TMWSIM_POINT *dbPoint = (TMWSIM_POINT *)pDbPoint;
    tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer),
                   ((FlexPoint *)(dbPoint->flexPointHandle))->interval_pub_rate,
                   serverSys.protocol_dependencies->dnp3.pChannel,
                   add_interval_counter_event_callback,
                   pDbPoint);
}

/// @brief
/// @param pDbPoint
void add_interval_binary_event_callback(void *pDbPoint)
{
    add_binary_event_callback(pDbPoint);
    TMWSIM_POINT *dbPoint = (TMWSIM_POINT *)pDbPoint;
    tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer),
                   ((FlexPoint *)(dbPoint->flexPointHandle))->interval_pub_rate,
                   serverSys.protocol_dependencies->dnp3.pChannel,
                   add_interval_binary_event_callback,
                   pDbPoint);
}
/// @brief
/// @param pCallbackParam
/// @param pPoint
void analog_input_callback(void *pCallbackParam, void *pPoint)
{
    TMWSIM_POINT *dbPoint = (TMWSIM_POINT *)pPoint;
    if (!tmwtimer_isActive(&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer))
    {
        if (((FlexPoint *)(dbPoint->flexPointHandle))->interval_pubs)
        {
            tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer),
                           ((FlexPoint *)(dbPoint->flexPointHandle))->interval_pub_rate,
                           serverSys.protocol_dependencies->dnp3.pChannel,
                           add_interval_analog_event_callback,
                           pPoint);
        }
        else if (((FlexPoint *)(dbPoint->flexPointHandle))->batch_pubs)
        {
            tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer),
                           ((FlexPoint *)(dbPoint->flexPointHandle))->batch_pub_rate,
                           serverSys.protocol_dependencies->dnp3.pChannel,
                           add_analog_event_callback,
                           pPoint);
        }
        else
        {
            ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.unlock();
            add_analog_event_callback(pPoint);
            ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.lock();
        }
    }
}

void counter_callback(void *pCallbackParam, void *pPoint)
{
    TMWSIM_POINT *dbPoint = (TMWSIM_POINT *)pPoint;
    if (!tmwtimer_isActive(&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer))
    {
        if (((FlexPoint *)(dbPoint->flexPointHandle))->interval_pubs)
        {
            tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer),
                           ((FlexPoint *)(dbPoint->flexPointHandle))->interval_pub_rate,
                           serverSys.protocol_dependencies->dnp3.pChannel,
                           add_interval_counter_event_callback,
                           pPoint);
        }
        else if (((FlexPoint *)(dbPoint->flexPointHandle))->batch_pubs)
        {
            tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer),
                           ((FlexPoint *)(dbPoint->flexPointHandle))->batch_pub_rate,
                           serverSys.protocol_dependencies->dnp3.pChannel,
                           add_counter_event_callback,
                           pPoint);
        }
        else
        {
            ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.unlock();
            add_counter_event_callback(pPoint);
            ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.lock();
        }
    }
}
/// @brief
/// @param pCallbackParam
/// @param pPoint
void binary_input_callback(void *pCallbackParam, void *pPoint)
{
    TMWSIM_POINT *dbPoint = (TMWSIM_POINT *)pPoint;
    if (!tmwtimer_isActive(&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer))
    {
        if (((FlexPoint *)(dbPoint->flexPointHandle))->interval_pubs)
        {
            tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer),
                           ((FlexPoint *)(dbPoint->flexPointHandle))->interval_pub_rate,
                           serverSys.protocol_dependencies->dnp3.pChannel,
                           add_interval_binary_event_callback,
                           pPoint);
        }
        else if (((FlexPoint *)(dbPoint->flexPointHandle))->batch_pubs)
        {
            tmwtimer_start((&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer), // if we're not batching the pub_timer will immediately go off anyway
                           ((FlexPoint *)(dbPoint->flexPointHandle))->batch_pub_rate,
                           serverSys.protocol_dependencies->dnp3.pChannel,
                           add_binary_event_callback,
                           pPoint);
        }
        else
        {
            ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.unlock();
            add_binary_event_callback(pPoint);
            ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.lock();
        }
    }
}
/// @brief
/// @param serverSys
/// @return
bool openTMWServerChannel(GcomSystem &serverSys)
{
    DNP3Dependencies *dnp3_sys = &(serverSys.protocol_dependencies->dnp3);
    if (serverSys.debug)
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
        dnp3_sys->channelConfig.chnlDiagMask = TMWDIAG_ID_ERROR;
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
/// @brief
/// @param serverSys
/// @return
bool openTMWServerSession(GcomSystem &serverSys)
{
    DNP3Dependencies *dnp3_sys = &(serverSys.protocol_dependencies->dnp3);
    sdnpsesn_initConfig(&(dnp3_sys->serverSesnConfig));
    dnp3_sys->serverSesnConfig.linkStatusPeriod = 30000;
    dnp3_sys->serverSesnConfig.source = serverSys.protocol_dependencies->station_address;
    dnp3_sys->serverSesnConfig.destination = serverSys.protocol_dependencies->master_address;
    dnp3_sys->serverSesnConfig.binaryInputMaxEvents = serverSys.protocol_dependencies->dnp3.event_buffer;
    dnp3_sys->serverSesnConfig.analogInputMaxEvents = serverSys.protocol_dependencies->dnp3.event_buffer;
    if (serverSys.debug)
    {
        dnp3_sys->serverSesnConfig.sesnDiagMask = (TMWDIAG_ID_PHYS | TMWDIAG_ID_LINK | TMWDIAG_ID_TPRT | TMWDIAG_ID_APPL |
                                                   TMWDIAG_ID_USER | TMWDIAG_ID_MMI | TMWDIAG_ID_STATIC_DATA |
                                                   TMWDIAG_ID_STATIC_HDRS | TMWDIAG_ID_EVENT_DATA | TMWDIAG_ID_EVENT_HDRS |
                                                   TMWDIAG_ID_CYCLIC_DATA | TMWDIAG_ID_CYCLIC_HDRS | TMWDIAG_ID_SECURITY_DATA |
                                                   TMWDIAG_ID_SECURITY_HDRS | TMWDIAG_ID_TX | TMWDIAG_ID_RX |
                                                   TMWDIAG_ID_TIMESTAMP | TMWDIAG_ID_ERROR | TMWDIAG_ID_TARGET);
    }
    else
    {
        dnp3_sys->serverSesnConfig.sesnDiagMask = (TMWDIAG_ID_TIMESTAMP | TMWDIAG_ID_ERROR);
    }
    if (dnp3_sys->unsolUpdate > 0)
    {
        dnp3_sys->serverSesnConfig.unsolClassMask = TMWDEFS_CLASS_MASK_ALL;
        dnp3_sys->serverSesnConfig.unsolClass1MaxEvents = 1;
        dnp3_sys->serverSesnConfig.unsolClass2MaxEvents = 1;
        dnp3_sys->serverSesnConfig.unsolClass3MaxEvents = 1;
    }
    dnp3_sys->serverSesnConfig.pStatCallback = dnp3stats_sesnStatCallback;
    dnp3_sys->serverSesnConfig.pStatCallbackParam = &serverSys;
    dnp3_sys->pSession = (TMWSESN *)sdnpsesn_openSession(dnp3_sys->pChannel, &(dnp3_sys->serverSesnConfig), (void *)1);
    if (dnp3_sys->pSession == TMWDEFS_NULL)
    {
        return false;
    }
    dnp3_sys->dbHandle = ((SDNPSESN *)(dnp3_sys->pSession))->pDbHandle;
    sdnpsim_setCallback(dnp3_sys->dbHandle, received_command_callback, dnp3_sys->dbHandle);
    return true;
}
/// @brief
/// @param dbPoint
/// @param value
void check_limits_server(TMWSIM_POINT *dbPoint, double &value)
{
    GcomSystem *sys = ((FlexPoint *)(dbPoint->flexPointHandle))->sys;
    if (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::Analog)
    {
        if (dbPoint->defaultStaticVariation == Group30Var1 || dbPoint->defaultStaticVariation == Group30Var3)
        {
            if (value > std::numeric_limits<int32_t>::max())
            {
                value = std::numeric_limits<int32_t>::max();
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Pub to analog input point [%d] (32-bit signed int) exceeded maximum (2,147,483,647). Setting to maximum value instead.", dbPoint->pointNumber);
                }
            }
            else if (value < std::numeric_limits<int32_t>::lowest())
            {
                value = std::numeric_limits<int32_t>::lowest();
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Pub to analog input point [%d] (32-bit signed int) exceeded minimum (-2,147,483,648). Setting to minimum value instead.", dbPoint->pointNumber);
                }
            }
        }
        else if (dbPoint->defaultStaticVariation == Group30Var2 || dbPoint->defaultStaticVariation == Group30Var4)
        {

            if (value > std::numeric_limits<int16_t>::max())
            {
                value = std::numeric_limits<int16_t>::max();
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Pub to analog input point [%d] (16-bit signed int) exceeded maximum (32,767). Setting to maximum value instead.", dbPoint->pointNumber);
                }
            }
            else if (value < std::numeric_limits<int16_t>::lowest())
            {
                value = std::numeric_limits<int16_t>::lowest();
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Pub to analog input point [%d] (16-bit signed int) exceeded minimum (-32,768). Setting to minimum value instead.", dbPoint->pointNumber);
                }
            }
        }
        else if (dbPoint->defaultStaticVariation == Group30Var5)
        {
            if (value > TMWDEFS_SFLOAT_MAX)
            {
                value = TMWDEFS_SFLOAT_MAX;
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Pub to analog input point [%d] (32-bit float) exceeded maximum (%g). Setting to maximum value instead.", dbPoint->pointNumber, TMWDEFS_SFLOAT_MAX);
                }
            }
            else if (value < TMWDEFS_SFLOAT_MIN)
            {
                value = TMWDEFS_SFLOAT_MIN;
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Pub to analog input point [%d] (32-bit float) exceeded minimum (%g). Setting to minimum value instead.", dbPoint->pointNumber, TMWDEFS_SFLOAT_MIN);
                }
            }
            else if (value != 0.0 && abs(value) < TMWDEFS_SFLOAT_SMALLEST)
            {
                value = 0.0;
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Pub to analog input point [%d] (32-bit float) exceeded minimum exponent value (%g). Setting to 0 instead.", dbPoint->pointNumber, TMWDEFS_SFLOAT_SMALLEST);
                }
            }
        }
    }
    else if (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::Counter)
    {
        if (dbPoint->defaultStaticVariation == Group20Var1 || dbPoint->defaultStaticVariation == Group20Var3 ||
            dbPoint->defaultStaticVariation == Group20Var5 || dbPoint->defaultStaticVariation == Group20Var7)
        {
            if (value > std::numeric_limits<uint32_t>::max())
            {
                value = std::numeric_limits<uint32_t>::max();
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Pub to counter [%d] (32-bit unsigned integer) exceeded maximum (4,294,967,295). Setting to maximum value instead.", dbPoint->pointNumber);
                }
            }
            else if (value < 0)
            {
                value = 0;
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Pub to counter [%d] (32-bit unsigned integer) exceeded minimum (0). Setting to minimum value instead.", dbPoint->pointNumber);
                }
            }
        }
        else if (dbPoint->defaultStaticVariation == Group20Var2 || dbPoint->defaultStaticVariation == Group20Var4 ||
                 dbPoint->defaultStaticVariation == Group20Var6 || dbPoint->defaultStaticVariation == Group20Var8)
        {

            if (value > std::numeric_limits<uint16_t>::max())
            {
                value = std::numeric_limits<uint16_t>::max();
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Pub to counter [%d] (16-bit unsigned int) exceeded maximum (32,767). Setting to maximum value instead.", dbPoint->pointNumber);
                }
            }
            else if (value < 0)
            {
                value = 0;
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Pub to counter [%d] (16-bit unsigned int) exceeded minimum (0). Setting to minimum value instead.", dbPoint->pointNumber);
                }
            }
        }
    }
    else
    {
        if (dbPoint->defaultStaticVariation == Group40Var1)
        {
            if (value > std::numeric_limits<int32_t>::max())
            {
                value = std::numeric_limits<int32_t>::max();
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Pub to analog output point [%d] (32-bit signed int) exceeded maximum (2,147,483,647). Setting to maximum value instead.", dbPoint->pointNumber);
                }
            }
            else if (value < std::numeric_limits<int32_t>::lowest())
            {
                value = std::numeric_limits<int32_t>::lowest();
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Pub to analog output point [%d] (32-bit signed int) exceeded minimum (-2,147,483,648). Setting to minimum value instead.", dbPoint->pointNumber);
                }
            }
        }
        else if (dbPoint->defaultStaticVariation == Group40Var2)
        {

            if (value > std::numeric_limits<int16_t>::max())
            {
                value = std::numeric_limits<int16_t>::max();
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Pub to analog output point [%d] (16-bit signed int) exceeded maximum (32,767). Setting to maximum value instead.", dbPoint->pointNumber);
                }
            }
            else if (value < std::numeric_limits<int16_t>::lowest())
            {
                value = std::numeric_limits<int16_t>::lowest();
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Pub to analog output point [%d] (16-bit signed int) exceeded minimum (-32,768). Setting to minimum value instead.", dbPoint->pointNumber);
                }
            }
        }
        else if (dbPoint->defaultStaticVariation == Group40Var3)
        {
            if (value > TMWDEFS_SFLOAT_MAX)
            {
                value = TMWDEFS_SFLOAT_MAX;
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Pub to analog input point [%d] (32-bit float) exceeded maximum (%g). Setting to maximum value instead.", dbPoint->pointNumber, TMWDEFS_SFLOAT_MAX);
                }
            }
            else if (value < TMWDEFS_SFLOAT_MIN)
            {
                value = TMWDEFS_SFLOAT_MIN;
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Pub to analog input point [%d] (32-bit float) exceeded minimum (%g). Setting to minimum value instead.", dbPoint->pointNumber, TMWDEFS_SFLOAT_MIN);
                }
            }
            else if (value != 0.0 && abs(value) < TMWDEFS_SFLOAT_SMALLEST)
            {
                value = 0.0;
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Pub to analog input point [%d] (32-bit float) exceeded minimum exponent value (%g). Setting to 0 instead.", dbPoint->pointNumber, TMWDEFS_SFLOAT_SMALLEST);
                }
            }
        }
    }
}

/**
 * @brief Parse incoming pub messages. For each incoming data point, update the point's
 * status to ONLINE and set the value of the point using tmwsim_set<Point_Type>Value().
 *
 * Upon receiving a fims message, parse the message (single or multi-point, with or without
 * the keyword "value"). For each valid point, update the point status to ONLINE (and
 * restart the timeout timer, if applicable) and update the point value using either
 * tmwsim_setAnalogValue() or tmwsim_setBinaryValue().
 *
 * @param sys GcomSystem for DNP3 Server
 * @param meta_data Meta_Data_Info from incoming fims message
 *
 * @pre the fims message header has been pre-processed such that
 * sys.fims_dependencies->uri_view corresponds to the current message uri.
 * @pre sys.fims_dependencies->data_buf has been properly initialized
 */
bool parseBodyServer(GcomSystem &sys, Meta_Data_Info &meta_data)
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
        if (!spam_limit(&sys, sys.parse_errors))
        {
            FPS_ERROR_LOG("Listener for '%s', from sender: '%s', with base uri '%s': could not parse json string from pub request, err = %s Message dropped", sys.fims_dependencies->name.c_str(), sys.fims_dependencies->name, sys.fims_dependencies->uri_view, simdjson::error_message(err));
            FPS_LOG_IT("parsing_error");
        }
        return false;
    }

    // now set onto channels based on multi or single set uri:
    Jval_buif to_set;
    bool ok = true;
    int uriType = getUriType(sys, sys.fims_dependencies->uri_view);
    if (uriType == 1) // multi-set input uri
    {
        simdjson::ondemand::object set_obj;
        if (const auto err = doc.get(set_obj); err)
        {
            if (!spam_limit(&sys, sys.parse_errors))
            {
                FPS_ERROR_LOG("could not get multi-pub fims message as a json object, err = %s Message dropped", simdjson::error_message(err));
                FPS_LOG_IT("parsing_error");
            }
            return false;
        }

        if (sys.debug)
        {
            FPS_INFO_LOG("Uri: %s", sys.fims_dependencies->uri_view);
        }
        for (auto pair : set_obj)
        {
            const auto key = pair.unescaped_key();
            if (const auto err = key.error(); err)
            {
                if (!spam_limit(&sys, sys.parse_errors))
                {
                    FPS_ERROR_LOG("parsing error on getting a multi-set key, err = %s Message dropped", simdjson::error_message(err));
                    FPS_LOG_IT("parsing_error");
                }
                ok = false;
                continue;
            }
            const auto key_view = key.value_unsafe();
            auto val = pair.value();
            if (const auto err = val.error(); err)
            {
                if (!spam_limit(&sys, sys.parse_errors))
                {
                    FPS_ERROR_LOG("base uri '%s', on multi-set key '%s': parse error on getting value, err = %s Message dropped", sys.fims_dependencies->uri_view, key_view, simdjson::error_message(err));
                    FPS_LOG_IT("parsing_error");
                }
                ok = false;
                continue;
            }

            auto curr_val = val.value_unsafe();
            auto val_clothed = curr_val.get_object();
            auto success = extractValueMulti(sys, val_clothed, curr_val, to_set);
            if (!success)
            {
                ok = false;
                continue;
            }

            TMWSIM_POINT *dbPoint = getDbVar(sys, sys.fims_dependencies->uri_view, key_view);
            if (!dbPoint)
            {
                ok = false;
                if (sys.debug > 0)
                {
                    FPS_ERROR_LOG("with multi-pub uri: '%s', could not find point '%s'", sys.fims_dependencies->uri_view, key_view);
                    FPS_LOG_IT("could_not_find_point");
                }
                continue;
            }

            if (!sys.fims_dependencies->uri_requests.contains_local_uri &&
                (((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::Analog ||
                 ((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::Binary ||
                 ((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::Counter))
            {
                setInputPointOnline(dbPoint);
            }

            double value = jval_to_double(to_set);
            sys.db_mutex.lock();
            if (((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::Analog &&
                (((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0 && sys.fims_dependencies->uri_requests.contains_local_uri) ||
                 ((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) == 0 && !sys.fims_dependencies->uri_requests.contains_local_uri)))
            {
                if (((FlexPoint *)dbPoint->flexPointHandle)->scale != 0)
                {
                    value = ((FlexPoint *)dbPoint->flexPointHandle)->scale * value;
                }
                sdnputil_getDateTime(sys.protocol_dependencies->dnp3.pSession, &dbPoint->timeStamp);
                check_limits_server(dbPoint, value);
                sdnpsim_anlgInWrite(dbPoint, value);
            }
            else if (((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::Counter &&
                     (((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0 && sys.fims_dependencies->uri_requests.contains_local_uri) ||
                      ((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) == 0 && !sys.fims_dependencies->uri_requests.contains_local_uri)))
            {
                if (((FlexPoint *)dbPoint->flexPointHandle)->scale != 0)
                {
                    value = ((FlexPoint *)dbPoint->flexPointHandle)->scale * value;
                }
                sdnputil_getDateTime(sys.protocol_dependencies->dnp3.pSession, &dbPoint->timeStamp);
                check_limits_server(dbPoint, value);
                sdnpsim_binCntrWrite(dbPoint, value);
            }
            else if (((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::Binary &&
                     (((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0 && sys.fims_dependencies->uri_requests.contains_local_uri) ||
                      ((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) == 0 && !sys.fims_dependencies->uri_requests.contains_local_uri)))
            {
                if (((FlexPoint *)dbPoint->flexPointHandle)->scale < 0)
                {
                    value = !value;
                }
                sdnputil_getDateTime(sys.protocol_dependencies->dnp3.pSession, &dbPoint->timeStamp);

                sdnpsim_binInWrite(dbPoint, static_cast<bool>(value));
            }
            else if (((((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::AnOPInt16) ||
                      (((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::AnOPInt32) ||
                      (((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::AnOPF32) ||
                      (((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::AnalogOS)) &&
                     (((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0 && sys.fims_dependencies->uri_requests.contains_local_uri) ||
                      ((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) == 0 && !sys.fims_dependencies->uri_requests.contains_local_uri))

            )
            {
                TMWTYPES_ANALOG_VALUE analogValue;
                analogValue.type = TMWTYPES_ANALOG_TYPE_DOUBLE;
                if (((FlexPoint *)(dbPoint->flexPointHandle))->scale == 0.0)
                {
                    analogValue.value.dval = value;
                }
                else
                {
                    analogValue.value.dval = value * ((FlexPoint *)(dbPoint->flexPointHandle))->scale;
                }
                check_limits_server(dbPoint, analogValue.value.dval);
                ((FlexPoint *)dbPoint->flexPointHandle)->operate_value = analogValue.value.dval;
                ((FlexPoint *)dbPoint->flexPointHandle)->sent_operate = true;
                sdnpsim_anlgOutWrite(dbPoint, &analogValue);
            }
            else if (((((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::BinaryOS) ||
                      (((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::CROB)) &&
                     (((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0 && sys.fims_dependencies->uri_requests.contains_local_uri) ||
                      ((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) == 0 && !sys.fims_dependencies->uri_requests.contains_local_uri))

            )
            {
                bool bool_value = static_cast<bool>(value);
                if (((FlexPoint *)(dbPoint->flexPointHandle))->scale < 0)
                {
                    bool_value = !bool_value;
                }
                ((FlexPoint *)dbPoint->flexPointHandle)->operate_value = static_cast<double>(bool_value);
                ((FlexPoint *)dbPoint->flexPointHandle)->sent_operate = true;
                sdnpsim_binOutSetValue(dbPoint, bool_value);
            }
            else if (((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0 && !sys.fims_dependencies->uri_requests.contains_local_uri))
            {
                ((FlexPoint *)dbPoint->flexPointHandle)->standby_value = value;
            }
            sys.db_mutex.unlock();
        }

        return ok;
    }
    else if (uriType == 2) // single-set input uri
    {
        if (sys.debug)
        {
            FPS_INFO_LOG("Single Uri: %s", sys.fims_dependencies->uri_view);
        }
        simdjson::ondemand::value curr_val;
        auto val_clothed = doc.get_object();

        auto success = extractValueSingle(sys, val_clothed, curr_val, to_set);
        if (!success)
        {
            return false;
        }

        TMWSIM_POINT *dbPoint = getDbVar(sys, sys.fims_dependencies->uri_view, "");
        if (!dbPoint)
        {
            if (sys.debug > 0)
            {
                FPS_ERROR_LOG("with single-pub uri: '%s', could not find point", sys.fims_dependencies->uri_view);
                FPS_LOG_IT("could_not_find_point");
            }
            return false;
        }

        if (!sys.fims_dependencies->uri_requests.contains_local_uri &&
            (((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::Analog ||
             ((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::Binary ||
             ((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::Counter))
        {
            setInputPointOnline(dbPoint);
        }

        double value = jval_to_double(to_set);
        sys.db_mutex.lock();
        if ((((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::Analog) &&
            (((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0 && sys.fims_dependencies->uri_requests.contains_local_uri) ||
             ((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) == 0 && !sys.fims_dependencies->uri_requests.contains_local_uri)))
        {
            if (((FlexPoint *)dbPoint->flexPointHandle)->scale != 0)
            {
                value = ((FlexPoint *)dbPoint->flexPointHandle)->scale * value;
            }
            sdnputil_getDateTime(sys.protocol_dependencies->dnp3.pSession, &dbPoint->timeStamp);
            check_limits_server(dbPoint, value);
            sdnpsim_anlgInWrite(dbPoint, value);
        }
        else if ((((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::Counter) &&
                 (((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0 && sys.fims_dependencies->uri_requests.contains_local_uri) ||
                  ((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) == 0 && !sys.fims_dependencies->uri_requests.contains_local_uri)))
        {
            if (((FlexPoint *)dbPoint->flexPointHandle)->scale != 0)
            {
                value = ((FlexPoint *)dbPoint->flexPointHandle)->scale * value;
            }
            sdnputil_getDateTime(sys.protocol_dependencies->dnp3.pSession, &dbPoint->timeStamp);
            check_limits_server(dbPoint, value);
            sdnpsim_binCntrWrite(dbPoint, value);
        }
        else if ((((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::Binary) && (((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0 && sys.fims_dependencies->uri_requests.contains_local_uri) ||
                                                                                               ((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) == 0 && !sys.fims_dependencies->uri_requests.contains_local_uri)))
        {
            if (((FlexPoint *)dbPoint->flexPointHandle)->scale < 0)
            {
                value = !value;
            }
            sdnputil_getDateTime(sys.protocol_dependencies->dnp3.pSession, &dbPoint->timeStamp);

            sdnpsim_binInWrite(dbPoint, static_cast<bool>(value));
        }
        else if (((((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::AnOPInt16) ||
                  (((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::AnOPInt32) ||
                  (((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::AnOPF32) ||
                  (((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::AnalogOS)) &&
                 (((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0 && sys.fims_dependencies->uri_requests.contains_local_uri) ||
                  ((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) == 0 && !sys.fims_dependencies->uri_requests.contains_local_uri))

        )
        {
            TMWTYPES_ANALOG_VALUE analogValue;
            analogValue.type = TMWTYPES_ANALOG_TYPE_DOUBLE;
            if (((FlexPoint *)(dbPoint->flexPointHandle))->scale == 0.0)
            {
                analogValue.value.dval = value;
            }
            else
            {
                analogValue.value.dval = value * ((FlexPoint *)(dbPoint->flexPointHandle))->scale;
            }
            check_limits_server(dbPoint, analogValue.value.dval);
            ((FlexPoint *)dbPoint->flexPointHandle)->operate_value = analogValue.value.dval;
            ((FlexPoint *)dbPoint->flexPointHandle)->sent_operate = true;
            sdnpsim_anlgOutWrite(dbPoint, &analogValue);
        }
        else if (((((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::BinaryOS) ||
                  (((FlexPoint *)dbPoint->flexPointHandle)->type == Register_Types::CROB)) &&
                 (((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0 && sys.fims_dependencies->uri_requests.contains_local_uri) ||
                  ((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) == 0 && !sys.fims_dependencies->uri_requests.contains_local_uri))

        )
        {
            bool bool_value = static_cast<bool>(value);
            if (((FlexPoint *)(dbPoint->flexPointHandle))->scale < 0)
            {
                bool_value = !bool_value;
            }
            ((FlexPoint *)dbPoint->flexPointHandle)->operate_value = static_cast<double>(bool_value);
            ((FlexPoint *)dbPoint->flexPointHandle)->sent_operate = true;
            sdnpsim_binOutSetValue(dbPoint, bool_value);
        }
        else if (((dbPoint->flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0 && !sys.fims_dependencies->uri_requests.contains_local_uri))
        {
            ((FlexPoint *)dbPoint->flexPointHandle)->standby_value = value;
        }
        sys.db_mutex.unlock();
        return true;
    }
    return false;
}

/**
 * @brief Assign callback functions to analog input, binary input, and counter points upon
 * initialization of the DNP3 server.
 *
 * The relevant callback functions are analog_input_callback, binary_input_callback, and
 * counter_callback, respectively. These functions handle event creation and point status
 * updates when tmwsim_setAnalogValue or tmwsim_setBinaryValue are called.
 *
 * @param dnp3_sys a pointer to a fully initialized DNP3Dependencies struct
 */
void add_input_callbacks(DNP3Dependencies *dnp3_sys)
{
    TMWSIM_POINT *dbPoint;
    for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
    {
        dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetPoint(dnp3_sys->dbHandle, i);
        if (dbPoint)
        {
            dbPoint->pCallbackFunc = analog_input_callback;
            dbPoint->pCallbackParam = dbPoint;
        }
    }
    for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryCounters); i++)
    {
        dbPoint = (TMWSIM_POINT *)sdnpsim_binCntrGetEnabledPoint(dnp3_sys->dbHandle, i);
        if (dbPoint)
        {
            dbPoint->pCallbackFunc = counter_callback;
            dbPoint->pCallbackParam = dbPoint;
        }
    }
    for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
    {
        dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetPoint(dnp3_sys->dbHandle, i);
        if (dbPoint)
        {
            dbPoint->pCallbackFunc = binary_input_callback;
            dbPoint->pCallbackParam = dbPoint;
        }
    }
}

#ifndef DNP3_TEST_MODE

/// @brief
/// @param argc
/// @param argv
/// @return
int main(int argc, char *argv[])
{
    std::string file_name = getFileName(argc, argv);
    signal(SIGTERM, signal_handler_server);
    signal(SIGINT, signal_handler_server);

    Logging::Init(file_name, argc, argv);
    serverSys.git_version_info.init();

    FPS_INFO_LOG("build: %s", serverSys.git_version_info.get_build());
    FPS_INFO_LOG("commit: %s", serverSys.git_version_info.get_commit());
    FPS_INFO_LOG("tag: %s", serverSys.git_version_info.get_tag());

    // assume 1 for right now
    num_configs_server = getConfigs(argc, argv, serverSys, DNP3_OUTSTATION);
    serverSys.config_file_name = file_name;

    if (num_configs_server > 0)
    // ZoneScoped;
    {
        // These are the basic things that need to happen with TMW
        // regardless of any config settings.
        // Later, we will need to modify IOConfig, initConfig for
        // the channel and the session, openChannel, and openSession.
        DNP3Dependencies *dnp3_sys = &(serverSys.protocol_dependencies->dnp3);
        dnp3_sys->openTMWChannel = openTMWServerChannel;
        dnp3_sys->openTMWSession = openTMWServerSession;
        initStatsMonitor(serverSys);
        if (!init_tmw(serverSys))
        {
            shutdown_tmw(dnp3_sys);
            FPS_LOG_IT("tmw_fail_to_initialize");
            return 0;
        }

        serverSys.fims_dependencies->parseBody = parseBodyServer;
        init_fims(serverSys);

        init_vars(serverSys, num_configs_server);
        add_input_callbacks(dnp3_sys); // add analog_input_callbacks to analog input points and binary_input_callbacks to binary input points
        getSysUris(serverSys, DNP3_OUTSTATION, num_configs_server);

        show_fims_subs(serverSys);

        if (!fims_connect(serverSys))
        {
            if (dnp3_sys->point_status_info)
            {
                delete dnp3_sys->point_status_info;
            }
            sdnpsesn_closeSession(dnp3_sys->pSession);
            dnpchnl_closeChannel(dnp3_sys->pChannel);
            tmwappl_closeApplication(dnp3_sys->pApplContext, TMWDEFS_FALSE);
            tmwtimer_close();
            FPS_LOG_IT("fims_fail_to_connect");
            return 0;
        }

        if(serverSys.dbi_save_frequency_seconds > 0){
            load_points_from_dbi_server(serverSys);
        }

        serverSys.listener_future = std::async(std::launch::async, listener_thread, std::ref(serverSys));
        if (dnp3_sys->stats_pub_frequency > 0)
        {
            serverSys.stats_pub_future = std::async(std::launch::async, pub_stats_thread, std::ref(serverSys));
        }
        initTimers(serverSys);
        usleep(500); // wait for things to settle before we start
        serverSys.start_signal = true;
        serverSys.keep_running = true;
        serverSys.main_cond.notify_all();
        outputPointsGoOnline(serverSys); // set all analog and binary outputs to be ONLINE (so they can receive commands)
        if(serverSys.dbi_save_frequency_seconds > 0){
            tmwtimer_start(&serverSys.dbi_save_timer, serverSys.dbi_save_frequency_seconds*1000, serverSys.protocol_dependencies->dnp3.pChannel, write_points_to_dbi_server, &serverSys);
        }
        FPS_INFO_LOG("DNP3 Server Setup complete: Entering main loop.");
        FPS_LOG_IT("startup");
        defer
        {
            serverSys.keep_running = false;
        };

        while (serverSys.keep_running)
        {
            serverSys.db_mutex.lock();
            tmwappl_checkForInput(serverSys.protocol_dependencies->dnp3.pApplContext);
            serverSys.db_mutex.unlock();
            tmwpltmr_checkTimer();
            tmwtarg_sleep(1);
        }
        FPS_INFO_LOG("Cleaning up loose threads before exit.");
        auto done_listening = serverSys.listener_future.get();
        bool done_pubbing = false;
        if (dnp3_sys->stats_pub_frequency > 0)
        {
            done_pubbing = serverSys.stats_pub_future.get();
        }
        else
        {
            done_pubbing = true;
        }
        if (dnp3_sys->point_status_info)
        {
            delete dnp3_sys->point_status_info;
        }
        sdnpsesn_closeSession(dnp3_sys->pSession);
        dnpchnl_closeChannel(dnp3_sys->pChannel);
        tmwappl_closeApplication(dnp3_sys->pApplContext, TMWDEFS_FALSE);
        tmwtimer_close();
        FPS_INFO_LOG("Fims listen thread complete: %s", done_listening ? "true" : "false");
        FPS_INFO_LOG("Pub stats thread complete: %s", done_pubbing ? "true" : "false");
    }
    FPS_LOG_IT("shutdown");
    return 0;
}
#endif
