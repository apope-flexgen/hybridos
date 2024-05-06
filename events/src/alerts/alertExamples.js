/* eslint-disable import/prefer-default-export */
/**
 * The contents of this file are both for reference and utilized by alerts.test.js
 */

// Example Mongo Documents

/**
 * Example data to be loaded into mongo when loadData is invoked.
 */
const times = {
    now: new Date().toISOString(),
    hours_ago: (n) => new Date(new Date().getTime() - (n * 60 * 60 * 1000)).toISOString(),
    days_ago: (n) => new Date(new Date().getTime() - (n * 24 * 60 * 60 * 1000)).toISOString(),
};
const alertDatabaseEntry = {
    // internal fields
    id: '28beecbc-232f-431b-ac7d-8d29350e9000',
    createdAt: times.days_ago(15),
    lastTriggered: times.hours_ago(1),

    // history fields
    updatedAt: times.hours_ago(5),
    version: 3,

    // UI settable fields
    deadline: 30,
    deleted: false,
    enabled: true, // not tracked in history
    expression: 'temp > 90 || high_temp_fault == true',
    organizationId: 'org_id1',
    severity: 0,
    title: 'VFakeCo - WH & JC Cell Voltage',
    aliases: [
        {
            alias: 'temp',
            uri: '/assets/ess/ess_2/max_temp',
            type: 'float',
        },
        {
            alias: 'high_temp_fault',
            uri: '/assets/ess/ess_2/is_faulted',
            type: 'boolean',
        },
    ],
    messages: [
        { 'temp > 90': 'Temperature: temp exceeded 90C' },
        { 'high_temp_fault == true': 'There was a high temperature fault' },
    ],

    // sorted by trigger time inherently, when resolved the item is sliced out.
    unresolved: [
        {
            id: 'dummy_incident_id3',
            status: 'active',
            version: 2,
            details: [
                {
                    message: 'Minimum Cell Voltage: > 3.22V',
                    timestamp: times.hours_ago(2),
                },
                {
                    message: 'Cell Voltage Delta: >.25V',
                    timestamp: times.hours_ago(1),
                },
            ],
        },
    ],
    // sorted by resolution_time
    resolved: [
        {
            id: 'dummy_incident_id',
            resolution_message: 'Cell voltage has returned to normal levels - noted and resolved',
            resolution_time: times.hours_ago(3),
            version: 2,
            details: [
                {
                    message: 'Minimum Cell Voltage: > 3.22V',
                    timestamp: times.days_ago(3),
                },
                {
                    message: 'Cell Voltage Delta: >.25V',
                    timestamp: times.days_ago(2),
                },
            ],
        },
    ],
    history: [
        {
            version: 1, title: 'VFakeCo - WH & JC Cell Voltage', deadline: 30, expression: 'Minimum Cell Voltage: > 3.22V OR Cell Voltage Delta: >.25V', organizationId: 'org_id1', severity: 0, updatedAt: times.days_ago(15),
        },
        {
            version: 2, title: 'VFakeCo - WH & JC Cell Voltage', deadline: 20, expression: 'Minimum Cell Voltage: > 3.22V OR Cell Voltage Delta: >.25V', organizationId: 'org_id1', severity: 0, updatedAt: times.days_ago(14),
        },
    ],
};

const alertDatabaseEntries = [
    alertDatabaseEntry,
    {
        ...alertDatabaseEntry,
        id: '71d291ea-d2b6-498b-ae7c-b2e97684acd2',
        title: 'Basic alert without any violations yet',
        organizationId: 'org_id5',
        expression: 'SOC > 95',
        aliases: [
            {
                alias: 'SOC',
                uri: '/assets/ess/ess_2/soc',
                type: 'float',
            },
        ],
        messages: [
            { 'SOC > 95': 'SOC: State of Charge is Dangerously High' },
        ],
        createdAt: times.hours_ago(3),
        updatedAt: times.hours_ago(3),
        version: 1,
        severity: 5,
        resolved: [],
        unresolved: [],
        history: [],
    },
];

const templateEntries = [
    {
        id: 'existing_template_id1',
        type: 'list',
        list: ['lima', 'sudden_valley'],
        token: '{site_id}',
    },
    {
        id: 'existing_template_id2',
        type: 'sequential',
        from: 1,
        to: 10,
        minWidth: 2,
        token: '{ess}',
    },
];

const organizationEntries = [
    { id: 'org_id12351235123', name: 'James at his desk' },
    { id: 'org_id1', name: 'VFakeCo' },
    { id: 'org_id5', name: 'VRealCo' },
];

/**
 * Requests and responses
 */
const genericSuccessResponse = {
    success: true,
    message: null,
};

