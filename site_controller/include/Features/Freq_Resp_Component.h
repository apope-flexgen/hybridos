/*
 * Freq_Resp_Component.h
 *
 * Created: Summer 2022
 */

#ifndef FEATURES_FREQUENCY_OBJECT_H_
#define FEATURES_FREQUENCY_OBJECT_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
#include <vector>
/* External Dependencies */
/* System Internal Dependencies */
#include <fims/libfims.h>
/* Local Internal Dependencies */
#include <Fims_Object.h>
#include <Slew_Object.h>

typedef struct frequency_response_input {
    float site_input_kW_demand;
    float ess_max_potential;
    float ess_min_potential;
    float ess_total_rated_active_power;
    float site_frequency;
    timespec current_time;
} Frequency_Response_Inputs;

typedef struct frequency_response_output {
    float output_kw;
    float ess_max_potential;
    float ess_min_potential;
} Frequency_Response_Outputs;

class Freq_Resp_Component {
public:
    Frequency_Response_Outputs frequency_response(const Frequency_Response_Inputs& inputs);

    bool initialize_state(float grid_target_freq_hz);
    bool parse_json_config(cJSON* JSON_config, bool* p_flag, Input_Source_List* inputs, const Fims_Object& defaults, std::vector<Fims_Object*>& multiple_inputs);
    void handle_fims_set(const cJSON* JSON_body, const char* variable_id);
    void get_feature_vars(std::vector<Fims_Object*>& var_list);
    void clear_outputs();

    Fims_Object component_id;

protected:
    // inputs
    Fims_Object active_cmd_kw;
    float prev_active_cmd_kw;
    Fims_Object inactive_cmd_kw;
    Fims_Object trigger_freq_hz;
    Fims_Object force_start; // a boolean input that a user can use to manually start the response
    Fims_Object trigger_duration_sec;
    Fims_Object droop_freq_hz;
    Fims_Object droop_limit_flag;
    bool prev_droop_limit_flag;
    Fims_Object droop_bypass_flag;
    Fims_Object recovery_freq_hz;
    Fims_Object recovery_duration_sec;
    Fims_Object recovery_latch;
    Fims_Object instant_recovery_freq_hz;
    Fims_Object cooldown_duration_sec;
    Fims_Object slew_rate_kw;
    int prev_slew_rate_kw;
    Fims_Object ess_slew_override;
    Fims_Object freeze_active_cmd_flag;
    // outputs
    Fims_Object active_response_status;
    Fims_Object in_cooldown;
    Fims_Object in_recovery;
    Fims_Object output_kw;
    std::vector<std::pair<Fims_Object*, std::string>> variable_ids = {
        // inputs
        { &active_cmd_kw, "active_cmd_kw" },
        { &inactive_cmd_kw, "inactive_cmd_kw" },
        { &trigger_freq_hz, "trigger_freq_hz" },
        { &force_start, "force_start" },
        { &trigger_duration_sec, "trigger_duration_sec" },
        { &droop_freq_hz, "droop_freq_hz" },
        { &droop_limit_flag, "droop_limit_flag" },
        { &droop_bypass_flag, "droop_bypass_flag" },
        { &recovery_freq_hz, "recovery_freq_hz" },
        { &recovery_duration_sec, "recovery_duration_sec" },
        { &recovery_latch, "recovery_latch" },
        { &instant_recovery_freq_hz, "instant_recovery_freq_hz" },
        { &cooldown_duration_sec, "cooldown_duration_sec" },
        { &slew_rate_kw, "slew_rate_kw" },
        { &ess_slew_override, "ess_slew_override" },
        { &freeze_active_cmd_flag, "freeze_active_cmd_flag" },
        // outputs
        { &active_response_status, "active_response_status" },
        { &in_cooldown, "in_cooldown" },
        { &in_recovery, "in_recovery" },
        { &output_kw, "output_kw" }
    };

    bool is_underfrequency_component;  // True when UF, false when OF
    timespec trigger_over_time;        // After this clock time, a trigger event must take a cooldown break
    timespec recovery_over_time;       // Clock time designating end of recovery countdown
    timespec cooldown_over_time;       // Clock time when next trigger event is allowed to happen
    std::vector<std::pair<float, float>> droop_curve;
    Slew_Object slew_cmd_kw;              // rate-limits the output
    float signed_active_cmd_kw;           // tracks the Fims_Object but + for UF responses and - for OF responses, whereas Fims_Object is always +
    float latest_active_cmd_kw_received;  // Tracks latest active_cmd_kw, used for unlatching when event is resolved.

    void sync_active_cmd_kw();
    void build_droop_curve();
    void update_state(timespec current_time, float site_frequency);
    void start_active_response();
    void end_active_response();
    float calculate_kw_output(float current_frequency);
    // frequency boundary checkers
    bool is_beyond_trigger(float freq) const;
    bool is_beyond_recovery(float freq) const;
    bool is_within_recovery(float freq) const;
    bool is_within_instant_recovery(float freq) const;
};

#endif /* FEATURES_FREQUENCY_OBJECT_H_ */
