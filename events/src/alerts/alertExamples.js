/**
 * The contents of this file are both for reference and utilized by alerts.test.js
 */

// Example Mongo Document
const times = {
    now: new Date().toISOString(),
    hours_ago: (n) => new Date(new Date().getTime() - (n * 60 * 60 * 1000)).toISOString(),
    days_ago: (n) => new Date(new Date().getTime() - (n * 24 * 60 * 60 * 1000)).toISOString(),
}
const databaseEntry = {
    // internal fields
    "id": "28beecbc-232f-431b-ac7d-8d29350e9000",
    "createdAt": times.days_ago(15),
    "lastTriggered": times.hours_ago(1),

    // history fields
    "updatedAt": times.hours_ago(5),
    "version": 3,

    "aliases": [{
        "alias": "ESS 2 SOC",
        "uri": "/assets/ess/ess_2/soc",
        "type": "float"
    }],
    "deadline": 30,
    "deleted": false,
    "enabled": true, // not tracked in history
    "expression": "Minimum Cell Voltage: > 3.22V OR Cell Voltage Delta: >.25V",
    "organization": "VFakeCo",
    "severity": 0,
    "sites": [ "Willow Hollow", "Juniper City" ],
	"templates": [
		{
			"type": "sequential",
			"from": 1,
			"to": 16,
			"step": 3,
			"token": "##"
        },
		{
			"type": "list",
			"list": ["bobcat", "cheetah", "lion"],
			"token": "qq"
        }
	],
    "title": "VFakeCo - WH & JC Cell Voltage",

    // sorted by trigger time inherently, when resolved the item is sliced out.
    // the max # here is # of sites.
    "unresolved": [
        // this one happened before there was an update to the alert, critically has a different "version" reference.
        {
            "id": "dummy_incident_id2",
            "status": "Inactive",
            "site": "Juniper City",
            "version": 2,
            "details": [
                {
                    message: 'Minimum Cell Voltage: > 3.22V',
                    "timestamp": times.hours_ago(15)
                }
            ]
        },
        {
            "id": "dummy_incident_id3",
            "status": "Active",
            "site": "Willow Hollow",
            "version": 3,
            "details": [
                {
                    message: 'Minimum Cell Voltage: > 3.22V', 
                    timestamp: times.hours_ago(2)
                },
                {
                    message: 'Cell Voltage Delta: >.25V',
                    timestamp: times.hours_ago(1) 
                }
            ]
        }
    ],
    // sorted by resolution_time
    "resolved": [
        {
            "id": "dummy_incident_id",
            "resolution_message": "Cell voltage has returned to normal levels - noted and resolved",
            "resolution_time": times.hours_ago(3),
            "site": "Juniper City",
            "version": 2,
            "details": [
                {
                    message: 'Minimum Cell Voltage: > 3.22V', 
                    timestamp: times.days_ago(3)
                },
                {
                    message: 'Cell Voltage Delta: >.25V',
                    timestamp: times.days_ago(2) 
                }
            ]
        }
    ],
    "history": [
        { "version": 1, "title": "VFakeCo - WH & JC Cell Voltage","deadline": 30, "expression": "Minimum Cell Voltage: > 3.22V OR Cell Voltage Delta: >.25V", "organization": "VFakeCo", "severity": 0, "sites": [ "Willow Hollow" ], "updatedAt": times.days_ago(15) },
        { "version": 2, "title": "VFakeCo - WH & JC Cell Voltage","deadline": 20, "expression": "Minimum Cell Voltage: > 3.22V OR Cell Voltage Delta: >.25V", "organization": "VFakeCo", "severity": 0, "sites": [ "Juniper City", "Willow Hollow" ], "updatedAt": times.days_ago(14)}
    ],
};

const databaseEntries = [
    databaseEntry,
    {
        ...databaseEntry,
        "id": "71d291ea-d2b6-498b-ae7c-b2e97684acd2",
        "title": "Basic alert without any violations yet",
        "organization": "VRealCo",
        "expression": "SOC > 20",
        "sites": "James's Laptop",
        createdAt: times.hours_ago(3),
        updatedAt: times.hours_ago(3),
        "version": 1,
        "severity": 5,
        resolved: [],
        unresolved: [],
        "history": []
    }
]


