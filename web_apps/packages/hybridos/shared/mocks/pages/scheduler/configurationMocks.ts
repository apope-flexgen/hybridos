import { Configuration } from '../../../../../hybridos/shared/types/dtos/scheduler.dto'

/** VALID SITE CONTROLLER CONFIGURATION MOCK */

// export const schedulerConfigurationData = {
//     scheduler_type: 'SC',
//     local_schedule: {
//         id: 'durham',
//         name: 'Durham',
//         setpoint_enforcement: {
//             enabled: true,
//             frequency_seconds: 5,
//         },
//     },
//     web_sockets: {
//         server: {
//             enabled: true,
//             port: 9000,
//         },
//     },
//     scada: {
//         stage_size: 5,
//         max_num_events: 20,
//         num_floats: 1,
//         num_ints: 0,
//         num_bools: 1,
//         num_strings: 0,
//     },
// }

/** VALID FLEET MANAGER CONFIGURATION MOCK */

// export const schedulerConfigurationData = {
//     scheduler_type: 'FM',
//     local_schedule: {
//         id: 'fleet_id',
//         name: 'Horizon Power Fleet',
//     },
//     web_sockets: {
//         clients: [
//             {
//                 id: 'raleigh',
//                 name: 'Raleigh',
//                 ip: '172.16.2.82',
//                 port: 9000
//             },
//             {
//                 id: 'durham',
//                 name: 'Durham',
//                 ip: '172.16.2.83',
//                 port: 9000
//             }
//         ],
//     },
//     scada: {
//         stage_size: 1,
//         max_num_events: 20,
//         num_floats: 1,
//         num_ints: 0,
//         num_bools: 1,
//         num_strings: 0,
//     },
// }

/** INVALID CONFIGURATION OBJECT. THIS WILL PULL FROM web_ui.json */

// export const schedulerConfigurationData = {}

/** TESTING: swap schedulerType to SC / FM to show other configuration states. */

export const schedulerConfigurationData = {
    scheduler_type: 'SC',
    local_schedule: {
        id: 'raleigh',
        name: 'Raleigh'
    }
}
