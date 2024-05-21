const mongoose = require('mongoose');
const fims = require('@flexgen/fims');
const { alertSchema } = require('./alertSchema');
const { getAlertOrganizations } = require('./alertsOrgDb');
const { orgIdsToNames: mapOrganizations } = require('./handlers/utils');

// add composite index on severity and timestamp to Schema
// I don't know what this does but it was done in events ¯\_(ツ)_/¯

alertSchema.index({ severity: 1 }, { background: true });

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
    async getAlertManagement(body) {
        const q = Alert.find();

        // including an ID means that this is a POST and we are looking for the 1 matching value, if any
        if (body.id) {
            q.where('id').in(body.id);
            const docs = await q.exec();
            if (docs.length === 0) {
                console.log(`warning: ID ${body.id} supplied but no matching entry in mongoDB.`);
            }
            if (docs.length >= 2) {
                console.log(`warning: Multiple entries with ID ${body.id} found in mongoDB.`);
            }
            return docs.length ? docs : [];
        }

        // otherwise we are doing a GET with these params
        if (body.limit) {
            q.limit(body.limit);
            if (body.page) {
                const skip = body.page ? (body.page - 1) * body.limit : 0;
                q.skip(skip);
            }
        }
        const sort = { updatedAt: (body.sort) ? 1 : -1 };
        q.sort(sort);
        const docs = await q.exec();
        return docs;
    },

    /**
     * Queries the database and mutates relevant alert management entry
     * @param {object} body contains params for the query
     * @param {function} callback processing function for alerts queried
     */
    async upsertAlertManagement(body) {
        try {
            await Alert.findOneAndUpdate(
                { id: body.id },
                body,
                { upsert: true },
            );
            return null;
        } catch (e) {
            console.log(e);
            return 'Error updating Alert entry in database';
        }
    },

    /**
     * Queries the database and returns alert incidents, either resolved or unresolved.
     * @param {object} body contains params for the query
     * @param {function} callback processing function for alerts queried
     */
    async getAlertIncidents(body) {
        const orgs = await getAlertOrganizations();
        const q = Alert.find();
        if (Object.keys(body || {}).includes('severityFilter')) {
            q.where('severity').in(body.severityFilter);
        }
        let incidents = [];
        const docs = await q.exec();
        docs.forEach((alertEntry) => {
            // never show incidents from a deleted alert config.
            if (alertEntry.deleted) {
                return;
            }
            // consider using lastIncident
            const resolved = !!body.resolvedFilter;
            // const resolved = true;
            const instances = resolved ? alertEntry.resolved : alertEntry.unresolved;

            instances.forEach((inst) => {
                // assume valid incident

                let config = alertEntry;
                // if this incident belongs to an older configuration
                if (alertEntry.version !== inst.version) {
                    // if there's a history entry recorded
                    const historyEntry = alertEntry.history.find((x) => x.version === inst.version);
                    if (historyEntry) {
                        config = historyEntry;
                    } else {
                        console.error('No relevant history entry found. Defaulting to current alert config.');
                    }
                }
                // last element is always the latest time-wise

                if (!inst.details.length) {
                    return;
                } 
                const triggerTime = inst.details[inst.details.length - 1].timestamp;
                if (body.orgFilter) {
                    q.where('organizationId').in(orgs.find((o) => o.name === body.orgFilter).id);
                }
                if (body.startTime && body.startTime > triggerTime) return;
                if (body.endTime && body.endTime < triggerTime) return;
                if (!resolved && body.statusFilter && body.statusFilter !== inst.status) {
                    return;
                }
                incidents.push({
                    // config inherited fields
                    organizationId: config.organizationId,
                    severity: config.severity,
                    deadline: config.deadline,
                    title: config.title,

                    // instance level fields
                    id: inst.id,
                    site: inst.site,
                    status: inst.status,
                    trigger_time: triggerTime,
                    details: inst.details.map((d) => ({ message: d.message, timestamp: d.timestamp })),

                    // resolved fields
                    resolved,
                    resolution_time: inst.resolution_time || null,
                    resolution_message: inst.resolution_message || null,
                });
            });
        });
        // default sort new => old
        incidents = incidents.sort((a, b) => (a.trigger_time < b.trigger_time ? 1 : -1));
        incidents = mapOrganizations(incidents, orgs);
        const toReturn = {
            count: incidents.length, // total incidents which match the filters captured here
        };
        if (body.limit) {
            const skip = body.page ? (body.page) * body.limit : 0;
            incidents = incidents.slice(skip, skip + body.limit);
        }
        toReturn.rows = incidents;
        return toReturn;
    },
};
