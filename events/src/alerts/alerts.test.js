/* eslint-disable */
// ideally some commands which compare responses and leverage alertExamples ¯\_(ツ)_/¯
const mongoose = require('mongoose');
const fims = require('@flexgen/fims');
const { v4: uuidv4 } = require('uuid');
const { clearDatabase } = require('../clearDatabase');
const { processEvent } = require('../events');
const { loadAlerts } = require('../loadDatabase');

const {
    genericSuccessResponse,
    getManagementRequest, getManagementResponses,
    postManagementRequest, outboundSetManagement,
    setIncidentRequest, setIncidentResponse,
    postIncidentRequest,
    getIncidentsRequest, getIncidentsResponseUnresolved, getIncidentsResponseResolved, getIncidentsResponseAfterResolution,
    outboundSetManagementNew,
    organizationEntries,
    postManagementRequestDeletion,
} = require('./alertExamples');
const { handleInitAlerts } = require('./handlers/alertIncidents');

jest.mock('@flexgen/fims');
jest.mock('uuid');
const mockedUuidv4 = jest.fn();
uuidv4.mockImplementation(mockedUuidv4);

// Generic amount of time to wait for processing
const sleepMs = 200;
function sleep(ms) { return new Promise((resolve) => setTimeout(resolve, ms)); }

/**
 * Extension of expect to make viewing JSON results of backchecking fims.send much, much easier.
 * 
 * Expected format on failure:
 *  ● Alerts tests › UI | user creates a new organization
 *  Json mismatch. Expected but not received:
 *  ["body.3.id: mocked-new-org-uuid-extrachars"]
 *  Received but not expected:
 *  ["body.3.id: mocked-new-org-uuid"]
 * 
 */
expect.extend({
  toMatchObjectByKey(received, expected) {
    if (Object.keys(received).includes("body") && typeof received.body === 'string') {
        received.body = JSON.parse(received.body);
    }
    if (Object.keys(expected).includes("body") && typeof expected.body === 'string') {
        expected.body = JSON.parse(expected.body);
    }
    // Custom comparison between two object to make printing out for tests MUCH better
    function collectCompares(obj1, obj2, keys) {
        if (!obj1) {
            return [];
        }
        if (typeof obj1 === 'string') {
            if (obj1 !== obj2) {
                return [`${keys.join(".")}: ${obj1}`];
            }
            return [];
        }
        if (Array.isArray(obj1)) {
            if (!Array.isArray(obj2)) {
                return obj1.map((x, index) => {
                    [`${[...keys, index].join(".")}: ${x}`]
                })
            }
            return obj1.reduce((acc, curr, index) => [
                ...acc,
                ...collectCompares(curr, obj2[index], [...keys, index]),
            ], []);
        }
        const obj1Keys = Object.keys(obj1);
        const results = obj1Keys.map((key) => {
            if (!obj1 || !Object.keys(obj1)) {
                return [];
            }
            if (!obj2 || !Object.keys(obj2).includes(key)) {
                return [`${[...keys, key].join(".")}: ${obj1[key]}`]
            }
            const msgs = collectCompares(obj1[key], obj2[key], [...keys, key]);
            return msgs;
        });
        return results.reduce((prev, curr) => ([...prev, ...curr]), []);
    }

    const expectedNotReceived = collectCompares(expected, received, []);
    const receivedNotExpected = collectCompares(received, expected, []);

    return {
        message: () =>
            `Json mismatch. Expected but not received:\n${this.utils.printExpected(expectedNotReceived)}\nReceived but not expected:\n${this.utils.printReceived(receivedNotExpected)}`,
        pass: expectedNotReceived.length + receivedNotExpected.length === 0,
    };
  },
});

