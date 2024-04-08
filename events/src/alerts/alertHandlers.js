const { getAlertManagement, upsertAlertManagement, getAlertIncidents } = require("./alertsDb");
const fims = require('@flexgen/fims');
const { v4: uuidv4 }= require('uuid');
const { validateAlert } = require("./validateAlert");

function replyToPost(msg, err) {
    const body = {
        success: !err?.length,
        message: err
    }
    fims.send({
        method: 'set',
        uri: msg.replyto,
        replyto: null,
        body: JSON.stringify(body),
        username: null
    });
}

function pubToUI(message) {
    fims.send({
        method: 'pub',
        uri: "/events/alerts",
        replyto: null,
        body: JSON.stringify({
            message,
        }),
        username: null
    });
}

function updateGoMetricsConfig(alert) {
    const inputs = (alert.aliases ?? []).reduce((acc, entry) => {
        acc[entry.alias.toLowerCase().replaceAll(" ", "_")] = {
            uri: entry.uri, type: entry.type
        }
        return acc;
    }, {});
    // send updated configuration to go_metrics
    const goMetricsObj = {
        inputs,
        templates: [ ...(alert.templates ?? []) ],
        outputs: {},
        metrics: []
    }
    for (const site of alert.sites) {
        const perSiteId = `${alert.id}/${site}`.replaceAll(" ", "_");
        goMetricsObj.outputs[perSiteId] = {
            "uri": "/events/alerts",
            "flags": ["clothed", "sparse"],
            "attributes": {
                id: alert.id,
                organization: alert.organization,
                site: site,
            }
        }
        goMetricsObj.metrics.push({
            id: perSiteId,
            outputs: perSiteId,
            type: "bool",
            alert: true,
            expression: alert.expression,
            enabled: (alert.enabled && !alert.deleted),
        });
    }

    // send new configuration to go_metrics
    fims.send({
        method: 'set',
        uri: "/go_metrics/alerts/configuration",
        replyto: null,
        body: JSON.stringify(goMetricsObj),
        username: null
    })
}

