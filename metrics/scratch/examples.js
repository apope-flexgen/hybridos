// eslint-disable-next-line no-unused-vars
const selectExamples = [
    {
        /*
    Example of selectorn. selectorn sets the output to the index (+1) of the first input
    in the listthat evaluates to truthy. For example, if below FFR_enable_flag = false,
    FRRS_enable_flag = true, and RRS_GEN_enable_flag = true, the output would be set to
    2 since the 2nd input in the list is the first that evaluates to truthy.
*/
        id: 'selectorn_output',
        inputs: [
            { uri: '/components/odessa', id: 'FFR_enable_flag' },
            { uri: '/components/odessa', id: 'FRRS_enable_flag' },
            { uri: '/components/odessa', id: 'RRS_GEN_enable_flag' },
            { uri: '/components/odessa', id: 'RRS_LOAD_enable_flag' },
            { uri: '/components/odessa', id: 'UF_events_disable_flag' },
        ],
        operation: 'selectorn',
        initialInput: 5,
    },
    /*
    Example of selectn. This acts like a mux. The first input contains an index that points to
    one of the other inputs in the array. The output will be set to the value of the selected input.
*/
    {
        id: 'UF_slew_rate',
        inputs: [
            { uri: '/components/odessa', id: 'selectorn_output' },
            { uri: '/components/constants', id: 'FFR_UF_slew_rate' },
            { uri: '/components/constants', id: 'FRRS_UF_slew_rate' },
            { uri: '/components/constants', id: 'RRS_GEN_UF_slew_rate' },
            { uri: '/components/constants', id: 'RRS_LOAD_UF_slew_rate' },
            { uri: '/components/constants', id: 'PFR_UF_slew_rate' },
        ],
        operation: 'selectn',
        initialInput: 0,
        outputs: [
            { uri: '/sites/odessa', id: 'fr_UF_slew_rate' },
        ],
    },
];

