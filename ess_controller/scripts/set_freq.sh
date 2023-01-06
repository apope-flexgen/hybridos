#!/bin/sh

# some Ess_Manager commands.

/usr/local/bin/fims/fims_send -m set -u /features/frequency/pfr_offset_hz    '{"value":1.45}'
/usr/local/bin/fims/fims_send -m get -u /features/frequency/pfr_offset_hz  -r/me  '{"value":1.45}'

/usr/local/bin/fims/fims_send -m set -u /site/configuration/reserved_float_1 '{"value":123.4}' -
/usr/local/bin/fims/fims_send -m get -u /site/configuration  -r /me

# how do we respond to an incoming value  from the BMS say soc.
# place this in the bms:bms_1:components  space
#              "soc": {
#                  "name": "State of Charge",
#                  "register_id": "soc",
#                  "value": null,
#                  "scaler": 1,
#                  "unit": "%",
#                  "twins_id": "soc"
#                },
#did this work ???.. yes
/usr/local/bin/fims/fims_send -m set -u /assets/bms/bms_1 '{"soc":{"value":1234}}'  -r /me
# so lets find a set that does something 
#bool Asset_Manager::process_asset_data(void)
#{
#    FPS_DEBUG_PRINT("\n***HybridOS Step 2: Process Component Data.\nIn Asset_Manager::process_asset_data.\n");
#
#    if (numBmsParsed > 0)
#        aggregate_bms_data();

# this is the ui_control path which is the response to an ui_command.
# the ui_control will trugger an action

/usr/local/bin/fims/fims_send -m set -u /assets/bms/bms_1 '{"start":{"value":true}}'
# this sends the  bms::start_value to   uri defined in the "start" ui_controls object ie "set" /components/bms_1cw/control_word_1] body [{"value":2}
#    "bms": {
#      "start_value": 2,
#      "stop_value": 1,
#      "asset_instances": [
#        {
#          "id": "bms_1",
#            "components": [
#            {
#                "component_id": "bms_1cw",
#                "ui_controls": {
#                "start": {
#                  "name": "Start",
#                  "register_id": "control_word_1",

# so we get all the pubs
#[flexgen@localhost ~]$ /usr/local/bin/fims/fims_listen -s /assets/bms/bms_1

#Method:  pub
#Uri:     /assets/bms/bms_1
#ReplyTo: (null)
#Body:
#     {
#     "name":"BMS System  01",
#     "soc":{"name":"State of Charge","value":null,"unit":"%","scaler":1,"enabled":false,"ui_type":"status","type":"number"},
#     "maint_mode":{"name":"Maintenance Mode","value":false,"unit":"","scaler":0,"enabled":true,"ui_type":"control","type":"enum_slider",
#             "options":[{"name":"No","return_value":false},{"name":"Yes","return_value":true}]},
#     "start":{"name":"Start","unit":"","scaler":0,"enabled":false,"ui_type":"control","type":"enum_button",
#             "options":[{"name":"On","return_value":true},{"name":"Off","return_value":false}]},
#     "stop":{"name":"Stop","unit":"","scaler":0,"enabled":false,"ui_type":"control","type":"enum_button",
#             "options":[{"name":"On","return_value":true},{"name":"Off","return_value":false}]},
#     "enter_standby":{"name":"Enter Standby","unit":"","scaler":0,"enabled":false,"ui_type":"control","type":"enum_button",
#             "options":[{"name":"On","return_value":true},{"name":"Off","return_value":false}]},
#     "exit_standby":{"name":"Exit Standby","unit":"","scaler":0,"enabled":false,"ui_type":"control","type":"enum_button",
#             "options":[{"name":"On","return_value":true},{"name":"Off","return_value":false}]},
#     "clear_faults":{"name":"Clear Faults","unit":"","scaler":0,"enabled":false,"ui_type":"control","type":"enum_button",
#             "options":[{"name":"Clear Faults","return_value":true}]},
#     "close_dc_contactors":{"name":"Close DC Contactors","unit":"","scaler":0,"enabled":false,"ui_type":"control","type":"enum_button",
#             "options":[{"name":"Close","return_value":true}]},
#     "open_dc_contactors":{"name":"Open DC Contactors","unit":"","scaler":0,"enabled":false,"ui_type":"control","type":"enum_button",
#             "options":[{"name":"Open","return_value":true}]},
#     "maint_active_power_setpoint":{"name":"Active Power Setpoint","value":0,"unit":"W","scaler":1000,"enabled":false,"ui_type":"control","type":"number",
#             "options":[]},
#     "maint_reactive_power_setpoint":{"name":"Reactive Power Setpoint","value":0,"unit":"VAR","scaler":1000,"enabled":false,"ui_type":"control","type":"number",
#             "options":[]}
#     }
# Timestamp:   2020-07-24 09:11:35.846287
# signal of type 2 caught.
# fims_listen: Failed to read anything from socket.


