#!/usr/bin/sh

red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`


VAR1=$(/usr/local/bin/fims_send -m get -r /me -u /ess/naked/status/bms/SOC)
echo $VAR1

if [ $VAR1 -eq 100 ]
then
    echo "got to part 1"

else
    echo "got to part 2"

fi

echo -e "this is a test ${red} of text color ${green} did I pass? ${reset}"