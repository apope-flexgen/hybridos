/* eslint-disable no-undef */
const {
    initializeAggregatedEndpoints,
    getAggregatedEndpoints,
    mergeEndpoints,
    filterAndReduceRequestedObjects,
} = require('../src/workers/aggregatedEndpoints');

const mockInitialAggregatedEndpoints = {
    '/sites': {
        alvin: {},
        brazoria: {},
        magnolia: {},
    },
    '/components': {
        alvin: {},
        brazoria: {},
    },
    '/assets': {
        bms: {},
        pcs: {},
    },
};

const mockAggregatedEndpoints = {
    '/sites': {
        alvin: {
            active_power: {
                value: 12,
                otherStuff: true,
                someString: 'string',
            },
            reactive_power: {
                object_field: {
                    field_1: true,
                    field_2: 99,
                    field_3: 'default value',
                },
            },
        },
        brazoria: {
            active_power: {
                value: 27,
                otherStuff: false,
                someString: 'different string',
            },
            reactive_power: {
                'object field': {
                    field_1: true,
                    field_2: 67,
                    field_3: 'reset value',
                },
            },
        },
        magnolia: {
            active_power: {
                value: 2,
                otherStuff: true,
                someString: 'another string',
            },
            reactive_power: {
                'object field': {
                    field_1: false,
                    field_2: 82,
                    field_3: 'value',
                },
            },
        },
    },
    '/components': {
        alvin: {},
        brazoria: {},
    },
    '/assets': {
        bms: {},
        pcs: {},
    },
};

const mockMessages = [
    {
        method: 'get',
        uri: '/sites/alvin',
        replyto: '/aggregate/sites/alvin',
        body: null,
    },
    {
        method: 'get',
        uri: '/sites/brazoria',
        replyto: '/aggregate/sites/brazoria',
        body: null,
    },
    {
        method: 'get',
        uri: '/sites/magnolia',
        replyto: '/aggregate/sites/magnolia',
        body: null,
    },
    {
        method: 'get',
        uri: '/components/alvin',
        replyto: '/aggregate/components/alvin',
        body: null,
    },
    {
        method: 'get',
        uri: '/components/brazoria',
        replyto: '/aggregate/components/brazoria',
        body: null,
    },
    {
        method: 'get',
        uri: '/assets/bms',
        replyto: '/aggregate/assets/bms',
        body: null,
    },
    {
        method: 'get',
        uri: '/assets/pcs',
        replyto: '/aggregate/assets/pcs',
        body: null,
    },
];

describe('initialize endpoints', () => {
    let mockConfig;
    test('with empty config', () => {
        mockConfig = {};
        expect(initializeAggregatedEndpoints(mockConfig)).toEqual({});
    });

    test('with empty aggregatedEndpoints field', () => {
        mockConfig = { aggregatedEndpoints: {} };
        expect(initializeAggregatedEndpoints(mockConfig)).toEqual({});
    });

    test('with mock config', () => {
        mockConfig = {
            aggregatedEndpoints: {
                '/sites': ['alvin', 'brazoria', 'magnolia'],
                '/components': ['alvin', 'brazoria'],
                '/assets': ['bms', 'pcs'],
            },
        };
        expect(initializeAggregatedEndpoints(mockConfig)).toEqual(mockInitialAggregatedEndpoints);
    });
});

describe('generate fims get messages', () => {
    const messages = [];

    function fimsSend(msg) {
        messages.push(msg);
    }

    test('with empty aggregatedEndpoints', () => {
        getAggregatedEndpoints({}, fimsSend);
        expect(messages).toEqual([]);
    });

    test('with mock aggregatedEndpoints', () => {
        getAggregatedEndpoints(mockInitialAggregatedEndpoints, fimsSend);
        expect(messages).toEqual(mockMessages);
    });
});

