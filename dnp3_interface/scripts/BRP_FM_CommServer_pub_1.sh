#/bin/sh
/usr/local/bin/fims/fims_send -m pub  -u /metrics/odessa '{
"gen_net_mw":                   1600,
"gen_net_mvar":                 1601,
"gen_gross_mw":                 1602,
"gen_gross_mvar":               1603,
"gen_normal_up_ramp_rate":      1604,
"gen_normal_down_ramp_rate":    1605,
"gen_emergency_up_ramp_rate":   1606,
"gen_emergency_down_ramp_rate": 1607,
"gen_max_operating_soc":        1608,
"gen_min_operating_soc":        1609,
"gen_soc":                      1610,
"gen_max_discharge_mw":         1611,
"gen_max_charge_mw":            1612,
"gen_line_flows_mw":            1613,
"gen_line_flows_mvar":          1614,
"gen_voltage":                  1615,
"gen_aux_mw":                   1616,
"gen_aux_mvar":                 1617,
"load_net_mw":                  1630,
"load_net_mvar":                1631,
"load_scheduled_mw":            1632,
"load_scheduled_mw_2hr":        1633,
"load_normal_up_ramp_rate":     1634,
"load_normal_down_ramp_rate":   1635,
"load_emergency_up_ramp_rate":  1636,
"load_emergency_down_ramp_rate":1637
}' 

/usr/local/bin/fims/fims_send -m pub -u /metrics/alvin '{
"gen_net_mw":                   1100,
"gen_net_mvar":                 1101,
"gen_gross_mw":                 1102,
"gen_gross_mvar":               1103,
"gen_normal_up_ramp_rate":      1104,
"gen_normal_down_ramp_rate":    1105,
"gen_emergency_up_ramp_rate":   1106,
"gen_emergency_down_ramp_rate": 1107,
"gen_max_operating_soc":        1108,
"gen_min_operating_soc":        1109,
"gen_soc":                      1110,
"gen_max_discharge_mw":         1111,
"gen_max_charge_mw":            1112,
"gen_line_flows_mw":            1113,
"gen_line_flows_mvar":          1114,
"gen_voltage":                  1115,
"gen_aux_mw":                   1116,
"gen_aux_mvar":                 1117,
"load_net_mw":                  1130,
"load_net_mvar":                1131,
"load_scheduled_mw":            1132,
"load_scheduled_mw_2hr":        1133,
"load_normal_up_ramp_rate":     1134,
"load_normal_down_ramp_rate":   1135,
"load_emergency_up_ramp_rate":  1136,
"load_emergency_down_ramp_rate":1137

}' &



