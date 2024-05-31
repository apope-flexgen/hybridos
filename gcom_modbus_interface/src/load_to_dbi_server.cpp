#include <cjson/cJSON.h>
#include <cstdint>
#include <fims/libfims.h>
#include "gcom_modbus_server.h"
#include "gcom_modbus_utils.h"

void add_point_to_cjson(cJSON* send_body, bool* reg_type, maps** settings)
{
    if (reg_type && settings)
    {
        for (int i = 0; i < Num_Register_Types; i++)
        {
            if (reg_type[i] == false)
                continue;
            if (!cJSON_IsNull(&settings[i]->raw_val))
            {
                cJSON_AddItemReferenceToObject(send_body, settings[i]->reg_name, &settings[i]->raw_val);
                break;
            }
        }
    }
}

void write_points_to_dbi_server(system_config* sys, fims* fims_gateway, uri_map* uri_to_register,
                                server_data* server_map)
{
    for (auto it = uri_to_register->begin(); it != uri_to_register->end(); ++it)
    {
        const char* parent_uri = it->first;
        body_map* io_point_map = it->second;
        cJSON* send_body = cJSON_CreateObject();
        std::string dbi_uri = "/dbi/" + std::string(sys->name) + "/saved_registers" + std::string(parent_uri);
        bool added_one = false;
        for (auto body_it = io_point_map->begin(); body_it != io_point_map->end(); ++body_it)
        {
            added_one = true;
            Uri_Info uri_info = body_it->second;
            add_point_to_cjson(send_body, uri_info.reg_types, uri_info.mappings);
        }
        if (added_one)
        {
            char* body_msg = cJSON_PrintUnformatted(send_body);
            fims_gateway->Send("set", dbi_uri.c_str(), NULL, body_msg);
            free(body_msg);
        }
        cJSON_Delete(send_body);
    }
}
