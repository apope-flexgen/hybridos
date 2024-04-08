

// ideally some commands which compare responses and leverage alertExamples ¯\_(ツ)_/¯

const { clearDatabase } = require("../clearDatabase");
const { processEvent } = require("../events");
const { loadAlerts } = require("../loadDatabase");
const mongoose = require('mongoose');

const fims = require('@flexgen/fims');
const { v4: uuidv4 } = require('uuid');
const { getManagementRequest, getManagementResponse, postManagementRequest, postManagementResponse, outboundSetManagement, setIncidentRequest, setIncidentResponse, postIncidentRequest, postIncidentResponse, outboundPubAlert, getIncidentsRequest, getIncidentsResponseUnresolved, getIncidentsResponseResolved, outboundSetManagementNew } = require("./alertExamples");
const { initializeAlerts } = require("./alertsDb");

jest.mock("@flexgen/fims");
jest.mock("uuid");
const mockedUuidv4 = jest.fn();
uuidv4.mockImplementation(mockedUuidv4);

const sleepMs = 1000;
function sleep(ms) {return new Promise(resolve => setTimeout(resolve, ms));}
  
describe("Alerts tests", () => {
    let realDate;

    beforeAll(async () => {
        realDate = Date;
        global.Date = jest.fn(() => new realDate('2020-07-07T12:00:00.000Z'));
        global.Date.now = realDate.now;
    })

    beforeEach(async () => {
        jest.clearAllMocks();
        await clearDatabase();
        await loadAlerts();
        await sleep(200);
        await mongoose.disconnect();
        await mongoose.connect(`mongodb://localhost:27017/standalone_storage`, { useNewUrlParser: true, useUnifiedTopology: true });
    })

    test("UI | Creates a new  alerting rule configuration", async () => {
        mockedUuidv4.mockReturnValue('mocked-new-alert-uuid');
        let body = {...postManagementRequest};
        delete body["id"];
        processEvent({
            "uri": "/events/alerts/management",
            "method": "post",
            "body": body,
            "replyto": "dummy_replyto"
        })
        await sleep(sleepMs);
        // go_metrics call
        expect(fims.send).toHaveBeenCalledTimes(3);
        expect(fims.send).toHaveBeenCalledWith({
            "body": JSON.stringify(outboundSetManagementNew),
            "method": "set",
            "replyto": null,
            "uri": "/go_metrics/alerts/configuration",
            "username": null,
        });
        expect(fims.send).toHaveBeenCalledWith({
            "body": JSON.stringify({ "message": "An alert configuration has been changed"}),
            "method": "pub",
            "replyto": null,
            "uri": "/events/alerts",
            "username": null,
        });
        expect(fims.send).toHaveBeenCalledWith({
            "body": JSON.stringify(postManagementResponse),
            "method": "set",
            "replyto": null,
            "uri": "dummy_replyto",
            "username": null,
        });
    });

    test("UI | Updates an existing alerting rule configuration", async () => {
        processEvent({
            "uri": "/events/alerts/management",
            "method": "post",
            "body": postManagementRequest,
            "replyto": "dummy_replyto"
        })
        await sleep(sleepMs);
        // go_metrics call
        expect(fims.send).toHaveBeenCalledTimes(3);
        expect(fims.send).toHaveBeenCalledWith({
            "body": JSON.stringify(outboundSetManagement),
            "method": "set",
            "replyto": null,
            "uri": "/go_metrics/alerts/configuration",
            "username": null,
        });
        expect(fims.send).toHaveBeenCalledWith({
            "body": JSON.stringify({ "message": "An alert configuration has been changed"}),
            "method": "pub",
            "replyto": null,
            "uri": "/events/alerts",
            "username": null,
        });
        expect(fims.send).toHaveBeenCalledWith({
            "body": JSON.stringify(postManagementResponse),
            "method": "set",
            "replyto": null,
            "uri": "dummy_replyto",
            "username": null,
        });
    });

    test("UI | navigates to alert management page", async () => {
        processEvent({
            "uri": "/events/alerts/management",
            "method": "get",
            "body": getManagementRequest,
            "replyto": "dummy_replyto"
        })
        await sleep(sleepMs);
        expect(fims.send).toHaveBeenCalledTimes(1);
        expect(fims.send).toHaveBeenCalledWith({
            "body": JSON.stringify(getManagementResponse),
            "method": "set",
            "replyto": null,
            "uri": "dummy_replyto",
            "username": null,
         });
    });

    test("go_metrics | Alerting incident is triggered", async () => {
        processEvent({
            "uri": "/events/alerts",
            "method": "post",
            "body": postIncidentRequest,
            "replyto": "dummy_replyto"
        });
        await sleep(sleepMs);
        expect(fims.send).toHaveBeenCalledTimes(2);
        expect(fims.send).toHaveBeenCalledWith({
            "body": JSON.stringify({ "message": "A new alert incident has been triggered"}),
            "method": "pub",
            "replyto": null,
            "uri": "/events/alerts",
            "username": null,
         });
        expect(fims.send).toHaveBeenCalledWith({
            "body": JSON.stringify(postIncidentResponse),
            "method": "set",
            "replyto": null,
            "uri": "dummy_replyto",
            "username": null,
        });
    });

    test("UI | navigates to unresolved alerts page", async () => {
        processEvent({
            "uri": "/events/alerts",
            "method": "get",
            "body": {
                ...getIncidentsRequest,
                resolvedFilter: false,
            },
            "replyto": "dummy_replyto"
        })
        await sleep(sleepMs);
        expect(fims.send).toHaveBeenCalledTimes(1);
        expect(fims.send).toHaveBeenCalledWith({
            "body": JSON.stringify(getIncidentsResponseUnresolved),
            "method": "set",
            "replyto": null,
            "uri": "dummy_replyto",
            "username": null,
        });
    });

    test("UI | navigates to resolved alerts page", async () => {
        processEvent({
            "uri": "/events/alerts",
            "method": "get",
            "body": {
                ...getIncidentsRequest,
                resolvedFilter: true,
            },
            "replyto": "dummy_replyto"
        })
        await sleep(sleepMs);
        expect(fims.send).toHaveBeenCalledTimes(1);
        expect(fims.send).toHaveBeenCalledWith({
            "body": JSON.stringify(getIncidentsResponseResolved),
            "method": "set",
            "replyto": null,
            "uri": "dummy_replyto",
            "username": null,
        });
    });

    test("UI | user resolves an inactive alert", async () => {
        processEvent({
            "uri": "/events/alerts/dummy_incident_id2",
            "method": "set",
            "replyto": "dummy_replyto",
            "body": setIncidentRequest,
        })
        await sleep(sleepMs);
        expect(fims.send).toHaveBeenCalledTimes(2);
        expect(fims.send).toHaveBeenCalledWith({
            "body": JSON.stringify(setIncidentResponse),
            "method": "set",
            "replyto": null,
            "uri": "dummy_replyto",
            "username": null,
         });
         expect(fims.send).toHaveBeenCalledWith({
            "body": JSON.stringify({ "message": "An alert has been resolved"}),
            "method": "pub",
            "replyto": null,
            "uri": "/events/alerts",
            "username": null,
         });
    });

    test("UI | user resolves an active alert", async () => {
        processEvent({
            "uri": "/events/alerts/dummy_incident_id3",
            "method": "set",
            "replyto": "dummy_replyto",
            "body": setIncidentRequest,
        })
        await sleep(sleepMs);
        expect(fims.send).toHaveBeenCalledTimes(3);
        // reevaluate signal
        expect(fims.send).toHaveBeenCalledWith({
            "body": JSON.stringify({ reevaluate: true }),
            "method": "set",
            "replyto": null,
            "uri": "/go_metrics/events/alerts/dummy_incident_id3",
            "username": null,
        });
        expect(fims.send).toHaveBeenCalledWith({
            "body": JSON.stringify(setIncidentResponse),
            "method": "set",
            "replyto": null,
            "uri": "dummy_replyto",
            "username": null,
         });
         expect(fims.send).toHaveBeenCalledWith({
            "body": JSON.stringify({ "message": "An alert has been resolved"}),
            "method": "pub",
            "replyto": null,
            "uri": "/events/alerts",
            "username": null,
         });
    });

    test("UI | user edits an already resolved alert", async () => {
        processEvent({
            "uri": "/events/alerts/dummy_incident_id",
            "method": "set",
            "replyto": "dummy_replyto",
            "body": setIncidentRequest,
        })
        await sleep(sleepMs);
        expect(fims.send).toHaveBeenCalledTimes(2);
        expect(fims.send).toHaveBeenCalledWith({
            "body": JSON.stringify(setIncidentResponse),
            "method": "set",
            "replyto": null,
            "uri": "dummy_replyto",
            "username": null,
         });
         expect(fims.send).toHaveBeenCalledWith({
            "body": JSON.stringify({ "message": "A resolved alert has been edited"}),
            "method": "pub",
            "replyto": null,
            "uri": "/events/alerts",
            "username": null,
         });
    });

    afterAll(async () => {
        await sleep(1000);
        await mongoose.disconnect()
        global.Date = realDate;
    })
})
