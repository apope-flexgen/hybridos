To handle the nested JSON structure in a clean manner, you can modularize the extraction process into smaller helper functions. Each function will handle a specific level or type of data.

Here's a step-by-step approach:

1. **Base Component Extraction**: You've already written this for extracting base-level details from components (like `id` and `device_id`).

2. **Register Extraction**: Create a separate function to extract details for each register.

3. **Map Extraction**: Similarly, create a function to extract the details of each map inside a register.

Let's organize the code:

1. **Extract Maps**:

```cpp
std::vector<cfg::map_detail> extract_maps(const std::any& rawMapData) {
    std::vector<cfg::map_detail> maps;

    if (rawMapData.type() == typeid(std::vector<std::any>)) {
        std::vector<std::any> rawMaps = std::any_cast<std::vector<std::any>>(rawMapData);
        for (const std::any& rawMap : rawMaps) {
            if (rawMap.type() == typeid(std::map<std::string, std::any>)) {
                std::map<std::string, std::any> mapData = std::any_cast<std::map<std::string, std::any>>(rawMap);

                // Extract map details (like id, offset, name, etc.) here...
                // Add to the maps vector...
            }
        }
    }

    return maps;
}
```

2. **Extract Registers**:

```cpp
std::vector<cfg::register_detail> extract_registers(const std::any& rawRegisterData) {
    std::vector<cfg::register_detail> registers;

    if (rawRegisterData.type() == typeid(std::vector<std::any>)) {
        std::vector<std::any> rawRegisters = std::any_cast<std::vector<std::any>>(rawRegisterData);
        for (const std::any& rawRegister : rawRegisters) {
            if (rawRegister.type() == typeid(std::map<std::string, std::any>)) {
                std::map<std::string, std::any> registerData = std::any_cast<std::map<std::string, std::any>>(rawRegister);

                // Extract register details (like type, starting_offset, etc.) here...
                // Call extract_maps for each register to extract map details...
                auto maps = extract_maps(registerData["map"]);

                // Add register details + extracted maps to the registers vector...
            }
        }
    }

    return registers;
}
```

3. **Integrate With Base Component Extraction**:

Inside your existing `extract_components` function, after extracting `id` and `device_id`, you can call `extract_registers` to get the details of each register:

```cpp
idIter = itemMap.find("registers");
if (idIter != itemMap.end()) {
    item.registers = extract_registers(idIter->second);
}
```

This approach breaks down the extraction process into manageable chunks, allowing you to handle different parts of your JSON data in a modular manner.