describe('Alerts tests', () => {
    let realDate;
    let fimsSendSpy;

    beforeAll(async () => {
        realDate = Date;
        global.Date = jest.fn(() => new realDate('2020-07-07T12:00:00.000Z'));
        global.Date.now = realDate.now;
    });

    beforeEach(async () => {
        jest.clearAllMocks();
        await clearDatabase();
        await loadAlerts();
        await sleep(200);
        await mongoose.disconnect();
        await mongoose.connect('mongodb://localhost:27017/standalone_storage', { useNewUrlParser: true, useUnifiedTopology: true });
        fimsSendSpy = jest.spyOn(fims, "send");
    });

    function expectFimsSendHelper(calls) {
        expect(fims.send).toHaveBeenCalledTimes(calls.length);
        calls.forEach((expectedCall, index) => {
            expect(fimsSendSpy.mock.calls[index][0]).toMatchObjectByKey(expectedCall)
        });
    }

    /**
     * Alert management
     */
    test('UI | Creates a new alerting rule configuration', async () => {
        mockedUuidv4.mockReturnValueOnce('mocked-new-uuid');
        mockedUuidv4.mockReturnValueOnce('mocked-new-template-uuid-0');
        mockedUuidv4.mockReturnValueOnce('mocked-new-template-uuid-1');
        mockedUuidv4.mockReturnValueOnce('mocked-reply-uuid');
        const {id, ...body} = postManagementRequest;
        processEvent({
            uri: '/events/alerts/management',
            method: 'post',
            body,
            replyto: 'dummy_replyto',
        });
        await sleep(sleepMs);
        processEvent({
            uri: '/events/alerts/reply/mocked-reply-uuid',
            method: 'set',
            body: genericSuccessResponse,
            replyto: null,
        });
        await sleep(sleepMs);
        processEvent({
            uri: '/events/alerts/management',
            method: 'get',
            body: getManagementRequest,
            replyto: 'dummy_replyto',
        });
        await sleep(sleepMs);
        expectFimsSendHelper([
            {
                body: JSON.stringify(outboundSetManagementNew),
                method: 'set',
                replyto: '/events/alerts/reply/mocked-reply-uuid',
                uri: '/go_metrics_alerting/configuration/mocked-new-uuid',
                username: null,
            },
            {
                body: JSON.stringify({ message: 'An alert configuration has been changed' }),
                method: 'pub',
                replyto: null,
                uri: '/events/alerts',
                username: null,
            },
            {
                body: JSON.stringify(genericSuccessResponse),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null,
            },
            {
                body: JSON.stringify(getManagementResponses.afterInsert),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null
            }
        ]);
    });

    test('UI | Updates an existing alerting rule configuration', async () => {
        mockedUuidv4.mockReturnValueOnce('mocked-new-template-uuid-0');
        mockedUuidv4.mockReturnValueOnce('mocked-new-template-uuid-1');
        mockedUuidv4.mockReturnValueOnce('mocked-reply-uuid');
        processEvent({
            uri: '/events/alerts/management',
            method: 'post',
            body: postManagementRequest,
            replyto: 'dummy_replyto',
        });
        await sleep(sleepMs);
        processEvent({
            uri: '/events/alerts/reply/mocked-reply-uuid',
            method: 'set',
            body: genericSuccessResponse,
            replyto: null,
        });
        await sleep(sleepMs);
        processEvent({
            uri: '/events/alerts/management',
            method: 'get',
            body: getManagementRequest,
            replyto: 'dummy_replyto',
        });
        await sleep(sleepMs);
        expectFimsSendHelper([
            {
                body: JSON.stringify(outboundSetManagement),
                method: 'set',
                replyto: '/events/alerts/reply/mocked-reply-uuid',
                uri: '/go_metrics_alerting/configuration/28beecbc-232f-431b-ac7d-8d29350e9000',
                username: null,
            },
            {
                body: JSON.stringify({ message: 'An alert configuration has been changed' }),
                method: 'pub',
                replyto: null,
                uri: '/events/alerts',
                username: null,
            },
            {
                body: JSON.stringify(genericSuccessResponse),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null,
            },
            {
                body: JSON.stringify(getManagementResponses.afterUpdate),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null
            }
        ]);
    });

    test('UI | Update rejection for an existing alerting rule configuration', async () => {
        mockedUuidv4.mockReturnValueOnce('mocked-new-template-uuid-0');
        mockedUuidv4.mockReturnValueOnce('mocked-new-template-uuid-1');
        mockedUuidv4.mockReturnValueOnce('mocked-reply-uuid');
        processEvent({
            uri: '/events/alerts/management',
            method: 'post',
            body: postManagementRequest,
            replyto: 'dummy_replyto',
        });
        await sleep(sleepMs);
        processEvent({
            uri: '/events/alerts/reply/mocked-reply-uuid',
            method: 'set',
            body: {success: false, message: "No bueno"},
            replyto: null,
        });
        await sleep(sleepMs);
        processEvent({
            uri: '/events/alerts/management',
            method: 'get',
            body: getManagementRequest,
            replyto: 'dummy_replyto',
        });
        await sleep(sleepMs);
        expectFimsSendHelper([
            {
                body: JSON.stringify(outboundSetManagement),
                method: 'set',
                replyto: '/events/alerts/reply/mocked-reply-uuid',
                uri: '/go_metrics_alerting/configuration/28beecbc-232f-431b-ac7d-8d29350e9000',
                username: null,
            },
            {
                body: JSON.stringify({success: false, message: `Invalid config: No bueno`}),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null,
            },
            {
                body: JSON.stringify(getManagementResponses.base),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null
            }
        ]);
    });

    test('UI | Deletes an existing alerting rule configuration', async () => {
        mockedUuidv4.mockReturnValue('mocked-new-template-uuid');
        mockedUuidv4.mockReturnValueOnce('mocked-reply-uuid');
        processEvent({
            uri: '/events/alerts/management',
            method: 'post',
            body: postManagementRequestDeletion,
            replyto: 'dummy_replyto',
        });
        await sleep(sleepMs);
        processEvent({
            uri: '/events/alerts/reply/mocked-reply-uuid',
            method: 'set',
            body: genericSuccessResponse,
            replyto: null,
        });
        await sleep(sleepMs);
        processEvent({
            uri: '/events/alerts/management',
            method: 'get',
            body: getManagementRequest,
            replyto: 'dummy_replyto',
        });
        await sleep(sleepMs);
        const expectedGetResponse = JSON.parse(JSON.stringify(getManagementResponses.base));
        expectedGetResponse.rows[1].deleted = true;
        expectedGetResponse.templates = [];
        expectFimsSendHelper([
            {
                body: null,
                method: 'del',
                replyto: '/events/alerts/reply/mocked-reply-uuid',
                uri: '/go_metrics_alerting/configuration/28beecbc-232f-431b-ac7d-8d29350e9000',
                username: null,
            },
            {
                body: JSON.stringify({ message: 'An alert configuration has been changed' }),
                method: 'pub',
                replyto: null,
                uri: '/events/alerts',
                username: null,
            },
            {
                body: JSON.stringify(genericSuccessResponse),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null,
            },
            {
                body: JSON.stringify(expectedGetResponse),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null
            }
        ]);
    });

    test('UI | navigates to alert management page', async () => {
        processEvent({
            uri: '/events/alerts/management',
            method: 'get',
            body: getManagementRequest,
            replyto: 'dummy_replyto',
        });
        await sleep(sleepMs);
        expectFimsSendHelper([
            {
                body: JSON.stringify(getManagementResponses.base),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null
            }
        ]);
    });

    /**
     * Alert incidents
     */
    test('go_metrics | Alerting incident is triggered', async () => {
        processEvent({
            uri: '/events/alerts',
            method: 'post',
            body: postIncidentRequest,
            replyto: 'dummy_replyto',
        });
        await sleep(sleepMs);
        processEvent({
            uri: '/events/alerts',
            method: 'get',
            body: {
                ...getIncidentsRequest,
                resolvedFilter: false,
            },
            replyto: 'dummy_replyto',
        });
        await sleep(sleepMs);

        // deep clone
        const incidentWithNewDetails = JSON.parse(JSON.stringify(getIncidentsResponseUnresolved))
        
        incidentWithNewDetails.rows[0].details.push(
            {
                message: 'Site discharge 11000.00kW exceeded max POI limit 10000.00kW',
                timestamp: '2020-07-07T12:00:00.000Z',
            }
        );
        expectFimsSendHelper([
            {
                body: JSON.stringify({ message: 'A new alert incident has been triggered' }),
                method: 'pub',
                replyto: null,
                uri: '/events/alerts',
                username: null,
            },
            {
                body: JSON.stringify(genericSuccessResponse),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null,
            },
            {
                body: JSON.stringify(incidentWithNewDetails),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null,
            }
        ]);
    });

    test('go_metrics | A second alerting incident is triggered', async () => {
        processEvent({
            uri: '/events/alerts',
            method: 'post',
            body: {
                ...postIncidentRequest,
                uuid: "unseen-incident-id"
            },
            replyto: 'dummy_replyto',
        });
        await sleep(sleepMs);
        processEvent({
            uri: '/events/alerts',
            method: 'get',
            body: {
                ...getIncidentsRequest,
                resolvedFilter: false,
            },
            replyto: 'dummy_replyto',
        });
        await sleep(sleepMs);

        // deep clone
        const alertWithNewIncident = JSON.parse(JSON.stringify(getIncidentsResponseUnresolved))
        
        alertWithNewIncident.rows = [
            {
                ...alertWithNewIncident.rows[0],
                details: postIncidentRequest.details,
                id: "unseen-incident-id"
            },
            ...alertWithNewIncident.rows,
        ];
        expectFimsSendHelper([
            {
                body: JSON.stringify({ message: 'A new alert incident has been triggered' }),
                method: 'pub',
                replyto: null,
                uri: '/events/alerts',
                username: null,
            },
            {
                body: JSON.stringify(genericSuccessResponse),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null,
            },
            {
                body: JSON.stringify(alertWithNewIncident),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null,
            }
        ]);
    });

    test('go_metrics | Initialize alerts entries on startup', async () => {
        await handleInitAlerts();
        await sleep(sleepMs);
        expectFimsSendHelper([{
            body: null,
            method: 'get',
            replyto: '/events_reply/refresh_alerts',
            uri: '/go_metrics_alerting/events/alerts',
            username: null,
        }]);
    });

    test('UI | navigates to unresolved alerts page', async () => {
        processEvent({
            uri: '/events/alerts',
            method: 'get',
            body: {
                ...getIncidentsRequest,
                resolvedFilter: false,
            },
            replyto: 'dummy_replyto',
        });
        await sleep(sleepMs);
        expectFimsSendHelper([{
            body: JSON.stringify(getIncidentsResponseUnresolved),
            method: 'set',
            replyto: null,
            uri: 'dummy_replyto',
            username: null,
        }]);
    });

    test('UI | navigates to resolved alerts page', async () => {
        processEvent({
            uri: '/events/alerts',
            method: 'get',
            body: {
                ...getIncidentsRequest,
                resolvedFilter: true,
            },
            replyto: 'dummy_replyto',
        });
        await sleep(sleepMs);

        expectFimsSendHelper([{
            body: getIncidentsResponseResolved,
            method: 'set',
            replyto: null,
            uri: 'dummy_replyto',
            username: null,
        }])
    });

    test('UI | user resolves an inactive alert', async () => {
        mockedUuidv4.mockReturnValue('mocked-new-resolved-alert-uuid');
        processEvent({
            uri: '/events/alerts/28beecbc-232f-431b-ac7d-8d29350e9000.lima',
            method: 'set',
            replyto: 'dummy_replyto',
            body: setIncidentRequest,
        });
        await sleep(sleepMs);
        processEvent({
            uri: '/events/alerts',
            method: 'get',
            body: {
                ...getIncidentsRequest,
                resolvedFilter: true,
            },
            replyto: 'dummy_replyto',
        });
        await sleep(sleepMs);
        expectFimsSendHelper([
            // reevaluate signal
            {
                body: JSON.stringify({ reevaluate: true }),
                method: 'set',
                replyto: null,
                uri: '/go_metrics_alerting/events/alerts/28beecbc-232f-431b-ac7d-8d29350e9000.lima',
                username: null,
            },
            {
                body: JSON.stringify({ message: 'An alert has been resolved' }),
                method: 'pub',
                replyto: null,
                uri: '/events/alerts',
                username: null,
            },
            {
                body: JSON.stringify(setIncidentResponse),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null,
            },
            {
                body: getIncidentsResponseAfterResolution,
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null,
            }
        ]);
    });

    test('UI | user resolves an active alert', async () => {
        processEvent({
            uri: '/events/alerts/28beecbc-232f-431b-ac7d-8d29350e9000.lima',
            method: 'set',
            replyto: 'dummy_replyto',
            body: setIncidentRequest,
        });
        await sleep(sleepMs);
        expectFimsSendHelper([
            // reevaluate signal
            {
                body: JSON.stringify({ reevaluate: true }),
                method: 'set',
                replyto: null,
                uri: '/go_metrics_alerting/events/alerts/28beecbc-232f-431b-ac7d-8d29350e9000.lima',
                username: null,
            },
            {
                body: JSON.stringify({ message: 'An alert has been resolved' }),
                method: 'pub',
                replyto: null,
                uri: '/events/alerts',
                username: null,
            },
            {
                body: JSON.stringify(setIncidentResponse),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null,
            },
        ]);
    });

    test('UI | user edits an already resolved alert', async () => {
        processEvent({
            uri: '/events/alerts/28beecbc-232f-431b-ac7d-8d29350e9000.sudden_valley',
            method: 'set',
            replyto: 'dummy_replyto',
            body: setIncidentRequest,
        });
        await sleep(sleepMs);
        expectFimsSendHelper([
            {
                body: JSON.stringify({ message: 'A resolved alert has been edited' }),
                method: 'pub',
                replyto: null,
                uri: '/events/alerts',
                username: null,
            },
            {
                body: JSON.stringify(setIncidentResponse),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null,
            }
        ]);
    });

    /**
     * Alert organizations
     */
    test('UI | user navigates to organization list', async () => {
        processEvent({
            uri: '/events/alerts/organizations',
            method: 'get',
            replyto: 'dummy_replyto',
            body: null,
        });
        await sleep(sleepMs);
        expectFimsSendHelper([
            {
                body: JSON.stringify(organizationEntries),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null,
            }
        ]);
    });

    test('UI | user creates a new organization', async () => {
        mockedUuidv4.mockReturnValue('mocked-new-org-uuid');
        processEvent({
            uri: '/events/alerts/organizations',
            method: 'post',
            replyto: 'dummy_replyto',
            body: {rows: [{ "name": "Friends from HS" }]},
        });
        await sleep(sleepMs);
        processEvent({
            uri: '/events/alerts/organizations',
            method: 'get',
            replyto: 'dummy_replyto',
            body: null,
        });
        await sleep(sleepMs);
        const expectedEntries = [{name: "Friends from HS", id: "mocked-new-org-uuid"}, ...organizationEntries];
        expectFimsSendHelper([
            {
                body: JSON.stringify(genericSuccessResponse),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null,
            },
            {
                body: JSON.stringify(expectedEntries),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null,
            }
        ]);
    });

    test('UI | user edits an existing organization', async () => {
        processEvent({
            uri: '/events/alerts/organizations',
            method: 'post',
            replyto: 'dummy_replyto',
            body: {rows: [{ "name": "James running a test", "id": "org_id12351235123" }]},
        });
        await sleep(sleepMs);
        processEvent({
            uri: '/events/alerts/organizations',
            method: 'get',
            replyto: 'dummy_replyto',
            body: null,
        });
        await sleep(sleepMs);
        const expectedEntries = organizationEntries;
        expectedEntries[
            organizationEntries.findIndex((x) => x.id === 'org_id12351235123')
        ].name = "James running a test";
        expectFimsSendHelper([
            {
                body: JSON.stringify(genericSuccessResponse),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null,
            },
            {
                body: JSON.stringify(expectedEntries),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null,
            }
        ]);
    });

    /**
     * SETUP
     * 1. Send org del request
     * 2. GET org list
     * 3. GET mgmt list
     * 
     * ASSERT
     * 1. generic success response
     * 2. organization list doesn't include org_id1
     * 3. management entries with org_id1 are deleted === true
     */
    test('UI | user deletes an existing organization', async () => {
        const orgId = "org_id1";
        const orgName = organizationEntries.find((x) => (orgId === x.id)).name
        processEvent({
            uri: '/events/alerts/organizations',
            method: 'del',
            replyto: 'dummy_replyto',
            body: { "id": "org_id1" },
        });
        await sleep(sleepMs);
        processEvent({
            uri: '/events/alerts/organizations',
            method: 'get',
            replyto: 'dummy_replyto',
            body: null,
        });
        processEvent({
            uri: '/events/alerts/management',
            method: 'get',
            body: getManagementRequest,
            replyto: 'dummy_replyto',
        });
        await sleep(sleepMs);
        const expectedOrgs = organizationEntries.filter((org) => org.id !== orgId);
        const expectedConfigs = {
            templates: getManagementResponses.base.templates,
            rows: getManagementResponses.base.rows.map((x) => {
                if (x.organization === orgName) {
                    return {
                        ...x,
                        organization: "Deleted",
                        deleted: true,
                        history: x.history.map((his) => ({
                            ...his,
                            organization: "Deleted",
                        }))
                    }
                }
                return x;
            })
        }
        
        expectFimsSendHelper([
            {
                body: JSON.stringify(genericSuccessResponse),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null,
            },
            {
                body: JSON.stringify(expectedOrgs),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null,
            },
            {
                body: JSON.stringify(expectedConfigs),
                method: 'set',
                replyto: null,
                uri: 'dummy_replyto',
                username: null
            }
        ]);
    });

    afterAll(async () => {
        await clearDatabase();
        await sleep(1000);
        await mongoose.disconnect();
        global.Date = realDate;
    });
});
