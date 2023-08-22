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

enum LDSS_Priority_Setting
{
    STATIC, DYNAMIC
};

enum states  //states used for site control
{
    Init = 0,
    Ready,
    Startup,
    RunMode1,
    RunMode2,
    Standby,
    Shutdown,
    Error
};

enum uriType
{
    uriError,
    features_uri,
    asset,
    uriError2,
    site_uri
};

//array of strings for displaying state names as strings
const char state_name[][MAX_STATE_NAME_LENGTH] = { "Init", "Ready", "Startup", "RunMode1", "RunMode2", "Standby", "Shutdown", "Error" };

enum priority //asset-type priority for power commands
{
    solar_gen_ess_feeder = 0,
    solar_feeder_ess_gen,
    gen_ess_solar_feeder,
    gen_solar_ess_feeder,
    ess_solar_gen_feeder,
    ess_solar_feeder_gen,
    solar_gen_feeder_ess
};

enum gridMode
{
    FOLLOWING, FORMING, UNDEFINED
};

enum powerMode
{
    REACTIVEPWR, PWRFCTR, UNKNOWN
};

enum demandMode {
    Uncontrolled = 0, 
    Indirect = 1, 
    Direct = 2,  
    Manual = 3
};

enum jsonBuildOption
{
    yesNoOption = 0,
    onOffOption,
    closeOption,
    openOption,
    resetOption,
    nullJson
};

enum displayFunction {
    noneMp,   //don't display
    statusMp, // status
    controlMp, // control
    faultMp, // fault
    alarmMp // alarm
};

enum displayType {
    emptyNull,
    enumStr,
    numberStr,
    sliderStr,
    buttonStr,
};

enum accessType {
    readWrite = 0, 
    readOnly,
    writeOnly 
};

enum valueType {
    Int, // integer
    Float, // float
    Bool,  // bool
    Bit_Field,  // bit_field status
    Random_Enum,
    String,
    Status,
    Invalid
};

enum assetType
{
    ESS, FEEDERS, GENERATORS, SOLAR, NUM_ASSET_TYPES
};

enum statusType {
    bit_field,
    random_enum,
    invalid
};

enum UI_Type
{
    STATUS, CONTROL, ALARM, FAULT, NONE
};

enum setpoint_states
{
    ACCEPTED, LIMITED, ZERO
};

enum template_type
{
    TEMPLATING_ERROR = -1, NON_TEMPLATE, TRADITIONAL, RANGED
}; 

const char UI_Type_Names[][16] = {"status", "control", "alarm", "fault", "none"};

/**
 * Settable ui_control variables in handle_set(), defined in Asset_Manager.cpp
 * As these variables are not stored in the asset_var_map they need to be accessed from the Asset Instances
 *      TODO: workaround to avoid hardcoded values
 */
extern std::vector<std::vector<std::string>> ui_control_set;

struct ESS_Calibration_Settings
{
    bool calibration_flag;
    float balancing_factor;
    bool power_dist_flag;
    bool limits_override;
    bool soc_limits_flag;
    float min_soc_limit;
    float max_soc_limit;
    bool voltage_limits;
    float min_voltage_limit;
    float max_voltage_limit;
    float raw_feature_setpoint;
};

#endif
