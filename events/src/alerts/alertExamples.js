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
            id: '28beecbc-232f-431b-ac7d-8d29350e9000.lima',
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
            id: '28beecbc-232f-431b-ac7d-8d29350e9000.sudden_valley',
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
            version: 1, aliases: [], title: 'VFakeCo - WH & JC Cell Voltage', deadline: 30, expression: 'Minimum Cell Voltage: > 3.22V OR Cell Voltage Delta: >.25V', organizationId: 'org_id1', severity: 0, updatedAt: times.days_ago(15),
        },
        {
            version: 2, aliases: [], title: 'VFakeCo - WH & JC Cell Voltage', deadline: 20, expression: 'Minimum Cell Voltage: > 3.22V OR Cell Voltage Delta: >.25V', organizationId: 'org_id1', severity: 0, updatedAt: times.days_ago(14),
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
    expression: 'ess_faults == true || ess_watchdog == true',
    organization: 'VFakeCo',
    severity: 0,
    title: 'VFakeCo - WH & JC Cell Voltage',
    aliases: [
        {
            alias: 'ess_faults',
            uri: '/{site}/ess_{ess}/is_faulted',
            type: 'bool',
        },
        {
            alias: 'ess_watchdog',
            uri: '/{site}/ess_{ess}/watchdog_status',
            type: 'bool',
        },
    ],
    messages: [
        {
            'ess_faults == true': 'ESS {ess} faults present. Value is {ess_faults}',
            'ess_watchdog == true': 'ESS {ess} watchdog timed out. Value is {ess_watchdog}',
        },
    ],
    templates: [
        {
            from: 1,
            to: 2,
            token: '{ess}',
            type: 'sequential',
        },
        {
            list: ['lima', 'sudden_valley'],
            token: '{site}',
            type: 'list',
        },
    ],
};

// POST /events/alerts/management
const postManagementRequestDeletion = {
    id: '28beecbc-232f-431b-ac7d-8d29350e9000',
    deleted: true,
    templates: [], // delete all templates
};

// GET /events/alerts/management
const getManagementRequest = {
    limit: 10,
    page: 1,
    sort: 1,
};

