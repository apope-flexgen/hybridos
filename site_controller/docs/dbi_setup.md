### Temporary Site_Controller DBI Setup Guide
This is a guide for setting up `DBI` to automatically load your configuration and user setpoints from storage on startup. This guide is temporary as DBI does not currently have a build/install script or executable, and only runs using node, which will be updated in the future.

1. Clone the [dbi repo](https://github.com/flexgen-power/dbi). The latest supported branch is `feature/add_show_collections_v2`
2. Checkout the corresponding scripts branch `feature/dbi-run-stop` 
3. Run HybridOS normally and wait for startup to complete
4. Then, in another terminal window, execute the config setup script located in `hybridos_controller/tests` by entering: `./db_config_setup.sh` . This will populate your database with the configuration data currently stored in config. To be clear, this script does not dynamically load in configuration data (which will be a future story). Instead, it contains its own copy of the current config data as of 4/9/2021. In the future, dbi configuration files can be uploaded via the UI.
6. The configuration data will be loaded into your MongoDB. You can confirm this by sending a git request to for the contents of each config file to `dbi` via the command line:
    `/usr/local/bin/fims/fims_send -m get -u /dbi/site_controller/config/assets -r /me`
    `/usr/local/bin/fims/fims_send -m get -u /dbi/site_controller/config/sequences -r /me`
    `/usr/local/bin/fims/fims_send -m get -u /dbi/site_controller/config/variables -r /me` 
7. Stop HybridOS
8. Edit the command line arguments in  `mcp/config/mcp_hybridos_controller.json`, replacing the default file path with the string argument `"storage"` . Line 6 should now read:
    `"args": [ "storage" ],`
    This tells HybridOS to read configuration from storage`. If you want to switch back to reading from files at any point, simply change this requirement back to the filepath.
9. Rebuild and install mcp with `./package_utility/build.sh` from the mcp directory
10. Run HybridOS

You should now be set up to automatically load in any configuration data or user setpoints from storage on each restart. Any updates to your configuration data can be made through sets to dbi ([documentation found here](https://github.com/flexgen-power/dbi/blob/feature/initial_implementation/documentation/Database_Interface_documentation.md)) or by modifying your `./db_config_setup.sh` script with the new data. Supported user setpoints will be loaded to dbi automatically any time a site setting is changed from the UI or via fims.

Please feel free to reach out to Jack Shade (jnshade) if you encounter any issues or have questions