# Q /How does data get from the Bms to the Ess  ?
# if ((strncmp(pmsg->pfrags[0],"features", strlen("features")) == 0)  || (strncmp(pmsg->pfrags[0],"site", strlen("site")) == 0))
# /features, /site go to
#                         essMgr->fims_data_parse(pmsg);
# /components, /assets
#                         assetMgr->fims_data_parse(pmsg);


# on the assets enum_sider
# void Asset_Manager::fims_data_parse(fims_message *pmsg)
# {
#     if (strcmp(pmsg->method, "pub") == 0)
#     {
#         FPS_DEBUG_PRINT("\n\n***HybridOS Step 1: Receive Component Data.\nIn Asset_Manager::fims_data_parse\n");
#         Asset_Manager::handle_pubs(pmsg->pfrags, pmsg->nfrags, pmsg->body);
#     }
#     else if (strcmp(pmsg->method, "get") == 0)
#     {
#         Asset_Manager::handle_get(pmsg->pfrags, pmsg->nfrags, pmsg->replyto);
#     }
#     else if (strcmp(pmsg->method, "set") == 0)
#     {
#         Asset_Manager::handle_set(pmsg->pfrags, pmsg->nfrags, pmsg->replyto, pmsg->body);
#     }
#     else if (strcmp(pmsg->method, "post") == 0)
#     {
#         Asset_Manager::handle_post(pmsg->nfrags, pmsg->body);
#     }
#     else if (strcmp(pmsg->method, "del") == 0)
#     {
#         Asset_Manager::handle_del(pmsg->nfrags, pmsg->body);
#     }
# # PUBS
# incoming asset pubs ... only interested in components  pfrag[1]
# try /components/bms/bms_1
#Hmm this got to it  
#        /usr/local/bin/fims/fims_send -m pub -u  /components/bms_1c "hi"
/usr/local/bin/fims/fims_send -m pub -u  /components/bms_1c '{"soc":1234}'
//but we did not see the value update in the PUB
//we are using  compVars[compIdx] to get the var list


# #SETS  this worked but we respond with values even if they are not used.. ( " ie soh in this case)"
/usr/local/bin/fims/fims_send -m set -u /assets/bms/bms_1 '{"soc":{"value":3456},"soh":{"value":1234}}'  -r /me
{"soc":{"value":3456},"soh":{"value":1234}}
# but these only set updates 
# how does the update get to the value duh its a pointer to the value



# #GETS
# void Asset_Manager::handle_get(char** pfrags, int nfrags, char* replyto)

