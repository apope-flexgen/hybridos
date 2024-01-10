# In the config folder's twins directory, Modbus client/server files and echo files
# are stored for each Modbus component. The Modbus client files are edited by the 
# configuration engineer, then the echo tool generates the Modbus server and echo
# files. This script calls the echo tool with the proper arguments to generate server
# and echo files for all Modbus components listed in components.txt.

bin_dir=/usr/local/bin
config_dir=/usr/local/etc/config

# support backward compatibility of echo's -config to -c change
tag=$(cat /usr/local/etc/config/repo.txt | grep echo) # get echo line of repo.txt
IFS='v' read -ra VSTRINGS <<< "$tag" # isolate tag
tag=$"${VSTRINGS[1]}"
IFS='.' read -ra DOTSTRINGS <<< "$tag" # isolate major version number
tag=$"${DOTSTRINGS[0]}"
re='^[0-9]+$' # should just have number at this point
if ! [[ $tag =~ $re ]]; then
    echo "Error reading echo tag"; exit 1
fi
cutoff=2 # echo started using -c instead of -config at echo 2.0.0
if [[ $tag -ge $cutoff ]]; then
    config_flag="c"
else
    config_flag="config"
fi

# capture Modbus components list
mapfile -t components < $config_dir/twins/components.txt

# purge the contents of the modbus_server/echo folders
rm -rf $config_dir/twins/modbus_server
rm -rf $config_dir/twins/echo
mkdir $config_dir/twins/modbus_server
mkdir $config_dir/twins/echo

# iterate through each component and use its Modbus client JSON file to generate/update its Modbus server JSON file and echo JSON file
for component in "${components[@]}"; do
    # echo generates both echo file and modbus_server file in same place,
    # so must make temp folder to output them, then move to separate folders
    mkdir $config_dir/twins/$component # make temp folder
    $bin_dir/echo -$config_flag=$config_dir/modbus_client/"$component"_client.json -mode=modbus -output=$config_dir/twins/$component # generate files
    mv $config_dir/twins/"$component"/"$component"_server.json $config_dir/twins/modbus_server/"$component"_server.json # move server file to modbus_server folder
    mv $config_dir/twins/"$component"/"$component"_echo.json $config_dir/twins/echo/"$component"_echo.json # move echo file to echo folder
    rm -rf $config_dir/twins/$component # delete temp folder
done

# call the helper script to generate the MCP configuration file for modbus_client
./generate_modbus_mcp.sh