describe('merge endpoints', () => {
    let originalAggregatedEndpoints;
    const uri = '/sites/alvin';
    test('with new data being empty', () => {
        originalAggregatedEndpoints = JSON.parse(JSON.stringify(mockAggregatedEndpoints));
        mergeEndpoints(originalAggregatedEndpoints, uri, {});
        expect(originalAggregatedEndpoints).toEqual(mockAggregatedEndpoints);
    });

    test('with new data having less fields', () => {
        originalAggregatedEndpoints = JSON.parse(JSON.stringify(mockAggregatedEndpoints));
        const lessDataEndpoints = {
            active_power: {
                value: 9,
                otherStuff: false,
                someString: 'string stuff',
            },
        };
        mergeEndpoints(originalAggregatedEndpoints, uri, lessDataEndpoints);
        expect(originalAggregatedEndpoints).toEqual({
            '/sites': {
                alvin: {
                    active_power: {
                        value: 9,
                        otherStuff: false,
                        someString: 'string stuff',
                    },
                    reactive_power: {
                        object_field: {
                            field_1: true,
                            field_2: 99,
                            field_3: 'default value',
                        },
                    },
                },
                brazoria: {
                    active_power: {
                        value: 27,
                        otherStuff: false,
                        someString: 'different string',
                    },
                    reactive_power: {
                        'object field': {
                            field_1: true,
                            field_2: 67,
                            field_3: 'reset value',
                        },
                    },
                },
                magnolia: {
                    active_power: {
                        value: 2,
                        otherStuff: true,
                        someString: 'another string',
                    },
                    reactive_power: {
                        'object field': {
                            field_1: false,
                            field_2: 82,
                            field_3: 'value',
                        },
                    },
                },
            },
            '/components': {
                alvin: {},
                brazoria: {},
            },
            '/assets': {
                bms: {},
                pcs: {},
            },
        });
    });

    test('with new data having equal fields', () => {
        originalAggregatedEndpoints = JSON.parse(JSON.stringify(mockAggregatedEndpoints));
        const equalDataEndpoints = {
            active_power: {
                value: 9,
                otherStuff: false,
                someString: 'string stuff',
            },
            reactive_power: {
                object_field: {
                    field_1: false,
                    field_2: 204,
                    field_3: 'new default value',
                },
            },
        };
        mergeEndpoints(originalAggregatedEndpoints, uri, equalDataEndpoints);
        expect(originalAggregatedEndpoints).toEqual({
            '/sites': {
                alvin: {
                    active_power: {
                        value: 9,
                        otherStuff: false,
                        someString: 'string stuff',
                    },
                    reactive_power: {
                        object_field: {
                            field_1: false,
                            field_2: 204,
                            field_3: 'new default value',
                        },
                    },
                },
                brazoria: {
                    active_power: {
                        value: 27,
                        otherStuff: false,
                        someString: 'different string',
                    },
                    reactive_power: {
                        'object field': {
                            field_1: true,
                            field_2: 67,
                            field_3: 'reset value',
                        },
                    },
                },
                magnolia: {
                    active_power: {
                        value: 2,
                        otherStuff: true,
                        someString: 'another string',
                    },
                    reactive_power: {
                        'object field': {
                            field_1: false,
                            field_2: 82,
                            field_3: 'value',
                        },
                    },
                },
            },
            '/components': {
                alvin: {},
                brazoria: {},
            },
            '/assets': {
                bms: {},
                pcs: {},
            },
        });
    });

    test('with new data having more fields', () => {
        originalAggregatedEndpoints = JSON.parse(JSON.stringify(mockAggregatedEndpoints));
        const equalDataEndpoints = {
            active_power: {
                value: 9,
                otherStuff: false,
                someString: 'string stuff',
            },
            reactive_power: {
                object_field: {
                    field_1: false,
                    field_2: 204,
                    field_3: 'new default value',
                },
            },
            maintenance_info: {
                value: 'alarms are off',
                levels: 'high',
                maintenance_active: true,
            },
        };
        mergeEndpoints(originalAggregatedEndpoints, uri, equalDataEndpoints);
        expect(originalAggregatedEndpoints).toEqual({
            '/sites': {
                alvin: {
                    active_power: {
                        value: 9,
                        otherStuff: false,
                        someString: 'string stuff',
                    },
                    reactive_power: {
                        object_field: {
                            field_1: false,
                            field_2: 204,
                            field_3: 'new default value',
                        },
                    },
                    maintenance_info: {
                        value: 'alarms are off',
                        levels: 'high',
                        maintenance_active: true,
                    },
                },
                brazoria: {
                    active_power: {
                        value: 27,
                        otherStuff: false,
                        someString: 'different string',
                    },
                    reactive_power: {
                        'object field': {
                            field_1: true,
                            field_2: 67,
                            field_3: 'reset value',
                        },
                    },
                },
                magnolia: {
                    active_power: {
                        value: 2,
                        otherStuff: true,
                        someString: 'another string',
                    },
                    reactive_power: {
                        'object field': {
                            field_1: false,
                            field_2: 82,
                            field_3: 'value',
                        },
                    },
                },
            },
            '/components': {
                alvin: {},
                brazoria: {},
            },
            '/assets': {
                bms: {},
                pcs: {},
            },
        });
    });
});

