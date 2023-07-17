/*
  file: modbus_dump.cpp
   g++ -o md doc/modbus_dump.cpp -lcjson
   export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
   ./md config/clou_ess_1.json  config/dump.json config/echo.sh
*/

#include <inttypes.h>
#include <modbus/modbus.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <iomanip>
#include <fims/libfims.h>
#include <fims/fps_utils.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include <unistd.h>

#include <pthread.h>

#include <cjson/cJSON.h>
#include <map>
#include <vector>

enum Type_of_Register {
    Coil, Discrete_Input, Input_Register, Holding_Register, Num_Register_Types
};

// this is a songle data point
typedef struct maps_t{
    unsigned int    reg_off;
    unsigned int    num_regs;
    unsigned int    num_strings;
    int    shift;
    double scale;
    char*  reg_name;
    char*  uri;
    bool   sign;
    bool   floating_pt;
    bool   is_bool;
    bool   bit_field;
    bool   individual_bits;
    uint64_t data; // used only for individual_bits, read-modify-write construct
    bool   enum_type;
    bool   random_enum_type;
    char ** bit_strings;
    std::map<int,char*> random_enum;
    //Type_of_Register 
    int reg_type;

    maps_t()
    {
        reg_name = uri = nullptr;
        bit_strings = nullptr;
        reg_off = num_regs = num_strings = 0;
        scale = 0.0;
        shift = 0;
        sign = floating_pt = is_bool = false;
        bit_field = individual_bits = enum_type = random_enum_type = false;
        data = 0;
    }
    ~maps_t()
    {
        if(reg_name != nullptr)
            free (reg_name);
        if(uri != nullptr)
            free (uri);
        if(bit_strings != nullptr)
        {
            for(unsigned int i = 0; i < num_regs * 16; i++)
                if(bit_strings[i] != nullptr) free(bit_strings[i]);
            delete [] bit_strings;
        }
    }
} maps;

// this maybe ok
//Holds all the information related to registers whose value needs to be read
typedef struct {
    Type_of_Register reg_type;
    unsigned int start_offset;
    unsigned int num_regs;
    unsigned int map_size;
    maps *register_map;
} datalog;

void emit_event(fims* pFims, const char* source, const char* message, int severity);
cJSON* get_config_json(int argc, char* argv[]);
const char* get_reg_type(int rtype);
const char* get_reg_stype(int rtype);

int decode_type(const char* val);

typedef struct {
    double cur_val;
    double nxt_val;
    maps *reg;
    /* tenth of a millisecond precision */
    long int last_write;
} modbus_var_traffic;

// manages a single modbus connection
typedef struct connection_config_t{
    connection_config_t ()
    {
        name = nullptr;
        p_fims = nullptr;
        mb = nullptr;
        ip_address = nullptr;
        serial_device = nullptr;
        eth_dev =  nullptr;
        dump_echo =  nullptr;
        dump_server =  nullptr;
    };
    ~connection_config_t ()
    {
        if(name) free((void *)name);
        if(ip_address) free((void *)ip_address);
        if(serial_device) free((void *)serial_device);
        //p_fims = nullptr;
        //mb = nullptr;
        if(eth_dev) free((void *) eth_dev);
        if(dump_echo) free((void *) dump_echo);
        if(dump_server) free((void *) dump_server);
    };

    char* name;
    fims *p_fims;
    modbus_t *mb;
    pthread_mutex_t lock;
    //info for TCP connections
    char* ip_address;
    int port;
    //info for serial connections
    char* serial_device;
    char parity;
    int baud;
    int data;
    int stop;
    char* eth_dev;
    char* dump_echo;
    char* dump_server;
} connection_config;

//  heartbeat stuff TODO
typedef struct {
    unsigned short heartbeat_val;
    int loop_iteration;
    int last_component_update;
    uint64_t last_read;
    bool component_connected;
    int iterations_per_modbus_heartbeat;
    int iterations_per_component_timeout;
    maps *heartbeat_read_reg; // make sure to include in registers in config
    maps *heartbeat_write_reg; // make sure to include in registers in config
} heartbeat_data;

int cc_inst = 0;

// A list of registers to be run at a specific data rate.
typedef struct component_config_t{
    component_config_t()
    {
        inst = cc_inst;
        FPS_ERROR_PRINT(" component_config new called inst %d\n", cc_inst++);
        id = nullptr;
        read_name = nullptr;
        write_name = nullptr;
        heartbeat = nullptr;
        reg_datalog = nullptr;
    };
    ~component_config_t()
    {
        FPS_ERROR_PRINT(" component_config delete called inst %d\n", inst);
        if(id) free((void *)id);
        if(read_name) free((void *)read_name);
        if(write_name) free((void *)write_name);
        //TODO heartbeat
        //TODO reg_datalog;
    }
    int inst;

    char *id;
    bool byte_swap;
    bool off_by_one;
    //component loop frequency and time buffer for handle messages
    int frequency;
    int offset_time;
    int debounce;
    int device_id;
    //watchdog feature
    bool heartbeat_enabled;
    heartbeat_data *heartbeat;
    int modbus_heartbeat_freq_ms; // in ms; should be multiple of frequency
    int component_heartbeat_timeout_ms; // in ms; should be multiple of frequency
    char *read_name;
    char *write_name;
    //registers for this component
    datalog *reg_datalog;
    int reg_cnt;
    // for write rate throttling
    std::map <std::string, modbus_var_traffic*> var_map;
} component_config;

// TODO use a vector here too
typedef struct urimap_t {
    std::map<std::string, maps_t*>urimap[Num_Register_Types];
    
}urimap;


typedef struct sys_t{
    sys_t() {
        conn_info = nullptr;
        //components = nullptr;
        //num_components = 0;
    };
    ~sys_t() {
        // TODO 
    };
    void add_component(component_config *comp);
//int get_conn_info(sys_t *sys, cJSON *config, connection_config *connection)

    //Set up configuration for entire connection
    connection_config *conn_info;
    //TODO vector
    //component_config *components;
    //int num_components;
    std::vector<component_config *>compvec;
    std::map<int, maps_t*>sysmaps[Num_Register_Types];
    // uri plus regs
    std::map<std::string, urimap_t*>urimaps;
    //regmaps
    //components
} sys;

