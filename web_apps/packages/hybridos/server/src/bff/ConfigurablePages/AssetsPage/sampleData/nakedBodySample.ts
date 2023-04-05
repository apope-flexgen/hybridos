import { nakedBodyFromFims } from '../../configurablePages.types'

export default (): nakedBodyFromFims => {
    return {
        name: 'BESS Inverter Block 01',
        active_power: 0,
        active_power_setpoint_component_ctrl_cmd: 0,
        active_power_setpoint: 0,
        alarms: 0,
        apparent_power: 0,
        autobalancing_status: false,
        bms_fault: 0,
        com_status: 0,
        component_connected: false,
        current_l1: 0,
        current_l2: 0,
        current_l3: 0,
        dc_contactors_closed: false,
        emmu_fault: 0,
        faults: 0,
        frequency: 0,
        frequency_setpoint: 60,
        grid_mode: 0,
        local_bms_status: 0,
        max_temp: 0,
        min_temp: 0,
        modbus_heartbeat: 0,
        reactive_power: 0,
        reactive_power_setpoint_component_ctrl_cmd: 0,
        reactive_power_setpoint: 0,
        soc: 49.462364196777344,
        soh: 0,
        status: 'Stopped',
        system_chargeable_energy: 0,
        system_chargeable_power: 5500,
        system_dischargeable_energy: 0,
        system_dischargeable_power: 5500,
        voltage_dc: 0,
        voltage_l1_l2: 95600.8203125,
        voltage_l2_l3: 0,
        voltage_l3_l1: 0,
        voltage_max: 0,
        voltage_min: 0,
        voltage_setpoint: 38333.33203125,
        lock_mode: {
            value: false,
            enabled: false,
            options: [
                {
                    name: 'No',
                    return_value: false,
                },
                {
                    name: 'Yes',
                    return_value: true,
                },
            ],
        },
        maint_mode: {
            value: false,
            enabled: true,
            options: [
                {
                    name: 'No',
                    return_value: false,
                },
                {
                    name: 'Yes',
                    return_value: true,
                },
            ],
        },
        start: {
            enabled: false,
            options: [
                {
                    name: 'On',
                    return_value: true,
                },
                {
                    name: 'Off',
                    return_value: false,
                },
            ],
        },
        stop: {
            enabled: false,
            options: [
                {
                    name: 'On',
                    return_value: true,
                },
                {
                    name: 'Off',
                    return_value: false,
                },
            ],
        },
        enter_standby: {
            enabled: false,
            options: [
                {
                    name: 'On',
                    return_value: true,
                },
                {
                    name: 'Off',
                    return_value: false,
                },
            ],
        },
        exit_standby: {
            enabled: false,
            options: [
                {
                    name: 'On',
                    return_value: true,
                },
                {
                    name: 'Off',
                    return_value: false,
                },
            ],
        },
        clear_faults: {
            enabled: false,
            options: [
                {
                    name: 'Clear Faults',
                    return_value: true,
                },
            ],
        },
        limits_override: {
            value: false,
            enabled: false,
            options: [
                {
                    name: 'On',
                    return_value: true,
                },
                {
                    name: 'Off',
                    return_value: false,
                },
            ],
        },
        autobalancing_enable: {
            enabled: false,
            options: [
                {
                    name: 'On',
                    return_value: true,
                },
                {
                    name: 'Off',
                    return_value: false,
                },
            ],
        },
        autobalancing_disable: {
            enabled: false,
            options: [
                {
                    name: 'On',
                    return_value: true,
                },
                {
                    name: 'Off',
                    return_value: false,
                },
            ],
        },
        maint_active_power_setpoint: {
            value: 0,
            enabled: false,
            options: [],
        },
        maint_reactive_power_setpoint: {
            value: 0,
            enabled: false,
            options: [],
        },
    }
}
