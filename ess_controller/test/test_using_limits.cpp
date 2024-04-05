/*
test_using_limits.cpp


Just some notes

onset
*/

/****/
// "/components/ess":{
//     "dc_current":{
//       "note1":"//mask 3  bit 0   0000000000000001       oncmd",
//       "note2":"//mask 3  bit 1   0000000000000010       kacclosecmd",
//       "note3":"//mask 48 bit 4   0000000000010000       offcmd",
//       "note4":"//mask 48 bit 5   0000000000100000       kacopencmd",
//       "value":0,
//           "actions":{
//               "onSet":{
//                   "limits":[
//                       { "amap":"standby","defaut":"default_current_limits"},
//                       {
//                       "max_error":"dc_current_max_error","alert","dc_max_error_alert"},
//                       {
//                       "max_warn":"dc_current_max_warn","alert","dc_max_warn_alert"},
//                       {
//                       "min_error":"dc_current_min_error","alert","dc_min_error_alert"},
//                       {
//                       "min_warn":"dc_current_max_warn","alert","dc_min_warn_alert"},

//                   ]
//             }
//       }
//     },

// then set up different amaps
//      default_current_limits
//      grid_form_current_limits
//      normal_charge_current_limits
//      .. fast_charge_current_limits

//   then set amap  "standby" to point to the "current_limits" amap.
// need not be fully specified since the defaylt will take over.
// Q what if you need different warnings ... urghhh then you need to swap out
// the whole onSet Limits object. We'll have a number of onSet limits tables and
// just swap those out perhaps.. this means we need to look up the limits for a
// particular var. getLimits for var dc_current in mode grid_form   etc. slight
// variation on this will do named amaps are it though..
///
//                 {"standby":{
//                        "/components/ess:dc_current": [
//                          {
//                          "max_error":"dc_current_max_error","alert","dc_max_error_alert"},
//                          {
//                          "max_warn":"dc_current_max_warn","alert","dc_max_warn_alert"},
//                          {
//                          "min_error":"dc_current_min_error","alert","dc_min_error_alert"},
//                          {
//                          "min_warn":"dc_current_max_warn","alert","dc_min_warn_alert"}
//                          ],
//                        "/components/ess:ac_current": [
//                          {
//                          "max_error":"ac_current_max_error","alert","dc_max_error_alert"},
//                          {
//                          "max_warn":"ac_current_max_warn","alert","dc_max_warn_alert"},
//                          {
//                          "min_error":"ac_current_min_error","alert","dc_min_error_alert"},
//                          {
//                          "min_warn":"ac_current_max_warn","alert","dc_min_warn_alert"}
//                          ]
//                          }
//                    }