typedef struct {
    connection_config* connection;
    component_config* component;
    sys_t* sys;  // TODO
} thread_config;

/* Configuration */
int setup_register_map(sys_t* sys,cJSON *reg, const component_config *config);
bool get_conn_info(sys_t *sys, cJSON *config);//, connection_config *connection);
int get_components(sys_t* sys, cJSON *config);

/* Thread Management */
void lock_connection(connection_config *connection, component_config *component);
void unlock_connection(connection_config *connection);
int establish_connection(connection_config *config);
void *component_loop(void *arguments);

/* FIMS Communication */
int handle_messages(fims *p_fims, char *base_uri, cJSON *data, connection_config *connection, component_config *component);

/* Modbus Communication Functions */
int set_coil(bool temp_coil_val, maps *reg, connection_config *connection, component_config *component);
maps *get_register(char *name, Type_of_Register reg_type, component_config *config);
int set_register(double temp_reg_val, maps *reg, connection_config *connection, component_config *component);
bool query_device(cJSON *root, const connection_config *connection, const component_config *component);

/* Heartbeat Feature */
void heartbeat_init(component_config *component);
void heartbeat(connection_config *connection, component_config *component);



sys_t sys_main;
sys_t* psys = &sys_main;


volatile bool running = true;
volatile bool modbus_reconnect = false;

long int get_time_ms()
{
    long int ltime_ms;
    timespec c_time;
    clock_gettime(CLOCK_MONOTONIC, &c_time);
    ltime_ms = (c_time.tv_sec * 1000) + (c_time.tv_nsec / 1000000);
    return ltime_ms;
}

long int get_time_us()
{
    long int ltime_us;
    timespec c_time;
    clock_gettime(CLOCK_MONOTONIC, &c_time);
    ltime_us = (c_time.tv_sec * 1000000) + (c_time.tv_nsec / 1000); 
    return ltime_us;
}
double g_base_time = 0.0;

double get_time_dbl()
{
    return  (double) get_time_us() - g_base_time;
}

void signal_handler (int sig)
{
    running = false;
    FPS_ERROR_PRINT("signal of type %d caught.\n", sig);
    signal(sig, SIG_DFL);
}

