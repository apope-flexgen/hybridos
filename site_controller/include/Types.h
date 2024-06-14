/**
 * Header for enums and type names
 */

#ifndef TYPES_H_
#define TYPES_H_

/* Local Internal Dependencies */
#include <macros.h>
#include <vector>
#include <string>

#define NUM_UI_TYPES 5

enum LDSS_Priority_Setting { STATIC, DYNAMIC };

enum states  // states used for site control
{ Init = 0,
  Ready,
  Startup,
  RunMode1,
  RunMode2,
  Standby,
  Shutdown,
  Error };

// the following is cringe, but integration sold out the other state enum
// via metrics so this one complies with their expected enum. The other 
// can be used internally.
enum alt_states  // states used for site controller 
{ 
  TRANSIENT_INIT = 0,
  SHUTDOWN,
  READY,
  STARTUP,
  RUNNING_FOLLOWING,
  FAULTED_STATE,
  RUNNING_FORMING,
  STANDBY_ERROR 
};

enum uriType { uriError, features_uri, asset, uriError2, site_uri };

// array of strings for displaying state names as strings
const char state_name[][MAX_STATE_NAME_LENGTH] = {
    "Init", "Ready", "Startup", "RunMode1", "RunMode2", "Standby", "Shutdown", "Error",
};

enum priority  // asset-type priority for power commands
{ solar_gen_ess_feeder = 0,
  solar_feeder_ess_gen,
  gen_ess_solar_feeder,
  gen_solar_ess_feeder,
  ess_solar_gen_feeder,
  ess_solar_feeder_gen,
  solar_gen_feeder_ess };

enum gridMode { FOLLOWING, FORMING, UNDEFINED };

enum powerMode { REACTIVEPWR, PWRFCTR, UNKNOWN };

enum demandMode { Uncontrolled = 0, Indirect = 1, Direct = 2, Manual = 3 };

enum jsonBuildOption { yesNoOption = 0, onOffOption, closeOption, openOption, resetOption, nullJson };

enum displayFunction {
    noneMp,     // don't display
    statusMp,   // status
    controlMp,  // control
    faultMp,    // fault
    alarmMp     // alarm
};

enum displayType {
    emptyNull,
    enumStr,
    numberStr,
    sliderStr,
    buttonStr,
};

enum accessType { readWrite = 0, readOnly, writeOnly };

enum valueType {
    Int = 0,    // integer
    Float,      // float
    Bool,       // bool
    Bit_Field,  // bit_field status
    Random_Enum,
    String,
    Status,
    Invalid
};

// array of strings for displaying value type names as strings
const std::string VALUE_TYPE_NAMES[] = {
    "Int", "Float", "Bool", "Bit_Field", "Random_Enum", "String", "Status", "Invalid",
};

enum asset_type { ESS, FEEDERS, GENERATORS, SOLAR, NUM_ASSET_TYPES };

enum statusType { bit_field, random_enum, invalid };

enum UI_Type { STATUS, CONTROL, ALARM, FAULT, NONE };

// Enum representation of alert types, starting with INFO at 1 to match the severity used by the events module
enum alert_type { INVALID, INFO_ALERT, STATUS_ALERT, ALARM_ALERT, FAULT_ALERT };

enum setpoint_states { ACCEPTED, LIMITED, ZERO };

enum template_type { TEMPLATING_ERROR = -1, NON_TEMPLATE, TRADITIONAL, RANGED };

const char UI_Type_Names[][16] = { "status", "control", "alarm", "fault", "none" };

// Lower case string representation of alert types alert_names_upper
const std::vector<const char*> alert_names_lower = { "invalid", "info", "status", "alarm", "fault" };
// Upper case string representation of alert types
const std::vector<const char*> alert_names_upper = { "Invalid", "Info", "Status", "Alarm", "Fault" };

struct ESS_Calibration_Settings {
    bool calibration_flag;
    float balancing_factor;
    bool power_dist_flag;
    bool soc_protection_buffers_disable;
    bool soc_limits_flag;
    float min_soc_limit;
    float max_soc_limit;
    bool cell_voltage_limits;
    float min_cell_voltage_limit;
    float max_cell_voltage_limit;
    bool rack_voltage_limits;
    float min_rack_voltage_limit;
    float max_rack_voltage_limit;
    float raw_feature_setpoint;
};

// One type for each asset + site
enum class Sequence_Type { Site, Asset_ESS, Asset_Solar, Asset_Generator, Asset_Feeder };

// This is used to report the status of an action. (actions.json)
enum ACTION_STATUS_STATE { INACTIVE, IN_PROGRESS, COMPLETED, FAILED, ABORTED, EXITING };

#endif
