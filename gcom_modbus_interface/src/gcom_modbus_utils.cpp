/*
 * modbus_utils.cpp
 *
 *  Created on: Oct 2, 2018
 *      Author: jcalcagni
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fims/libfims.h>
#include <fims/fps_utils.h>
#include <modbus/modbus.h>
#include "gcom_modbus_utils.h"

#include "fims/defer.hpp"
#include "spdlog/fmt/fmt.h"

void emit_event(fims* pFims, const char* source, const char* message, int severity)
{
    cJSON* body_object;
    body_object = cJSON_CreateObject();
    cJSON_AddStringToObject(body_object, "source", source);
    cJSON_AddStringToObject(body_object, "message", message);
    cJSON_AddNumberToObject(body_object, "severity", severity);
    char* body = cJSON_PrintUnformatted(body_object);
    pFims->Send("post", "/events", NULL, body);
    free(body);
    cJSON_Delete(body_object);
}

cJSON* get_config_json(int argc, char* argv[])
{
    FILE *fp = NULL;

    enum class Arg_Types : uint8_t
    {
        File,
        Uri // for a "get" over fims
    };

    std::pair<Arg_Types, std::string> args;
    args.first = Arg_Types::File; // file by default

    if (argc <= 1)
    {
        FPS_ERROR_PRINT("Need to pass at least 2 arguments.\n");
        return NULL;
    }

    if (!argv[1])
    {
        FPS_ERROR_PRINT(" Failed to get the second argument\n");
        return NULL;
    }
    args.second = argv[1]; // for file by default

    static constexpr std::size_t Max_Arg_Size = std::numeric_limits<uint8_t>::max();

    // uri init code for  file:
    if (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "--config") == 0)
    {
        if (argc >= 2 && argv[2] && strlen(argv[2]) <= Max_Arg_Size)
        {
            args.first = Arg_Types::File;
            args.second = argv[2];
        }
        else
        {
            FPS_ERROR_PRINT("error in --config: config json path not provided, or config json path was more than %ld characters\n", Max_Arg_Size);
            return nullptr;
        }
    }
    // uri init code for server:
    if (strcmp(argv[1], "-u") == 0 || strcmp(argv[1], "--uri") == 0)
    {
        if (argc >= 2 && argv[2] && strlen(argv[2]) <= Max_Arg_Size)
        {
            args.first = Arg_Types::Uri;
            args.second = argv[2];
        }
        else
        {
            FPS_ERROR_PRINT("error in --uri: fims json path not provided, or fims json path was more than %ld characters\n", Max_Arg_Size);
            return nullptr;
        }
    }

    char* config_json = nullptr;
    defer { if(config_json) free(config_json); };

    if (args.first == Arg_Types::File)
    {
        // prune the extensions of the input file then append .json
        // this way input extension doesn't matter (it can be optional or messed up by accident)
        //const auto first_extension_index = args.second.find_first_of('.');
        // const auto first_extension_index = args.second.find_last_of('.');
        // if (first_extension_index != args.second.npos) args.second.resize(first_extension_index);

        fp = fopen(args.second.c_str(), "r");
        if(fp == NULL)
        {
            args.second.append(".json");
            fp = fopen(args.second.c_str(), "r");
        }
        if(fp == NULL)
        {
            FPS_ERROR_PRINT("Failed to open file %s\n", args.second.c_str());
            return NULL;
        }

        fseek(fp, 0, SEEK_END);
        long unsigned file_size = ftell(fp);
        rewind(fp);

        // create Configuration_file and read file in Configuration_file
        config_json = (char*) calloc(file_size+1, 1);
        if(config_json == NULL)
        {
            FPS_ERROR_PRINT("Memory allocation error\n");
            fclose(fp);
            return NULL;
        }

        size_t bytes_read = fread(config_json, 1, file_size, fp);
        fclose(fp);
        if(bytes_read != file_size)
        {
            FPS_ERROR_PRINT("Read error.\n");
            return NULL;
        }
    }
    else if (args.first == Arg_Types::Uri)
    {
        if (args.second.front() != '/')
        {
            FPS_ERROR_PRINT("For server with init uri \"%s\": the uri does not begin with `/`\n", args.second.data());
            return nullptr;
        }
        fims fims_gateway;
        const auto conn_id_str = fmt::format("modbus_server_uri_init@{}", args.second);
        if (!fims_gateway.Connect(conn_id_str.data()))
        {
            FPS_ERROR_PRINT("For server with init uri \"%s\": could not connect to fims_server\n", args.second.data());
            return nullptr;
        }
        const auto sub_string = fmt::format("/modbus_server_uri_init{}", args.second);
        if (!fims_gateway.Subscribe(std::vector<std::string>{sub_string}))
        {
            FPS_ERROR_PRINT("For server with init uri \"%s\": failed to subscribe for uri init\n", args.second.data());
            return nullptr;
        }
        if (!fims_gateway.Send(fims::str_view{"get", sizeof("get") - 1}, fims::str_view{args.second.data(), args.second.size()}, fims::str_view{sub_string.data(), sub_string.size()}, fims::str_view{nullptr, 0}, fims::str_view{nullptr, 0}))
        {
            FPS_ERROR_PRINT("For server with inti uri \"%s\": failed to send a fims get message\n", args.second.data());
            return nullptr;
        }
        auto config_msg = fims_gateway.Receive_Timeout(5000000); // give them 5 seconds to respond before erroring
        defer { fims::free_message(config_msg); };
        if (!config_msg)
        {
            FPS_ERROR_PRINT("For server with init uri \"%s\": failed to receive a message in 5 seconds\n", args.second.data());
            return nullptr;
        }
        if (!config_msg->body)
        {
            FPS_ERROR_PRINT("For server with init uri \"%s\": message was received, but body doesn't exist\n", args.second.data());
            return nullptr;
        }
        config_json = strdup(config_msg->body);
    }

    cJSON* config = cJSON_Parse(config_json);
    if(config == NULL)
        FPS_ERROR_PRINT("Invalid JSON object for config\n");
    return config;
}


const char* regTypes[] = {
    "Coil", "Discrete_Input", "Input_Register", "Holding_Register", "Unknown"
};

const char *getRegisterType(Type_of_Register idx)
{
    if(idx < Num_Register_Types)
        return regTypes[idx];
    return regTypes[Num_Register_Types];
}

// #define MODBUS_FC_READ_COILS                0x01
// #define MODBUS_FC_READ_DISCRETE_INPUTS      0x02
// #define MODBUS_FC_READ_HOLDING_REGISTERS    0x03
// #define MODBUS_FC_READ_INPUT_REGISTERS      0x04
// #define MODBUS_FC_WRITE_SINGLE_COIL         0x05
// #define MODBUS_FC_WRITE_SINGLE_REGISTER     0x06
// #define MODBUS_FC_READ_EXCEPTION_STATUS     0x07
// #define MODBUS_FC_WRITE_MULTIPLE_COILS      0x0F
// #define MODBUS_FC_WRITE_MULTIPLE_REGISTERS  0x10
// #define MODBUS_FC_REPORT_SLAVE_ID           0x11
// #define MODBUS_FC_MASK_WRITE_REGISTER       0x16
// #define MODBUS_FC_WRITE_AND_READ_REGISTERS  0x17
const char *getFunction(uint8_t function)
{
    switch (function & 0x7f)
    {
        case  MODBUS_FC_READ_COILS:
            return "write_multiple_registers";
        case MODBUS_FC_READ_DISCRETE_INPUTS:
            return "read_discrete_inputs";
        case MODBUS_FC_READ_HOLDING_REGISTERS:
            return "read_holding_registers";
        case MODBUS_FC_READ_INPUT_REGISTERS:
            return "read_input_registers";
        case MODBUS_FC_WRITE_SINGLE_COIL:     
            return "write_single_coil";
        case MODBUS_FC_WRITE_SINGLE_REGISTER: 
            return "write_single_register";
        case MODBUS_FC_READ_EXCEPTION_STATUS:
            return "read_exception_registers";
        case MODBUS_FC_WRITE_MULTIPLE_COILS:  
            return "write_multiple_coils";
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS: 
            return "write_multiple_registers";
        case MODBUS_FC_REPORT_SLAVE_ID:         
            return "report_slave_id";
        case MODBUS_FC_MASK_WRITE_REGISTER:      
            return "mask_write_register";
        case MODBUS_FC_WRITE_AND_READ_REGISTERS: 
            return "write_and_read_registers";

    }
    return "unknown function code";
}