/* Configuration */
int setup_register_map(sys_t* sys, cJSON *reg, const component_config *comp)
{
    datalog* rlog;
    int index =  comp->reg_cnt;
    FPS_DEBUG_PRINT("Setting up register map for register type, index: %d\n", index);
    // get the starting offset
    cJSON* start_off = cJSON_GetObjectItem(reg, "starting_offset");
    if (start_off == nullptr)
    {
        FPS_ERROR_PRINT("Failed to find Start_Offset in JSON.\n");
        return -1;
    }
    rlog =  &comp->reg_datalog[index];
    rlog->start_offset = start_off->valueint;
    if(comp->off_by_one )
        rlog->start_offset--;

    //check to make sure that the number of bytes to read is reasonable
    if (rlog->start_offset > 65536)
    {
        FPS_ERROR_PRINT("Valid offsets are between 0 - 65536.\n");
        return -1;
    }

    //to get the start address and store it
    cJSON* cjnregs = cJSON_GetObjectItem(reg, "number_of_registers");
    if (cjnregs == nullptr)
    {
        FPS_ERROR_PRINT("Failed to find Number_of_Registers in JSON.\n");
        return -1;
    }
    rlog->num_regs = cjnregs->valueint;

    //check to make sure that the number of bytes to read is reasonable
    if (rlog->start_offset + rlog->num_regs > 65536)
    {
        FPS_ERROR_PRINT("Invalid number of registers.\n");
        return -1;
    }

    cJSON* cjtype = cJSON_GetObjectItem(reg, "type");
    if(cjtype == nullptr)
    {
        FPS_ERROR_PRINT("Register type not defined in config.\n");
        return -1;
    }
    rlog->reg_type = (Type_of_Register)decode_type(cjtype->valuestring);

    //to get the register name and its offset address
    cJSON *cjarray = cJSON_GetObjectItem(reg, "map");
    if (cjarray == nullptr)
    {
        FPS_ERROR_PRINT("Failed to find 'Map' in JSON.\n");
        return -1;
    }

    rlog->map_size  = cJSON_GetArraySize(cjarray);
    rlog->register_map = new maps [comp->reg_datalog[index].map_size];
    maps_t *rmap = rlog->register_map;

    if(rmap == nullptr)
    {
        FPS_ERROR_PRINT("Failed to allocate register map.\n");
        return -1;
    }
    int array_idx = 0;
    cJSON* arg1;
    cJSON_ArrayForEach(arg1, cjarray)
//    for (unsigned int array_idx = 0; array_idx < rlog->map_size; array_idx ++)
    {
        maps_t *ritem = &rlog->register_map[array_idx];

        cJSON* offset = cJSON_GetObjectItem(arg1,"offset");
        if (offset == nullptr)
        {
            FPS_ERROR_PRINT("Failed to get object item 'Offset' in JSON.\n");
            delete[] rmap;
            rlog->register_map = nullptr;
            return -1;
        }
        ritem->reg_off = (offset->valueint);
        if (comp->off_by_one)
            ritem->reg_off--;

        //Checking whether the offset is valid
        if (ritem->reg_off < rlog->start_offset ||
            ritem->reg_off > rlog->start_offset + rlog->num_regs )
        {
            FPS_ERROR_PRINT("offset %d is outside of registers read %d - %d.\n", ritem->reg_off, rlog->start_offset, rlog->start_offset + rlog->num_regs);
            delete[] rmap;
            rlog->register_map = nullptr;
            return -1;
        }

        cJSON* name_reg = cJSON_GetObjectItem(arg1,"id");
        if (name_reg == nullptr)
        {
            FPS_ERROR_PRINT("Failed to get object item 'Name' in JSON.\n");
            delete[] rmap;
            rlog->register_map = nullptr;
            return -1;
        }
        ritem->reg_name = strdup(name_reg->valuestring);
        ritem->reg_type = rlog->reg_type;

        cJSON* multi_reg = cJSON_GetObjectItem(arg1,"size");
        ritem->num_regs =
            (multi_reg != nullptr && multi_reg->type == cJSON_Number) ? multi_reg->valueint : 1;
        
        cJSON* floating_pt = cJSON_GetObjectItem(arg1, "float");
        ritem->floating_pt = (floating_pt != nullptr && floating_pt->type == cJSON_True);

        int string_type = 0;

        cJSON* bit_field = cJSON_GetObjectItem(arg1, "bit_field");
        ritem->bit_field = (bit_field != nullptr && bit_field->type == cJSON_True && ++string_type);

        cJSON* indv_bits = cJSON_GetObjectItem(arg1, "individual_bits");
        ritem->individual_bits = (indv_bits != nullptr && indv_bits->type == cJSON_True && ++string_type);

        cJSON* enum_type = cJSON_GetObjectItem(arg1, "enum");
        ritem->enum_type = (enum_type != nullptr && enum_type->type == cJSON_True && ++string_type);

        cJSON* random_enum_type = cJSON_GetObjectItem(arg1, "random_enum");
        ritem->random_enum_type = (random_enum_type != nullptr && random_enum_type->type == cJSON_True && ++string_type);

        if(string_type > 1)
        {
            FPS_ERROR_PRINT("More than one bit_strings type defined.\n");
            delete[] rmap;
            rlog->register_map = nullptr;
            return -1;
        }

        cJSON* bit_strings = cJSON_GetObjectItem(arg1, "bit_strings");
        if (string_type == 1)
        {
            if (bit_strings == nullptr || bit_strings->type != cJSON_Array)
            {
                FPS_ERROR_PRINT("No bit_strings object to define bit_field values.\n");
                delete[] rmap;
                rlog->register_map = nullptr;
                return -1;
            }

            int array_size = cJSON_GetArraySize(bit_strings);
            if (array_size == 0)
            {
                FPS_ERROR_PRINT("No bit_strings included in bit_strings array.\n");
                delete[] rmap;
                rlog->register_map = nullptr;
                return -1;
            }

            if (ritem->enum_type || ritem->random_enum_type)
            {
                ritem->bit_strings = new char*[array_size];
                memset(ritem->bit_strings, 0, sizeof(char*) * array_size);
            }
            else
            {
                ritem->bit_strings = new char*[ritem->num_regs * 16];
                memset(ritem->bit_strings, 0, sizeof(char*) * ritem->num_regs * 16);
            }

            if(ritem->random_enum_type)
            {
                for(int i = 0; i < array_size; i++)
                {
                     cJSON* bit_string_item = cJSON_GetArrayItem(bit_strings, i);
                     if(bit_string_item != nullptr)
                     {
                         cJSON* value = cJSON_GetObjectItem(bit_string_item, "value");
                         cJSON* bit_string = cJSON_GetObjectItem(bit_string_item, "string");
                         if(value != nullptr && value->type == cJSON_Number && bit_string != nullptr && bit_string->valuestring != nullptr)
                         {
                             ritem->bit_strings[i] = strdup(bit_string->valuestring);
                             ritem->random_enum.insert(std::pair<int, char*>(value->valueint,rmap->bit_strings[i]));
                         }
                     }
                }
            }
            else
            {
                for(int i = 0; i < array_size; i++)
                {
                     cJSON* bit_string_item = cJSON_GetArrayItem(bit_strings, i);
                     if(bit_string_item != nullptr && bit_string_item->valuestring != nullptr)
                        ritem->bit_strings[i] = strdup(bit_string_item->valuestring);
                }
            }
            ritem->num_strings = array_size;
        }
        else if (string_type == 0 && bit_strings != nullptr)
        {
            FPS_ERROR_PRINT("Bit_strings object defined but no corresponding bit_field type values.\n");
            delete[] rmap;
            rlog->register_map = nullptr;
            return -1;
        }

        cJSON* is_signed = cJSON_GetObjectItem(arg1,"signed");
        ritem->sign = (is_signed != nullptr && is_signed->type == cJSON_True);

        cJSON* scale_value = cJSON_GetObjectItem(arg1,"scale");
        ritem->scale = (scale_value != nullptr) ? (scale_value->valuedouble) : 0.0;

        cJSON* shift_value = cJSON_GetObjectItem(arg1,"shift");
        ritem->shift = (shift_value != nullptr) ? (shift_value->valueint) : 0;
        array_idx++;

    }
    return 1;
}

bool getCJint (cJSON* cj, const char* name, int& val, int def, bool required)
{
    bool ok = !required;
    cJSON *cji = cJSON_GetObjectItem(cj, name);
    if (cji) {
        val = cji->valueint;
        ok = true;
    }
    else
    {
        val = def;
    }    
    return ok;
}

bool getCJbool (cJSON* cj, const char* name, bool &val, bool def, bool required)
{
    bool ok = !required;
    cJSON *cji = cJSON_GetObjectItem(cj, name);
    if (cji) {
        val = (cji->type == cJSON_True);
        ok = true;
    }
    else
    {
        val = def;
    }    
    return ok;
}

