#include "gcom_dnp3_flags.h"
extern "C"
{
#include "tmwscl/dnp/sdnpo002.h"
#include "tmwscl/dnp/sdnpo022.h"
#include "tmwscl/dnp/sdnpo032.h"
#include "tmwscl/dnp/sdnpsim.h"
#include "tmwscl/dnp/mdnpsim.h"
}

#include <fims/defer.hpp>

#include "gcom_dnp3_system_structs.h"
#include "gcom_dnp3_utils.h"
#include "logger/gcom_dnp3_logger.h"
/// @brief
/// @param sys
void outputPointsGoOnline(GcomSystem &sys)
{
    if (sys.protocol_dependencies->who == DNP3_MASTER)
    {
        return;
    }
    TMWSIM_POINT *dbPoint;
    DNP3Dependencies *dnp3_sys = &(sys.protocol_dependencies->dnp3);
    for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs); i++)
    {
        dbPoint = (TMWSIM_POINT *)sdnpsim_anlgOutGetEnabledPoint(dnp3_sys->dbHandle, i);
        if (dbPoint)
        {
            sys.db_mutex.lock();
            dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
            sys.db_mutex.unlock();
            sys.protocol_dependencies->dnp3.point_status_info->point_status_mutex.lock();
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart--;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_online++;
            sys.protocol_dependencies->dnp3.point_status_info->point_status_mutex.unlock();
        }
    }
    for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
    {
        dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetEnabledPoint(dnp3_sys->dbHandle, i);
        if (dbPoint)
        {
            sys.db_mutex.lock();
            dbPoint->flags = DNPDEFS_DBAS_FLAG_ON_LINE;
            sys.db_mutex.unlock();
            sys.protocol_dependencies->dnp3.point_status_info->point_status_mutex.lock();
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_restart--;
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_online++;
            sys.protocol_dependencies->dnp3.point_status_info->point_status_mutex.unlock();
        }
    }
}
/// @brief
/// @param pPointTimeoutStruct
void pointTimeout(void *pPointTimeoutStruct)
{
    TMWSIM_POINT *dbPoint = ((PointTimeoutStruct *)pPointTimeoutStruct)->dbPoint;
    DNP3Dependencies *dnp3_sys = ((PointTimeoutStruct *)pPointTimeoutStruct)->dnp3_sys;
    GcomSystem *sys = ((FlexPoint *)(dbPoint->flexPointHandle))->sys;

    ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.lock_shared();
    if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_RESTART) == 0)
    {
        ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.unlock_shared();
        ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.lock();
        dbPoint->flags |= DNPDEFS_DBAS_FLAG_COMM_LOST;
        dbPoint->flags &= ~DNPDEFS_DBAS_FLAG_ON_LINE;
        ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.unlock();
        if (dbPoint->type == TMWSIM_TYPE_ANALOG)
        {
            TMWTYPES_ANALOG_VALUE analogValue;
            analogValue.type = TMWTYPES_ANALOG_TYPE_DOUBLE;
            ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.lock_shared();
            analogValue.value.dval = tmwsim_getAnalogValue(dbPoint);
            ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.unlock_shared();
            TMWDTIME timeStamp;
            sdnputil_getDateTime(((TMWSESN *)(dbPoint->pSCLHandle)), &timeStamp);
            if (((FlexPoint *)dbPoint->flexPointHandle)->event_pub)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.lock_shared();
                sdnpo032_addEvent(((TMWSESN *)(dbPoint->pSCLHandle)), dbPoint->pointNumber, &analogValue, dbPoint->flags, &timeStamp);
                ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.unlock_shared();
            }
            if (!spam_limit(sys, sys->comms_errors))
            {
                FPS_ERROR_LOG("Analog input point [%d] status is [COMM_LOST]", dbPoint->pointNumber);
                FPS_LOG_IT("comm_lost");
            }
            dnp3_sys->point_status_info->point_status_mutex.lock();
            dnp3_sys->point_status_info->num_analog_inputs_online--;
            dnp3_sys->point_status_info->num_analog_inputs_comm_lost++;
            dnp3_sys->point_status_info->point_status_mutex.unlock();
        }
        else if (dbPoint->type == TMWSIM_TYPE_COUNTER)
        {
            TMWTYPES_ULONG counterValue;
            ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.lock_shared();
            counterValue = tmwsim_getCounterValue(dbPoint);
            ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.unlock_shared();
            TMWDTIME timeStamp;
            sdnputil_getDateTime(((TMWSESN *)(dbPoint->pSCLHandle)), &timeStamp);
            if (((FlexPoint *)dbPoint->flexPointHandle)->event_pub)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.lock_shared();
                sdnpo022_addEvent(((TMWSESN *)(dbPoint->pSCLHandle)), dbPoint->pointNumber, counterValue, dbPoint->flags, &timeStamp);
                ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.unlock_shared();
            }
            if (!spam_limit(sys, sys->comms_errors))
            {
                FPS_ERROR_LOG("Counter [%d] status is [COMM_LOST]", dbPoint->pointNumber);
                FPS_LOG_IT("comm_lost");
            }
            dnp3_sys->point_status_info->point_status_mutex.lock();
            dnp3_sys->point_status_info->num_counters_online--;
            dnp3_sys->point_status_info->num_counters_comm_lost++;
            dnp3_sys->point_status_info->point_status_mutex.unlock();
        }
        else
        {
            TMWDTIME timeStamp;
            sdnputil_getDateTime(((TMWSESN *)(dbPoint->pSCLHandle)), &timeStamp);
            if (((FlexPoint *)dbPoint->flexPointHandle)->event_pub)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.lock_shared();
                sdnpo002_addEvent(((TMWSESN *)(dbPoint->pSCLHandle)), dbPoint->pointNumber, dbPoint->flags, &timeStamp);
                ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.unlock_shared();
            }
            // TODO we may need to reset the count with a _request
            if (!spam_limit(sys, sys->comms_errors))
            {
                FPS_ERROR_LOG("Binary input point [%d] status is [COMM_LOST]", dbPoint->pointNumber);
                FPS_LOG_IT("comm_lost");
            }
            dnp3_sys->point_status_info->point_status_mutex.lock();
            dnp3_sys->point_status_info->num_binary_inputs_online--;
            dnp3_sys->point_status_info->num_binary_inputs_comm_lost++;
            dnp3_sys->point_status_info->point_status_mutex.unlock();
        }
    }
    else
    {
        ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.unlock_shared();
        tmwtimer_start(&((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer, ((FlexPoint *)dbPoint->flexPointHandle)->timeout, ((TMWCHNL *)(((TMWSESN *)(dbPoint->pSCLHandle))->pChannel)), pointTimeout, pPointTimeoutStruct);
    }
}

void initTimers(GcomSystem &sys)
{
    if (sys.protocol_dependencies->who == DNP3_MASTER)
    {
        return;
    }
    DNP3Dependencies *dnp3_sys = &(sys.protocol_dependencies->dnp3);
    TMWSIM_POINT *dbPoint;

    for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
    {
        dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetEnabledPoint(dnp3_sys->dbHandle, i);

        if (dbPoint && (dbPoint->flexPointHandle != nullptr) && ((FlexPoint *)dbPoint->flexPointHandle)->timeout > 0)
        {
            PointTimeoutStruct *point_timeout_struct = new PointTimeoutStruct();
            point_timeout_struct->dbPoint = dbPoint;
            point_timeout_struct->dnp3_sys = dnp3_sys;
            tmwtimer_start(&((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer, ((FlexPoint *)dbPoint->flexPointHandle)->timeout, dnp3_sys->pChannel, pointTimeout, point_timeout_struct);
        }
    }
    for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryCounters); i++)
    {
        dbPoint = (TMWSIM_POINT *)sdnpsim_binCntrGetEnabledPoint(dnp3_sys->dbHandle, i);

        if (dbPoint && (dbPoint->flexPointHandle != nullptr) && ((FlexPoint *)dbPoint->flexPointHandle)->timeout > 0)
        {
            PointTimeoutStruct *point_timeout_struct = new PointTimeoutStruct();
            point_timeout_struct->dbPoint = dbPoint;
            point_timeout_struct->dnp3_sys = dnp3_sys;
            tmwtimer_start(&((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer, ((FlexPoint *)dbPoint->flexPointHandle)->timeout, dnp3_sys->pChannel, pointTimeout, point_timeout_struct);
        }
    }
    for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
    {
        dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetEnabledPoint(dnp3_sys->dbHandle, i);

        if (dbPoint && (dbPoint->flexPointHandle != nullptr) && ((FlexPoint *)dbPoint->flexPointHandle)->timeout > 0)
        {
            PointTimeoutStruct *point_timeout_struct = new PointTimeoutStruct();
            point_timeout_struct->dbPoint = dbPoint;
            point_timeout_struct->dnp3_sys = dnp3_sys;
            tmwtimer_start(&((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer, ((FlexPoint *)dbPoint->flexPointHandle)->timeout, dnp3_sys->pChannel, pointTimeout, point_timeout_struct);
        }
    }
};

void setInputPointOnline(TMWSIM_POINT *dbPoint)
{
    GcomSystem *sys = ((FlexPoint *)dbPoint->flexPointHandle)->sys;
    DNP3Dependencies *dnp3_sys = &(sys->protocol_dependencies->dnp3);
    ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.lock();
    if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_COMM_LOST) != 0)
    {
        if (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::Analog)
        {
            dnp3_sys->point_status_info->point_status_mutex.lock();
            dnp3_sys->point_status_info->num_analog_inputs_comm_lost--;
            dnp3_sys->point_status_info->num_analog_inputs_online++;
            dnp3_sys->point_status_info->point_status_mutex.unlock();
            if (!spam_limit(sys, sys->comms_errors))
            {
                FPS_INFO_LOG("Analog input point [%d] status is [ONLINE]", dbPoint->pointNumber);
            }
        }
        else if (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::Counter)
        {
            dnp3_sys->point_status_info->point_status_mutex.lock();
            dnp3_sys->point_status_info->num_counters_comm_lost--;
            dnp3_sys->point_status_info->num_counters_online++;
            dnp3_sys->point_status_info->point_status_mutex.unlock();
            if (!spam_limit(sys, sys->comms_errors))
            {
                FPS_INFO_LOG("Counter [%d] status is [ONLINE]", dbPoint->pointNumber);
            }
        }
        else
        {
            dnp3_sys->point_status_info->point_status_mutex.lock();
            dnp3_sys->point_status_info->num_binary_inputs_comm_lost--;
            dnp3_sys->point_status_info->num_binary_inputs_online++;
            dnp3_sys->point_status_info->point_status_mutex.unlock();
            if (!spam_limit(sys, sys->comms_errors))
            {
                FPS_INFO_LOG("Binary input point [%d] status is [ONLINE]", dbPoint->pointNumber);
            }
        }
    }
    else if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_RESTART) != 0)
    {
        if (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::Analog)
        {
            dnp3_sys->point_status_info->point_status_mutex.lock();
            dnp3_sys->point_status_info->num_analog_inputs_restart--;
            dnp3_sys->point_status_info->num_analog_inputs_online++;
            dnp3_sys->point_status_info->point_status_mutex.unlock();
        }
        else if (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::Counter)
        {
            dnp3_sys->point_status_info->point_status_mutex.lock();
            dnp3_sys->point_status_info->num_counters_restart--;
            dnp3_sys->point_status_info->num_counters_online++;
            dnp3_sys->point_status_info->point_status_mutex.unlock();
        }
        else
        {
            dnp3_sys->point_status_info->point_status_mutex.lock();
            dnp3_sys->point_status_info->num_binary_inputs_restart--;
            dnp3_sys->point_status_info->num_binary_inputs_online++;
            dnp3_sys->point_status_info->point_status_mutex.unlock();
        }
    }
    dbPoint->flags &= ~DNPDEFS_DBAS_FLAG_RESTART;
    dbPoint->flags &= ~DNPDEFS_DBAS_FLAG_COMM_LOST;
    dbPoint->flags |= DNPDEFS_DBAS_FLAG_ON_LINE;
    ((FlexPoint *)(dbPoint->flexPointHandle))->sys->db_mutex.unlock();

    if (((FlexPoint *)dbPoint->flexPointHandle)->timeout > 0)
    {
        if (((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer.pCallbackParam)
        { // we should really always get into this part of the conditional, but the else statement is there just in case...
            tmwtimer_start(&((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer, ((FlexPoint *)dbPoint->flexPointHandle)->timeout, ((TMWCHNL *)(((TMWSESN *)(dbPoint->pSCLHandle))->pChannel)), pointTimeout, ((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer.pCallbackParam);
        }
        else
        {
            PointTimeoutStruct *point_timeout_struct = new PointTimeoutStruct();
            point_timeout_struct->dbPoint = dbPoint;
            point_timeout_struct->dnp3_sys = dnp3_sys;
            tmwtimer_start(&((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer, ((FlexPoint *)dbPoint->flexPointHandle)->timeout, ((TMWCHNL *)(((TMWSESN *)(dbPoint->pSCLHandle))->pChannel)), pointTimeout, point_timeout_struct);
        }
    }
}
/// @brief
/// @param dbPoint
void checkPointCommLost(TMWSIM_POINT *dbPoint)
{
    GcomSystem *sys = ((FlexPoint *)dbPoint->flexPointHandle)->sys;
    if (dbPoint->flags != ((FlexPoint *)dbPoint->flexPointHandle)->lastFlags)
    {
        if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_COMM_LOST) != 0)
        {
            if (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::Analog)
            {
                if (!spam_limit(sys, sys->comms_errors))
                {
                    FPS_ERROR_LOG("Analog input point [%d] status is [COMM_LOST]", dbPoint->pointNumber);
                    FPS_LOG_IT("comm_lost");
                }
            }
            else if (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::Counter)
            {
                if (!spam_limit(sys, sys->comms_errors))
                {
                    FPS_ERROR_LOG("Counter [%d] status is [COMM_LOST]", dbPoint->pointNumber);
                    FPS_LOG_IT("comm_lost");
                }
            }
            else
            {
                if (!spam_limit(sys, sys->comms_errors))
                {
                    FPS_ERROR_LOG("Binary input point [%d] status is [COMM_LOST]", dbPoint->pointNumber);
                    FPS_LOG_IT("comm_lost");
                }
            }
        }
        else if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_ON_LINE) != 0 && (((FlexPoint *)dbPoint->flexPointHandle)->lastFlags & DNPDEFS_DBAS_FLAG_COMM_LOST) != 0)
        {
            if (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::Analog)
            {
                if (!spam_limit(sys, sys->comms_errors))
                {
                    FPS_INFO_LOG("Analog input point [%d] status is [ONLINE]", dbPoint->pointNumber);
                }
            }
            else if (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::Counter)
            {
                if (!spam_limit(sys, sys->comms_errors))
                {
                    FPS_INFO_LOG("Counter [%d] status is [ONLINE]", dbPoint->pointNumber);
                }
            }
            else
            {
                if (!spam_limit(sys, sys->comms_errors))
                {
                    FPS_INFO_LOG("Binary input point [%d] status is [ONLINE]", dbPoint->pointNumber);
                }
            }
        }
        if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_OVER_RANGE) != 0)
        {
            if (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::Analog)
            {
                if (!spam_limit(sys, sys->comms_errors))
                {
                    FPS_ERROR_LOG("Analog input point [%d] status is [OVER_RANGE]", dbPoint->pointNumber);
                    FPS_LOG_IT("overflow");
                }
            }
            else if (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::Counter)
            {
                if (!spam_limit(sys, sys->comms_errors))
                {
                    FPS_ERROR_LOG("Counter [%d] status is [ROLLOVER]", dbPoint->pointNumber);
                    FPS_LOG_IT("overflow");
                }
            }
        }
        else if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_OVER_RANGE) == 0 && (((FlexPoint *)dbPoint->flexPointHandle)->lastFlags & DNPDEFS_DBAS_FLAG_OVER_RANGE) != 0)
        {
            if (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::Analog)
            {
                if (!spam_limit(sys, sys->comms_errors))
                {
                    FPS_INFO_LOG("Analog input point [%d] is back in range.", dbPoint->pointNumber);
                }
            }
            else if (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::Counter)
            {
                if (!spam_limit(sys, sys->comms_errors))
                {
                    FPS_INFO_LOG("Counter [%d] is back in range.", dbPoint->pointNumber);
                }
            }
        }
    }
    ((FlexPoint *)dbPoint->flexPointHandle)->lastFlags = dbPoint->flags;
}

void updatePointStatus(GcomSystem &sys)
{
    TMWSIM_POINT *dbPoint = nullptr;
    DNP3Dependencies *dnp3_sys = &sys.protocol_dependencies->dnp3;

    sys.protocol_dependencies->dnp3.point_status_info->point_status_mutex.lock();
    defer
    {
        sys.protocol_dependencies->dnp3.point_status_info->point_status_mutex.unlock();
    };
    sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_online = 0;
    sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_online = 0;
    sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_online = 0;
    sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_online = 0;
    sys.protocol_dependencies->dnp3.point_status_info->num_counters_online = 0;

    sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart = 0;
    sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart = 0;
    sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart = 0;
    sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_restart = 0;
    sys.protocol_dependencies->dnp3.point_status_info->num_counters_restart = 0;

    sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_comm_lost = 0;
    sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_comm_lost = 0;
    sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_comm_lost = 0;
    sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_comm_lost = 0;
    sys.protocol_dependencies->dnp3.point_status_info->num_counters_comm_lost = 0;

    dbPoint = tmwsim_tableGetFirstPoint(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs);
    for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs); i++)
    {
        if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
        {
            sys.db_mutex.lock_shared();
            if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_ON_LINE) != 0)
            {
                sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_online++;
            }
            else if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_RESTART) != 0)
            {
                sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart++;
            }
            else if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_COMM_LOST) != 0)
            {
                sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_comm_lost++;
            }
            sys.db_mutex.unlock_shared();
        }
        dbPoint = tmwsim_tableGetNextPoint(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs, dbPoint);
    }

    dbPoint = tmwsim_tableGetFirstPoint(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs);
    for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
    {
        if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
        {
            sys.db_mutex.lock_shared();
            if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_ON_LINE) != 0)
            {
                sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_online++;
            }
            else if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_RESTART) != 0)
            {
                sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_restart++;
            }
            else if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_COMM_LOST) != 0)
            {
                sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_comm_lost++;
            }
            sys.db_mutex.unlock_shared();
        }
        dbPoint = tmwsim_tableGetNextPoint(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs, dbPoint);
    }

    dbPoint = tmwsim_tableGetFirstPoint(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs);
    for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
    {
        if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
        {
            sys.db_mutex.lock_shared();
            if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_ON_LINE) != 0)
            {
                sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_online++;
            }
            else if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_RESTART) != 0)
            {
                sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart++;
            }
            else if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_COMM_LOST) != 0)
            {
                sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_comm_lost++;
            }
            sys.db_mutex.unlock_shared();
        }
        dbPoint = tmwsim_tableGetNextPoint(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs, dbPoint);
    }

    dbPoint = tmwsim_tableGetFirstPoint(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs);
    for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
    {
        if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
        {
            sys.db_mutex.lock_shared();
            if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_ON_LINE) != 0)
            {
                sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_online++;
            }
            else if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_RESTART) != 0)
            {
                sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart++;
            }
            else if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_COMM_LOST) != 0)
            {
                sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_comm_lost++;
            }
            sys.db_mutex.unlock_shared();
        }
        dbPoint = tmwsim_tableGetNextPoint(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs, dbPoint);
    }

    dbPoint = tmwsim_tableGetFirstPoint(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryCounters);
    for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryCounters); i++)
    {
        if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
        {
            sys.db_mutex.lock_shared();
            if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_ON_LINE) != 0)
            {
                sys.protocol_dependencies->dnp3.point_status_info->num_counters_online++;
            }
            else if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_RESTART) != 0)
            {
                sys.protocol_dependencies->dnp3.point_status_info->num_counters_restart++;
            }
            else if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_COMM_LOST) != 0)
            {
                sys.protocol_dependencies->dnp3.point_status_info->num_counters_comm_lost++;
            }
            sys.db_mutex.unlock_shared();
        }
        dbPoint = tmwsim_tableGetNextPoint(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryCounters, dbPoint);
    }
}