const getManagementResponses = (() => {
    const base = {
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
    const afterInsert = {
        rows: [
            base.rows[0],
            base.rows[1],
            {
                id: 'mocked-new-uuid',
                updatedAt: '2020-07-07T12:00:00.000Z',
                aliases: [
                    {
                        alias: 'ess_faults',
                        uri: '/{site}/ess_{ess}/is_faulted',
                        type: 'bool',
                    },
                    {
                        alias: 'ess_watchdog',
                        uri: '/{site}/ess_{ess}/watchdog_status',
                        type: 'bool',
                    },
                ],
                messages: [
                    {
                        'ess_faults == true': 'ESS {ess} faults present. Value is {ess_faults}',
                        'ess_watchdog == true': 'ESS {ess} watchdog timed out. Value is {ess_watchdog}',
                    },
                ],
                enabled: true,
                deleted: false,
                deadline: 30,
                expression: 'ess_faults == true || ess_watchdog == true',
                lastTriggered: null,
                severity: null,
                title: 'VFakeCo - WH & JC Cell Voltage',
                version: 0,
                history: [],
                organization: 'VFakeCo',
            },
        ],
        templates: JSON.parse(JSON.stringify(postManagementRequest.templates)),
    };
    afterInsert.templates[0].id = 'mocked-new-template-uuid-0';
    afterInsert.templates[1].id = 'mocked-new-template-uuid-1';
    const {
        history, id, lastTriggered, updatedAt, enabled, deleted, ...historyEntry
    } = afterInsert.rows[1];
    const afterUpdate = {
        rows: [
            base.rows[0],
            {
                ...afterInsert.rows[2],
                id: '28beecbc-232f-431b-ac7d-8d29350e9000',
                version: 4,
                lastTriggered: '2020-07-07T12:00:00.000Z',
                history: [
                    base.rows[1].history[0],
                    base.rows[1].history[1],
                    historyEntry,
                ],
            },
        ],
        // deep clone
        templates: JSON.parse(JSON.stringify(afterInsert.templates)),
    };
    return {
        base,
        afterUpdate,
        afterInsert,
    };
})();

// POST /events/alerts
const postIncidentRequest = {
    details: [
        {
            message: 'Site discharge 11000.00kW exceeded max POI limit 10000.00kW',
            timestamp: '2020-07-07T12:00:00.000Z',
        },
    ],
    name: 'doesnt-matter',
    status: 'active',
    value: true,
    source: 'Alerts',
    config_id: '28beecbc-232f-431b-ac7d-8d29350e9000',
    incident_id: '28beecbc-232f-431b-ac7d-8d29350e9000.lima',
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
            id: '28beecbc-232f-431b-ac7d-8d29350e9000.lima',
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
            id: '28beecbc-232f-431b-ac7d-8d29350e9000.sudden_valley',
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

// -> GET /go_metrics_alerting/events/alerts
const outboundGetAlerts = {
    // TODO
};

// -> SET /go_metrics_alerting/configuration/<id>
const outboundSetManagement = {
    templates: [
        {
            id: 'mocked-new-template-uuid-0',
            from: 1,
            to: 2,
            token: '{ess}',
            type: 'sequential',
        },
        {
            id: 'mocked-new-template-uuid-1',
            list: ['lima', 'sudden_valley'],
            token: '{site}',
            type: 'list',
        },
    ],
    inputs: {
        'ess_faults_{ess}_{site}': {
            uri: '/{site}/ess_{ess}/is_faulted',
            type: 'bool',
        },
        'ess_watchdog_{ess}_{site}': {
            uri: '/{site}/ess_{ess}/watchdog_status',
            type: 'bool',
        },
    },
    outputs: {
        '28beecbc-232f-431b-ac7d-8d29350e9000_{ess}_{site}': {
            uri: '/events/alerts',
            flags: ['clothed', 'post', 'sparse', 'flat', 'lonely', 'no_heartbeat'],
            attributes: { source: 'Alerts', config_id: '28beecbc-232f-431b-ac7d-8d29350e9000' },
        },
    },
    metrics: [
        {
            id: '28beecbc-232f-431b-ac7d-8d29350e9000_{ess}_{site}',
            type: 'bool',
            outputs: '28beecbc-232f-431b-ac7d-8d29350e9000_{ess}_{site}',
            expression: 'ess_faults_{ess}_{site} == true || ess_watchdog_{ess}_{site} == true',
            alert: true,
            messages: [
                {
                    'ess_faults_{ess}_{site} == true': 'ESS {ess} faults present. Value is {ess_faults_{ess}_{site}}',
                    'ess_watchdog_{ess}_{site} == true': 'ESS {ess} watchdog timed out. Value is {ess_watchdog_{ess}_{site}}',
                },
            ],
            enabled: true,
        },
    ],
};
const outboundSetManagementNew = {
    templates: outboundSetManagement.templates,
    inputs: outboundSetManagement.inputs,
    outputs: {
        'mocked-new-uuid_{ess}_{site}': {
            uri: '/events/alerts',
            flags: ['clothed', 'post', 'sparse', 'flat', 'lonely', 'no_heartbeat'],
            attributes: { source: 'Alerts', config_id: 'mocked-new-uuid' },
        },
    },
    metrics: [
        {
            id: 'mocked-new-uuid_{ess}_{site}',
            type: 'bool',
            outputs: 'mocked-new-uuid_{ess}_{site}',
            expression: 'ess_faults_{ess}_{site} == true || ess_watchdog_{ess}_{site} == true',
            alert: true,
            messages: [
                {
                    'ess_faults_{ess}_{site} == true': 'ESS {ess} faults present. Value is {ess_faults_{ess}_{site}}',
                    'ess_watchdog_{ess}_{site} == true': 'ESS {ess} watchdog timed out. Value is {ess_watchdog_{ess}_{site}}',
                },
            ],
            enabled: true,
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
    getManagementResponses, // to the UI

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