bool getCJstr (cJSON* cj, const char* name, char**val, const char* def, bool required)
{
    bool ok = !required;
    if(*val) free((void *)*val);
    *val = nullptr;

    cJSON *cji = cJSON_GetObjectItem(cj, name);
    if (cji) 
    {
        *val = strdup(cji->valuestring);
        ok = true;
    }
    else 
    {
        if(def)
            *val = strdup(def);
    }
    return ok;
}
//TODO use the sys conn_info
bool get_conn_info(sys_t *sys, cJSON *config)//, connection_config *connection)
{
    sys->conn_info = new connection_config;
    //component_config *components = nullptr;
    bool ret = true;

    cJSON* cj = cJSON_GetObjectItem(config, "connection");
    if (cj == nullptr)
    { 
        cj = cJSON_GetObjectItem(config, "system");
    }
    if (cj == nullptr)
    { 
        FPS_ERROR_PRINT("Invalid config object.\n");
        return -1;
    }

    if(ret) ret = getCJstr(cj, "name",        &sys->conn_info->name,       "Dummy Name",     true);
    if(ret) ret = getCJstr(cj, "ip_address",  &sys->conn_info->ip_address, "127.0.0.1",      false);
    if(ret) ret = getCJint(cj, "port",        sys->conn_info->port,        502,             false);
    

    //cJSON* serial_device = cJSON_GetObjectItem(connection_json, "serial_device");
    // thise two allow us to dump a server config to match the client
    // and dump an echo config
    //cJSON* dump_server = cJSON_GetObjectItem(connection_json, "dump_server");
    //cJSON* dump_echo = cJSON_GetObjectItem(connection_json, "dump_echo");

    // if (dump_server && dump_server->valuestring)     
    // {
    //     connection->dump_server = strdup(dump_server->valuestring);
    // }
    // if (dump_echo && dump_echo->valuestring)     
    // {
    //     connection->dump_echo = strdup(dump_echo->valuestring);
    // }
    FPS_DEBUG_PRINT("Connection to use TCP. IP: %s, Port: %d\n", sys->conn_info->ip_address, sys->conn_info->port);
    //}
    // else if (serial_device != nullptr)
    // {
    //     FPS_DEBUG_PRINT("Connection to use Serial device\n");
    //     cJSON* baud_rate = cJSON_GetObjectItem(connection_json, "baud_rate");
    //     connection->baud = (baud_rate == nullptr) ? 115200 : baud_rate->valueint;
    //     cJSON* parity_str = cJSON_GetObjectItem(connection_json, "parity");
    //     connection->parity = 'N';
    //     // if parity field exists make sure it is valid
    //     if(parity_str != nullptr)
    //     {
    //         if(parity_str->valuestring == nullptr)
    //         {
    //             FPS_ERROR_PRINT("Invalid Parity value in json\n");
    //             free(connection->name);
    //             return -1;
    //         }
    //         else if(strcmp("none", parity_str->valuestring) == 0)
    //             connection->parity = 'N';
    //         else if(strcmp("even", parity_str->valuestring) == 0)
    //             connection->parity = 'E';
    //         else if(strcmp("odd", parity_str->valuestring) == 0)
    //             connection->parity = 'O';
    //         else
    //         {
    //             FPS_ERROR_PRINT("Invalid Parity value in json\n");
    //             free(connection->name);
    //             return -1;
    //         }
    //     }

    //     cJSON* data_bits = cJSON_GetObjectItem(connection_json, "data_bits");
    //     connection->data = (data_bits == nullptr) ? 8 : data_bits->valueint;
    //     if(connection->data > 8 || connection->data < 5)
    //     {
    //         FPS_ERROR_PRINT("Invalid number of data bits.\n");
    //         free(connection->name);
    //         return -1;
    //     }

    //     cJSON* stop_bits = cJSON_GetObjectItem(connection_json, "stop_bits");
    //     connection->stop = (stop_bits == nullptr) ? 1 : stop_bits->valueint;
    //     if(connection->stop > 2 || connection->stop < 1)
    //     {
    //         FPS_ERROR_PRINT("Invalid number of stop bits.\n");
    //         free(connection->name);
    //         return -1;
    //     }

    //     if(serial_device->valuestring == nullptr)
    //     {
    //         FPS_ERROR_PRINT("Invalid string for serial device.\n");
    //         free(connection->name);
    //         return -1;
    //     }
    //     connection->serial_device = strdup(serial_device->valuestring);
    //     if(connection->serial_device == nullptr)
    //     {
    //         FPS_ERROR_PRINT("Allocation error string for serial device.\n");
    //         free(connection->name);
    //         return -1;
    //     }
    // }
    // else
    // {
    //     FPS_ERROR_PRINT("Failed to find either an IP_Address or Serial_Device in JSON.\n");
    //     free(connection->name);
    //     return -1;
    // }
    return 0;
}

// two kinds of component 

//    component_config *components = component_addr;
 
// "system": {
//     "name": "CATL BMU",
//     "protocol": "Modbus TCP",
//     "version": "6.0 WIP",
//     "id": "catl_BMU",
//     "ip_address": "192.168.1.86",
//     "port": 506,
//     "data_bits": "Columns marked in green are REQUIRED"
//   },
// "registers": [
//     {
    // "type": "Holding Registers",
    //   "starting_offset": 896,
    //   "number_of_registers": 13,
    //   "map": [
    //     {
    //       "id": "ems_heartbeat",
    //       "offset": 896,
    //       "name": "MS heartbeat"
    //     }(),)
    //  }

// "components": [
//     {
//         "id": "clou_ess_1_high_speed",
//         "frequency": 16,
//         "offset_time": 2,
//         "heartbeat_enabled": true,
//         "component_heartbeat_read_uri": "life_signal",
//         "component_heartbeat_write_uri": "life",
//         "modbus_heartbeat_timeout_ms": 200,
//         "component_heartbeat_timeout_ms": 200,
            // "registers": [
            //         {
            //             "type": "Holding Registers",

// so we need parse_component and parse registers



int get_comp_registers(sys_t* sys, cJSON *config, cJSON* registers, component_config* comp);


int get_component(sys_t* sys, cJSON *config, component_config* comp, cJSON *component)
{
    int rc = 0;
    bool ret = true;
    cJSON * cj = component;
    if(ret) ret = getCJstr(cj,  "id",          &comp->id,         "Dummy Name",     true);
    if(ret) ret = getCJbool(cj, "byte_swap",   comp->byte_swap,  false,          false);
    if(ret) ret = getCJbool(cj, "off_by_one",  comp->off_by_one, false,          false);
    if(ret) ret = getCJint(cj,  "frequency",   comp->frequency,  500,            false);
    if(ret) ret = getCJint(cj,  "offset_time", comp->offset_time, 100,           false);
    if(ret) ret = getCJint(cj,  "debounce",    comp->debounce, 0,                false);
    comp->debounce *= 10;
    if(ret) ret = getCJint(cj, "device_id", comp->device_id, -1,                 false);

    // if(comp->device_id < 0 || comp->device_id > 255)
    // {
    //     FPS_ERROR_PRINT("Invalid Device_ID.\n");
    //     free(&comp->id);
    //     return -1;
    // }

    if(ret) ret = getCJbool(cj, "heartbeat_enabled", comp->heartbeat_enabled, false,     false);

    if (comp->heartbeat_enabled)
    {
        if(ret) ret = getCJint(cj, "modbus_heartbeat_freq_ms",       comp->modbus_heartbeat_freq_ms,       500,     false);
        if(ret) ret = getCJint(cj, "component_heartbeat_timeout_ms", comp->component_heartbeat_timeout_ms, 500,     false);

        if(ret) ret = getCJstr(cj, "component_heartbeat_read_uri",   &comp->read_name,                      nullptr,    true);
        if (comp->read_name == nullptr)
        {
            FPS_ERROR_PRINT("No heartbeat read register provided.\n");
            delete comp;
            //free(&comp->id);
            return -1;
        }

        // heartbeat write register is optional
        if(ret) ret = getCJstr(cj, "component_heartbeat_write_uri",  &comp->write_name,                    nullptr,     false);
    }

    //parsing the configuration file to get the Type of the registers, its start and end address, and to get the value of the specified registers
    //We first get the object and then store it in the data structure
    //to get Register object
    cJSON *cjreg = cJSON_GetObjectItem(component, "registers");
    if (cjreg == nullptr)
    {
        FPS_ERROR_PRINT("Failed to get object item 'registers' in JSON.\n");
        //cJSON_Delete(component);
        return 1;
    }
    return get_comp_registers(sys, config, cjreg, comp);
}