// POST /events/alerts/management
const postManagementRequest = {
    "id": "28beecbc-232f-431b-ac7d-8d29350e9000",
    "deadline": 30,
    "deleted": false,
    "enabled": true, // not tracked in history
    "expression": "Minimum Cell Voltage: > 3.22V OR Cell Voltage Delta: >.25V",
    "organization": "VFakeCo",
    "severity": 0,
    "sites": [ "Willow Hollow", "Juniper City" ],
    "title": "VFakeCo - WH & JC Cell Voltage",
    "aliases": [
        {
            "alias": "ESS 2 SOC",
            "uri": "/assets/ess/ess_2/soc",
            "type": "float"
        }
    ],
    "templates": [
        {
            "type": "sequential",
            "from": 1,
            "to": 16,
            "step": 3,
            "token": "##"
        },
        {
            "type": "list",
            "list": [
                "bobcat",
                "cheetah",
                "lion"
            ],
            "token": "qq"
        }
    ],
};
const postManagementResponse = {
    "success": true,
    "message": null
}

// GET /events/alerts/management
const getManagementRequest = {
    limit: 10,
    page: 1,
    sort: 1,
};
const getManagementResponse = [
    {
        "id": "71d291ea-d2b6-498b-ae7c-b2e97684acd2",
        "updatedAt": "2020-07-07T12:00:00.000Z",
        "aliases": [
            {
                "alias": "ESS 2 SOC",
                "uri": "/assets/ess/ess_2/soc",
                "type": "float"
            }
        ],
        "enabled": true,
        "deleted": false,
        "deadline": 30,
        "expression": "SOC > 20",
        "lastTriggered": "2020-07-07T12:00:00.000Z",
        "organization": "VRealCo",
        "severity": 5,
        "sites": [
            "James's Laptop"
        ],
        "templates": [
            {
                "type": "sequential",
                "from": 1,
                "to": 16,
                "step": 3,
                "token": "##"
            },
            {
                "type": "list",
                "list": [
                    "bobcat",
                    "cheetah",
                    "lion"
                ],
                "token": "qq"
            }
        ],
        "title": "Basic alert without any violations yet",
        "version": 1,
        "history": []
    },
    {
        "id": "28beecbc-232f-431b-ac7d-8d29350e9000",
        "updatedAt": "2020-07-07T12:00:00.000Z",
        "aliases": [
            {
                "alias": "ESS 2 SOC",
                "uri": "/assets/ess/ess_2/soc",
                "type": "float"
            }
        ],
        "enabled": true,
        "deleted": false,
        "deadline": 30,
        "expression": "Minimum Cell Voltage: > 3.22V OR Cell Voltage Delta: >.25V",
        "lastTriggered": "2020-07-07T12:00:00.000Z",
        "organization": "VFakeCo",
        "severity": 0,
        "sites": [
            "Willow Hollow",
            "Juniper City"
        ],
        "templates": [
            {
                "type": "sequential",
                "from": 1,
                "to": 16,
                "step": 3,
                "token": "##"
            },
            {
                "type": "list",
                "list": [
                    "bobcat",
                    "cheetah",
                    "lion"
                ],
                "token": "qq"
            }
        ],
        "title": "VFakeCo - WH & JC Cell Voltage",
        "version": 3,
        "history": [
            {
                "aliases": [],
                "deadline": 30,
                "expression": "Minimum Cell Voltage: > 3.22V OR Cell Voltage Delta: >.25V",
                "organization": "VFakeCo",
                "severity": 0,
                "sites": [
                    "Willow Hollow"
                ],
                "templates": [],
                "title": "VFakeCo - WH & JC Cell Voltage",
                "version": 1
            },
            {
                "aliases": [],
                "deadline": 20,
                "expression": "Minimum Cell Voltage: > 3.22V OR Cell Voltage Delta: >.25V",
                "organization": "VFakeCo",
                "severity": 0,
                "sites": [
                    "Juniper City",
                    "Willow Hollow"
                ],
                "templates": [],
                "title": "VFakeCo - WH & JC Cell Voltage",
                "version": 2
            }
        ]
    }
];

