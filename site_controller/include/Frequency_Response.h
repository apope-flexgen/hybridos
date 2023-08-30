/*
 * Frequency_Response.h
 * Created: Summer 2022
 */

#ifndef INCLUDE_FREQUENCY_RESPONSE_H_
#define INCLUDE_FREQUENCY_RESPONSE_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
#include <vector>
#include <fims/fps_utils.h>
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Freq_Resp_Component.h>

class Frequency_Response {
public:
    bool parse_json_config(cJSON* JSON_config, bool* p_flag, Input_Source_List* inputs, const Fims_Object& defaults, std::vector<Fims_Object*>& multiple_inputs);
    void handle_fims_set(const fims_message& msg);
    void get_feature_vars(std::vector<Fims_Object*>& var_list);
    void get_summary_vars(std::vector<Fims_Object*>& var_list);
    Frequency_Response_Outputs aggregate_response_components(Frequency_Response_Inputs input_settings);

protected:
    std::vector<std::shared_ptr<Freq_Resp_Component>> response_components;

    // inputs
    Fims_Object enable_mask;
    Fims_Object freq_offset_hz;
    Fims_Object target_freq_hz;
    Fims_Object baseload_cmd_kw;
    // outputs
    Fims_Object total_output_kw;

    std::vector<std::pair<Fims_Object*, std::string>> variable_ids = {
        // inputs
        { &enable_mask, "fr_enable_mask" },
        { &baseload_cmd_kw, "fr_baseload_cmd_kw" },
        { &target_freq_hz, "fr_target_freq_hz" },
        { &freq_offset_hz, "fr_freq_offset_hz" },
        // outputs
        { &total_output_kw, "fr_total_output_kw" }
    };
};

#endif /* INCLUDE_FREQUENCY_RESPONSE_H_ */