int get_comp_registers(sys_t* sys, cJSON *config, cJSON* cjreg, component_config* comp)
{
    int rc = 0;
    // int get_comp_registers(sys_t* sys,component_config* comp, cJSON* registers, cJSON* component)
    //to get the size of the array
    int total_regs = cJSON_GetArraySize(cjreg);
    if (total_regs == 0)
    {
        FPS_ERROR_PRINT("No registers to be mapped in the json file\n");
        //cJSON_Delete(cjcomp);
        return 1;
    }
    FPS_ERROR_PRINT("%d registers are mapped in the json file for comp_id [%s]\n", total_regs, comp->id);

    //dynamic array based on the register type entries created
    comp->reg_datalog = new datalog [total_regs];
    if( comp->reg_datalog == nullptr)
    {
        FPS_ERROR_PRINT("Memory allocation error\n");
        //cJSON_Delete(cjcomp);
        rc = 1;
    }
    comp->reg_cnt = 0;

    cJSON* reg;
    cJSON_ArrayForEach(reg, cjreg)
    {
        FPS_ERROR_PRINT(" getting reg set %d\n", comp->reg_cnt);
        rc = setup_register_map(sys, reg, comp);
        FPS_ERROR_PRINT(" setup  array item  %d rc %d\n", comp->reg_cnt, rc);
        if(rc == -1)
        {
            FPS_ERROR_PRINT("Failed to setup register map.\n");
            //cJSON_Delete(config);
            //comp->reg_cnt--;
            rc = 1;
        }
        else
        {
            comp->reg_cnt++;
        }
        
    }
    return rc;
}

//int get_component(sys_t* sys, cJSON *config, component_config* comp, cJSON *component)

void sys_t::add_component(component_config *comp)
{
    compvec.push_back(comp);
}

int get_components(sys_t* sys, cJSON *config)
{
    int rc = 0;
    cJSON* cji;
    cJSON *cjcomp = cJSON_GetObjectItem(config, "components");
    cJSON_ArrayForEach(cji, cjcomp)
    {
        component_config *comp = new component_config();
        get_component(sys, config, comp, cji); 
        sys->add_component(comp);
        rc++;
    }
    FPS_ERROR_PRINT(" got components %d\n", rc);

    return rc;

}

// this it like the get / set from fims
maps *get_sys_uri(sys_t *sys, const char* name, const char *uri)
{
    //std::map<std::string, urimap_t*>:: iterator it;
    if (sys->urimaps.find(uri) != sys->urimaps.end())
    {
        // it->fisrt == uri
        // it->second == urimap_t*
        urimap_t* umap = sys->urimaps[uri];
        FPS_ERROR_PRINT(" seeking name [%s] in uri [%s] \n"
                            , name,  uri); 

        //}urimap;
        // could use reg_tpe here
        for (int i = 0 ; i < Num_Register_Types; i++)
        {
            if(umap->urimap[i].find(name) != umap->urimap[i].end())
            {
                maps_t* ritem = umap->urimap[i][name];
                FPS_ERROR_PRINT(" found [%s] type %d in uri [%s] offset %d\n"
                            , name, 1, uri ,ritem->reg_off ); 
            }
        }

    }
     return nullptr;
}

