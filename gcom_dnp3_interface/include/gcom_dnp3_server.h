#pragma once
#include <cstdint>

extern "C"
{
#include "tmwscl/utils/tmwtypes.h"
#include "tmwscl/utils/tmwsim.h"
#include "tmwscl/dnp/dnpdefs.h"
}

struct GcomSystem;
struct DNP3Dependencies;
struct Meta_Data_Info;

/**
 * @brief Prepare and send a fims set message for an analog or binary output point.
 *
 * This function is called from (or set as a timer callback) from received_command_callback
 * after a DNP3 command message is received.
 *
 * @param pSetWork void * pointer to the SetWork for a particular point
 */
void fimsSendSetCallback(void *pSetWork);

/**
 * @brief Call fimsSendSetCallback and then restart the interval timer for the point to call
 * this function again after the interval period.
 *
 * This function is called from (or set as a timer callback) from received_command_callback
 * after a DNP3 command message is received.
 *
 * @param pSetWork void * pointer to the SetWork for a particular point
 */
void fimsSendIntervalSetCallback(void *pSetWork);

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
void received_command_callback(void *pDbHandle, TMWSIM_EVENT_TYPE type, DNPDEFS_OBJ_GROUP_ID objectGroup, TMWTYPES_UINT pointNumber);

void add_analog_event_callback(void *pDbPoint);
void add_binary_event_callback(void *pDbPoint);
void add_interval_analog_event_callback(void *pDbPoint);
void add_interval_binary_event_callback(void *pDbPoint);
void analog_input_callback(void *pCallbackParam, void *pPoint);
void binary_input_callback(void *pCallbackParam, void *pPoint);

/**
 * @brief Assign callback functions to analog and binary input points upon
 * initialization of the DNP3 server.
 *
 * The relevant callback functions are analog_input_callback and binary_input_callback,
 * respectively. These functions handle event creation and point status updates when
 * tmwsim_setAnalogValue or tmwsim_setBinaryValue are called.
 *
 * @param dnp3_sys a pointer to a fully initialized DNP3Dependencies struct
 */
void add_input_callbacks(DNP3Dependencies *dnp3_sys);

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
bool parseBodyServer(GcomSystem &sys, Meta_Data_Info &meta_data);

/**
 * @brief Upon receiving a value via fims, check that the received value is within the dbPoint's
 * numeric limits based on its assigned static variation.
 *
 * If outside those limits, update the value and provide an appropriate error message.
 *
 * @param dbPoint the TMWSIM_POINT * currently being updated
 * @param value the value that was passed over fims that will be stored in dbPoint->data.analog.value
 */
void check_limits_server(TMWSIM_POINT *dbPoint, double &value);