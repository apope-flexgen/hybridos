# Programmatic API: Integrating Documentation into an Existing System

P. Wilshire
4/4/2024

## Overview

Programmatic API documentatin embedds systm documentation into the compiled executables in this example we extend an existing configuration structure (`cfg`) within a system to include details of a system to modify fims requests. 
This enhancement enables not just the execution of predefined actions but also facilitates runtime access to comprehensive documentation for each action. It is designed to increase the system's interactivity, transparency, and usability by allowing users to programmatically query and understand the capabilities available to them.


## Existing System Configuration (`cfg`)

The `cfg` structure is the existing  system configuration, encapsulating various settings and operational parameters. The integration of the Programmatic API introduces a new structure to `cfg`, embedding a mechanism for action definition and documentation.

### Updated `cfg` Structure

With the Programmatic API, the `cfg` structure now includes:

```cpp
struct cfg {
    // Preserved elements of the original cfg structure
    std::unordered_map<std::string, std::shared_ptr<request>> requests;
};
```

- **Requests Map**: Associates action names with their corresponding `request` objects, each representing a distinct action within the system.

## Key Components of the Requests System

### `request` Structure

A `request` object is the atomic unit in the Requests System, characterized by:

- **Name**: The action's unique identifier.
- **Help**: A textual description providing insights into what the action does and how to use it.
- **Function**: An executable function encapsulating the action's logic. ( this is optional)

```cpp
struct request {
    std::string name;
    std::string help;
    std::function<bool(cfg&, std::shared_ptr<request>)> reqfun;
};
```

### Added Functions

#### `register_request`

Registers new actions within the `cfg` structure, ensuring the system's capabilities are both extendable and documented.

```cpp
void register_request(const std::string& name, const std::string& help, std::function<bool(cfg&, std::shared_ptr<request>)> reqfun, cfg& myCfg);
```

- **Purpose**: Dynamically adds new or updates existing actions, keeping the system's functional documentation current.

#### `showRequestHelp`

Compiles and presents a JSON-formatted summary of all actions registered within the system.

```cpp
void showRequestHelp(cfg& myCfg, std::stringstream &ss);
```

- **Purpose**: Offers an immediate, readable reference to the system's capabilities, enhancing user understanding and interaction.

## Practical Example: Enhancing System Documentation

Illustrating the dynamic registration of actions and the extraction of their documentation:

```cpp
// Assuming an initialized and configured system 'myCfg'
cfg myCfg;

// Example: Registering actions
bool register_requests(cfg& myCfg) {
    auto reqFun = std::bind(voidRequestFcn, std::placeholders::_1, std::placeholders::_2);
    register_request("_help",             "Show request options",                                         reqFun, myCfg);
    register_request("_full",             "Show full data point details",                                 reqFun, myCfg);
    register_request("_local",            "Show the local data point details",                            reqFun, myCfg);
    register_request("_enable",           "Enable data point",                                            reqFun, myCfg);
    register_request("_disable",          "Disable data point",                                           reqFun, myCfg);
    register_request("_connect",          "Try to connect data point",                                    reqFun, myCfg);
    register_request("_disconnect",       "Disconnect  data point",                                       reqFun, myCfg);
    register_request("_force",            "Force data point values (points and values defined in body)",  reqFun, myCfg);
    register_request("_unforce",          "Unforce data point values (points defined in body)",           reqFun, myCfg);
    register_request("_stats",            "Get Stats",                                                    reqFun, myCfg);
    register_request("_server",           "Get server config",                                            reqFun, myCfg);
    register_request("_debug_connection", "(true/false) Set connection debug ",                           reqFun, myCfg);
    register_request("_debug_response",   "(true/false) Set  response debug ",                            reqFun, myCfg);

    return true;
}

// Example: Accessing documentation

void showRequestHelp(struct cfg& myCfg, std::stringstream &ss)
{
    int count = 0;

    ss << "\"help\":[";
    for (const auto& req: myCfg.requests)
    {
        if(count>0)
        {
             ss<< ',';
        }
        count++;

        ss << "{\"name\" :\"" << req.second->name <<"\","; 
        ss << "\"desc\" :\"" << req.second->help <<"\"}"; 
    }
    ss << "]";
}

// create a request structure, use shared pointers make then auto destruct
std::shared_ptr<cfg::request> make_shared_request(const std::string& name, const std::string& help,
                     std::function<bool(struct cfg&, std::shared_ptr<cfg::request>)> reqfun) {
    return std::make_shared<cfg::request>(name, help, reqfun);
}
// register a request. 
// note that this can be done while the system  is running.
// it would also be possible to remove request options if needed.

void register_request(const std::string& request, const std::string& help
            , std::function<bool(struct cfg&, std::shared_ptr<cfg::request>)> reqfun, struct cfg& myCfg)
{
    if ( myCfg.requests.find(request) == myCfg.requests.end())
    {
        myCfg.requests[request] = make_shared_request(request, help, reqfun); 
    }
}

bool voidRequestFcn(struct cfg& myCfg,std::shared_ptr<cfg::request> req)
{
    std::cout << "running request :" << req->name << std::endl; 
    return true;
}

```

