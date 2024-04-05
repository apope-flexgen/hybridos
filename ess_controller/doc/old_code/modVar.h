/*
  file: modbus_dump.cpp
   g++ -o md doc/modbus_dump.cpp -lcjson
   export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
   ./md config/clou_ess_1.json  config/dump.json config/echo.sh
*/
#ifndef MBUS_MAP_H
#define MBUS_MAP_H

// #include <inttypes.h>
// #include <modbus/modbus.h>
// #include <stdlib.h>
// #include <cjson/cJSON.h>
// #include <string.h>
// #include <sys/types.h>
// #include <unistd.h>
// #include <errno.h>
// #include <sys/wait.h>
// #include <iomanip>
// #include <fims/libfims.h>
// #include <fims/fps_utils.h>
// #include <time.h>
// #include <sys/time.h>
// #include <stdio.h>
// #include <math.h>
// #include <string>
// #include <unistd.h>

// #include <pthread.h>

// #include <cjson/cJSON.h>
// #include <vector>

#include <map>
#include <stdint.h>

#ifndef FPS_ERROR_PRINT
#define FPS_ERROR_PRINT printf
#define FPS_DEBUG_PRINT printf
#endif

// modbus maps are arranged by components.
// they are all of a certain data type.
// this will be able to read a config file into a list of components.

enum Type_of_Register
{
    None,
    Coil,
    Discrete_Input,
    Input_Register,
    Holding_Register,
    Num_Register_Types
};

// this is a single data point
typedef struct maps_t
{
    unsigned int reg_off;
    unsigned int num_regs;
    unsigned int num_strings;
    int shift;
    double scale;
    char* reg_name;
    char* uri;
    bool sign;
    bool floating_pt;
    bool is_bool;
    bool bit_field;
    bool individual_bits;
    uint64_t data;  // used only for individual_bits, read-modify-write construct
    bool enum_type;
    bool random_enum_type;
    char** bit_strings;
    std::map<int, char*> random_enum;
    Type_of_Register reg_type;

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
        reg_type = None;
    }

    ~maps_t()
    {
        if (reg_name != nullptr)
            free(reg_name);
        if (uri != nullptr)
            free(uri);
        if (bit_strings != nullptr)
        {
            for (unsigned int i = 0; i < num_regs * 16; i++)
                if (bit_strings[i] != nullptr)
                    free(bit_strings[i]);
            delete[] bit_strings;
        }
    }
} maps;

// this maybe ok
// Holds all the information related to registers whose value needs to be read
typedef struct regList_t
{
    regList_t()
    {
        register_map = nullptr;
        reg_type = None;
        num_regs = 0;
        start_offset = 0;
        map_size = 0;
    };
    ~regList_t(){};

    Type_of_Register reg_type;
    unsigned int start_offset;
    unsigned int num_regs;
    unsigned int map_size;
    maps* register_map;

} regList;

// void emit_event(fims* pFims, const char* source, const char* message, int
// severity);
cJSON* get_config_json(int argc, char* argv[]);
const char* get_reg_type(int rtype);
const char* get_reg_stype(int rtype);

int decode_type(const char* val);
// not needed
// typedef struct {
//     double cur_val;
//     double nxt_val;
//     maps *reg;
//     /* tenth of a millisecond precision */
//     long int last_write;
// } modbus_var_traffic;

// not needed we'll read it from the config but it is not used
// manages a single modbus connection
typedef struct connection_t
{
    connection_t()
    {
        name = nullptr;
        p_fims = nullptr;
        mb = nullptr;
        ip_address = nullptr;
        serial_device = nullptr;
        eth_dev = nullptr;
        dump_echo = nullptr;
        dump_server = nullptr;
    };
    ~connection_t()
    {
        if (name)
            free((void*)name);
        if (ip_address)
            free((void*)ip_address);
        if (serial_device)
            free((void*)serial_device);
        // p_fims = nullptr;
        // mb = nullptr;
        if (eth_dev)
            free((void*)eth_dev);
        if (dump_echo)
            free((void*)dump_echo);
        if (dump_server)
            free((void*)dump_server);
    };

    char* name;
    // fims *p_fims;
    // modbus_t *mb;
    pthread_mutex_t lock;
    // info for TCP connections
    char* ip_address;
    int port;
    // info for serial connections
    char* serial_device;
    char parity;
    int baud;
    int data;
    int stop;
    char* eth_dev;
    char* dump_echo;
    char* dump_server;
} connection;