/// @brief
/// @param dbPoint
void checkInputOverflow(TMWSIM_POINT *dbPoint)
{
    GcomSystem *sys = ((FlexPoint *)dbPoint->flexPointHandle)->sys;
    if (dbPoint->flags != ((FlexPoint *)dbPoint->flexPointHandle)->lastFlags)
    {
        if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_OVER_RANGE) != 0 && (((FlexPoint *)dbPoint->flexPointHandle)->lastFlags & DNPDEFS_DBAS_FLAG_OVER_RANGE) == 0)
        {
            if (dbPoint->type == TMWSIM_TYPE_ANALOG)
            {
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Analog input point [%d] status is [OVER_RANGE].", dbPoint->pointNumber);
                    FPS_LOG_IT("overflow");
                }
            }
            else if (dbPoint->type == TMWSIM_TYPE_BINARY)
            {
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Binary input point [%d] status is [CHATTER_FILTER].", dbPoint->pointNumber);
                    FPS_LOG_IT("chatter");
                }
            }
        }
        else if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_OVER_RANGE) == 0 && (((FlexPoint *)dbPoint->flexPointHandle)->lastFlags & DNPDEFS_DBAS_FLAG_OVER_RANGE) != 0)
        {
            if (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::Analog)
            {
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_INFO_LOG("Analog input point [%d] is back in range.", dbPoint->pointNumber);
                }
            }
            else if (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::Counter)
            {
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_INFO_LOG("Counter [%d] is back in range.", dbPoint->pointNumber);
                }
            }
            else if (dbPoint->type == TMWSIM_TYPE_BINARY)
            {
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Binary input point [%d] is no longer chattering.", dbPoint->pointNumber);
                }
            }
        }
    }

    ((FlexPoint *)dbPoint->flexPointHandle)->lastFlags = dbPoint->flags;
}
/// @brief
/// @param dbPoint
void checkOutputOverflow(TMWSIM_POINT *dbPoint)
{
    GcomSystem *sys = ((FlexPoint *)dbPoint->flexPointHandle)->sys;
    if (dbPoint->flags != ((FlexPoint *)dbPoint->flexPointHandle)->lastFlags)
    {
        if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_OVER_RANGE) != 0 && (((FlexPoint *)dbPoint->flexPointHandle)->lastFlags & DNPDEFS_DBAS_FLAG_OVER_RANGE) == 0)
        {
            if (dbPoint->type == TMWSIM_TYPE_ANALOG)
            {
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Analog output point [%d] status is [OVER_RANGE].", dbPoint->pointNumber);
                    FPS_LOG_IT("overflow");
                }
            }
            else if (dbPoint->type == TMWSIM_TYPE_BINARY)
            {
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Binary output point [%d] status is [CHATTER_FILTER].", dbPoint->pointNumber);
                    FPS_LOG_IT("chatter");
                }
            }
        }
        else if ((dbPoint->flags & DNPDEFS_DBAS_FLAG_OVER_RANGE) == 0 && (((FlexPoint *)dbPoint->flexPointHandle)->lastFlags & DNPDEFS_DBAS_FLAG_OVER_RANGE) != 0)
        {
            if (dbPoint->type == TMWSIM_TYPE_ANALOG)
            {
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_INFO_LOG("Analog output point [%d] is back in range.", dbPoint->pointNumber);
                }
            }
            else if (dbPoint->type == TMWSIM_TYPE_BINARY)
            {
                if (!spam_limit(sys, sys->point_errors))
                {
                    FPS_ERROR_LOG("Binary output point [%d] is no longer chattering.", dbPoint->pointNumber);
                }
            }
        }
    }
    ((FlexPoint *)dbPoint->flexPointHandle)->lastFlags = dbPoint->flags;
}
