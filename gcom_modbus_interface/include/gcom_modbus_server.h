#ifndef _MODBUS_SERVER_H_
#define _MODBUS_SERVER_H_

#include "gcom_modbus_utils.h"

typedef struct {
    char* name;
    char* ip_address;
    char* service;
    char* serial_device;
    char* eth_dev;
    bool byte_swap;
    char parity;
    int port;
    int baud;
    int data;
    int stop;
    int device_id;
    int reg_cnt;

    #ifdef SERVER_DELAY
    int connect_delay;
    int server_delay;
    int server_delay_count;
    int single_delay;
    int exception;
    int mdebug;
    #endif
    
    bool debug;
    bool low_debug; 

    bool bypass_init;
    int bad_id_count;
    int max_bad_id_count;

    modbus_t *mb;
} system_config;

struct char_cmp {
    bool operator () (const char *a,const char *b) const
    {
        return strcmp(a,b)<0;
    }
};

struct Uri_Info
{
    bool inited = false;
    uint8_t device_id; // NOTE(WALKER): This is the device_id mapping this uri belongs to
    bool* reg_types;
    maps** mappings;
};

//typedef std::map<const char*, std::pair<bool*, maps**>, char_cmp> body_map;
typedef std::map<const char*, Uri_Info, char_cmp> body_map;
typedef std::map<const char*, body_map*, char_cmp> uri_map;

typedef struct sdata
{
    //uri_map uri_to_register;
    maps** regs_to_map[Num_Register_Types];
    datalog data[Num_Register_Types];
    modbus_mapping_t* mb_mapping;
 
    //uint8_t device_id; // NOTE(WALKER): This is the device_id mapping this uri belongs to
 
    sdata()
    {
        memset(regs_to_map, 0, Num_Register_Types * sizeof(maps**));
        memset(data, 0, sizeof(datalog)*Num_Register_Types);
        mb_mapping = NULL;
        //p_fims = NULL;
        //uris = NULL;
        //base_uri = NULL;
        //num_uris = 0;
    }
    //void load_uri_array();
} server_data;

/* Configuration */
int parse_system(cJSON *system, system_config *config);
server_data *create_register_map(cJSON *registers, datalog *data);

/* Modbus and FIMS Communication Functions */
cJSON* unwrap_if_clothed(cJSON* obj);
int extract_valueint_of_last_array_object(cJSON* arr);
uint32_t json_to_uint32(maps* settings, cJSON* obj);
void update_holding_or_input_register_value(maps* settings, cJSON* value, uint16_t* regs, system_config* sys_cfg);
void update_variable_value(modbus_mapping_t *map, bool *reg_type, maps **settings, cJSON *value, system_config* sys_cfg);
bool process_fims_message(fims_message *msg, server_data *server_map, system_config& sys_cfg);
bool process_modbus_message(int bytes_read, int header_length, datalog *data, system_config *config, server_data *server_map, bool serial, uint8_t *query);

/* Setup */
int establish_connection(system_config *config);
bool initialize_map(server_data* server_map, system_config* sys_cfg, bool is_main_map = true);

//bool initialize_map(server_data *server_map, system_config* sys_cfg);

#endif /* _MODBUS_SERVER_H_ */
