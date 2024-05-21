/*
    macros.h

    Created by: October 13, 2020
    Author: Jack Timothy (jtimothy)

    This file contains all necessary #defines for the site_controller repo.
    Please try to maintain this list in alphabetical order.
*/

#define ASSET_PUBLISH_RATE_uS (500000)
#define ASSET_TIMER_LOOP_uS_LOW (910000)
#define CHARGE_CONTROL_AVAILABLE_MASK (0x3F)  // ANDed with available active power features mask for active power features that are compatible with charge control
#define COMPONENT_CONTROL_SUFFIX ("_component_ctrl_cmd")
#define DB_RESPONSE_TIMEOUT_uS (10000000)  // Give enough time to send a response by hand. Will be reduced based on db response time when the storage module is completed
#define DISPLAY_DECIMATE (100)
#define ESS_TYPE_ID ("ess")
#define FEEDERS_TYPE_ID ("feeders")
#define GENERATORS_TYPE_ID ("generators")
#define SOLAR_TYPE_ID ("solar")
#define LONG_MSG_LEN (1024)
#define LONG_MSG_LEN_COPY (1023)  // make this one less than LONG_MSG_LEN for using strncpy. We want null termination.
#define MAX_SCAN (50) // used as a maximum word size when evaluating c strings with strnlen() etc.
#define MAX_COMPS (32)
#define MAX_MODE_NAME_LENGTH (32)
#define MAX_NUM_ASSETS (100)
#define MAX_STATE_NAME_LENGTH (16)
#define MAX_STATUS_BITS (64)
#define MAX_SCAN (50)
#define MEDIUM_MSG_LEN (512)
#define MEDIUM_MSG_LEN_COPY (511)  // make this one less than MEDIUM_MSG_LEN for using strncpy. We want null termination.
#define MS_TO_NS_MULTIPLIER (1000000)
#define MS_TO_uS_MULTIPLIER (1000)
#define NUM_ASSET_PRIORITIES (6)
#define NUM_CONTROLLER_SUBS (5)
#define NUM_ESS_UI_CONTROLS (9)
#define NUM_FEEDER_UI_CONTROLS (6)
#define NUM_GEN_UI_CONTROLS (7)
#define NUM_SITE_MODES (4)  // number of site modes supported
#define NUM_SOLAR_UI_CONTROLS (7)
#define NUM_STATES (7)  // less error state because it is not yet needed in sequences.json
#define NUM_TYPE_MANAGERS (4)
#define ONE_SECOND_IN_TERMS_OF_MS (1000)
#define ONE_SECOND_IN_TERMS_OF_NS (1000000000)
#define RESERVED_BOOL_FAULTS_OFFSET (15)
#define MAX_VARIABLE_LENGTH (256)
#define SHORT_MSG_LEN (256)
#define SHORT_MSG_LEN_COPY (255)  // make this one less than SHORT_MSG_LEN for using strncpy. We want null termination.
#define STATE_MACHINE_INTERVAL_uS (10000)
#define MAX_SLEW 100000000