describe('filter endpoints', () => {
    test('without permissions', () => {
        const permissions = [];
        function hasPermissions(userName, role, path) {
            // mocking permissions. Essentially the same process
            return permissions.includes(path);
        }

        const copyOfMockAggregatedEndpoints = JSON.parse(JSON.stringify(mockAggregatedEndpoints));
        const authorizedTopLevelEndpoint = filterAndReduceRequestedObjects(copyOfMockAggregatedEndpoints, '/sites', null, null, hasPermissions);
        expect(copyOfMockAggregatedEndpoints).toEqual(mockAggregatedEndpoints);
        expect(authorizedTopLevelEndpoint).toEqual({});
    });

    test('with some permissions', () => {
        const permissions = ['/sites/alvin', '/sites/magnolia'];
        function hasPermissions(userName, role, path) {
            // mocking permissions. Essentially the same process
            return permissions.includes(path);
        }

        const copyOfMockAggregatedEndpoints = JSON.parse(JSON.stringify(mockAggregatedEndpoints));
        const authorizedTopLevelEndpoint = filterAndReduceRequestedObjects(copyOfMockAggregatedEndpoints, '/sites', null, null, hasPermissions);
        // expect original object to stay the same
        expect(copyOfMockAggregatedEndpoints).toEqual(mockAggregatedEndpoints);
        expect(authorizedTopLevelEndpoint).toEqual({
            alvin: {
                active_power: {
                    value: 12,
                    otherStuff: true,
                    someString: 'string',
                },
                reactive_power: {
                    object_field: {
                        field_1: true,
                        field_2: 99,
                        field_3: 'default value',
                    },
                },
            },
            magnolia: {
                active_power: {
                    value: 2,
                    otherStuff: true,
                    someString: 'another string',
                },
                reactive_power: {
                    'object field': {
                        field_1: false,
                        field_2: 82,
                        field_3: 'value',
                    },
                },
            },
        });
    });

    test('with all permissions', () => {
        const permissions = ['/sites/alvin', '/sites/brazoria', '/sites/magnolia'];
        function hasPermissions(userName, role, path) {
            // mocking permissions. Essentially the same process
            return permissions.includes(path);
        }

        const copyOfMockAggregatedEndpoints = JSON.parse(JSON.stringify(mockAggregatedEndpoints));
        const authorizedTopLevelEndpoint = filterAndReduceRequestedObjects(copyOfMockAggregatedEndpoints, '/sites', null, null, hasPermissions);
        expect(copyOfMockAggregatedEndpoints).toEqual(mockAggregatedEndpoints);
        expect(authorizedTopLevelEndpoint).toEqual(mockAggregatedEndpoints['/sites']);
    });
});

