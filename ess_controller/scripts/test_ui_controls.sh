
fims_send -m set -u /assets/ess/ess_1/maint_mode  {"value":false}
fims_send -m set -u /assets/ess/ess_1/maint_mode  {"value":true}
fims_send -m set -u /assets/ess/ess_1/start  {"value":true}
fims_send -m set -u /assets/ess/ess_1/start  {"value":false}
fims_send -m set -u /assets/ess/ess_1/maint_active_power_setpoint {"value":2000}
fims_send -m set -u /assets/ess/ess_1/maint_reactive_power_setpoint {"value":23000}


# In control object
# unitPrefix
# scaler
# base_uri
# category
# asset_id
# api_endpoint
# {
#   "name": "Maintenance Mode",
#   "value": true,
#   "unit": "",
#   "scaler": 0,
#   "enabled": true,
#   "ui_type": "control",
#   "type": "enum_slider",
#   "options": [
#     {
#       "name": "No",
#       "return_value": false
#     },
#     {
#       "name": "Yes",
#       "return_value": true
#     }
#   ],
#   "displayValue": true,
#   "unitPrefix": "",
#   "id": "maint_mode",
#   "base_uri": "assets",
#   "category": "ess",
#   "asset_id": "ess_1",
#   "api_endpoint": "maint_mode"
# },