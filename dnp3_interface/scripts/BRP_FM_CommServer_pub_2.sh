#/bin/sh
/usr/local/bin/fims/fims_send -m pub  -u /metrics/odessa '{
"gen_net_mw":                   2600,
"gen_net_mvar":                 2601,
"gen_gross_mw":                 2602,
"gen_gross_mvar":               2603,
"gen_normal_up_ramp_rate":      2604,
"gen_normal_down_ramp_rate":    2605,
"gen_emergency_up_ramp_rate":   2606,
"gen_emergency_down_ramp_rate": 2607,
"gen_max_operating_soc":        2608,
"gen_min_operating_soc":        2609,
"gen_soc":                      2610,
"gen_max_discharge_mw":         2611,
"gen_max_charge_mw":            2612,
"gen_line_flows_mw":            2613,
"gen_line_flows_mvar":          2614,
"gen_voltage":                  2615,
"gen_aux_mw":                   2616,
"gen_aux_mvar":                 2617,
"load_net_mw":                  2630,
"load_net_mvar":                2631,
"load_scheduled_mw":            2632,
"load_scheduled_mw_2hr":        2633,
"load_normal_up_ramp_rate":     2634,
"load_normal_down_ramp_rate":   2635,
"load_emergency_up_ramp_rate":  2636,
"load_emergency_down_ramp_rate":2637
}' 

/usr/local/bin/fims/fims_send -m pub -u /metrics/alvin '{
"gen_net_mw":                   2100,
"gen_net_mvar":                 2101,
"gen_gross_mw":                 2102,
"gen_gross_mvar":               2103,
"gen_normal_up_ramp_rate":      2104,
"gen_normal_down_ramp_rate":    2105,
"gen_emergency_up_ramp_rate":   2106,
"gen_emergency_down_ramp_rate": 2107,
"gen_max_operating_soc":        2108,
"gen_min_operating_soc":        2109,
"gen_soc":                      2110,
"gen_max_discharge_mw":         2111,
"gen_max_charge_mw":            2112,
"gen_line_flows_mw":            2113,
"gen_line_flows_mvar":          2114,
"gen_voltage":                  2115,
"gen_aux_mw":                   2116,
"gen_aux_mvar":                 2117,
"load_net_mw":                  2130,
"load_net_mvar":                2131,
"load_scheduled_mw":            2132,
"load_scheduled_mw_2hr":        2133,
"load_normal_up_ramp_rate":     2134,
"load_normal_down_ramp_rate":   2135,
"load_emergency_up_ramp_rate":  2136,
"load_emergency_down_ramp_rate":2137

}' &



