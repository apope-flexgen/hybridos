const mongoose = require('mongoose');
const { alertSchema } = require('./alertSchema');
const fims = require('@flexgen/fims');

// add composite index on severity and timestamp to Schema
// I don't know what this does but it was done in events ¯\_(ツ)_/¯

alertSchema.index({timestamp: -1, severity: 1, source: 1}, {background: true})

const Alert = mongoose.model('alerts', alertSchema);
/**
 * Query events.alerts module
 * @module alertsDb
 * 
 */
module.exports = {
    Alert,
    /**
     * Queries the database and returns alert management entries. If an ID Is supplied it simply returns the entry with that ID.
     * @param {object} body contains params for the query
     * @param {function} callback processing function for alerts queried
     */
    getAlertManagement(body, callback) {
        const q = Alert.find();

        // including an ID means that this is a POST and we are looking for the 1 matching value, if any
        if (body?.id) {
            q.where('id').in(body?.id)
            q.exec((err, docs) => {
                if (docs?.length == 0) {
                    console.log(`warning: ID ${body?.id} supplied but no matching entry in mongoDB.`);
                }
                if (docs?.length >= 2) {
                    console.log(`warning: Multiple entries with ID ${body?.id} found in mongoDB.`);
                }
                if (err) {
                    console.log(`mongoDB query error: ${err}`);
                    return false;
                }
                callback(docs.length ? docs : []);
                return true;
            });
            return;
        }

        // otherwise we are doing a GET with these params
        if (body?.limit) {
            q.limit(body?.limit);
            if (body?.page) {
                const skip = body?.page ? (body?.page - 1) * body?.limit : 0;
                q.skip(skip);
            }
        }
        const sort = { updatedAt: (body?.sort) ? 1 : -1 };
        q.sort(sort)
            .exec((err, docs) => {
                if (err) {
                    console.log(`mongoDB query error: ${err}`);
                    return false;
                }
                callback(docs);
                return true;
            });
    },

    /**
     * Queries the database and mutates relevant alert management entry
     * @param {object} body contains params for the query
     * @param {function} callback processing function for alerts queried
     */
    upsertAlertManagement(body, callback) {
        try {
            Alert.findOneAndUpdate(
                { "id": body?.id },
                body,
                { upsert: true },
                callback
            )
            return null;
        } catch (e) {
            console.log(e);
            return "Error updating Alert entry in database";
        }
    },

    /**
     * Queries the database and returns alert incidents, either resolved or unresolved.
     * @param {object} body contains params for the query
     * @param {function} callback processing function for alerts queried
     */
    getAlertIncidents(body, callback) {
        const q = Alert.find();
        if (body?.orgFilter) {
            q.where('organization').in(body?.orgFilter);
        }
        if (Object.keys(body || {}).includes("severityFilter")) {
            q.where('severity').in(body?.severityFilter);
        }
        if (body?.sitesFilter) {
            q.where('sites').in(body?.sitesFilter); 
        }
        q.exec((err, docs) => {
            if (err) {
                console.log(`mongoDB query error: ${err}`);
                return false;
            }
            let incidents = [];
            for (const alertEntry of docs) {
                // consider using lastIncident
                const resolved = !!body?.resolvedFilter;
                // const resolved = true;
                const instances = resolved ? alertEntry["resolved"] : alertEntry["unresolved"];
                
                for (const inst of instances) {
                    
                    // assume valid incident
                    
                    let config = alertEntry;
                    // if this incident belongs to an older configuration
                    if (alertEntry.version !== inst.version) {
                        // if there's a history entry recorded
                        const historyEntry = alertEntry.history.find((x) => x.version == inst.version);
                        if (historyEntry) {
                            config = historyEntry;
                        } else {
                            console.error("No relevant history entry found. Defaulting to current alert config.")
                        }
                    }
                    const triggerTime = inst.details[0].timestamp;
                    if (body?.sitesFilter && !body?.sitesFilter.includes(inst.site)) continue;
                    if (body?.startTime && body?.startTime > triggerTime) continue;
                    if (body?.endTime && body?.endTime < triggerTime) continue;
                    if (!resolved && body?.statusFilter && body?.statusFilter !== inst.status) {
                        continue;
                    }
                    incidents.push({
                        // config inherited fields
                        organization: config.organization,
                        severity: config.severity,
                        deadline: config.deadline,
                        title: config.title,

                        // instance level fields
                        id: inst.id,
                        site: inst.site,
                        status: inst.status,
                        resolved: inst.resolved,
                        trigger_time: triggerTime,
                        details: inst.details.map((d) => ({message: d.message, timestamp: d.timestamp})),

                        // resolved fields
                        resolved,
                        resolution_time: inst.resolution_time || null,
                        resolution_message: inst.resolution_message || null,
                    });
                }
            }
            // default sort new => old
            incidents = incidents.sort((a, b) =>
                a.trigger_time < b.trigger_time ? 1 : -1
            );
            const toReturn = {
                count: incidents.length, // total incidents which match the filters captured here
            }
            if (body?.limit) {
                incidents = incidents.slice(0, body?.limit);
            }
            toReturn.rows = incidents;
            callback(toReturn);
        })
    },

    // Run at startup of /events to fetch any existing alert incidents from go_metrics.
    initializeAlerts() {
        fims.send({
            method: 'get',
            uri: "/go_metrics/events/alerts",
            replyto: "/events/refresh_alerts",
            body: null,
            username: null
        });
        fims.receiveWithTimeout(500, (result) => {
            if (result) {
                for (const incident of result) {
                    handlePostAlerts({
                        "body": incident,
                        "replyto": "/events/debug"
                    });
                }
            }
        })
    }
};