const messages = [
    {
        method: 'pub',
        uri: '/components/ess_1',
        body: {
            bms1_max_cell_temp: 27.4,
            bms2_max_cell_temp: 26.4,
            bms3_max_cell_temp: 26.9,
            bms4_max_cell_temp: 27.4,
        },
        comment: 'URI is complete, bodies are naked, max 27.4',
        pubUri: '/metrics/kpi',
        id: ['system_max_cell_temp'],
    },
    {
        method: 'pub',
        uri: '/components/ess_1',
        body: {
            bms1_max_cell_temp: { value: 17.4 },
            bms2_max_cell_temp: { value: 16.4 },
            bms3_max_cell_temp: { value: 16.9 },
            bms4_max_cell_temp: { value: 17.4 },
        },
        comment: 'URI is complete, bodies are valued, max 17.4',
        pubUri: '/metrics/kpi',
        id: ['system_max_cell_temp', 'compare_eq', 'compare_lt', 'compare_ltref',
            'compareand_eq', 'compareand_lt', 'compareor_eq', 'compareor_lt'],
    },
    {
        method: 'pub',
        uri: '/components',
        body: {
            ess_1: {
                bms1_max_cell_temp: { value: 7.4 },
                bms2_max_cell_temp: { value: 6.9 },
                bms3_max_cell_temp: { value: 6.4 },
                bms4_max_cell_temp: { value: 7.4 },
            },
            ess_2: {
                "don't": { value: 988 },
                care: 12.2,
            },
        },
        comment: 'URI is partial, need to get rest from body, max 7.4',
        pubUri: '/metrics/kpi',
        id: ['system_max_cell_temp', 'compare_eq', 'compare_lt', 'compare_ltref',
            'compareand_eq', 'compareand_lt', 'compareor_eq', 'compareor_lt'],
    },
    {
        method: 'pub',
        uri: '/components/ess_1/bms1_max_cell_temp',
        body: { value: 8.0 },
        comment: 'URI also contains id, max 8.0',
        pubUri: '/metrics/kpi',
        id: ['system_max_cell_temp'],
    },
    {
        method: 'pub',
        uri: '/assets/components/ess_1',
        body: {
            whoot: 'iver',
            gerry: { bander: 1 },
        },
        comment: 'Garbage pub, should not die',
        pubUri: '/metrics/kpi',
        id: ['system_max_cell_temp'],
    },
    {
        method: 'pub',
        uri: '/assets/feeders/feed_1',
        body: {
            ac_active_power: { value: 4000 },
        },
        comment: 'Test for accumulate',
        pubUri: '/metrics/kpi',
        id: ['accumulate_test'],
    },
    {
        method: 'pub',
        uri: '/assets/feeders/feed_1',
        body: {
            ac_active_power: { value: 4500 },
        },
        comment: 'Test for accumulate',
        pubUri: '/metrics/kpi',
        id: ['accumulate_test'],
    },
    {
        method: 'pub',
        uri: '/metrics/kpi',
        body: {
            echo_test: 908,
        },
        comment: 'Test for echo on pub',
        pubUri: '/metrics/kpi',
        id: ['echo_test'],
    },
    {
        method: 'set',
        uri: '/metrics/kpi',
        body: {
            echo_test: 537,
        },
        comment: 'Test for echo on set',
        pubUri: '/metrics/kpi',
        id: ['echo_test'],
    },
    {
        method: 'pub',
        uri: '/components/ess_1',
        body: {
            divisor_small_pos: 0.000000001,
            divisor_small_neg: -0.000000001,
            divisor_zero: 0,
        },
        comment: 'Tests for quotients',
        pubUri: '/metrics/kpi',
        id: ['system_product_quotient_cell_temp'],
    },
    {
        uri: '/components/feeders/feed_1',
        method: 'pub',
        body: {
            boolean_1: false,
            boolean_2: false,
            boolean_3: false,
            boolean_4: true,
        },
        comment: 'Test for OR',
        pubUri: '/metrics/kpi',
        id: ['or_true', 'srff', 'select', 'select_ref'],
    },
    {
        method: 'pub',
        uri: '/components/feeders/feed_1',
        body: {
            boolean_3: false,
            boolean_4: false,
        },
        comment: 'Test for SRFF keep',
        pubUri: '/metrics/kpi',
        id: ['srff', 'select', 'select_ref'],
    },
    {
        method: 'pub',
        uri: '/components/feeders/feed_1',
        body: {
            boolean_3: true,
            boolean_4: false,
        },
        comment: 'Test for SRFF false',
        pubUri: '/metrics/kpi',
        id: ['srff'],
    },
    {
        method: 'pub',
        uri: '/components/ess_1',
        body: {
            status: [{ value: 5, string: 'Fault' }, { value: 7, string: 'Stop' }],
        },
        comment: 'Test for AND',
        pubUri: '/metrics/kpi',
        id: ['and_integer'],
    },
    {
        method: 'pub',
        uri: '/components/ess_2',
        body: {
            status: [{ value: 4, string: 'Running' }],
        },
        comment: 'Test for AND pt 2',
        pubUri: '/metrics/kpi',
        id: ['and_bitfield'],
    },
    {
        method: 'pub',
        uri: '/components/ess_3',
        body: {
            status: [{ value: 4, string: 'Running' }],
        },
        comment: 'Test for Bitfield Count',
        pubUri: '/metrics/kpi',
        id: ['bitfield_count'],
    },
    {
        method: 'pub',
        uri: '/metrics',
        body: {
            charge_energy: 200,
            discharge_energy: 600,
            power: 100,
            power2: -100,
        },
        comment: 'Test for runtime',
        pubUri: '/metrics/kpi',
        id: ['runtime_charge', 'runtime_discharge'],
    },
    {
        method: 'pub',
        uri: '/metrics',
        body: {
            charge_energy: 200,
            discharge_energy: 600,
            power: 10,
            power2: -10,
        },
        comment: 'Test for runtime under threshold',
        pubUri: '/metrics/kpi',
        id: ['runtime_charge', 'runtime_discharge'],
    },
    {
        method: 'pub',
        uri: '/components/11_f5_sel_351s',
        body: {
            breaker_failure: false,
            frequency_trip: false,
            overvoltage_trip: false,
            undervoltage_trip: false,
            time_overcurrent_trip: false,
            instant_definite_overcurrent_trip: false,
            switch_on_to_fault: true,
        },
        comment: 'Testing bitfield empty',
        pubUri: '/components/11_f5_sel_351s',
        id: ['breaker_relay_faults'],
    },
    {
        method: 'pub',
        uri: '/components/11_f5_sel_351s',
        body: {
            breaker_failure: false,
            frequency_trip: false,
            overvoltage_trip: true,
            undervoltage_trip: false,
            time_overcurrent_trip: false,
            instant_definite_overcurrent_trip: false,
            switch_on_to_fault: true,
        },
        comment: 'Testing bitfield position 2',
        pubUri: '/components/11_f5_sel_351s',
        id: ['breaker_relay_faults'],
    },
    {
        method: 'pub',
        uri: '/components/11_f5_sel_351s',
        body: {
            breaker_failure: false,
            frequency_trip: false,
            overvoltage_trip: false,
            undervoltage_trip: true,
            time_overcurrent_trip: false,
            instant_definite_overcurrent_trip: true,
            switch_on_to_fault: false,
        },
        comment: 'Testing bitfield position 3, 5, 6',
        pubUri: '/components/11_f5_sel_351s',
        id: ['breaker_relay_faults'],
    },
    {
        method: 'pub',
        uri: '/components/ess_1',
        body: {
            active_power: 150,
        },
        comment: 'Testing discharge',
        pubUri: '/metrics/kpi',
        id: ['charge_from_solar', 'charge_from_grid', 'discharge'],
    },
    {
        method: 'pub',
        uri: '/components/pv_1',
        body: {
            active_power: 650,
        },
        comment: 'Setting up solar',
        pubUri: '/metrics/kpi',
        id: ['charge_from_solar', 'charge_from_grid', 'discharge'],
    },
    {
        method: 'pub',
        uri: '/components/ess_1',
        body: {
            active_power: -150,
        },
        comment: 'Testing full charge from solar',
        pubUri: '/metrics/kpi',
        id: ['charge_from_solar', 'charge_from_grid', 'discharge'],
    },
    {
        method: 'pub',
        uri: '/components/pv_1',
        body: {
            active_power: 50,
        },
        comment: 'Testing partial charge from solar',
        pubUri: '/metrics/kpi',
        id: ['charge_from_solar', 'charge_from_grid', 'discharge'],
    },
];

module.exports = messages;
