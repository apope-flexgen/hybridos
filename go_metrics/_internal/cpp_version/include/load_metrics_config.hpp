#pragma once

#include <string>
#include <vector>
#include <limits>
#include <map>
#include <unordered_set>
#include <typeindex>
#include <cctype>

#include "spdlog/fmt/fmt.h"

#include "simdjson_noexcept.hpp"
#include "shared_utils.hpp"
#include "string_storage.hpp"
#include "Jval_buif.hpp"
#include "exprtk.hpp"

#include "fims/fims.h"


namespace config_loader
{

struct json_accessor {
    std::string key;
    u64 index = 0;
};

struct error_location {
    std::vector<json_accessor> json_location;
    std::string error;
};

struct error_locations {
    std::vector<error_location> error_locations;
};

struct meta_data {

};

struct input {
    std::string var_name;
    std::string uri;
    // std::string_view parent_uri;
    // std::string_view frag;
    std::type_index type = typeid("");
};

struct output {
    std::string var_name;
    std::string uri;
    std::vector<output_flags> flags;
    Jval_buif value;
};

struct expression {
    std::string str_expression;
    std::vector<std::string> output_vars;
    std::vector<std::string> input_vars;
};

struct filter {
    std::vector<expression> filter_funcs;
    std::vector<std::string> inputs;
};

struct MetricsConfig {
    config_loader::meta_data meta_data;
    std::unordered_map<std::string, input> inputs;
    std::unordered_map<std::string, filter> filters;
    std::unordered_map<std::string, output> outputs;
    std::vector<expression> expressions;

    std::unordered_set<std::string> subscribe_uris;
    std::unordered_map<std::string, std::string> uri_to_input_map;
    
    bool load_config(std::string filename) {
        simdjson::dom::parser parser;
        simdjson::dom::object object;
        bool success = true;

        // load the config as a whole
        auto error = parser.load(filename).get(object);
        if(error) { 
            fmt::print("could not load config file {}: {}\n", filename, error);
            return false;
        }

        // extract the meta data from the config
        success = load_meta_data(object);
        if(!success) {
            return false;
        }

        // extract the inputs from the config
        success = load_inputs(object);
        if(!success) {
            return false;
        }

        // extract the filters from the config
        success = load_filters(object);
        if(!success) {
            return false;
        }

        // extract the outputs from the config
        success = load_outputs(object);
        if(!success) {
            return false;
        }

        // extract the metrics expressions from the config
        success = load_expressions(object);
        if(!success) {
            return false;
        }

        return true;

    } // end load_config

    bool load_meta_data(simdjson::dom::object object) {
        simdjson::dom::object meta_object;
        auto error_meta = object["meta"].get_object().get(meta_object);
        if(error_meta) {
            fmt::print("config error: expected \"meta\" to be a json object\n");
            return false;
        }
        // TODO: Figure out meta data stuff
        return true;
    }

    bool load_inputs(simdjson::dom::object object) {
        bool success;
        simdjson::dom::object inputs_object;
        auto error_inputs = object["inputs"].get_object().get(inputs_object);
        if(error_inputs) {
            fmt::print("config error: expected \"inputs\" to be a json object\n");
            return false;
        }
 
        for(auto [key, value] : inputs_object) {
            success = load_input(key, value);
            if(!success){
                return false;
            }
        }

        return true;
    }

    bool load_filters(simdjson::dom::object object) {
        bool success;
        simdjson::dom::object filters_object;
        auto error_filters = object["filters"].get_object().get(filters_object);
        if(error_filters) {
            fmt::print("config error: expected \"filters\" to be a json object\n");
            return false;
        }

        for(auto [key, value] : filters_object) {
            success = load_filter(key, value);
            if(!success){
                return false;
            }
        }
        
        return true;
    }

    bool load_outputs(simdjson::dom::object object) {
        bool success;
        simdjson::dom::object outputs_object;
        auto error_outputs = object["outputs"].get_object().get(outputs_object);
        if(error_outputs) {
            fmt::print("config error: expected \"outputs\" to be a json object\n");
            return false;
        }

        for(auto [key, value] : outputs_object) {
            success = load_output(key, value);
            if(!success){
                return false;
            }
        }
        
        return true;
    }

    bool load_expressions(simdjson::dom::object object) {
        bool success;
        simdjson::dom::array expressions_array;
        auto error_expressions = object["metrics"].get_array().get(expressions_array);
        if(error_expressions) {
            fmt::print("config error: expected \"metrics\" to be a json array of metrics objects\n");
            return false;
        }

        int i = 0;
        for(auto expressions_object : expressions_array) {
            success = load_expression(expressions_object, i);
            if(!success){
                return false;
            }
            i++;
        }
        
        return true;
    }