module.exports = {
    /**
     * POST /events/alerts/management
     * UI is upserting a single alert management entry.
     * 1. Fetch relevant entry
     * 2. Combine with supplied entry, notably stuffing fields into history.
     * 3. Return to UI
     */
    handlePostAlertsManagement(msg) {
        const validation = validateAlert(msg, {
            requiredGroups: [
                ["deadline","enabled","expression","organization","severity","sites","title"],
                ["id"],
            ],
            optional: ["deleted", "aliases","templates"]
        });
        if (!validation.success) {
            fims.send({
                method: 'set',
                uri: msg.replyto,
                replyto: null,
                body: JSON.stringify(validation),
                username: null
            });
            return;
        }
        const { body } = msg;
        getAlertManagement(body, (alerts) => {
            let old = undefined;
            if (body.id) {
                old = alerts?.[0];
                if (!old) {
                    fims.send({
                        method: 'set',
                        uri: msg.replyto,
                        replyto: null,
                        body: JSON.stringify({
                            message: `Error: no alert found for id [${body.id}]`,
                            success: false,
                        }),
                        username: null
                    });
                    return;
                }
            }
            let newAlert = {
                // not settable from UI
                id: old?.id || uuidv4(),
                createdAt: old?.createdAt || new Date(),
                updatedAt: new Date(),
                version: (old?.version || 0) + 1,
                lastTriggered: old?.lastTriggered,

                // settable from UI
                aliases: body.aliases || old?.aliases,
                deadline: body.deadline || old?.deadline,
                enabled: Object.keys(body).includes("enabled") ? body.enabled : old?.enabled,
                deleted: Object.keys(body).includes("deleted") ? body?.deleted: old?.deleted ?? false,
                expression: body.expression || old?.expression,
                organization: body.organization || old?.organization,
                severity: body.severity ?? old?.severity,
                sites: body.sites || old?.sites,
                templates: body.templates || old?.templates,
                title: body.title || old?.title,

                // stuff former ui settable fields into history array except enabled
                
                history: old ? ([...old?.history, old]).map((his) => ({
                    version: his.version,

                    aliases: his.aliases,
                    deadline: his.deadline,
                    deleted: his.deleted,
                    expression: his.expression,
                    organization: his.organization,
                    severity: his.severity,
                    sites: his.sites,
                    templates: his.templates,
                    title: his.title,
                })) : []
            }

            // if we're only sending certain keys, don't append more history or change updatedAt
            if (Object.keys(body).every(key => [
                    "id", "enabled"
                ].includes(key))) {
                newAlert = {
                    ...newAlert,
                    history: old?.history,
                    updatedAt: old?.updatedAt,
                    version: old?.version,
                }
            }

            updateGoMetricsConfig(newAlert);

            // Upsert mongo
            upsertAlertManagement(newAlert, (err) => {
                if (!err) {
                    pubToUI("An alert configuration has been changed");
                }
                replyToPost(msg, err);
            });
        });
    },

    /**
     * GET /events/alerts/management
     * UI is upserting a single alert management entry.
     * 1. Fetch entries from DB
     * 2. Reformat and return to UI
     * 
     * Parameters:
     *  limit    {number} (number of results to return)
     *  page     {number} (which page in the results to return)
     *  sort     {number} (whether or not to invert the order)
     */
    handleGetAlertsManagement(msg) {
        const validation = validateAlert(msg, {
            requiredGroups: [],
            optional: ["limit", "page", "sort"]
        });
        if (!validation.success) {
            fims.send({
                method: 'set',
                uri: msg.replyto,
                replyto: null,
                body: JSON.stringify(validation),
                username: null
            });
            return;
        }
        // UI is retrieving alert management entries
        const { body } = msg;

        getAlertManagement(body, (alerts) => {
            // We don't need the incidents and we hardcode only the fields we need
            const toReturn = alerts.map(
                (alt) => ({
                    id: alt.id,
                    updatedAt: alt.updatedAt,
                    aliases: alt.aliases,
                    enabled: alt.enabled,
                    deleted: alt.deleted,
                    deadline: alt.deadline,
                    expression: alt.expression,
                    lastTriggered: alt.lastTriggered,
                    organization: alt.organization,
                    severity: alt.severity,
                    sites: alt.sites,
                    templates: alt.templates,
                    title: alt.title,
                    version: alt.version,
    
                    history: alt.history.map((his) => ({
                        aliases: his.aliases,
                        deadline: his.deadline,
                        expression: his.expression,
                        lastTriggered: his.lastTriggered,
                        organization: his.organization,
                        severity: his.severity,
                        sites: his.sites,
                        templates: his.templates,
                        title: his.title,
                        version: his.version,
                    })),
                })
            );

            fims.send({
                method: 'set',
                uri: msg.replyto,
                replyto: null,
                body: JSON.stringify(toReturn),
                username: null
            });
        })
    },

    /**
     * POST /events/alerts
     * go_metrics is sending a new incident.
     * 1. Upserts incident into DB
     * 2. PUB to UI notifying that there has been an incident.
     * 3. Returns OK to go_metrics
     */
    handlePostAlerts(msg) {
        const validation = validateAlert(msg, {
            requiredGroups: [[
                "id",
                "site",
                "status",
                "details",
            ]],
            optional: []
        });
        if (!validation.success) {
            fims.send({
                method: 'set',
                uri: msg.replyto,
                replyto: null,
                body: JSON.stringify(validation),
                username: null
            });
            return;
        }
        const { body } = msg;

        // fetch alert row by included ID.
        if (!body.id) {
            replyToPost(msg, "body.id of alert config entry is required to POST a new incident.");
            return;
        }
        getAlertManagement(body, (alerts) => {
            if (alerts.length <= 0) {
                replyToPost(msg, `No alert entry found for id ${body.id}.`);
                return;
            }
            const [alert] = alerts;
            if (!alert?.enabled || !!alert?.deleted) {
                console.warn("Request ignored because alert is !enabled or deleted.")
                replyToPost(msg, null);
                return;
            }

            // This is a new incident from go_metrics, so we need to find which instance it is for.
            // SITE is the only instance division at present so we can find relevant instance based on that.
            let instanceIndex = alert.unresolved.findIndex((inst) => {
                inst.site == alert.site
            });
            if (instanceIndex < 0) {
                alert.unresolved.push({
                    id: uuidv4(),
                    site: body.site,
                    status: body.status,
                    version: alert.version,
                    details: [],
                });
                instanceIndex = alert.unresolved.length - 1;
            }
            if (!body.details?.length) {
                replyToPost(msg, `Invalid details array supplied.`);
                return;
            }
            for (const det of body.details) {
                alert.unresolved[instanceIndex].details.push(det);
            }

            // changes were made right on the object, so we send the same obj back.
            upsertAlertManagement(alert, (err) => {
                // if no err, notify UI with a PUB
                if (!err) {
                    pubToUI("A new alert incident has been triggered");
                }
                replyToPost(msg, err);
            })

        });
    },

    /**
     * SET /events/alerts/{ID}
     * UI is resolving an alert of id {ID}
     * 1. Updates database with resolution
     * 2. Sends request to go_metrics to disable the alert
     * 3. Sends back OK to UI
     * 
     * Parameters
     *  id of incident (path parameter)
     *  resolved: t/f
     *  resolution_message: string
     */
    handleSetAlerts(msg) {
        const validation = validateAlert(msg, {
            requiredGroups: [[
                "resolution_message",
                // below there is a requirement for "id" in the path parameter
            ]],
            optional: []
        });
        if (!validation.success) {
            fims.send({
                method: 'set',
                uri: msg.replyto,
                replyto: null,
                body: JSON.stringify(validation),
                username: null
            });
            return;
        }
        const { body } = msg;
        const segments = msg.uri.split("/");
        const instanceId = segments?.[3]

        // fetch alert row by included ID.
        if (!instanceId) {
            replyToPost(msg, "ID of alert incident entry is required to set an incident /events/alerts/{incidentId}");
            return;
        }
        getAlertManagement(body, (alerts) => {
            // look for and mutate the correct alert entry
            for (const alert of alerts) {
                for (const instance of alert.unresolved) {
                    if (instance.id === instanceId) {
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
                            (inst) => inst.id !== instanceId
                        );

                        // if the alert is still active, also ping go_metrics to reevaluate the endpoint
                        if (instance.status === "Active") {
                            fims.send({
                                method: 'set',
                                uri: `/go_metrics/events/alerts/${instanceId}`,
                                replyto: null,
                                body: JSON.stringify({ reevaluate: true }),
                                username: null
                            });
                        }
                        
                        // changes were made right on the object, so we upsert the same obj which will be set by ID.
                        upsertAlertManagement(alert, (err) => {
                            if (!err) {
                                pubToUI("An alert has been resolved");
                            }
                            replyToPost(msg, err);
                        });
                        return;
                    }
                }
            }
            // double check to see if we're just editing an already resolved alert
            for (const alert of alerts) {
                for (const [index, instance] of alert.resolved.entries()) {
                    if (instance.id !== instanceId) {
                        continue;
                    }
                    alert.resolved[index].resolution_message = body.resolution_message;
                    // changes were made right on the object, so we upsert the same obj which will be set by ID.
                    upsertAlertManagement(alert, (err) => {
                        if (!err) {
                            pubToUI("A resolved alert has been edited");
                        }
                        replyToPost(msg, err);
                    });
                    return;
                }
            }
            // we never found it
            replyToPost(msg, `alert instance with id '${instanceId}' not found.`);
        });
    },

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
     *   sitesFilter: [ "Willow Hollow", "Juniper City" ]
     */
    handleGetAlerts(msg) {
        const validation = validateAlert(msg, {
            requiredGroups: [],
            optional: [
                "limit",
                "startTime",
                "endTime",
                "severityFilter",
                "orgFilter",
                "resolvedFilter",
                "sitesFilter",
                "statusFilter",
            ]
        });
        if (!validation.success) {
            fims.send({
                method: 'set',
                uri: msg.replyto,
                replyto: null,
                body: JSON.stringify(validation),
                username: null
            });
            return;
        }
        const { body } = msg;
        getAlertIncidents(body, (incidents) => {
            fims.send({
                method: 'set',
                uri: msg.replyto,
                replyto: null,
                body: JSON.stringify(incidents),
                username: null
            });
        });
    },
}
