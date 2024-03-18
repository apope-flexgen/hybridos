#!/bin/bash
# Run script used as entrypoint for DTS image.

dts_executable_file_path="/usr/local/bin/dts"
config_file_path_passed_to_dts_executable="/home/config/dts.json"
staged_config_file_path="/home/staging/dts.json"

# In production, the DTS configuration file will be mounted directly to /home/config.
# If no file is found there, then this is a virtual environment so copy from the staging directory.
if [ ! -f "$config_file_path_passed_to_dts_executable" ]; then
    echo "Production configuration file not found. Copying staged configuration file..."
    cp "$staged_config_file_path" "$config_file_path_passed_to_dts_executable"
    if [ $? -ne 0 ]; then
        echo "Failed to copy staged configuration file to $config_file_path_passed_to_dts_executable."
        exit 1
    fi
    echo "Staged configuration file copied successfully. Applying Virtual Site defaults..."
    # The user may bind-mount a custom dts.json that overwrites the default dts.json.
    # They are allowed to customize things like number of workers, health check delay, etc.
    # But to be compatible with Virtual Site, the InfluxDB and MongoDB addresses as well as the input directory path must be standardized.
    python3 /home/scripts/apply_virtual_site_defaults_to_dts_cfg_file.py --cfg="$config_file_path_passed_to_dts_executable"
    touch /home/config/virtual_site.txt
else
    touch /home/config/production.txt
    echo "Production configuration file found."
fi

# Launch dts with the config file
echo "Launching $dts_executable_file_path..."
$dts_executable_file_path --c="$config_file_path_passed_to_dts_executable"
