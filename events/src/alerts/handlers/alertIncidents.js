const fims = require('@flexgen/fims');
const { v4: uuidv4 } = require('uuid');
const { getAlertManagement, upsertAlertManagement, getAlertIncidents } = require('../alertsDb');
const { validateAlert } = require('../validateAlert');
const { pubToUI, genericReply } = require('./utils');

/**
     * POST /events/alerts
     * go_metrics is sending a new incident.
     * 1. Upserts incident into DB
     * 2. PUB to UI notifying that there has been an incident.
     * 3. Returns OK to go_metrics
     */
async function handlePostAlerts(msg) {
    const validation = validateAlert(msg, {
        requiredGroups: [[
            'config_id',
            'status',
            'details',
        ]],
        optional: ['value', 'name', 'incident_id'],
    });
    if (!validation.success) {
        if (msg.replyto) {
            fims.send({
                method: 'set',
                uri: msg.replyto,
                replyto: null,
                body: JSON.stringify(validation),
                username: null,
            });
        }
        return;
    }
    const { body } = msg;

    const alerts = await getAlertManagement({ id: body.config_id });
    if (alerts.length <= 0) {
        genericReply(msg, `No alert config entry found for id ${body.config_id}.`);
        return;
    }
    const [alert] = alerts;
    if (!alert.enabled || !!alert.deleted) {
        console.warn('Request ignored because alert is !enabled or deleted.');
        genericReply(msg, null);
        return;
    }

    if (!Array.isArray(body.details)) {
        genericReply(msg, 'Invalid details array supplied.');
        return;
    }

    if (body.details.length > 0) {
        alert.lastTriggered = body.details[body.details.length - 1].timestamp || null;
    }

    let incidentIndex = alert.unresolved.findIndex((inst) => inst.id === body.incident_id);
    // if there isn't an entry with this incident_id, make a new one
    if (incidentIndex < 0) {
        incidentIndex = alert.unresolved.length;
        alert.unresolved.push({
            id: body.incident_id || uuidv4(),
            version: alert.version,
            status: body.status,
            details: [],
        });
    }

    // UPDATE status and APPEND details
    alert.unresolved[incidentIndex].status = body.status;
    alert.unresolved[incidentIndex].details.push(...body.details);

    // changes were made right on the object, so we send the same obj back.
    const err = await upsertAlertManagement(alert);
    if (!err) {
        pubToUI('A new alert incident has been triggered');
    }
    genericReply(msg, err);
}

/**
 * SET /events/alerts/{ID}
 * UI is resolving an alert of id {ID}
 * 1. Updates database with resolution
 * 2. Sends request to go_metrics to disable the alert
 * 3. Sends back OK to UI
 *
 * This process is checked firstly against unresolved alerts, and if none are found, it is repeated
 * to check for a mutation on a resolved alert.
 *
 * Parameters
 *  id of incident (path parameter)
 *  resolved: t/f
 *  resolution_message: string
 */
