#
# Creates an MCP config file specifically for the modbus_client processes,
# drawing on the component IDs found in components.txt to do so.
#

# identify where the output config file should live
config_dir=/usr/local/etc/config
mkdir -p ${config_dir}/mcp
output_file=${config_dir}/mcp/mcp_modbus_client.json

# delete the already-existing file and replace it with new file instantiation
rm -f $output_file
touch $output_file

# begin the config file with opening brackets / processes array
echo '{' > $output_file
echo -e '\t"processes":' >> $output_file
echo -e '\t[' >> $output_file

# function to add an entry to the processes array
# expecting vars component_id and include_final_comma to be configured before calling this function
add_entry () {
    # open entry
    echo -e '\t\t{' >> $output_file

    # add name: path to modbus_client executable
    echo -e '\t\t\t"name":"/usr/local/bin/modbus_client",' >> $output_file

    # add args: path to modbus client config file
    args_line_1='\t\t\t"args": [ '
    arg_string_1='"/usr/local/etc/config/modbus_client/'
    arg_string_2="${component_id}"
    arg_string_3='_client.json"'
    args_line_2=" ],"
    args_line=${args_line_1}${arg_string_1}${arg_string_2}${arg_string_3}${args_line_2}
    echo -e $args_line >> $output_file

    # add max_restarts: -1 indicates unlimited restarts
    echo -e '\t\t\t"max_restarts": -1' >> $output_file

    # close entry and include comma if configured to do so
    final_line='\t\t}'
    if [[ "$include_final_comma" == true ]]; then
        comma=','
        final_line=${final_line}${comma}
    fi

    # add entry to output file
    echo -e $final_line >> $output_file
}

# load component names into array from components.txt
mapfile -t components < $config_dir/twins/components.txt
num_components=$(wc -l < $config_dir/twins/components.txt)
i=$1
# loop through each component and add an entry for each one
# include a comma at the end of each entry except for the last one
for component in "${components[@]}"; do
    component_id=$component
    if [[ $num_components != $i ]]; then
        include_final_comma=true
    else
        include_final_comma=false
    fi
    add_entry
    i=$((i + 1))
done

# close the config file's array/objects
echo -e '\t]' >> $output_file
echo '}' >> $output_file