# if ((nfrags == 1) || ((nfrags == 2 || nfrags == 3) && strncmp(pfrags[1],"ess", strlen("ess")) == 0))
# /usr/local/bin/fims/fims_send -m get -u /assets/ess/summary  -r /me
# {
# "name":"ESS Summary","num_ess_available":
#       {"value":" 4","type":"string","name":"Units Available","unit":"","scaler":1,"time":false,"enabled":true,"ui_type":"status","options":[]},
# # "num_ess_running":{"value":" 0","type":"string","name":"Units Running","unit":"","scaler":1,"time":false,"enabled":true,"ui_type":"status","options":[]},
# "ess_total_active_power":{"value":0,"type":"number","name":"Active Power","unit":"W","scaler":1000,"time":false,"enabled":true,"ui_type":"status","options":[]},
# "ess_total_reactive_power":{"value":0,"type":"number","name":"Reactive Power","unit":"VAR","scaler":1000,"time":false,"enabled":true,"ui_type":"status","options":[]},
# "ess_total_apparent_power":{"value":0,"type":"number","name":"Apparent Power","unit":"VA","scaler":1000,"time":false,"enabled":true,"ui_type":"status","options":[]},
# "ess_average_soc":{"value":0,"type":"number","name":"State of Charge","unit":"%","scaler":1,"time":false,"enabled":true,"ui_type":"status","options":[]},
# "ess_chargeable_power":{"value":0,"type":"number","name":"Chargeable Power","unit":"W","scaler":1000,"time":false,"enabled":true,"ui_type":"status","options":[]},
# "ess_dischargeable_power":{"value":0,"type":"number","name":"Dischargeable Power","unit":"W","scaler":1000,"time":false,"enabled":true,"ui_type":"status","options":[]},
# "ess_chargeable_energy":{"value":0,"type":"number","name":"Chargeable Energy","unit":"Wh","scaler":1000,"time":false,"enabled":true,"ui_type":"status","options":[]},
# "ess_dischargeable_energy":{"value":0,"type":"number","name":"Dischargeable Energy","unit":"Wh","scaler":1000,"time":false,"enabled":true,"ui_type":"status",
#     "options":[]},
# "ess_total_alarms":{"value":0,"type":"number","name":"Alarms","unit":"","scaler":1,"time":false,"enabled":true,"ui_type":"alarm","options":[]},
# "ess_total_faults":{"value":0,"type":"number","name":"Faults","unit":"","scaler":1,"time":false,"enabled":true,"ui_type":"fault","options":[]},
# "running_ess_num":{"value":0,"type":"number","name":"Number units running","unit":"","scaler":1,"time":false,"enabled":true,"ui_type":"none","options":[]},
# "available_ess_num":{"value":4,"type":"number","name":"Number units available","unit":"","scaler":1,"time":false,"enabled":true,"ui_type":"none","options":[]},
# "grid_forming_voltage_slew":{"value":0,"type":"number","name":"Grid Forming Voltage Slew","unit":"%/s","scaler":1,"time":false,"enabled":true,"ui_type":"status",
#     "options":[]}}
# this now works
/usr/local/bin/fims/fims_send -m get -u /assets/bms/summary  -r /me
# {
# "name":"BMS Summary","num_bms_available":
# this works
/usr/local/bin/fims/fims_send -m get -u /assets/bms/bms_1  -r /me

# now system subscriptions  are to
#subscriptions[num_subs++] = strdup("/site");
#    subscriptions[num_subs++] = strdup("/features");
#    subscriptions[num_subs++] = strdup("/components");
#    subscriptions[num_subs++] = strdup("/assets");

# the bms_1 thread will have to respond to 
/usr/local/bin/fims/fims_send -m get -u /assets/bms/bms_1  -r /me
/usr/local/bin/fims/fims_send -m set -u /assets/bms/bms_1 '{"soc":{"value":3456},"soh":{"value":1234}}'  -r /me
/usr/local/bin/fims/fims_send -m pub -u  /components/bms_1c '{"soc":1234}'
/usr/local/bin/fims/fims_send -m set -u /assets/bms/bms_1 '{"soc":{"value":1234}}'  -r /me



#Moving on to the Ess for real...
# the uris are /eatures and /site
#This is where this sad sequence runs 
# //if (strncmp(msg->pfrags[1], "energy_arbitrage", strlen("energy_arbitrage")) == 0)
# //                    {
# //                       local_threshold_charge_1.set_fims_float(msg->pfrags[2], body_float);
#                        local_threshold_charge_2.set_fims_float(msg->pfrags[2], body_float);
# lets create maps for energy_arbitrage
#  else if (strncmp(msg->pfrags[1], 
#  "manual_mode"
#  , strlen("manual_mode")) == 0)
#                     {
#                         float chg_dischg_limit = fabsf((local_chg_dischg_flag.value.value_bool) ? max_dischg_2.value.value_float : max_charge_2.value.value_float);
#                         local_manual_power.set_fims_float(msg->pfrags[2], fabsf(body_float) > chg_dischg_limit ? chg_dischg_limit : fabsf(body_float));
#                         remote_manual_power.set_fims_float(msg->pfrags[2], fabsf(body_float));
#                     }
#                     else if (strncmp(msg->pfrags[1], 
# "curtailment"
# , strlen("curtailment")) == 0)
#                     {
#                         curtailment_target.set_fims_float(msg->pfrags[2], body_float);
#                         inverter_output_percent.set_fims_float(msg->pfrags[2], body_float);
#                     }
#                     else if (strncmp(msg->pfrags[1], 
# "frequency", strlen("frequency")) == 0)
#                     {
     else if (strncmp(msg->pfrags[1], "active_power", strlen("active_power")) == 0)