// now get_register looks a bit different 
maps* get_sys_register(sys_t *sys, const char* name, Type_of_Register reg_type) 
{
    //we have a number of components
    //fix_maps
    //component_config* pcomp;
    //datalog* reg_datalog;
    //datalog* regd;
    //int cnum = sys->compvec.size();
    // foreach comp
    //for (int i = 0 ; i < cnum; i++)
    //{
    //pcomp = sys->compvec[i];
        // each component has a list of registers datalog *
        // this maybe ok
        //Holds all the information related to registers whose value needs to be read
        // typedef struct {
        // Type_of_Register reg_type;
        // unsigned int start_offset;
        // unsigned int num_regs;
        // unsigned int map_size;
        // maps *register_map;
        // } datalog;
        // and here they are
        //  for (int j = 0 ; j < pcomp->reg_cnt; j++)
        // {
        //     regd = &reg_datalog[j];
        //     FPS_ERROR_PRINT("Component [%d] id [%s]  regd %p map_size %d\n", i, pcomp->id,  regd, regd?regd->map_size:0);
        //     fprintf(stdout,"   [%s] \n",get_reg_stype(regd->reg_type));


        //     //    regdata [%d] type [%s] start [%d] num %d map_size [%d].\n"
        //     //                 , i, get_reg_stype(regd->reg_type), (int)regd->start_offset
        //     //                 , (int)regd->num_regs
        //     //                 , (int)regd->map_size
        //     //                 );
        //     // for each reg
        //     for (int k = 0; k < (int)regd->map_size; k++)
        //     {
        //         maps_t* ritem = &regd->register_map[k];
        // but that is a tiresome search
        //  this lot may be better
        //           add_sys_sysmap(sys, ritem);
        //   nope this is an index by type :: offset
        //         sys->sysmaps[ritem->reg_type][(int)ritem->reg_off] = ritem;
        //   add_sys_urimap(sys, ritem);
        // check for uri in urimaps
        // if (sys->urimaps.find(ritem->uri) == sys->urimaps.end())
        // {
        //     FPS_ERROR_PRINT("adding new uri [%s].\n", ritem->uri);
        //     sys->urimaps[ritem->uri] =  new urimap[Num_Register_Types];
        // }
        // //std::map<char *, urimap_t*>urimapx;

        // urimap_t*urimapx = sys->urimaps[ritem->uri];
        // urimapx->urimap[ritem->reg_type][ritem->reg_name] =  ritem;
        
        // best bet is to search through the uri maps for the var

        std::map<std::string, urimap_t*>:: iterator it;
        for (it = sys->urimaps.begin(); it != sys->urimaps.end(); ++it)
        {
            // it->fisrt == uri
            // it->second == urimap_t*
            urimap_t* umap = it->second;
            //typedef struct urimap_t {
            //  std::map<std::string, maps_t*>urimap[Num_Register_Types];
            FPS_ERROR_PRINT(" seeking name [%s] in uri [%s] \n"
                                , name,  it->first.c_str()); 
    
            //}urimap;
            // could use reg_tpe here
            for (int i = 0 ; i < Num_Register_Types; i++)
            {
                if(umap->urimap[i].find(name) != umap->urimap[i].end())
                {
                    maps_t* ritem = umap->urimap[i][name];
                    FPS_ERROR_PRINT(" found [%s] type %d in uri [%s] offset %d\n"
                                , name, i, it->first.c_str(),ritem->reg_off ); 
                }
            }

        }


    return nullptr;
}
maps* get_register (char* name, Type_of_Register reg_type, component_config* config)
{
    for ( int reg_set = 0; reg_set < config->reg_cnt; reg_set++)
    {
        if(config->reg_datalog[reg_set].reg_type == reg_type)
        {
            for (unsigned int i = 0;  i < config->reg_datalog[reg_set].map_size; i++)
            {
                if (strcmp(name, config->reg_datalog[reg_set].register_map[i].reg_name) == 0)
                    return &config->reg_datalog[reg_set].register_map[i];
                else if (config->reg_datalog[reg_set].register_map[i].individual_bits) // override for individual_bits, register name read as bit_strings
                {
                    for (size_t j = 0; j < config->reg_datalog[reg_set].register_map[i].num_strings; j++)
                    {
                        if(config->reg_datalog[reg_set].register_map[i].bit_strings[j] != nullptr)
                            if(strcmp(name, config->reg_datalog[reg_set].register_map[i].bit_strings[j]) == 0)
                                return &config->reg_datalog[reg_set].register_map[i];
                    }
                }
            }
        }
    }
    return nullptr;
}

/* Main Thread */

// TODO place all the components and reg_types in an overall system object
void add_sys_sysmap(sys_t *sys, maps_t *ritem)
{
    sys->sysmaps[ritem->reg_type][(int)ritem->reg_off] = ritem;
}

// TODO place all the components and reg_types in an overall system object
void add_sys_urimap(sys_t *sys, maps_t *ritem)
{
    // check for uri in urimaps
    if (sys->urimaps.find(ritem->uri) == sys->urimaps.end())
    {
        FPS_ERROR_PRINT("adding new uri [%s].\n", ritem->uri);
        sys->urimaps[ritem->uri] =  new urimap[Num_Register_Types];
    }
    //std::map<char *, urimap_t*>urimapx;

    urimap_t*urimapx = sys->urimaps[ritem->uri];
    urimapx->urimap[ritem->reg_type][ritem->reg_name] =  ritem;
}

// got to combine all register types
void fix_maps(sys_t* sys, connection_config *cfg)
{
    component_config* pcomp;
    datalog* reg_datalog;
    datalog* regd;
    int cnum = sys->compvec.size();
    // foreach comp
    for (int i = 0 ; i < cnum; i++)
    {
        pcomp = sys->compvec[i];
        //comp->reg_cnt--;
        FPS_ERROR_PRINT("Component [%d] id [%s]  regd cnt %d\n", i, pcomp->id, pcomp->reg_cnt);
        reg_datalog = pcomp->reg_datalog;
        //foreach regtype ??
        for (int j = 0 ; j < pcomp->reg_cnt; j++)
        {
            regd = &reg_datalog[j];
            FPS_ERROR_PRINT("Component [%d] id [%s]  regd %p map_size %d\n", i, pcomp->id,  regd, regd?regd->map_size:0);
            fprintf(stdout,"   [%s] \n",get_reg_stype(regd->reg_type));


            //    regdata [%d] type [%s] start [%d] num %d map_size [%d].\n"
            //                 , i, get_reg_stype(regd->reg_type), (int)regd->start_offset
            //                 , (int)regd->num_regs
            //                 , (int)regd->map_size
            //                 );
            // for each reg
            for (int k = 0; k < (int)regd->map_size; k++)
            {
                maps_t* ritem = &regd->register_map[k];
                if(ritem->uri == nullptr)
                    ritem->uri = strdup(pcomp->id);
                if(psys->sysmaps[regd->reg_type].find((int)ritem->reg_off) 
                        != psys->sysmaps[regd->reg_type].end()) 
                {
                fprintf(stdout, 
                    " name [%s] already found at index %d\n"
                    , ritem->reg_name
                    , ritem->reg_off
                    );

                }
                else
                {
                    //psys->sysmaps[regd->reg_type][(int)ritem->reg_off] = ritem;
                    // check for reg in urimap
                    add_sys_sysmap(sys, ritem);
                    add_sys_urimap(sys, ritem);
                }
            }
        }
    }
}