In this example, the requests object refers to a fims uri modifier.
For example, a fims message is normally sent to the system  as shown

```
fims_send -m get -r /$$ -u /components/flexgen_ess_65_ls | jq
{
  "life": 1,
  "start_stop": 0,
  "run_mode": 0,
  "on_off_grid_mode": 0,
  "active_power_setpoint": 0,
  "reactive_power_setpoint": 0,
  "clear_faults": false,
  "bms_dc_contactors": 0,
  "life_signal": 0,
  "chargeable_energy": 0,
  "dischargeable_energy": 0,
  "chargeable_power": 0,
  "dischargeable_power": 0,
  "active_power": 0,
  "reactive_power": 0,
  "apparent_power": 0,
  "pf": 0,
  "voltage_dc": 0,
  "current_dc": 0,
  "active_power_dc": 0,
  "bms_maximum_cell_voltage": 0,
  "bms_minimum_cell_voltage": 0,
  "bms_average_cell_voltage": 0,
  "bms_maximum_cell_temperature": 0,
  "bms_minimum_cell_temperature": 0,
  "bms_average_cell_temperature": 0,
  "dc_contactors_closed": false,
  "racks_in_service": 0,
  "dc_charging": false,
  "dc_discharging": false,
  "bms_fault_active": false,
  "bms_max_container_temp": 0,
  "bms_alarm_active": false,
  "bms_1_alarms1": [],
  "bms_1_faults1": [],
  "sbmu_1_alarms": [],
  "sbmu_1_faults": [],
  "sbmu_2_alarms": [],
  "sbmu_2_faults": [],
  "ess_2_faults": [],
  "ess_2_alarms": [],
  "bms_2_alarms1": [],
  "bms_2_faults1": [],
  "bms_2_sbmu_1_alarms": [],
  "bms_2_sbmu_1_faults": [],
  "bms_2_sbmu_2_alarms": [],
  "bms_2_sbmu_2_faults": []
 
}
```

The requests system will modify the meaning of the initial uri to provide alternative information.

In this example the "_help" request is used to get details of the other options available.

```
fims_send -m get -r /$$ -u /components/flexgen_ess_65_ls/_help | jq
{
  "help": [
    { "name": "_debug_connection","desc": "(true/false) Set connection debug "},
    { "name": "_server",           "desc": "Get server config"},
    { "name": "_enable",           "desc": "Enable data point"},
    { "name": "_full",             "desc": "Show full data point details"},
    { "name": "_connect",           "desc": "Try to connect data point"},
    { "name": "_help",              "desc": "Show request options"},
    { "name": "_disconnect",        "desc": "Disconnect  data point"},
    { "name": "_disable",            "desc": "Disable data point"},
    { "name": "_force",              "desc": "Force data point values (points and values defined in body)"},
    { "name": "_local",              "desc": "Show the local data point details"},
    { "name": "_unforce",            "desc": "Unforce data point values (points defined in body)"},
    { "name": "_debug_response",     "desc": "(true/false) Set  response debug "},
    { "name": "_stats",              "desc": "Get Stats"}
  ]
}
```

## Enhancements 
The requests structure, in this example could be extended to control more of the requests decode. The string to decode and the function to run could be defined using the structure.
In this case the existing code was left intact, the extra code was added to provide run time documentation of the system.  

## Conclusion

The integration of the Programmatic API concept into an existing system not only broadens the system's capabilities through the dynamic registration of actions but incorporates an essential aspect of self-documentation. This approach significantly enhances usability, allowing both developers and end-users to interact with the system more effectively, with a clear understanding of its functionalities. By embedding documentation directly within the operational context, the system becomes more transparent, adaptable, and easier to navigate.