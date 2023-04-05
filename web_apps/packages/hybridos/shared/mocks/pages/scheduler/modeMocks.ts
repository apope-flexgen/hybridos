import { Mode } from '../../../types/dtos/scheduler.dto'
import { ApiMode } from '../../../types/dtos/scheduler.dto';

export const modesData: ApiMode = {
    default: {
        name: 'Default',
        color_code: 'gray',
        icon: 'ArrowUp',
        variables: [],
        constants: [
            {
                id: 'charge_flag',
                name: 'Charge Flag',
                type: 'Bool',
                unit: '',
                uri: '/features/active_power/absolute_ess_direction_flag',
                value: false,
            },
            {
                id: 'kw_cmd',
                name: 'Absolute ESS Setpoint',
                type: 'Float',
                unit: 'kW',
                uri: '/features/active_power/absolute_ess_kW_cmd',
                value: 0.0,
            },
            {
                id: 'soc_target',
                name: 'Target SoC Setpoint',
                type: 'Float',
                unit: '%',
                uri: '/features/active_power/ess_charge_control_target_soc',
                value: 0,
            },
        ],
    },
    charging: {
        name: 'Charging',
        color_code: 'lightGreen',
        icon: 'BatteryCharging',
        variables: [
            {
                id: 'charge_cmd',
                name: 'Manual Charge command',
                type: 'Float',
                unit: 'kW',
                uri: '/features/active_power/manual_ess_kW_cmd',
                value: 5000,
            },
            {
                id: 'active_pwr',
                name: 'Active Power command',
                type: 'Float',
                unit: 'kW',
                uri: '/features/active_power/',
                value: 2500,
            },
        ],
        constants: [
            {
                id: 'charge_feature',
                name: 'Feature Mode Selection',
                type: 'Int',
                unit: '',
                uri: '/features/active_power/features_kW_Mode_cmd',
                value: 2,
            },
        ],
    },
    discharging: {
        name: 'Discharging',
        color_code: 'red',
        icon: 'BatteryVert',
        variables: [],
        constants: [
            {
                id: 'charge_feature',
                name: 'Feature Mode Selection',
                type: 'Int',
                unit: '#FF5449',
                uri: '/features/active_power/features_kW_Mode_cmd',
                value: 2,
            },
        ],
    },
    maintenance: {
        name: 'Maintenance',
        color_code: 'gray',
        icon: 'Build',
        variables: [
            {
                id: 'reactive_pwr',
                name: 'Reactive Power command',
                type: 'Float',
                unit: 'kW',
                uri: '/features/active_power/',
                value: 350,
            },
        ],
        constants: [
            {
                id: 'charge_feature',
                name: 'Feature Mode Selection',
                type: 'Int',
                unit: '',
                uri: '/features/active_power/features_kW_Mode_cmd',
                value: 2,
            },
        ],
    },
}
