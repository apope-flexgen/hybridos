extern "C" {
#include "tmwscl/utils/tmwtimer.h"
#include "tmwscl/dnp/mdnpbrm.h"
#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/utils/tmwdefs.h"
}
#include "gcom_dnp3_system_structs.h"
#include "shared_utils.hpp"
#include "gcom_dnp3_utils.h"
#include "logger/gcom_dnp3_logger.h"

/**
 * @brief Upon receiving a value via fims, check that the received value is within the dbPoint's
 * numeric limits based on its assigned static variation.
 *
 * If outside those limits, update the value and provide an appropriate error message.
 *
 * @param dbPoint the TMWSIM_POINT * currently being updated
 * @param value the value that was passed over fims that will be stored in dbPoint->data.analog.value
 */
void check_limits_client(TMWSIM_POINT* dbPoint, double& value)
{
    GcomSystem* sys = ((FlexPoint*)(dbPoint->flexPointHandle))->sys;

    if (dbPoint->defaultStaticVariation == Group40Var1)
    {
        if (value > std::numeric_limits<int32_t>::max())
        {
            value = std::numeric_limits<int32_t>::max();
            if (!spam_limit(sys, sys->point_errors))
            {
                FPS_ERROR_LOG(
                    "Set request to analog output point [%d] (32-bit signed int) exceeded maximum (2,147,483,647). Setting to maximum value instead.",
                    dbPoint->pointNumber);
            }
        }
        else if (value < std::numeric_limits<int32_t>::lowest())
        {
            value = std::numeric_limits<int32_t>::lowest();
            if (!spam_limit(sys, sys->point_errors))
            {
                FPS_ERROR_LOG(
                    "Set request to analog output point [%d] (32-bit signed int) exceeded minimum (-2,147,483,648). Setting to minimum value instead.",
                    dbPoint->pointNumber);
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
                FPS_ERROR_LOG(
                    "Set request to analog output point [%d] (16-bit signed int) exceeded maximum (32,767). Setting to maximum value instead.",
                    dbPoint->pointNumber);
            }
        }
        else if (value < std::numeric_limits<int16_t>::lowest())
        {
            value = std::numeric_limits<int16_t>::lowest();
            if (!spam_limit(sys, sys->point_errors))
            {
                FPS_ERROR_LOG(
                    "Set request to analog output point [%d] (16-bit signed int) exceeded minimum (-32,768). Setting to minimum value instead.",
                    dbPoint->pointNumber);
            }
        }
    }
    else if (dbPoint->defaultStaticVariation == Group40Var3)
    {
        if (value > TMWDEFS_SFLOAT_MAX)
        {
            value = std::numeric_limits<float>::max();
            if (!spam_limit(sys, sys->point_errors))
            {
                FPS_ERROR_LOG(
                    "Set request to analog output point [%d] (32-bit float) exceeded maximum (%g). Setting to maximum value instead.",
                    dbPoint->pointNumber, TMWDEFS_SFLOAT_MAX);
            }
        }
        else if (value < TMWDEFS_SFLOAT_MIN)
        {
            value = TMWDEFS_SFLOAT_MIN;
            if (!spam_limit(sys, sys->point_errors))
            {
                FPS_ERROR_LOG(
                    "Set request to analog output point [%d] (32-bit float) exceeded minimum (%g). Setting to minimum value instead.",
                    dbPoint->pointNumber, TMWDEFS_SFLOAT_MIN);
            }
        }
        else if (value != 0.0 && abs(value) < TMWDEFS_SFLOAT_SMALLEST)
        {
            value = 0.0;
            if (!spam_limit(sys, sys->point_errors))
            {
                FPS_ERROR_LOG(
                    "Set request to analog output point [%d] (32-bit float) exceeded minimum exponent value (%g). Setting to 0 instead.",
                    dbPoint->pointNumber, TMWDEFS_SFLOAT_SMALLEST);
            }
        }
    }
}

