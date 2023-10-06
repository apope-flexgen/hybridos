/*
 * Input_Sources.h
 *
 * Created on: Feb 10, 2022
 */

#ifndef INCLUDE_INPUT_SOURCE_H_
#define INCLUDE_INPUT_SOURCE_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
#include <string>
#include <vector>
#include <memory>
#include <map>
/* External Dependencies */
#include <cjson/cJSON.h>
#include <spdlog/fmt/fmt.h>
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Types.h>
#include <Config_Validation_Result.h>

struct Input_Source {
    Input_Source(bool init_enabled = false);
    std::string name;
    std::string uri_suffix;
    UI_Type ui_type;
    std::map<std::string, UI_Type> alt_ui_types;
    bool enabled;
    void parse_json_obj(cJSON*);
    std::string get_ui_type_of_var(std::string);
};

class Input_Source_List {
    // internal list of input sources, ordered in the vector by selection priority.
    // the first source in the vector that is enabled is selected.
    std::vector<std::shared_ptr<Input_Source>> input_sources;

    uint selected_input_source_index;

public:
    Config_Validation_Result parse_json_obj(cJSON*);
    void add_to_JSON_buffer(fmt::memory_buffer&, const char* const var = NULL);
    size_t get_num_sources();
    std::string set_source_enable_flag(std::string source_id, bool enable_flag);
    uint get_selected_input_source_index();
    std::string get_uri_suffix_of_input(uint);
    std::string get_name_of_input(uint);
    std::string get_ui_type_of_input(uint source_index, std::string var_id);

    // maps input source IDs (aka uri_suffix) to their indices in the vector, which will match to their indices in the member vectors of control variables' Fims_Objects
    std::map<std::string, uint> id_to_index;
};

#endif /* INCLUDE_INPUT_SOURCE_H_ */
