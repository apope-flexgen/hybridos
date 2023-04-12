echo "Is this machine an ESS(1) or a Site(2) Controller?"
read machinetype
updatefiles=()
if [ $machinetype == 1 ] || [ "${machinetype,,}" = "ess" ]
then
  echo "Editing IP addresses for modbus client config files on an ESS Controller"
  # in ess controller machine, change bms_1 and bms_2 modbus clients, pcs modbus client
  for file in /home/hybridos/configs/modbus_client/*modbus_client.json
  do
    updatefiles+=("$file")
  done
elif [ $machinetype == 2 ] || [ "${machinetype,,}" = "site" ]
then
  echo "Editing IP addresses for modbus and dnp3 client config files on a Site Controller"
  # in site controller machine, change bop rtac dnp3 client and flexgen_ess_*_modbus_client
  for file in /home/hybridos/config/modbus_client/flexgen*modbus_client.json
  do
    updatefiles+=("$file")
  done
  for file in /home/hybridos/config/dnp3_client/*dnp3_client.json
  do
    updatefiles+=("$file")
  done
else
  echo "Sorry, I didn't understand what type of machine this is. Exiting script now."
  exit 0
fi

for file in "${updatefiles[@]}"
do
  #echo -e "$file\n" 
  sed -i 's/\"ip_address\":.*,/\"ip_address\": \"10.26.196.167\",/g' $file
done