// POST /events/alerts
const postIncidentRequest = {
    "id": "28beecbc-232f-431b-ac7d-8d29350e9000",
    "uri": "/events/alerts",
    "method": "post",
    "organization": "VFakeCo",
    "site": "Willow Hollow",
    "source": "Alerts",
    "status": "Active",
    "severity": 0,
    "details": [
        {
            message: 'Minimum Cell Voltage: > 3.22V',
            "timestamp": times.now
        }
    ]
};
const postIncidentResponse = {
    success: true,
    message: null,
};

// SET /events/alerts
const setIncidentRequest = {
    "resolved": true,
    "resolution_message": "Cell voltage has returned to normal levels!!!"
};
const setIncidentResponse = {
    success: true,
    message: null,
};

// GET /events/alerts
const getIncidentsRequest = {
    limit: 10,
    startTime: "2023-03-13T17:47:33.820Z",
    endTime: times.now,

    // filters
    severityFilter: 0,
    orgFilter: "VFakeCo",
    resolvedFilter: false,
    sitesFilter: [ "Willow Hollow", "Juniper City" ],
    statusFilter: "Active",
};
const getIncidentsResponseUnresolved = {
    "count": 1,
    "rows": [
        {
            "organization": "VFakeCo",
            "severity": 0,
            "deadline": 30,
            "title": "VFakeCo - WH & JC Cell Voltage",
            "id": "dummy_incident_id3",
            "site": "Willow Hollow",
            "status": "Active",
            "resolved": false,
            "trigger_time": "2020-07-07T12:00:00.000Z",
            "details": [
                {
                    "message": "Minimum Cell Voltage: > 3.22V",
                    "timestamp": "2020-07-07T12:00:00.000Z"
                },
                {
                    "message": "Cell Voltage Delta: >.25V",
                    "timestamp": "2020-07-07T12:00:00.000Z"
                }
            ],
            "resolution_time": null,
            "resolution_message": null
        }
    ]
}

const getIncidentsResponseResolved = {
    "count": 1,
    "rows": [
        {
            "organization": "VFakeCo",
            "severity": 0,
            "deadline": 20,
            "title": "VFakeCo - WH & JC Cell Voltage",
            "id": "dummy_incident_id",
            "site": "Juniper City",
            "resolved": true,
            "trigger_time": "2020-07-07T12:00:00.000Z",
            "details": [
                {
                    "message": "Minimum Cell Voltage: > 3.22V",
                    "timestamp": "2020-07-07T12:00:00.000Z"
                },
                {
                    "message": "Cell Voltage Delta: >.25V",
                    "timestamp": "2020-07-07T12:00:00.000Z"
                }
            ],
            "resolution_time": "2020-07-07T12:00:00.000Z",
            "resolution_message": "Cell voltage has returned to normal levels - noted and resolved"
        }
    ]
}

// -> PUB /events/alerts (message varies)

// -> GET /go_metrics/events/alerts
const outboundGetAlerts = {
    // TODO
}

