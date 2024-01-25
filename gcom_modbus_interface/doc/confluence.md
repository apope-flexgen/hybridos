
### Feature Description: FlexGen Modbus Client/Server 11.3 Release

**Release Date:** January 19, 2024

#### Overview
The latest release of the Flexgen Gcom Modbus client/server software (version 11.3) introduces enhanced capabilities for handling device IDs in Modbus systems. This update is crucial for systems utilizing both Modbus RTU and Modbus TCP protocols, particularly when dealing with complex configurations involving multiple devices with identical register mappings.

#### Key Features

1. **Device ID Support in Modbus RTU:**
   - The Modbus RTU protocol requires a  Device ID field.
   - This feature allows for precise identification of individual devices in systems where multiple devices share the same input/output bus.
   - The RS485 specification ensures that only one selected device drives the shared output line at any given time.

2. **Improved Register Mapping Flexibility:**
   - Devices can have identical register/offset  mappings, differentiated by the Device ID.
   - This functionality allows for streamlined configuration of systems with similar equipment, enhancing both setup efficiency and system robustness.

3. **Modbus TCP Compatibility:**
   - The original Modbus TCP specification, which does not utilize device IDs, is fully supported.
   - Flexgen Gcom Modbus Server defaults to a device ID of 255 (acting as a wildcard) when device IDs are not used, ensuring backward compatibility and ease of integration with existing systems.

4. **Configurable Device IDs for Vendor Systems:**
   - The FlexGen Gcom Modbus Client can now specify device IDs, facilitating communication with vendor equipment that requires a defined device ID.
   - This enhancement addresses the need to interface with various vendor-supplied systems that deviate from standard Modbus guidelines.

5. **Simulation and Testing Flexibility:**
   - When simulating or testing systems, device IDs can be omitted in the Modbus Server configurations.
   - For complex Modbus Client configurations with no duplicated device offsets, it's safe to leave the device ID out of the Server configuration, simplifying the simulation process.

6. **Updated Register Definitions:**
   - The 11.3 release allows an optional modification of the register definitions to accommodate device IDs.
   - The original register definitions can still be used but the original system does not permit duplicated register / offset pairs.
   - The register object structure is revised to allow arrays of register objects, each associated with a unique device ID.

#### Benefits

- **Enhanced Flexibility and Scalability:** The new features provide greater flexibility in configuring and managing Modbus systems, especially in complex environments with multiple devices.
- **Backward Compatibility:** Maintaining compatibility with older configurations and the original Modbus TCP spec ensures seamless integration into existing systems.
- **Simplified Configuration:** The ability to omit device IDs in certain scenarios simplifies the setup and testing processes, reducing the complexity and potential for configuration errors.