/// @brief
/// @param pSetWork
void send_analog_command_callback(void* pSetWork)
{
    SetWork* set_work = (SetWork*)pSetWork;
    TMWSIM_POINT* dbPoint = set_work->dbPoint;
    double value = set_work->value;
    MDNPBRM_ANALOG_INFO analogValue;
    analogValue.pointNumber = dbPoint->pointNumber;

    analogValue.value.type = TMWTYPES_ANALOG_TYPE_DOUBLE;
    if (((FlexPoint*)(dbPoint->flexPointHandle))->scale == 0.0)
    {
        analogValue.value.value.dval = value;
    }
    else
    {
        analogValue.value.value.dval = value * ((FlexPoint*)(dbPoint->flexPointHandle))->scale;
    }

    check_limits_client(dbPoint, analogValue.value.value.dval);

    ((FlexPoint*)(dbPoint->flexPointHandle))->operate_value = analogValue.value.value.dval;
    ((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate = true;
    ((FlexPoint*)(dbPoint->flexPointHandle))->last_operate_time = get_time_double();

    mdnpbrm_analogCommand(
        &(((FlexPoint*)dbPoint->flexPointHandle)->sys)->protocol_dependencies->dnp3.pAnalogCommandRequestDesc,
        TMWDEFS_NULL, DNPDEFS_FC_DIRECT_OP, MDNPBRM_AUTO_MODE_NONE, 0, DNPDEFS_QUAL_16BIT_INDEX,
        dbPoint->defaultStaticVariation, 1, &analogValue);
}

/// @brief
/// @param pSetWork
void send_binary_command_callback(void* pSetWork)
{
    SetWork* set_work = (SetWork*)pSetWork;
    TMWSIM_POINT* dbPoint = set_work->dbPoint;
    double value = set_work->value;
    bool bool_value = static_cast<bool>(value);
    if (((FlexPoint*)(dbPoint->flexPointHandle))->scale < 0)
    {
        bool_value = !bool_value;
    }
    MDNPBRM_CROB_INFO CROBInfo;
    CROBInfo.pointNumber = dbPoint->pointNumber;
    if (((FlexPoint*)(dbPoint->flexPointHandle))->pulse_crob &&
        (((FlexPoint*)dbPoint->flexPointHandle)->sys)->fims_dependencies->uri_requests.is_pulse_request)
    {
        CROBInfo.control = (TMWTYPES_UCHAR)(bool_value ? DNPDEFS_CROB_CTRL_PULSE_ON : DNPDEFS_CROB_CTRL_PULSE_OFF);
        CROBInfo.onTime = ((FlexPoint*)(dbPoint->flexPointHandle))->pulse_on_ms;
        CROBInfo.offTime = ((FlexPoint*)(dbPoint->flexPointHandle))->pulse_off_ms;
        CROBInfo.count = ((FlexPoint*)(dbPoint->flexPointHandle))->pulse_count;
        // invert bool_value because it ends up opposite of what was sent
        ((FlexPoint*)(dbPoint->flexPointHandle))->operate_value = static_cast<double>(!bool_value);
    }
    else
    {
        CROBInfo.control = (TMWTYPES_UCHAR)(bool_value ? DNPDEFS_CROB_CTRL_LATCH_ON : DNPDEFS_CROB_CTRL_LATCH_OFF);
        CROBInfo.onTime = 0;
        CROBInfo.offTime = 0;
        CROBInfo.count = 0;
        ((FlexPoint*)(dbPoint->flexPointHandle))->operate_value = static_cast<double>(bool_value);
    }
    ((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate = true;
    ((FlexPoint*)(dbPoint->flexPointHandle))->last_operate_time = get_time_double();

    mdnpbrm_binaryCommand(
        &(((FlexPoint*)dbPoint->flexPointHandle)->sys)->protocol_dependencies->dnp3.pBinaryCommandRequestDesc,
        TMWDEFS_NULL, DNPDEFS_FC_DIRECT_OP, MDNPBRM_AUTO_MODE_NONE, 0, DNPDEFS_QUAL_16BIT_INDEX, 1, &CROBInfo);
}

/// @brief
/// @param pSetWork
void send_interval_analog_command_callback(void* pSetWork)
{
    send_analog_command_callback(pSetWork);
    TMWSIM_POINT* dbPoint = ((SetWork*)pSetWork)->dbPoint;
    tmwtimer_start((&((FlexPoint*)(dbPoint->flexPointHandle))->set_timer),
                   ((FlexPoint*)(dbPoint->flexPointHandle))->interval_set_rate,
                   (((FlexPoint*)dbPoint->flexPointHandle)->sys)->protocol_dependencies->dnp3.pChannel,
                   send_interval_analog_command_callback, pSetWork);
}

/// @brief
/// @param pSetWork
void send_interval_binary_command_callback(void* pSetWork)
{
    send_binary_command_callback(pSetWork);
    TMWSIM_POINT* dbPoint = ((SetWork*)pSetWork)->dbPoint;
    tmwtimer_start((&((FlexPoint*)(dbPoint->flexPointHandle))->set_timer),
                   ((FlexPoint*)(dbPoint->flexPointHandle))->interval_set_rate,
                   (((FlexPoint*)dbPoint->flexPointHandle)->sys)->protocol_dependencies->dnp3.pChannel,
                   send_interval_binary_command_callback, pSetWork);
}
/// @brief  handle the batching of set points
/// @param dbPoint
/// @param value
void handle_batch_sets(TMWSIM_POINT* dbPoint, double value)
{
    GcomSystem* sys = (((FlexPoint*)dbPoint->flexPointHandle)->sys);
    if (dbPoint->type == TMWSIM_TYPE_ANALOG && ((FlexPoint*)(dbPoint->flexPointHandle))->is_output_point &&
        ((((FlexPoint*)(dbPoint->flexPointHandle))->is_forced &&
          sys->fims_dependencies->uri_requests.contains_local_uri) ||
         (!((FlexPoint*)(dbPoint->flexPointHandle))->is_forced &&
          !sys->fims_dependencies->uri_requests.contains_local_uri)))
    {
        ((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value = value;

        if (!tmwtimer_isActive(&((FlexPoint*)(dbPoint->flexPointHandle))->set_timer))
        {
            if (((FlexPoint*)(dbPoint->flexPointHandle))->interval_sets)
            {
                tmwtimer_start((&((FlexPoint*)(dbPoint->flexPointHandle))->set_timer),
                               ((FlexPoint*)(dbPoint->flexPointHandle))->interval_set_rate,
                               (((FlexPoint*)dbPoint->flexPointHandle)->sys)->protocol_dependencies->dnp3.pChannel,
                               send_interval_analog_command_callback,
                               (void*)(&((FlexPoint*)(dbPoint->flexPointHandle))->set_work));
            }
            else if (((FlexPoint*)(dbPoint->flexPointHandle))->batch_sets)
            {
                tmwtimer_start((&((FlexPoint*)(dbPoint->flexPointHandle))->set_timer),
                               ((FlexPoint*)(dbPoint->flexPointHandle))->batch_set_rate,
                               (((FlexPoint*)dbPoint->flexPointHandle)->sys)->protocol_dependencies->dnp3.pChannel,
                               send_analog_command_callback,
                               (void*)(&((FlexPoint*)(dbPoint->flexPointHandle))->set_work));
            }
            else
            {
                send_analog_command_callback((void*)(&((FlexPoint*)(dbPoint->flexPointHandle))->set_work));
            }
        }
    }
    else if (dbPoint->type == TMWSIM_TYPE_BINARY && ((FlexPoint*)(dbPoint->flexPointHandle))->is_output_point &&
             ((((FlexPoint*)(dbPoint->flexPointHandle))->is_forced &&
               sys->fims_dependencies->uri_requests.contains_local_uri) ||
              (!((FlexPoint*)(dbPoint->flexPointHandle))->is_forced &&
               !sys->fims_dependencies->uri_requests.contains_local_uri)))
    {
        ((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value = value;
        if (!tmwtimer_isActive(&((FlexPoint*)(dbPoint->flexPointHandle))->set_timer))
        {
            if (((FlexPoint*)(dbPoint->flexPointHandle))->interval_sets)
            {
                tmwtimer_start((&((FlexPoint*)(dbPoint->flexPointHandle))->set_timer),
                               ((FlexPoint*)(dbPoint->flexPointHandle))->interval_set_rate,
                               (((FlexPoint*)dbPoint->flexPointHandle)->sys)->protocol_dependencies->dnp3.pChannel,
                               send_interval_binary_command_callback,
                               (void*)(&((FlexPoint*)(dbPoint->flexPointHandle))->set_work));
            }
            else if (((FlexPoint*)(dbPoint->flexPointHandle))->batch_sets)
            {
                tmwtimer_start((&((FlexPoint*)(dbPoint->flexPointHandle))->set_timer),
                               ((FlexPoint*)(dbPoint->flexPointHandle))->batch_set_rate,
                               (((FlexPoint*)dbPoint->flexPointHandle)->sys)->protocol_dependencies->dnp3.pChannel,
                               send_binary_command_callback,
                               (void*)(&((FlexPoint*)(dbPoint->flexPointHandle))->set_work));
            }
            else
            {
                send_binary_command_callback((void*)(&((FlexPoint*)(dbPoint->flexPointHandle))->set_work));
            }
        }
    }
    else if (((((FlexPoint*)(dbPoint->flexPointHandle))->is_forced &&
               sys->fims_dependencies->uri_requests.contains_local_uri)))
    {
        if (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::Analog ||
            ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::Counter)
        {
            if (((FlexPoint*)(dbPoint->flexPointHandle))->scale == 0.0)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->standby_value = value;
            }
            else
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))
                    ->standby_value = value * ((FlexPoint*)(dbPoint->flexPointHandle))->scale;
            }
        }
        else if (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::Binary)
        {
            bool bool_value = static_cast<bool>(value);
            if (((FlexPoint*)(dbPoint->flexPointHandle))->scale < 0)
            {
                bool_value = !bool_value;
            }
            ((FlexPoint*)(dbPoint->flexPointHandle))->standby_value = static_cast<double>(bool_value);
        }
    }
    else if (((FlexPoint*)(dbPoint->flexPointHandle))->is_output_point &&
             ((((FlexPoint*)(dbPoint->flexPointHandle))->is_forced &&
               !sys->fims_dependencies->uri_requests.contains_local_uri)))
    {
        ((FlexPoint*)(dbPoint->flexPointHandle))->standby_value = value;
        ((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate_before_or_during_force = true;
    }
}