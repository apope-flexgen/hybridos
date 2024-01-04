#!/bin/bash

# Define an array of commands to run
commands=(
"fims_send -m pub -u /components/sel_2440 '{\"enum_256_1\":0}'" #1
"fims_send -m pub -u /components/sel_2440 '{\"enum_256_1\":1}'" 
"fims_send -m pub -u /components/sel_2440 '{\"enum_256_1\":2}'"
"fims_send -m pub -u /components/sel_2440 '{\"enum_256_1\":3}'"
"fims_send -m pub -u /components/sel_2440 '{\"enum_256_1\":4}'" #5
"fims_send -m pub -u /components/sel_2440 '{\"enum_256_1\":-1}'"
"fims_send -m pub -u /components/sel_2440 '{\"enum_257_2\":0}'"
"fims_send -m pub -u /components/sel_2440 '{\"enum_257_2\":1}'"
"fims_send -m pub -u /components/sel_2440 '{\"enum_257_2\":2}'"
"fims_send -m pub -u /components/sel_2440 '{\"enum_257_2\":3}'" #10
"fims_send -m pub -u /components/sel_2440 '{\"enum_257_2\":4}'"
"fims_send -m pub -u /components/sel_2440 '{\"enum_257_2\":5}'"
"fims_send -m pub -u /components/sel_2440 '{\"enum_257_2\":6}'"
"fims_send -m pub -u /components/sel_2440 '{\"enum_257_2\":-1}'"
"fims_send -m pub -u /components/sel_2440 '{\"enum_259_4\":0}'" #15
"fims_send -m pub -u /components/sel_2440 '{\"enum_259_4\":1}'"
"fims_send -m pub -u /components/sel_2440 '{\"enum_259_4\":2}'"
"fims_send -m pub -u /components/sel_2440 '{\"enum_259_4\":3}'"
"fims_send -m pub -u /components/sel_2440 '{\"enum_259_4\":4}'"
"fims_send -m pub -u /components/sel_2440 '{\"enum_259_4\":5}'" #20
"fims_send -m pub -u /components/sel_2440 '{\"enum_259_4\":6}'"
"fims_send -m pub -u /components/sel_2440 '{\"enum_259_4\":-1}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2000_1\":1}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2000_1\":2}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2000_1\":3}'" #25
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2000_1\":4}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2000_1\":7}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2000_1\":8}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2000_1\":15}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2000_1\":16}'" #30
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2000_1\":31}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2000_1\":32}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2000_1\":64}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2001_2\":1}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2001_2\":2}'" #35
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2001_2\":3}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2001_2\":4}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2001_2\":7}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2001_2\":8}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2001_2\":15}'" #40
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2001_2\":16}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2001_2\":31}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2001_2\":32}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2001_2\":64}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2003_4\":1}'" #45
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2003_4\":2}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2003_4\":3}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2003_4\":4}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2003_4\":7}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2003_4\":8}'" #50
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2003_4\":15}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2003_4\":16}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2003_4\":31}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2003_4\":32}'"
"fims_send -m pub -u /components/sel_2440 '{\"ibits_2003_4\":64}'" #55
"fims_send -m pub -u /components/sel_2440 '{\"bf_2007_1\":1}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2007_1\":2}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2007_1\":3}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2007_1\":4}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2007_1\":7}'" #60
"fims_send -m pub -u /components/sel_2440 '{\"bf_2007_1\":8}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2007_1\":15}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2007_1\":16}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2007_1\":31}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2007_1\":32}'" #65
"fims_send -m pub -u /components/sel_2440 '{\"bf_2007_1\":64}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2008_2\":1}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2008_2\":2}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2008_2\":3}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2008_2\":4}'" #70
"fims_send -m pub -u /components/sel_2440 '{\"bf_2008_2\":7}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2008_2\":8}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2008_2\":15}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2008_2\":16}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2008_2\":31}'" #75
"fims_send -m pub -u /components/sel_2440 '{\"bf_2008_2\":32}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2008_2\":64}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2010_4\":1}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2010_4\":2}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2010_4\":3}'" #80
"fims_send -m pub -u /components/sel_2440 '{\"bf_2010_4\":4}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2010_4\":7}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2010_4\":8}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2010_4\":15}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2010_4\":16}'" #85
"fims_send -m pub -u /components/sel_2440 '{\"bf_2010_4\":31}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2010_4\":32}'"
"fims_send -m pub -u /components/sel_2440 '{\"bf_2010_4\":64}'"
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":1}'"
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":2}'" #90
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":3}'"
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":4}'"
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":5}'"
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":6}'"
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":7}'" #95
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":8}'"
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":9}'"
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":10}'"
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":11}'"
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":12}'" #100
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":13}'"
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":14}'"
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":15}'"
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":16}'"
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":17}'" #105
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":18}'"
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":32}'"
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":64}'"
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":128}'" #110
"fims_send -m pub -u /components/sel_2440 '{\"pack_1\":256}'"
)

# Loop through the commands and execute them with a 2.5-second delay
test_id=1
for cmd in "${commands[@]}"; do
  echo "fims_send -m pub -u /components/sel_2440 '{\"test_id\":$((test_id))}'"
  eval "fims_send -m pub -u /components/sel_2440 '{\"test_id\":$((test_id))}'"
  echo "$cmd" # Echo the command
  eval "$cmd" # Execute the command
  sleep 2.5 # Wait for 2.5 seconds
  ((test_id++))
done