/*
 * OSControl.h
 *
 *  Created on: Aug 10, 2018
 *      Author: ghoward
 */

#ifndef OSCONTROL_H_
#define OSCONTROL_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */

// Added /config subscription
#define NUM_CONTROLLER_SUBS  5

#define MAX_COMPS  32
#define MAX_STATUS_BITS  64

#define DISPLAY_DECIMATE    100

#define STATE_MACHINE_INTERVAL_uS (10000)
#define ASSET_PUBLISH_RATE_uS (500000)

#define NUM_SITE_MODES 4 // number of site modes supported

// Give enough time to send a response by hand
// Will be reduced based on db response time when the storage module is completed
#define DB_RESPONSE_TIMEOUT_uS 10000000

// Moved to header for access from hybridos_controller_test
cJSON *parseJSONConfig(fims *p_fims, std::string config_type);

#endif /* OSCONTROL_H_ */
