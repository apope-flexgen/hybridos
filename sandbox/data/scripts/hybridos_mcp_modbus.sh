#!/bin/bash

# $repository - name of repository (i.e. modbus_interface)
# $binary - binary (i.e. modbus_client or modbus_server)
# $output_file - /path/to/mcp_config.json

config_dir=/usr/local/etc/config
output_dir=/usr/local/bin/mcp/config

# utility functions

function begin()
{
    (
        echo -e "{"
        echo -e "	\"processes\":" 
        echo -e "	["
        echo -e "		{"
        echo -e "            \"name\":\"/usr/local/bin/$repository/$binary\","
    ) > "$output_file"
}

function end()
{
    (
        echo -e "			\"max_restarts\": -1"
        echo -e "		}"
        echo -e "	]"
        echo -e "}"
    ) >> "$output_file"
}

function section()
{
    (
        echo -e "			\"max_restarts\": -1"
        echo -e "		},"
        echo -e "		{"
        echo -e "			\"name\":\"/usr/local/bin/$repository/$binary\","
    ) >> "$output_file"
}

function insert()
{
    (
        echo -e "			\"args\": [ \"/usr/local/etc/config/$binary/$1\" ],"
    ) >> "$output_file"
}

# control logic
if [ $# = 0 ]; then # no argments provided, print help text
    echo -e "usage:"
    echo -e "\tsudo ./hybridos_mcp_modbus.sh <options>"
    echo -e "options:"
    echo -e "\tmodbus_client"
    echo -e "\tmodbus_server"
    echo -e "make sure to run ./hybridos_config.sh first."
    exit 1
else
    repository=modbus_interface # TODO: future expansion to all MCP config files
    binary=$1
    output_file=$output_dir/mcp_$1.json

    files=($(ls $config_dir/"$binary".json)) # exclude non .json files
    num_files=${#files[@]}
fi

index=0
begin # generate file preamble

for i in "${files[@]}"; do
    index=$((index+1))

    echo -e "inserting $i"
    insert "$i" # insert modbus config argument
    if [ $index != "$num_files" ]; then 
        section # append json object section
    else
        end # append file ending
    fi
done

echo -e "generated $output_file"