// POST /events/alerts/management
const postManagementRequest = {
    id: '28beecbc-232f-431b-ac7d-8d29350e9000',
    deadline: 30,
    deleted: false,
    enabled: true, // not tracked in history
    expression: 'Minimum Cell Voltage: > 3.22V OR Cell Voltage Delta: >.25V',
    organization: 'VFakeCo',
    severity: 0,
    title: 'VFakeCo - WH & JC Cell Voltage',
    messages: [
        {
            'temp > 90': 'Temperature: temp exceeded 451F',
        },
        {
            'high_temp_fault == true': 'There was a high temperature fault',
        },
    ],
    aliases: [
        {
            alias: 'ESS 2 SOC',
            uri: '/assets/ess/ess_2/soc',
            type: 'float',
        },
    ],
    templates: [
        {
            type: 'sequential',
            from: 1,
            to: 16,
            step: 3,
            token: '##',
        },
        {
            id: 'existing_template_id1',
            type: 'list',
            list: [
                'bobcat',
                'cheetah',
                'lion',
            ],
            token: 'qq',
        },
    ],
};

// POST /events/alerts/management
const postManagementRequestDeletion = {
    id: '28beecbc-232f-431b-ac7d-8d29350e9000',
    deleted: true,
};

// GET /events/alerts/management
const getManagementRequest = {
    limit: 10,
    page: 1,
    sort: 1,
};
const getManagementResponse = {
    templates: templateEntries,
    rows: [
        {
            id: '71d291ea-d2b6-498b-ae7c-b2e97684acd2',
            updatedAt: '2020-07-07T12:00:00.000Z',
            expression: 'SOC > 95',
            aliases: [
                {
                    alias: 'SOC',
                    uri: '/assets/ess/ess_2/soc',
                    type: 'float',
                },
            ],
            messages: [
                { 'SOC > 95': 'SOC: State of Charge is Dangerously High' },
            ],
            enabled: true,
            deleted: false,
            deadline: 30,
            lastTriggered: '2020-07-07T12:00:00.000Z',
            organization: 'VRealCo',
            severity: 5,
            title: 'Basic alert without any violations yet',
            version: 1,
            history: [],
        },
        {
            id: '28beecbc-232f-431b-ac7d-8d29350e9000',
            updatedAt: '2020-07-07T12:00:00.000Z',
            expression: 'temp > 90 || high_temp_fault == true',
            aliases: [
                {
                    alias: 'temp',
                    uri: '/assets/ess/ess_2/max_temp',
                    type: 'float',
                },
                {
                    alias: 'high_temp_fault',
                    uri: '/assets/ess/ess_2/is_faulted',
                    type: 'boolean',
                },
            ],
            messages: [
                { 'temp > 90': 'Temperature: temp exceeded 90C' },
                { 'high_temp_fault == true': 'There was a high temperature fault' },
            ],
            enabled: true,
            deleted: false,
            deadline: 30,
            lastTriggered: '2020-07-07T12:00:00.000Z',
            organization: 'VFakeCo',
            severity: 0,
            title: 'VFakeCo - WH & JC Cell Voltage',
            version: 3,
            history: [
                {
                    aliases: [],
                    messages: [],
                    deadline: 30,
                    expression: 'Minimum Cell Voltage: > 3.22V OR Cell Voltage Delta: >.25V',
                    organization: 'VFakeCo',
                    severity: 0,
                    title: 'VFakeCo - WH & JC Cell Voltage',
                    version: 1,
                },
                {
                    aliases: [],
                    messages: [],
                    deadline: 20,
                    expression: 'Minimum Cell Voltage: > 3.22V OR Cell Voltage Delta: >.25V',
                    organization: 'VFakeCo',
                    severity: 0,
                    title: 'VFakeCo - WH & JC Cell Voltage',
                    version: 2,
                },
            ],
        },
    ],
};

// POST /events/alerts
const postIncidentRequest = {
    details: [
        {
            message: 'Site discharge 11000.00kW exceeded max POI limit 10000.00kW',
            timestamp: '2024-04-29T13:09:45-04:00',
        },
    ],
    name: '28beecbc-232f-431b-ac7d-8d29350e9000',
    status: 'active',
    value: true,
};

// SET /events/alerts
const setIncidentRequest = {
    resolved: true,
    resolution_message: 'Cell voltage has returned to normal levels!!!',
};

// GET /events/alerts
const getIncidentsRequest = {
    limit: 10,
    startTime: '2023-03-13T17:47:33.820Z',
    endTime: times.now,

    // filters
    severityFilter: 0,
    orgFilter: 'VFakeCo',
    resolvedFilter: false,
    aliasFilters: ['alias1', 'alias2'],
    statusFilter: 'active',
};
const getIncidentsResponseUnresolved = {
    count: 1,
    rows: [
        {
            organization: 'VFakeCo',
            severity: 0,
            deadline: 30,
            title: 'VFakeCo - WH & JC Cell Voltage',
            id: 'dummy_incident_id3',
            status: 'active',
            trigger_time: '2020-07-07T12:00:00.000Z',
            details: [
                {
                    message: 'Minimum Cell Voltage: > 3.22V',
                    timestamp: '2020-07-07T12:00:00.000Z',
                },
                {
                    message: 'Cell Voltage Delta: >.25V',
                    timestamp: '2020-07-07T12:00:00.000Z',
                },
            ],
            resolved: false,
            resolution_time: null,
            resolution_message: null,
        },
    ],
};

