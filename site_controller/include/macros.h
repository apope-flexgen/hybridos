/*
    macros.h

    Created by: October 13, 2020
    Author: Jack Timothy (jtimothy)

    This file contains all necessary #defines for the site_controller repo.
    Please try to maintain this list in alphabetical order.
*/

#define ASSET_TIMER_LOOP_uS_LOW  (910000)
#define CHARGE_CONTROL_AVAILABLE_MASK (0x3F) // ANDed with available active power features mask for active power features that are compatible with charge control
#define COMPONENT_CONTROL_SUFFIX    ("_component_ctrl_cmd")
#define ESS_TYPE_ID                 ("ess")
#define FEEDERS_TYPE_ID             ("feeders")
#define GENERATORS_TYPE_ID          ("generators")
#define MAX_MODE_NAME_LENGTH        (32)
#define MAX_NUM_ASSETS              (100)
#define MAX_STATE_NAME_LENGTH       (16)
#define MS_TO_NS_MULTIPLIER         (1000000)
#define MS_TO_uS_MULTIPLIER         (1000)
#define NUM_STATES                  (7) //less error state because it is not yet needed in sequences.json
#define NUM_TYPE_MANAGERS           (4)
#define ONE_SECOND_IN_TERMS_OF_MS   (1000)
#define ONE_SECOND_IN_TERMS_OF_NS   (1000000000)
#define RESERVED_BOOL_FAULTS_OFFSET (15)
#define SOLAR_TYPE_ID               ("solar")
#define NUM_ESS_UI_CONTROLS         (9)
#define NUM_FEEDER_UI_CONTROLS      (6)
#define NUM_GEN_UI_CONTROLS         (7)
#define NUM_SOLAR_UI_CONTROLS       (7)