describe('complete process', () => {
    const config = {
        aggregatedEndpoints: {
            '/sites': ['alvin', 'brazoria', 'magnolia'],
            '/components': ['alvin', 'brazoria'],
            '/assets': ['bms', 'pcs'],
        },
    };

    const aggregatedEndpoints = initializeAggregatedEndpoints(config);
    test('initialize aggregatedEndpoints', () => {
        expect(aggregatedEndpoints).toEqual(mockInitialAggregatedEndpoints);
    });

    const messages = [];

    function fimsSend(msg) {
        messages.push(msg);
    }
    test('generate FIMS messages', () => {
        getAggregatedEndpoints(mockInitialAggregatedEndpoints, fimsSend);
        expect(messages).toEqual(mockMessages);
    });

    test('merge data to alvin', () => {
        const equalDataEndpoints = {
            active_power: {
                value: 9,
                otherStuff: false,
                someString: 'string stuff',
            },
            reactive_power: {
                object_field: {
                    field_1: false,
                    field_2: 204,
                    field_3: 'new default value',
                },
            },
            maintenance_info: {
                value: 'alarms are off',
                levels: 'high',
                maintenance_active: true,
            },
        };
        mergeEndpoints(aggregatedEndpoints, '/sites/alvin', equalDataEndpoints);
        expect(aggregatedEndpoints).toEqual({
            '/sites': {
                alvin: {
                    active_power: {
                        value: 9,
                        otherStuff: false,
                        someString: 'string stuff',
                    },
                    reactive_power: {
                        object_field: {
                            field_1: false,
                            field_2: 204,
                            field_3: 'new default value',
                        },
                    },
                    maintenance_info: {
                        value: 'alarms are off',
                        levels: 'high',
                        maintenance_active: true,
                    },
                },
                brazoria: {},
                magnolia: {},
            },
            '/components': {
                alvin: {},
                brazoria: {},
            },
            '/assets': {
                bms: {},
                pcs: {},
            },
        });
    });

    test('merge data to brazoria', () => {
        const equalDataEndpoints = {
            active_power: {
                value: 9,
                otherStuff: false,
                someString: 'string stuff',
            },
            reactive_power: {
                object_field: {
                    field_1: false,
                    field_2: 204,
                    field_3: 'new default value',
                },
            },
            maintenance_info: {
                value: 'alarms are off',
                levels: 'high',
                maintenance_active: true,
            },
        };
        mergeEndpoints(aggregatedEndpoints, '/sites/brazoria', equalDataEndpoints);
        expect(aggregatedEndpoints).toEqual({
            '/sites': {
                alvin: {
                    active_power: {
                        value: 9,
                        otherStuff: false,
                        someString: 'string stuff',
                    },
                    reactive_power: {
                        object_field: {
                            field_1: false,
                            field_2: 204,
                            field_3: 'new default value',
                        },
                    },
                    maintenance_info: {
                        value: 'alarms are off',
                        levels: 'high',
                        maintenance_active: true,
                    },
                },
                brazoria: {
                    active_power: {
                        value: 9,
                        otherStuff: false,
                        someString: 'string stuff',
                    },
                    reactive_power: {
                        object_field: {
                            field_1: false,
                            field_2: 204,
                            field_3: 'new default value',
                        },
                    },
                    maintenance_info: {
                        value: 'alarms are off',
                        levels: 'high',
                        maintenance_active: true,
                    },
                },
                magnolia: {},
            },
            '/components': {
                alvin: {},
                brazoria: {},
            },
            '/assets': {
                bms: {},
                pcs: {},
            },
        });
    });

    test('merge data to magnolia', () => {
        const equalDataEndpoints = {
            active_power: {
                value: 9,
                otherStuff: false,
                someString: 'string stuff',
            },
            reactive_power: {
                object_field: {
                    field_1: false,
                    field_2: 204,
                    field_3: 'new default value',
                },
            },
            maintenance_info: {
                value: 'alarms are off',
                levels: 'high',
                maintenance_active: true,
            },
        };
        mergeEndpoints(aggregatedEndpoints, '/sites/magnolia', equalDataEndpoints);
        expect(aggregatedEndpoints).toEqual({
            '/sites': {
                alvin: {
                    active_power: {
                        value: 9,
                        otherStuff: false,
                        someString: 'string stuff',
                    },
                    reactive_power: {
                        object_field: {
                            field_1: false,
                            field_2: 204,
                            field_3: 'new default value',
                        },
                    },
                    maintenance_info: {
                        value: 'alarms are off',
                        levels: 'high',
                        maintenance_active: true,
                    },
                },
                brazoria: {
                    active_power: {
                        value: 9,
                        otherStuff: false,
                        someString: 'string stuff',
                    },
                    reactive_power: {
                        object_field: {
                            field_1: false,
                            field_2: 204,
                            field_3: 'new default value',
                        },
                    },
                    maintenance_info: {
                        value: 'alarms are off',
                        levels: 'high',
                        maintenance_active: true,
                    },
                },
                magnolia: {
                    active_power: {
                        value: 9,
                        otherStuff: false,
                        someString: 'string stuff',
                    },
                    reactive_power: {
                        object_field: {
                            field_1: false,
                            field_2: 204,
                            field_3: 'new default value',
                        },
                    },
                    maintenance_info: {
                        value: 'alarms are off',
                        levels: 'high',
                        maintenance_active: true,
                    },
                },
            },
            '/components': {
                alvin: {},
                brazoria: {},
            },
            '/assets': {
                bms: {},
                pcs: {},
            },
        });
    });

    test('filter and send aggregatedEndpoints', () => {
        const permissions = ['/sites/alvin', '/sites/magnolia'];
        function hasPermissions(userName, role, path) {
            // mocking permissions. Essentially the same process
            return permissions.includes(path);
        }

        expect(filterAndReduceRequestedObjects(aggregatedEndpoints, '/sites', null, null, hasPermissions)).toEqual({
            alvin: {
                active_power: {
                    value: 9,
                    otherStuff: false,
                    someString: 'string stuff',
                },
                reactive_power: {
                    object_field: {
                        field_1: false,
                        field_2: 204,
                        field_3: 'new default value',
                    },
                },
                maintenance_info: {
                    value: 'alarms are off',
                    levels: 'high',
                    maintenance_active: true,
                },
            },
            magnolia: {
                active_power: {
                    value: 9,
                    otherStuff: false,
                    someString: 'string stuff',
                },
                reactive_power: {
                    object_field: {
                        field_1: false,
                        field_2: 204,
                        field_3: 'new default value',
                    },
                },
                maintenance_info: {
                    value: 'alarms are off',
                    levels: 'high',
                    maintenance_active: true,
                },
            },
        });
    });

    test('check if original object is unchanged', () => {
        expect(aggregatedEndpoints).toEqual({
            '/sites': {
                alvin: {
                    active_power: {
                        value: 9,
                        otherStuff: false,
                        someString: 'string stuff',
                    },
                    reactive_power: {
                        object_field: {
                            field_1: false,
                            field_2: 204,
                            field_3: 'new default value',
                        },
                    },
                    maintenance_info: {
                        value: 'alarms are off',
                        levels: 'high',
                        maintenance_active: true,
                    },
                },
                brazoria: {
                    active_power: {
                        value: 9,
                        otherStuff: false,
                        someString: 'string stuff',
                    },
                    reactive_power: {
                        object_field: {
                            field_1: false,
                            field_2: 204,
                            field_3: 'new default value',
                        },
                    },
                    maintenance_info: {
                        value: 'alarms are off',
                        levels: 'high',
                        maintenance_active: true,
                    },
                },
                magnolia: {
                    active_power: {
                        value: 9,
                        otherStuff: false,
                        someString: 'string stuff',
                    },
                    reactive_power: {
                        object_field: {
                            field_1: false,
                            field_2: 204,
                            field_3: 'new default value',
                        },
                    },
                    maintenance_info: {
                        value: 'alarms are off',
                        levels: 'high',
                        maintenance_active: true,
                    },
                },
            },
            '/components': {
                alvin: {},
                brazoria: {},
            },
            '/assets': {
                bms: {},
                pcs: {},
            },
        });
    });
});