// got to combine all register types
void dump_server(sys_t* sys, connection_config *cfg)
{
    component_config* pcomp;
    datalog* reg_datalog;
    datalog* regd;
    FILE*fp = stdout;
    //std::map<int,maps_t*>mymaps[Num_Register_Types];
    std::map<int,maps_t*>::iterator it;
    
    if(cfg->dump_server)
    {
        fp = fopen (cfg->dump_server, "w");
        if(!fp)
        {
            FPS_ERROR_PRINT("unable to open server_dump[%s].\n", cfg->dump_server);
            return;
        }
    }
    fprintf(fp,"{\n   \"system\":\n");
    fprintf(fp,"   {   \"name\":\"%s\",\n",cfg->name);  
    fprintf(fp,"       \"protocol\": \"Modbus TCP\",\n");
    fprintf(fp,"       \"id\": \"%s\",\n",cfg->name);     //TODO
    fprintf(fp,"       \"ip_address\": \"%s\",\n",cfg->ip_address); 
    fprintf(fp,"       \"port\": %d \n",cfg->port);  
    fprintf(fp,"   },\n");
    fprintf(fp,"    \"registers\":\n     {\n");
    // // foreach(component)
    // for (int i = 0 ; i< num; i++)
    // {
    //     pcomp = &comp[i];
    //     //comp->reg_cnt--;
    //     FPS_ERROR_PRINT("Component [%d] id [%s] comp num %d regd cnt %d\n", i, pcomp->id, num, pcomp->reg_cnt);
    //     reg_datalog = pcomp->reg_datalog;
    //     //foreach regtype ??
    //     for (int j =0 ; j < pcomp->reg_cnt; j++)
    //     {
    //         regd=&reg_datalog[j];
    //         FPS_ERROR_PRINT("Component [%d] id [%s] num %d regd %p map_size %d\n", i, pcomp->id, num, regd, regd?regd->map_size:0);
    //         fprintf(stdout,"   [%s] \n",get_reg_stype(regd->reg_type));


    //         //    regdata [%d] type [%s] start [%d] num %d map_size [%d].\n"
    //         //                 , i, get_reg_stype(regd->reg_type), (int)regd->start_offset
    //         //                 , (int)regd->num_regs
    //         //                 , (int)regd->map_size
    //         //                 );
    //         // for each reg
    //         for (int j = 0; j < (int)regd->map_size; j++)
    //         {
    //             maps_t* ritem = &regd->register_map[j];
    //             if(ritem->uri == nullptr)
    //                 ritem->uri = strdup(pcomp->id);
    //             if(psys->sysmaps[regd->reg_type].find((int)ritem->reg_off) 
    //                     != psys->sysmaps[regd->reg_type].end()) 
    //             {
    //             fprintf(stdout, 
    //                 " name [%s] already found at index %d\n"
    //                 , ritem->reg_name
    //                 , ritem->reg_off
    //                 );

    //             }
    //             else
    //             {
    //                 //psys->sysmaps[regd->reg_type][(int)ritem->reg_off] = ritem;
    //                 // check for reg in urimap
    //                 add_sys_sysmap(sys, ritem);
    //                 add_sys_urimap(sys, ritem);
    //             }
    //         }
    //     }
    // }
    int last_reg = -1;
    for (int j = 0 ; j < Num_Register_Types ; j++ )
    {
        if(psys->sysmaps[j].size() > 0) 
           last_reg = j;
    }
    for (int j = 0 ; j < Num_Register_Types ; j++ )
    {
        if(psys->sysmaps[j].size() > 0) 
        {
            fprintf(fp,"      \"%s\":\n       [\n",get_reg_stype(j));
            for (it = psys->sysmaps[j].begin(); it != psys->sysmaps[j].end(); ++it)
            {
               maps_t * rmap = it->second;
                if (it != psys->sysmaps[j].begin())
                {
                    fprintf(fp,",");
                    fprintf(fp,"\n");
                }
                fprintf(fp,
                    "          { \"name\":\"%s\",\"id\":\"%s\",\"offset\":%d,"
                                    , rmap->reg_name, rmap->reg_name,(int)rmap->reg_off) ;
                if(rmap->scale != 0.0)
                {
                    fprintf(fp,
                        "\"scale\": %f, "
                        , rmap->scale) ;
                }
                if(rmap->shift != 0)
                {
                    fprintf(fp,
                        "\"shift\": %d, "
                        , rmap->shift) ;
                }
                if(rmap->num_regs != 1)
                {
                    fprintf(fp,
                        "\"size\": %d, "
                        , rmap->num_regs) ;
                }

                // signed
                if(rmap->sign)
                {
                    fprintf(fp,
                        "\"signed\": true, "
                        );
                }

                fprintf(fp,
                    "\"uri\":\"/components/%s\" }"
                    , rmap->uri) ;
            }
            fprintf(fp,"       ]");
            if(j < last_reg)
                fprintf(fp,",");
            fprintf(fp,"\n");
        }
    }
    fprintf(fp,"    }\n}\n");
    fclose(fp);
}

void dump_echo(sys_t* sys, char * fname)//, component_config* comp, int num)
{
    component_config *pcomp;
    datalog *reg_datalog;
    datalog *regd;
    FILE*fp = stdout;
    if(fname)
    {
        fp = fopen (fname, "w");
        if(!fp)
        {
            FPS_ERROR_PRINT("unable to open echo_dump[%s].\n", fname);
            return;
        }
    }
    int mcount = 0;
    {
        //FPS_ERROR_PRINT("adding new uri [%s].\n", ritem->uri);
        //sys->urimaps[ritem->uri] =  new urimap[Num_Register_Types];
        maps_t*ritem;
        //foreach urimap
        for (std::map<std::string, urimap_t*>::iterator it = sys->urimaps.begin(); it != sys->urimaps.end(); ++it)
        {
            mcount++;
            urimap_t *uitem = it->second;
            // typedef struct urimap_t {
            //     std::map<std::string, maps_t*>urimap[Num_Register_Types];
            // }urimap;
            int phead = 1;

            for (int i = 0 ; i < Num_Register_Types; i++)
            {
                // get the register map for datatype (i)
                std::map<std::string, maps_t*> *imap = &uitem->urimap[i];
                FPS_ERROR_PRINT(" uri  [%s] type %d size (%d)\n"
                    , it->first.c_str()
                    , i
                    , (int)imap->size()
                    );
                if ((int)imap->size() > 0)
                {
                    if(phead == 1)
                    {
                        phead = 0;
                        fprintf(fp, "/usr/local/bin/fims/fims_echo -u /components/%s -b '{\n"
                                              ,it->first.c_str());
                    }
                    //map for each
                    // the map should already have the min offset at the hed and the max offset at the end
                    std::map<std::string, maps_t*>::iterator xit;
                    int k = 0;
                    int min_offset;
                    int max_offset;
                    for (xit = imap->begin(); xit != imap->end();++xit)
                    {

                        maps_t* ritem = xit->second;
                        if(k == 0) 
                        {
                            min_offset = ritem->reg_off;
                            max_offset = ritem->reg_off;
                        }
                        if (max_offset < ritem->reg_off + ritem->num_regs)
                            max_offset = ritem->reg_off + ritem->num_regs;
                        if (min_offset > ritem->reg_off)
                            min_offset = ritem->reg_off;
                            
                        FPS_ERROR_PRINT("  ....  name [%s]\n", ritem->reg_name);
                        fprintf(fp," \"%s\":{\"value\":0}"
                                    , ritem->reg_name) ;
                        if(k <(int)imap->size()-1)
                            fprintf(fp,",");
                        fprintf(fp,"\n");
                        k++;
                    }
                    FPS_ERROR_PRINT("  ....  name [%s] min %d max %d \n", it->first.c_str(), min_offset, max_offset);
                }
            }
            if(phead == 0)
            {
                fprintf(fp,"}'&\n");
            }
        }

    }
    FPS_ERROR_PRINT(" urimaps mcount (%d)\n", mcount);
    fclose(fp);
}