// -> SET /go_metrics/alerts/configuration
const outboundSetManagement = {
    "inputs": {
        "ess_2_soc": {
            "uri": "/assets/ess/ess_2/soc",
            "type": "float"
        }
    },
    "templates": [
        {
            "type": "sequential",
            "from": 1,
            "to": 16,
            "step": 3,
            "token": "##"
        },
        {
            "type": "list",
            "list": [
                "bobcat",
                "cheetah",
                "lion"
            ],
            "token": "qq"
        }
    ],
    "outputs": {
        "28beecbc-232f-431b-ac7d-8d29350e9000/Willow_Hollow": {
            "uri": "/events/alerts",
            "flags": [
                "clothed",
                "sparse"
            ],
            "attributes": {
                "id": "28beecbc-232f-431b-ac7d-8d29350e9000",
                "organization": "VFakeCo",
                "site": "Willow Hollow",
            }
        },
        "28beecbc-232f-431b-ac7d-8d29350e9000/Juniper_City": {
            "uri": "/events/alerts",
            "flags": [
                "clothed",
                "sparse"
            ],
            "attributes": {
                "id": "28beecbc-232f-431b-ac7d-8d29350e9000",
                "organization": "VFakeCo",
                "site": "Juniper City",
            }
        }
    },
    "metrics": [
        {
            "id": "28beecbc-232f-431b-ac7d-8d29350e9000/Willow_Hollow",
            "outputs": "28beecbc-232f-431b-ac7d-8d29350e9000/Willow_Hollow",
            "type": "bool",
            "alert": true,
            "expression": "Minimum Cell Voltage: > 3.22V OR Cell Voltage Delta: >.25V",
            "enabled": true
        },
        {
            "id": "28beecbc-232f-431b-ac7d-8d29350e9000/Juniper_City",
            "outputs": "28beecbc-232f-431b-ac7d-8d29350e9000/Juniper_City",
            "type": "bool",
            "alert": true,
            "expression": "Minimum Cell Voltage: > 3.22V OR Cell Voltage Delta: >.25V",
            "enabled": true
        }
    ]
};

const outboundSetManagementNew = {
    "inputs": {
        "ess_2_soc": {
            "uri": "/assets/ess/ess_2/soc",
            "type": "float"
        }
    },
    "templates": [
        {
            "type": "sequential",
            "from": 1,
            "to": 16,
            "step": 3,
            "token": "##"
        },
        {
            "type": "list",
            "list": [
                "bobcat",
                "cheetah",
                "lion"
            ],
            "token": "qq"
        }
    ],
    "outputs": {
        "mocked-new-alert-uuid/Willow_Hollow": {
            "uri": "/events/alerts",
            "flags": [
                "clothed",
                "sparse"
            ],
            "attributes": {
                "id": "mocked-new-alert-uuid",
                "organization": "VFakeCo",
                "site": "Willow Hollow"
            }
        },
        "mocked-new-alert-uuid/Juniper_City": {
            "uri": "/events/alerts",
            "flags": [
                "clothed",
                "sparse"
            ],
            "attributes": {
                "id": "mocked-new-alert-uuid",
                "organization": "VFakeCo",
                "site": "Juniper City"
            }
        }
    },
    "metrics": [
        {
            "id": "mocked-new-alert-uuid/Willow_Hollow",
            "outputs": "mocked-new-alert-uuid/Willow_Hollow",
            "type": "bool",
            "alert": true,
            "expression": "Minimum Cell Voltage: > 3.22V OR Cell Voltage Delta: >.25V",
            "enabled": true
        },
        {
            "id": "mocked-new-alert-uuid/Juniper_City",
            "outputs": "mocked-new-alert-uuid/Juniper_City",
            "type": "bool",
            "alert": true,
            "expression": "Minimum Cell Voltage: > 3.22V OR Cell Voltage Delta: >.25V",
            "enabled": true
        }
    ]
};

module.exports = {
    // mongo entries
    databaseEntry,
    databaseEntries,

    // management
    postManagementRequest,  // from the UI
    postManagementResponse, // to the UI
    getManagementRequest,   // from the UI
    getManagementResponse,  // to the UI

    // incidents
    postIncidentRequest,    // from go_metrics
    postIncidentResponse,   // to go_metrics
    setIncidentRequest,     // from the UI
    setIncidentResponse,    // to the UI
    getIncidentsRequest,    // from the UI
    getIncidentsResponseUnresolved,   // to the UI
    getIncidentsResponseResolved,   // to the UI

    // outbound events requests
    // outboundPubAlert,       // to the UI
    outboundGetAlerts,      // to go_metrics
    outboundSetManagement,   // to go_metrics
    outboundSetManagementNew
}
