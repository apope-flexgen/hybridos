/*
 * Frequency_Response.h
 * Created: Summer 2022
 */

#ifndef FEATURES_FREQUENCY_RESPONSE_H_
#define FEATURES_FREQUENCY_RESPONSE_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
#include <vector>
#include <fims/fps_utils.h>
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Freq_Resp_Component.h>
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>

/**
 * Frequency Response is an active power feature that allows the user to configure power responses from available assets
 * due to measured frequency deviations.
 */
class features::Frequency_Response : public Feature {
public:
    Frequency_Response();

    std::vector<std::shared_ptr<Freq_Resp_Component>> response_components;

    // inputs
    Fims_Object enable_mask;
    Fims_Object freq_offset_hz;
    Fims_Object target_freq_hz;
    // outputs
    Fims_Object total_output_kw;

    Config_Validation_Result parse_json_config(cJSON* JSON_config, bool* primary_flag, Input_Source_List* inputs, const Fims_Object& field_defaults, std::vector<Fims_Object*>& multiple_inputs) override;

    std::vector<std::string> get_variable_ids_list() const override;

    void get_feature_vars(std::vector<Fims_Object*>& var_list);
    void get_summary_vars(std::vector<Fims_Object*>& var_list);

    // Executes feature logic on asset_cmd data by aggregating response components and setting calculated and constant outputs
    void execute(Asset_Cmd_Object&, float ess_total_rated_active_power, float site_frequency, timespec current_time);
    Frequency_Response_Outputs aggregate_response_components(Frequency_Response_Inputs input_settings);

    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override;
};

#endif /* FEATURES_FREQUENCY_RESPONSE_H_ */