cJSON* get_config_json(int argc, char* argv[])
{
    FILE *fp = nullptr;
    if(argc <= 1)
    {
        FPS_ERROR_PRINT("Need to pass argument of name.\n");
        return nullptr;
    }

    if (argv[1] == nullptr)
    {
        FPS_ERROR_PRINT(" Failed to get the path of the config file. \n");
        return nullptr;
    }

    fp = fopen(argv[1], "r");
    if(fp == nullptr)
    {
        FPS_ERROR_PRINT("Failed to open file %s\n", argv[1]);
        return nullptr;
    }

    fseek(fp, 0, SEEK_END);
    long unsigned file_size = ftell(fp);
    rewind(fp);

    // create Configuration_file and read file in Configuration_file
    char *config_file = new char[file_size];
    if(config_file == nullptr)
    {
        FPS_ERROR_PRINT("Memory allocation error\n");
        fclose(fp);
        return nullptr;
    }

    size_t bytes_read = fread(config_file, 1, file_size, fp);
    fclose(fp);
    if(bytes_read != file_size)
    {
        FPS_ERROR_PRINT("Read size error.\n");
        delete[] config_file;
        return nullptr;
    }

    cJSON* config = cJSON_Parse(config_file);
    delete[] config_file;
    if(config == nullptr)
        FPS_ERROR_PRINT("Invalid JSON object in file\n");
    return config;
}

const char * reg_types[] = {
    "Coil", "Discrete_Input", "Input_Register", "Holding_Register"
};
const char * reg_stypes[] = {
    "coils", "discrete_inputs", "input_registers", "holding_registers"
};

int decode_type(const char* val)
{
    if(strcmp(val, "Coils") == 0)
        return Coil;
    else if(strcmp(val, "Discrete Inputs") == 0)
        return Discrete_Input;
    else if(strcmp(val, "Input Registers") == 0)
        return Input_Register;
    else if(strcmp(val, "Holding Registers") == 0)
        return Holding_Register;
    else
    {
        FPS_ERROR_PRINT("Invalid register type.\n");
        return -1;
    }
}

const char* get_reg_type(int rtype)
{
    return reg_types[rtype];

}
const char* get_reg_stype(int rtype)
{
    return reg_stypes[rtype];

}
int main(int argc,char *argv[])
{
    const char* dump_file =  nullptr;
    const char* echo_file = nullptr;

    double dtime  = get_time_dbl();
    g_base_time = dtime;


    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    
    //Set up configuration for entire connection
    //psys->conn_info = new connection_config;
    component_config *components = nullptr;
    //init_connection_config(psys->conn_info);
   
    //read in and parse config file to json
    cJSON* config = get_config_json(argc, argv);
    if(config == nullptr)
    {
        FPS_ERROR_PRINT("Could not get config json from file.\n");
        return 1;
    }
    if(argc > 2)
      dump_file = argv[2];
    if(argc > 3)
      echo_file = argv[3];

    //obtain connection information
    bool conn_val = get_conn_info(psys, config);//, conn_info);
    if(conn_val == -1)
    {
        FPS_ERROR_PRINT("Could not get connection info from config json.\n");
        return 1;
    }
    int num_components = 0;
// we either have "registers" or "components" at this time
// if we see registers it is a single component or single threaded system.
   
    cJSON *cjsys = cJSON_GetObjectItem(config, "system");
    cJSON *cjcomp = cJSON_GetObjectItem(config, "components");
    if(cjsys && !cjcomp)
    {
        cJSON *cjreg = cJSON_GetObjectItem(config, "registers");
        component_config *comp = new component_config();
        get_component(psys, config, comp, cjreg); 
        psys->add_component(comp);
        //component_config* comp = get_component(psys, config, cjsys);
        num_components = 1; 
        get_comp_registers(psys, config, cjreg, comp);
    }
    else
    {    
        //obtain information for components, including register maps
        num_components = get_components(psys, config);
        FPS_ERROR_PRINT("Done with get_components num %d\n", num_components);
    }

    if(num_components <= 0)
    {
        FPS_ERROR_PRINT("Could not get components from config json.\n");
        cJSON_Delete(config);
        return 1; 
    }

    FPS_ERROR_PRINT("Running fix maps >>>>>\n");
    fix_maps(psys, psys->conn_info);
    FPS_ERROR_PRINT("<<<<<Completed fix maps \n");

    if(dump_file)
    { 
        psys->conn_info->dump_server = strdup(dump_file);
    }
    if(psys->conn_info->dump_server)
    {
        FPS_ERROR_PRINT("Dump Server to [%s]\n",psys->conn_info->dump_server);
        dump_server(psys, psys->conn_info);
    }
    if(echo_file)
    { 
        psys->conn_info->dump_echo = strdup(echo_file);
    }

    if(psys->conn_info->dump_echo)
    {
        FPS_ERROR_PRINT("Dump Echo to [%s]\n",psys->conn_info->dump_echo);
        dump_echo(psys, psys->conn_info->dump_echo);
        FPS_ERROR_PRINT("Done with Dump Echo\n");
    }
    // now get_register looks a bit different 
    get_sys_register(psys, "sbmu1_soc", Coil/*not used*/); 
    // 
    get_sys_uri(psys, "sbmu1_soc", "catl_smbu_warn_r"); 

    FPS_ERROR_PRINT("Done with Dumps\n");
    cJSON_Delete(config);

    delete psys->conn_info;
    return 0;
}