    // TODO: change everything away from pointers
    bool load_input(std::string_view name, simdjson::dom::element input_json) {
        if(inputs.count(static_cast<std::string>(name))) {    // if we already have this as a variable name (shouldn't happen if proper json doc)
            fmt::print("config error: found duplicate input variable name {}\n", name);
            return false;
        }

        simdjson::dom::object object;
        auto error = input_json.get(object);
        if(error) {    // if input is not a json object
            fmt::print("config error: input [{}] is not a json object\n", name);
            return false;
        }

        config_loader::input input;

        std::string_view uri;
        auto error_uri = object["uri"].get(uri);
        if(error_uri == simdjson::NO_SUCH_FIELD) {   // if the input doesn't have a corresponding uri
            fmt::print("config error: input [{}] does not have an assigned uri\n", name);
            return false;
        }

        std::string_view type;
        auto error_type = object["type"].get(type);
        if(error_type == simdjson::NO_SUCH_FIELD) {   // if the input doesn't have a corresponding type
            fmt::print("config error: input [{}] does not have an assigned type\n", name);
            return false;
        }

        input.var_name = name;
        input.uri = uri;
        // TODO: get parent_uri
        // TODO: get frag


        if(type == "null") {
            input.type = typeid(NULL);
        } else if(type == "bool") {
            input.type = typeid(true);
        } else if(type == "uint") {
            input.type = typeid(5u);
        } else if(type == "int") {
            input.type = typeid(5);
        } else if(type == "float") {
            input.type = typeid(double(5.3));
        } else {
            input.type = typeid("string");
        }
        
        inputs.insert(std::pair<std::string_view, config_loader::input>(name, input));
        uri_to_input_map.insert(std::pair<std::string_view, std::string_view>(uri, name));
        return true;
    }

    bool load_filter(std::string_view name, simdjson::dom::element input_json) {
        if(filters.count(static_cast<std::string>(name))) {    // if we already have this as a filter variable name (shouldn't happen if proper json doc)
            fmt::print("config error: found duplicate filter variable name {}\n", name);
            return false;
        }

        if(inputs.count(static_cast<std::string>(name))) {    // if we already have this as an input variable name (CAN happen, even in a valid json)
            fmt::print("config error: filter variable name {} already exists as an input variable\n", name);
            return false;
        }
        std::string_view string_filter;
        auto error = input_json.get(string_filter);
        if(error) {    // if filter is not a json object
            fmt::print("config error:  [{}] is not a json string\n", name);
            return false;
        }

        config_loader::filter filter;
        config_loader::expression expression;
        expression.str_expression = string_filter;
        filter.filter_funcs.push_back(expression);
        // TODO: Actually do something with the filter?

        filters.insert(std::pair<std::string_view, config_loader::filter>(name,filter));

        return true;
    }

    bool load_output(std::string_view name, simdjson::dom::element input_json) {
        if(outputs.count(static_cast<std::string>(name))) {    // if we already have this as a variable name (shouldn't happen if proper json doc)
            fmt::print("config error: found duplicate output variable name {}\n", name);
            return false;
        }

        simdjson::dom::object object;
        auto error = input_json.get(object);
        if(error) {    // if output is not a json object
            fmt::print("config error: output [{}] is not a json object\n", name);
            return false;
        }

        config_loader::output output;

        std::string_view uri;
        auto error_uri = object["uri"].get(uri);
        if(error_uri == simdjson::NO_SUCH_FIELD) {   // if the output doesn't have a corresponding uri
            fmt::print("config error: output [{}] does not have an assigned uri\n", name);
            return false;
        }

        simdjson::dom::array flags_array;
        auto error_type = object["flags"].get(flags_array);
        if(error_type == simdjson::NO_SUCH_FIELD) {   // if the input doesn't have a corresponding type
            fmt::print("no flags found for output [{}]; are you sure that's what you wanted?\n", name);
        }

        output.var_name = name;
        output.uri = uri;

        for(auto flag_element : flags_array) {
            std::string_view flag;
            auto error_type = flag_element.get(flag);
            if(error_type == simdjson::NO_SUCH_FIELD) {   // if the input doesn't have a corresponding type
                fmt::print("flag for output [{}] is invalid; ignoring flag and continuing anyway...\n", name);
            }
            if(flag == "clothed") {
                output.flags.push_back(output_flags::CLOTHED);
            } else if (flag == "pub_as_set") {
                output.flags.push_back(output_flags::PUB_AS_SET);
            } else {
                fmt::print("flag for output [{}] is invalid; ignoring flag and continuing anyway...\n", name);
            }
        }
        
        outputs.insert(std::pair<std::string_view, config_loader::output>(name, output));

        return true;
    }