async function handleSetAlerts(msg) {
    const validation = validateAlert(msg, {
        requiredGroups: [[
            'resolution_message',
            // below there is a requirement for "id" in the path parameter
        ]],
        optional: [],
    });
    if (!validation.success) {
        if (msg.replyto) {
            fims.send({
                method: 'set',
                uri: msg.replyto,
                replyto: null,
                body: JSON.stringify(validation),
                username: null,
            });
        }
        return;
    }
    const { body } = msg;
    const segments = msg.uri.split('/');
    const instanceId = segments.length > 2 ? segments[3] : undefined;

    // fetch alert row by included ID.
    if (!instanceId) {
        genericReply(msg, 'ID of alert incident entry is required to set an incident /events/alerts/{incidentId}');
        return;
    }
    const alerts = await getAlertManagement(body);
    function searchInstances(byKey) {
        for (const alert of alerts) {
            for (const [instanceIndex, instance] of Object.entries(alert[byKey])) {
                if (instance.id === instanceId) {
                    return {
                        alert, instance, instanceIndex, found: true,
                    };
                }
            }
        }
        return { found: false };
    }
    // look for the correct alert entry
    const unresolvedSearch = searchInstances('unresolved');

    // if we found it
    if (unresolvedSearch.found) {
        const { alert, instance } = unresolvedSearch;
        // append entry to resolved
        alert.resolved.push({
            id: instance.id,
            site: instance.site,
            version: instance.version,
            details: instance.details,
            resolution_time: new Date().toISOString(),
            resolution_message: body.resolution_message,
        });
        // trim out entry from unresolved
        alert.unresolved = alert.unresolved.filter(
            (inst) => inst.id !== instanceId,
        );

        // if the alert is still active, also ping go_metrics to reevaluate the endpoint
        if (instance.status === 'active') {
            fims.send({
                method: 'set',
                uri: `/go_metrics_alerting/events/alerts/${alert.id}`,
                replyto: null,
                body: JSON.stringify({ reevaluate: true }),
                username: null,
            });
        }

        // changes were made right on the object, so we upsert the same obj which will be set by ID.
        const err = await upsertAlertManagement(alert);
        if (!err) {
            pubToUI('An alert has been resolved');
        }
        genericReply(msg, err);
        return;
    }

    // we didn't find it in unresolved, double check to see if we're just editing an already resolved alert
    const resolvedSearch = searchInstances('resolved');
    if (resolvedSearch.found) {
        const { alert, instanceIndex } = resolvedSearch;
        alert.resolved[instanceIndex].resolution_message = body.resolution_message;
        // changes were made right on the object, so we upsert the same obj which will be set by ID.
        const err = await upsertAlertManagement(alert);
        if (!err) {
            pubToUI('A resolved alert has been edited');
        }
        genericReply(msg, err);
        return;
    }

    // we never found it
    genericReply(msg, `alert instance with id '${instanceId}' not found.`);
}

/**
 * GET /events/alerts
 * UI is retrieving alerts.
 * 1. Fetches alert entries from Mongo
 * 2. Compiles list of incidents
 * 3. Sends list to UI
 *
 * Parameters:
 *   limit: 10,
 *   startTime: "2024-03-13T17:47:33.820Z",
 *   endTime: "2024-03-15T17:47:33.820Z",
 *
 *   // filters
 *   statusFilter: "Active",
 *   severityFilter: 1,
 *   orgFilter: "VFakeCo",
 *   resolvedFilter: false,
 */
async function handleGetAlerts(msg) {
    const validation = validateAlert(msg, {
        requiredGroups: [],
        optional: [
            'limit',
            'page',
            'startTime',
            'endTime',
            'severityFilter',
            'orgFilter',
            'resolvedFilter',
            'statusFilter',
        ],
    });
    if (!validation.success) {
        if (msg.replyto) {
            fims.send({
                method: 'set',
                uri: msg.replyto,
                replyto: null,
                body: JSON.stringify(validation),
                username: null,
            });
        }
        return;
    }
    const { body } = msg;
    const incidents = await getAlertIncidents(body);
    if (msg.replyto) {
        fims.send({
            method: 'set',
            uri: msg.replyto,
            replyto: null,
            body: JSON.stringify(incidents),
            username: null,
        });
    }
}

// Run at startup of /events to fetch any existing alert incidents from go_metrics.
async function handleInitAlerts() {
    fims.send({
        method: 'get',
        uri: '/go_metrics_alerting/events/alerts',
        replyto: '/events/refresh_alerts',
        body: null,
        username: null,
    });
    fims.receiveWithTimeout(500, (result) => {
        try {
            if (result && result.body && Array.isArray(result.body)) {
                result.body.forEach((incident) => {
                    handlePostAlerts({
                        body: incident,
                        replyto: '/events/refresh_alerts_reply',
                    });
                });
                console.info(`Initialized with ${result.body.length} incidents from go_metrics`);
            } else {
                console.warn('No valid response from /go_metrics_alerting/events/alerts on events startup');
            }
        } catch (e) {
            console.error(`Error initializing alert entries from go_metrics ${e}`);
        }
    });
}

module.exports = {
    handleGetAlerts,
    handleInitAlerts,
    handlePostAlerts,
    handleSetAlerts,
};
