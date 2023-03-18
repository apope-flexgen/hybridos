# SCADA Interface Test Scripts

## clear_schedule.sh
Executes the SCADA Clear Schedule command.

## clear_stage.sh
Executes the SCADA Clear Stage command.

## delete.sh
Executes the SCADA Delete command.

## get_scada.sh
Sends a FIMS GET to /scheduler/scada to retrieve status of all SCADA event slots and the staging area.

## listen.sh
Conducts a fims_listen on the URI /scheduler/scada.

## load.sh
Executes the SCADA Load command.

## mass_set.sh
A helper script that sends a `set_field` command for each field of an event. Meant to be edited by the tester to send their preferred event so that they do not have to keep sending multiple set_fields.

## post.sh
Executes the SCADA POST command.

## set.sh
Executes the SCADA SET command.

## set_field.sh
Takes two arguments: first is the name of the field to be set, second is the raw value that the field should be set to. The script clothes the value in a `{"value": raw_value}` body and sends it to the SCADA staging area's URI.

## track_stage.sh
Loops until the user kills the script. Every 2 seconds, clears the terminal and sends a get_scada command so that tester can keep track of the SCADA staging area.