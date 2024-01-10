Modbus_Client Design Notes: Config

p. wilshire
09_20_2023

The Modbus_client (in fact all GCOM systms after MVP) revisits the config problem.

Rather than readng a config file into a   predetermined config layout or structure the config is captures in a std::any structue.
The whole config json entity is pulled into this abstract strucure.
We are no lonvr version locked in config layouts etc.
The application will then attempt to "extract" various aspects of this abstract config into its hard coded config format.

The application can even "probe"  the config to see if it really does have the correct components for the application to function.

Consider this example :
```

            std::map<std::string, std::any> gcom_any;
            const char* filename = argv[2];
            gcom_parse_file(gcom_any, filename, true);
```

After this is run you will have an abstrct object gcom_any added to the system.
This can be a temp object if we want to recover the space.

NOTE : The incoming Fims message body  to a process can also be processed in te same way and data extracted.


now , having got the gcom_any object we can then query it to discover details.
Note that we can use the dot path to look inside the gcom_any object to find a value.
In this example the value can also be set to a default value and flag a failure if we did not specify a default and the "connection.port" ite IS required.


```
    int port;
    bool use_default true;
    bool required true;
    bool debug true;

    auto ok = getItemFromMap(gcom_any, "connection.port",       port,       503,                      use_default, required, debug);
// ok will be a null if the lookup failed.

```


In your code you can collect the "extract_xxx" operations to pull out the designated configs .
```

     // pull out connection from gcom_any
    auto ok = extract_connection(gcom_any, "connection", myCfg);

    // pull out the components into myCfg
    ok &= extract_components(gcom_any, "components", myCfg);

```


### std::any  
Processing of this type of variable is a little complex. But we hide that complexity in the accessor functions so we dont have to deal with "any" of them.

If we encounter an unhandled error then fix the accessor function so that the rproblem is fixed never  to return.

```
bool extract_components(std::map<std::string, std::any>gcom_any, const std::string& query, struct cfg& myCfg, bool debug = false) { 

     auto optArrayData = getMapValue<std::vector<std::any>>(gcom_any, query);

    if (optArrayData.has_value()) {
        if(debug) std::cout << " Found components " << std::endl;
    } else {
        std::cout << " Could not find components " << std::endl;
        return false;
    }
    bool ok = true;
    std::vector<std::any> rawarrayData = optArrayData.value();

    for(const std::any& rawElement : rawarrayData) {
            if(debug)std::cout << " Processing  item"<< std::endl;
            if (rawElement.type() == typeid(std::map<std::string, std::any>)) {
                std::map<std::string, std::any> itemMap = std::any_cast<std::map<std::string, std::any>>(rawElement);
                struct cfg::comp_struct item;
                ok &= getItemFromMapOk(itemMap, "id",           item.id,           std::string("noId"),                   true, true, debug);
                ok &= getItemFromMapOk(itemMap, "device_id",    item.device_id,    myCfg.connection.device_id,            true, true, debug);
                ok &= getItemFromMapOk(itemMap, "frequency",    item.frequency,    100,                                   true, true, debug;
                ok &= getItemFromMapOk(itemMap, "offset_time",  item.offset_time,  0,                                     true, true, debug);

                auto idIter = itemMap.find("registers");
                if (idIter != itemMap.end()) {
                    if(debug)std::cout << " Processing  registers"<< std::endl;
                    ok &= extract_registersOk(item.registers, idIter->second, item, myCfg, debug);
                }
                else
                {
                    ok = false;
                }
                if (ok)
                    myCfg.components.push_back(item);
            }
    }
    return ok;
}
```










