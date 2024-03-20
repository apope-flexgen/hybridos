extern "C"
{
#include "tmwscl/utils/tmwtypes.h"
#include "tmwscl/utils/tmwsim.h"
#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/dnpchnl.h"
#include "tmwscl/dnp/mdnpsesn.h"
}

struct GcomSystem;
struct Meta_Data_Info;

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
void addPointToPubWork(void *pDbPoint);

/**
 * @brief Call addPointToPubWork and then restart the interval timer for the point to call
 * this function again after the interval period.
 *
 * @param pDbPoint void * pointer to the TMWSIM_POINT * to be added to the publish work queue.
 *
 * @pre pDbPoint is a valid pointer to a TMWSIM_POINT struct
 */
void addPointToIntervalPubWork(void *pDbPoint);

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
void updatePointCallback(void *pDbHandle, TMWSIM_EVENT_TYPE type, DNPDEFS_OBJ_GROUP_ID objectGroup, TMWTYPES_USHORT pointNumber);

/**
 * @brief Call tmwdb_storeEntry for all entries on the tmwdb queue.
 *
 * Typically called directly from DNP3 response callbacks when a message is received.
 *
 * @param sys pointer to fully initialized GcomSystem for client
 */
void storeData(GcomSystem *sys);

/**
 * @brief Format fims messages depending on point format and type. Publish all data
 * that has been queued for publishing via fims.
 *
 * @param sys pointer to fully initialized GcomSystem for client
 */
void queuePubs(GcomSystem *sys);

/**
 * @brief Update timings for direct operate responses.
 *
 * Callback function for direct operate responses. Initialized alongside
 * direct operate request header.
 *
 * @param sys void* pointer to fully initialized GcomSystem for client
 * @param response DNPCHNL_RESPONSE_INFO pointer (filled out by TMW)
 */
void process_analog_direct_operate_response(void *sys, DNPCHNL_RESPONSE_INFO *response);

/**
 * @brief Update timings for direct operate responses.
 *
 * Callback function for direct operate responses. Initialized alongside
 * direct operate request header.
 *
 * @param sys void* pointer to fully initialized GcomSystem for client
 * @param response DNPCHNL_RESPONSE_INFO pointer (filled out by TMW)
 */
void process_binary_direct_operate_response(void *sys, DNPCHNL_RESPONSE_INFO *response);

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
void process_integrity_response(void *sys, DNPCHNL_RESPONSE_INFO *response);

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
void process_class1_response(void *sys, DNPCHNL_RESPONSE_INFO *response);

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
void process_class2_response(void *sys, DNPCHNL_RESPONSE_INFO *response);

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
void process_class3_response(void *sys, DNPCHNL_RESPONSE_INFO *response);

/**
 * @brief Store incoming data in MDNPSIM_DATABASE and queue fims pubs for all input points.
 *
 * Callback function for unsolicited responses. Initialized at the start of a
 * TMW session using mdnpsesn_setUnsolUserCallback.
 *
 * @param sys void* pointer to fully initialized GcomSystem for client
 * @param response MDNPSESN_UNSOL_RESP_INFO pointer (filled out by TMW)
 */
void process_unsolicited_response(void *sys, MDNPSESN_UNSOL_RESP_INFO *response);
void send_analog_command_callback(void *pSetWork);
void send_binary_command_callback(void *pSetWork);
void send_interval_analog_command_callback(void *pSetWork);
void send_interval_binary_command_callback(void *pSetWork);
void handle_batch_sets(TMWSIM_POINT *dbPoint, double value);

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
bool parseBodyClient(GcomSystem &sys, Meta_Data_Info &meta_data);

/**
 * @brief Upon receiving a value via fims, check that the received value is within the dbPoint's
 * numeric limits based on its assigned static variation.
 *
 * If outside those limits, update the value and provide an appropriate error message.
 *
 * @param dbPoint the TMWSIM_POINT * currently being updated
 * @param value the value that was passed over fims that will be stored in dbPoint->data.analog.value
 */
void check_limits_client(TMWSIM_POINT *dbPoint, double &value);