// //  heartbeat stuff TODO
// typedef struct {
//     unsigned short heartbeat_val;
//     int loop_iteration;
//     int last_component_update;
//     uint64_t last_read;
//     bool component_connected;
//     int iterations_per_modbus_heartbeat;
//     int iterations_per_component_timeout;
//     maps *heartbeat_read_reg; // make sure to include in registers in config
//     maps *heartbeat_write_reg; // make sure to include in registers in config
// } heartbeat_data;

static int cc_inst = 0;

// A list of registers to be run at a specific data rate.
typedef struct component_t
{
    component_t()
    {
        FPS_ERROR_PRINT(" component_config new called inst %d\n", cc_inst);

        inst = cc_inst++;
        id = nullptr;
        read_name = nullptr;
        write_name = nullptr;
        heartbeat = nullptr;
        reg_datalog = nullptr;
    };
    ~component_t()
    {
        FPS_ERROR_PRINT(" component_config delete called inst %d\n", inst);
        if (id)
            free((void*)id);
        if (read_name)
            free((void*)read_name);
        if (write_name)
            free((void*)write_name);
        // TODO heartbeat
        // TODO reg_datalog;
    }
    int inst;
    char* id;
    bool byte_swap;
    bool off_by_one;
    // component loop frequency and time buffer for handle messages
    int frequency;
    int offset_time;
    int debounce;
    int device_id;
    // watchdog feature
    // bool heartbeat_enabled;
    // heartbeat_data *heartbeat;
    // int modbus_heartbeat_freq_ms; // in ms; should be multiple of frequency
    // int component_heartbeat_timeout_ms; // in ms; should be multiple of
    // frequency
    char* read_name;
    char* write_name;
    // registers for this component
    datalog* reg_datalog;
    int reg_cnt;
    std::map<Type_of_Register, regList*> regMap;

    // for write rate throttling
    // std::map <std::string, modbus_var_traffic*> var_map;
} component;

// TODO use a vector here too
typedef struct urimap_t
{
    std::map<std::string, maps_t*> urimap[Num_Register_Types];

} urimap;

// this may be the item given to the asset

typedef struct sys_t
{
    sys_t()
    {
        conn_info = nullptr;
        // components = nullptr;
        // num_components = 0;
    };
    ~sys_t(){
        // TODO
    };
    void add_component(component* comp);
    // int get_conn_info(sys_t *sys, cJSON *config, connection_config *connection)

    // Set up configuration for entire connection
    connection* conn_info;
    // TODO vector
    // component_config *components;
    // int num_components;
    std::vector<component*> compvec;
    // std::map<int, maps_t*>sysmaps[Num_Register_Types];
    // uri plus regs
    std::map<std::string, urimap_t*> urimaps;
    // regmaps
    // components
} sys;

typedef struct
{
    connection* connection;
    component* component;
    sys_t* sys;  // TODO
} thread_config;

/* Configuration */
int setup_register_map(sys_t* sys, cJSON* reg, const component_config* config);
bool get_conn_info(sys_t* sys,
                   cJSON* config);  //, connection_config *connection);
int get_components(sys_t* sys, cJSON* config);

/* Thread Management */
// void lock_connection(connection_config *connection, component_config
// *component);  void unlock_connection(connection_config *connection);  int
// establish_connection(connection_config *config);
void* component_loop(void* arguments);

/* FIMS Communication */
// int handle_messages(fims *p_fims, char *base_uri, cJSON *data,
// connection_config *connection, component_config *component);

/* Modbus Communication Functions */
// int set_coil(bool temp_coil_val, maps *reg, connection_config *connection,
// component_config *component);
maps* get_register(char* name, Type_of_Register reg_type, component* config);
// int set_register(double temp_reg_val, maps *reg, connection *connection,
// component *component);  bool query_device(cJSON *root, const
// connection_config *connection, const component_config *component);

/* Heartbeat Feature */
// void heartbeat_init(component_config *component);
// void heartbeat(connection_config *connection, component_config *component);

// this may be the end of the header file
// int get_comp_registers(sys_t* sys, cJSON *config, cJSON* registers,
// component_config* comp);  double get_time_dbl(void);  void
// set_base_time(void); void signal_handler (int sig);

int get_component(sys_t* sys, cJSON* cjconf, component* comp, cJSON* cjcomp);
// got to combine all register types
// void fix_maps(sys_t* sys, connection_config *cfg);
// void dump_server(sys_t* sys, connection_config *cfg);
// void dump_echo(sys_t* sys, char * fname);//, component_config* comp, int
// num);  maps* get_sys_register(sys_t *sys, const char* name, Type_of_Register
// reg_type);

// maps *get_sys_uri(sys_t *sys, const char* name, const char *uri);

#endif