    bool load_expression(simdjson::dom::element input_json, int i) {
        simdjson::dom::object object;
        auto error = input_json.get(object);
        if(error) {    // if metrics item is not a json object
            fmt::print("config error: invalid metrics object\n");
            return false;
        }

        config_loader::expression expression;

        simdjson::dom::element initial_value;
        auto error_value = object["value"].get(initial_value);
        if(error_value == simdjson::NO_SUCH_FIELD) {   // if the metrics config doesn't have a default value field
            fmt::print("config error: metrics object does not have an assigned default value/type; skipping metric {}\n", i);
            return true;
        }
        if(!(initial_value.is_bool() || initial_value.is_uint64() || initial_value.is_int64() || initial_value.is_double())) {
            fmt::print("config error: metrics object {} has invalid output value type (must be int, uint, float, or bool); skipping metric\n", i);
            return true;
        }

        simdjson::dom::array outputs_array;
        auto error_outputs = object["outputs"].get(outputs_array);
        if(error_outputs == simdjson::NO_SUCH_FIELD) {   // if the metrics config doesn't have a output array
            fmt::print("config error: no outputs found for metrics object; skipping metric{}\n", i);
            return true;
        } else if (error_outputs == simdjson::INCORRECT_TYPE) {
            fmt::print("config error: outputs for metrics object must be an array of strings; skipping metric {}\n", i);
            return true;
        }

        std::string_view expression_str;
        auto error_expression = object["expression"].get(expression_str);
        if(error_expression == simdjson::NO_SUCH_FIELD) {   // if the metrics config doesn't have an expression
            fmt::print("config error: no outputs found for metrics object; skipping metric {}\n", i);
            return true;
        } else if (error_expression == simdjson::INCORRECT_TYPE) {
            fmt::print("config error: metrics expressions should be string; skipping metric{}\n", i);
            return true;
        }

        int j = 0;
        for(auto output_element : outputs_array) {
            std::string_view output_var_sv;
            auto error_output_var = output_element.get(output_var_sv);
            if(error_expression == simdjson::INCORRECT_TYPE) {   // if the metrics output_var name isn't a string
                fmt::print("config error: output variable name in metric {}'s output list at position {} is not a string\n", i, j);
                continue;
            }

            std::string output_var = static_cast<std::string>(output_var_sv);

            if(outputs.count(output_var)) { //if output_var isn't in our list of outputs (matched with URIs)
                fmt::print("config error: output var {} in metrics object {} doesn't have a corresponding output object in \"outputs\"\n", output_var, i);
                continue;
            }

            if(initial_value.is_bool()) {
                outputs[output_var].value.tag = jval_types::Bool;
                auto error_type_conversion = initial_value.get(outputs[output_var].value.b);
            } else if(initial_value.is_uint64()) {
                outputs[output_var].value.tag = jval_types::UInt;
                auto error_type_conversion = initial_value.get(outputs[output_var].value.u);
            } else if(initial_value.is_int64()) {
                outputs[output_var].value.tag = jval_types::Int;
                auto error_type_conversion = initial_value.get(outputs[output_var].value.i);
            } else if(initial_value.is_double()) {
                outputs[output_var].value.tag = jval_types::Float;
                auto error_type_conversion = initial_value.get(outputs[output_var].value.f);
            }

            expression.output_vars.push_back(output_var);
            j++;
        }

        expression.str_expression = expression_str;
        expression.input_vars  = static_cast<std::vector<std::string>>(extract_vars(expression_str));

        
        expressions.push_back(expression);

        return true;
    }

    std::vector<std::string> extract_vars(std::string_view expression_string) {
        std::unordered_set<std::string> vars;
        std::string current_word = "";

        for(int i = 0; i < expression_string.length(); i++) {
            if(isalpha(expression_string[i])) {
                current_word += expression_string[i];
            } else if(isdigit(expression_string[i])) {
                if(current_word.length() > 0) {
                    current_word += expression_string[i];
                }
            } else {
                if(current_word.length() > 0) {
                    vars.insert(current_word);
                    current_word = "";
                }
            }
        }

        std::vector<std::string> output_vars;

        for(std::string str : vars) {
            output_vars.push_back(str);
        }
        return output_vars;
    }
};


}
