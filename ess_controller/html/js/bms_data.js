{
    "tables": [
        { 
            "BMS Status" :{
    
                "cols":[
                    {"name":"bms_poweron", "source":"catl_bms_ems_r","sdef":0},
                    {"name":"bms_status", "source":"catl_bms_ems_r","sdef":1}
                ],
                "rows":[
                    {"name":"status",     "sources":["bms_poweron","bms_status"]}
                ]
            }
        },
        {               
            "BMS Data" :{
    
                "cols":[
                    {"name":"mbmu", "source":"catl_mbmu_summary_r","sdef":0},
                    {"name":"sbmu1", "source":"catl_sbmu_1","sdef":1},
                    {"name":"sbmu2", "source":"catl_sbmu_2","sdef":1},
                    {"name":"sbmu3", "source":"catl_sbmu_3","sdef":1},
                    {"name":"sbmu4", "source":"catl_sbmu_4","sdef":1},
                    {"name":"sbmu5", "source":"catl_sbmu_5","sdef":1},
                    {"name":"sbmu6", "source":"catl_sbmu_6","sdef":1},
                    {"name":"sbmu7", "source":"catl_sbmu_7","sdef":1},
                    {"name":"sbmu8", "source":"catl_sbmu_8","sdef":1},
                    {"name":"sbmu9", "source":"catl_sbmu_9","sdef":1}
                ],
                "rows":[
                    {"name":"soc",     "sources":["mbmu_soc","sbmu_soc"]},
                    {"name":"soh",     "sources":["mbmu_soh","sbmu_soh"]},
                    {"name":"current", "sources":["mbmu_current",
                                                        "sbmu_current"]},
                    {"name":"voltage", "sources":["mbmu_voltage",
                                                        "sbmu_voltage"]},
                    {"name":"avg_temp", "sources":["mbmu_avg_cell_temp","sbmu_avg_cell_temp"]},

                    {"name":"avg_volt", "sources":["","sbmu_avg_cell_voltage"]},
                    {"name":"avg_curr", "sources":["","sbmu_avg_cell_current"]},
                    {"name":"max_temp", "sources":["mbmu_max_cell_temperature",
                                                        "sbmu_max_cell_temp"]},
                    
                    {"name":"max_volt", "sources":["mbmu_max_cell_voltage",
                                                        "sbmu_max_cell_voltage"]},
                    
                    {"name":"max_curr", "sources":["","sbmu_max_cell_current"]},

                    {"name":"min_temp", "sources":["mbmu_min_cell_temperature",
                                                    "sbmu_min_cell_temp"]},
                    {"name":"min_volt", "sources":["mbmu_min_cell_voltage",
                                                    "sbmu_min_cell_voltage"]},
                    {"name":"min_curr", "sources":["mbmu_min_cell_current",
                                                    "sbmu_min_cell_current"]},
                    {"name":"max_ch_curr", "sources":["mbmu_max_charge_current",
                                                    "sbmu_max_charge_current"]},
                    {"name":"max_dis_curr", "sources":["mbmu_max_discharge_current",
                                                "sbmu_max_discharge_current"]}
                ]
            }
        },
        { 
            "PCS Status" :{
    
                "cols":[
                    {"name":"Module 1", "source":"pcs_registers_fast","sdef":0},
                    {"name":"Module 2", "source":"pcs_registers_fast","sdef":1},
                    {"name":"Module 3", "source":"pcs_registers_fast","sdef":2},
                    {"name":"Module 4", "source":"pcs_registers_fast","sdef":3},
                    {"name":"Module 5", "source":"pcs_registers_fast","sdef":4},
                    {"name":"Module 6", "source":"pcs_registers_fast","sdef":5}
                ],
                "rows":[
                    {"name":"status",     
                        "sources":[
                                    "module_1_status"
                                    ,"module_2_status"
                                    ,"module_3_status"
                                    ,"module_4_status"
                                    ,"module_5_status"
                                    ,"module_6_status"
                                ]},
                    {"name":"dc_voltage",     
                        "sources":[
                                    "module_1_dc_voltage"
                                    ,"module_2_dc_voltage"
                                    ,"module_3_dc_voltage"
                                    ,"module_4_dc_voltage"
                                    ,"module_5_dc_voltage"
                                    ,"module_6_dc_voltage"
                                ]},
                    {"name":"dc_current",     
                        "sources":[
                                    "module_1_dc_current"
                                    ,"module_2_dc_current"
                                    ,"module_3_dc_current"
                                    ,"module_4_dc_current"
                                    ,"module_5_dc_current"
                                    ,"module_6_dc_current"
                                ]}
            
                ]
            }
        }
    
    ],

"queries":[
    {
        "measurement":"sbmu_modbus_data",
        "limit":9,
        "sources":[
            "catl_sbmu_1"
            ,"catl_sbmu_2"
            ,"catl_sbmu_3"
            ,"catl_sbmu_4"
            ,"catl_sbmu_5"
            ,"catl_sbmu_6"
            ,"catl_sbmu_7"
            ,"catl_sbmu_8"
            ,"catl_sbmu_9"
        ],
        "fields":[
            "sbmu_soc"
            ,"sbmu_soh"
            ,"sbmu_current"
            ,"sbmu_voltage"
            ,"sbmu_avg_cell_temp"
            ,"sbmu_avg_cell_voltage"
            ,"sbmu_avg_cell_current"
            ,"sbmu_max_cell_temp"
            ,"sbmu_max_cell_voltage"
            ,"sbmu_max_cell_current"
            ,"sbmu_min_cell_temp"
            ,"sbmu_min_cell_voltage"
            ,"sbmu_min_cell_current"
            ,"sbmu_max_charge_current"
            ,"sbmu_max_discharge_current"
        ]
    },
    {
        "measurement":"mbmu_modbus_data",
        "limit":1,
        "sources":[
            "catl_mbmu_summary_r"
        ],
        "fields":[
            "mbmu_soc"
            ,"mbmu_soh"
            ,"mbmu_current"
            ,"mbmu_voltage"
            ,"mbmu_avg_cell_temp"
            ,"mbmu_avg_cell_voltage"
            ,"mbmu_avg_cell_current"
            ,"mbmu_max_cell_temp"
            ,"mbmu_max_cell_voltage"
            ,"mbmu_max_cell_current"
            ,"mbmu_min_cell_temp"
            ,"mbmu_min_cell_voltage"
            ,"mbmu_min_cell_current"
            ,"mbmu_max_charge_current"
            ,"mbmu_max_discharge_current"

        ]
    },
    {
        "measurement":"mbmu_modbus_data",
        "limit":1,
        "sources":[
            "catl_ems_bms_r"
        ],
        "fields":[
            "bms_poweron"
            ,"bms_status"

        ]
    },
    {
        "measurement":"pcs_modbus_data",
        "limit":1,
        "sources":[
            "pcs_registers_fast"
        ],
        "fields":[
            "module_1_status"
            ,"module_2_status"
            ,"module_3_status"
            ,"module_4_status"
            ,"module_5_status"
            ,"module_6_status"
            ,"module_1_dc_voltage"
            ,"module_2_dc_voltage"
            ,"module_3_dc_voltage"
            ,"module_4_dc_voltage"
            ,"module_5_dc_voltage"
            ,"module_6_dc_voltage"
            ,"module_1_dc_current"
            ,"module_2_dc_current"
            ,"module_3_dc_current"
            ,"module_4_dc_current"
            ,"module_5_dc_current"
            ,"module_6_dc_current"

        ]
    }

],
 "junk":{
"sbmu_warning_1":0,
"sbmu_warning_21":0,
"sbmu_warning_22":0,
"sbmu_warning_23":4096,
"sbmu_precharge_status":0,
"sbmu_master_positive":0,
"sbmu_master_negitive":0,
"sbmu_balance_status":0,
"sbmu_max_cell_voltage_position":5,
"sbmu_min_cell_voltage_position":1,
"sbmu_max_cell_temp_position":27,
"sbmu_min_cell_temp_position":2,
"sbmu_sum_cells":13709,
"sbmu_tms_mode_command":0,
"sbmu_tms_temp_setting":60,
"sbmu_tms_real_mode":60,
"sbmu_ambient_temp":60,
"sbmu_tms_demand_power":0,
"sbmu_tms_fault_code":0,
"sbmu_door_state":0,
"sbmu_fan_in_box":0
 }
}
