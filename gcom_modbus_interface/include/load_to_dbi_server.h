#include <cjson/cJSON.h>
#include <cstdint>
#include <fims/libfims.h>
#include "gcom_modbus_server.h"
#include "gcom_modbus_utils.h"

void add_point_to_cjson(cJSON* send_body, bool* reg_type, maps** settings);
void write_points_to_dbi_server(system_config* sys, fims* fims_gateway, uri_map* uri_to_register,
                                server_data* server_map);