const getIncidentsResponseResolved = {
    count: 1,
    rows: [
        {
            organization: 'VFakeCo',
            severity: 0,
            deadline: 20,
            title: 'VFakeCo - WH & JC Cell Voltage',
            id: 'dummy_incident_id',
            resolved: true,
            trigger_time: '2020-07-07T12:00:00.000Z',
            details: [
                {
                    message: 'Minimum Cell Voltage: > 3.22V',
                    timestamp: '2020-07-07T12:00:00.000Z',
                },
                {
                    message: 'Cell Voltage Delta: >.25V',
                    timestamp: '2020-07-07T12:00:00.000Z',
                },
            ],
            resolution_time: '2020-07-07T12:00:00.000Z',
            resolution_message: 'Cell voltage has returned to normal levels - noted and resolved',
        },
    ],
};

// -> PUB /events/alerts (message varies)

// -> GET /go_metrics/events/alerts
const outboundGetAlerts = {
    // TODO
};

// -> SET /go_metrics/alerts/configuration
const outboundSetManagement = {
    inputs: {
        ess_2_soc: {
            uri: '/assets/ess/ess_2/soc',
            type: 'float',
        },
    },
    templates: [
        {
            id: 'existing_template_id1',
            type: 'list',
            list: [
                'bobcat',
                'cheetah',
                'lion',
            ],
            token: 'qq',
        },
        {
            id: 'existing_template_id2',
            type: 'sequential',
            from: 1,
            to: 10,
            minWidth: 2,
            token: '{ess}',
        },
        {
            id: 'mocked-new-template-uuid',
            type: 'sequential',
            from: 1,
            to: 16,
            step: 3,
            token: '##',
        },
    ],
    outputs: {
        '28beecbc-232f-431b-ac7d-8d29350e9000': {
            uri: '/events/alerts',
            flags: ['clothed', 'post', 'sparse', 'flat', 'lonely', 'no_heartbeat'],
        },
    },
    attributes: { source: 'Alerts' },
    metrics: [
        {
            id: '28beecbc-232f-431b-ac7d-8d29350e9000',
            outputs: '28beecbc-232f-431b-ac7d-8d29350e9000',
            type: 'bool',
            alert: true,
            expression: 'Minimum Cell Voltage: > 3.22V OR Cell Voltage Delta: >.25V',
            enabled: true,
            messages: [
                {
                    'temp > 90': 'Temperature: temp exceeded 451F',
                },
                {
                    'high_temp_fault == true': 'There was a high temperature fault',
                },
            ],
        },
    ],
};

const outboundSetManagementNew = {
    inputs: {
        ess_2_soc: {
            uri: '/assets/ess/ess_2/soc',
            type: 'float',
        },
    },
    templates: [
        {
            id: 'existing_template_id1',
            type: 'list',
            list: [
                'bobcat',
                'cheetah',
                'lion',
            ],
            token: 'qq',
        },
        {
            id: 'existing_template_id2',
            type: 'sequential',
            from: 1,
            to: 10,
            minWidth: 2,
            token: '{ess}',
        },
        {
            id: 'mocked-new-uuid',
            type: 'sequential',
            from: 1,
            to: 16,
            step: 3,
            token: '##',
        },
    ],
    outputs: {
        'mocked-new-uuid': {
            uri: '/events/alerts',
            flags: ['clothed', 'post', 'sparse', 'flat', 'lonely', 'no_heartbeat'],
        },
    },
    attributes: { source: 'Alerts' },
    metrics: [
        {
            id: 'mocked-new-uuid',
            outputs: 'mocked-new-uuid',
            type: 'bool',
            alert: true,
            expression: 'Minimum Cell Voltage: > 3.22V OR Cell Voltage Delta: >.25V',
            enabled: true,
            messages: [
                {
                    'temp > 90': 'Temperature: temp exceeded 451F',
                },
                {
                    'high_temp_fault == true': 'There was a high temperature fault',
                },
            ],
        },
    ],
};

module.exports = {
    // mongo entries
    alertDatabaseEntry,
    alertDatabaseEntries,
    templateEntries,
    organizationEntries,

    // generic
    genericSuccessResponse,

    // management
    postManagementRequest, // from the UI
    postManagementRequestDeletion, // from the UI
    getManagementRequest, // from the UI
    getManagementResponse, // to the UI

    // incidents
    postIncidentRequest, // from go_metrics
    setIncidentRequest, // from the UI
    setIncidentResponse: genericSuccessResponse, // to the UI
    getIncidentsRequest, // from the UI
    getIncidentsResponseUnresolved, // to the UI
    getIncidentsResponseResolved, // to the UI

    // outbound events requests
    outboundGetAlerts, // to go_metrics
    outboundSetManagement, // to go_metrics
    outboundSetManagementNew, // to go_metrics

    // utils
    times,
};
