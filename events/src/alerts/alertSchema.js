const mongoose = require('mongoose');

const baseAlertSchema = {
    // internal fields
    version: Number,
    updatedAt: Date,

    // ui settable fields
    deadline: Number,
    deleted: Boolean,
    enabled: Boolean, // doesn't trigger a history entry
    expression: String,
    organization: String,
    severity: Number,
    sites: [String],
    title: String,
    templates: [ Object ],
    aliases: [ Object ],
};

const baseIncidentSchema = {
    id: String,
    site: String,
    version: Number,
    details: [{
        message: String,
        timestamp: Date
    }]
}

/**
 * Schema definition for alerts table
 * @type {object}
 *
 * primary fields
 * @property {string} id (also lives in go_metrics. Used to cross reference)
 * @property {string} organization (string) [hidden on control UI]
 * @property {string} sites (string) [hidden on control UI]
 * @property {number} severity (number)
 * @property {number} deadline (int)
 * @property {object} unresolved [ baseIncidentSchema ]
 * @property {object} resolved [ baseIncidentSchema ]
 * 
 * internal fields
 * @property {number} version (int)
 * @property {object} history ([ baseAlertSchema ])
 */
const alertSchema = mongoose.Schema({
    id: String,
    createdAt: Date,
    lastTriggered: Date,
    ...baseAlertSchema,
    unresolved: [{
        ...baseIncidentSchema,
        status: String,
    }],
    resolved: [{
        ...baseIncidentSchema,
        resolution_message: String,
        resolution_time: Date
    }],
    history: [baseAlertSchema]
});

module.exports = {
    alertSchema,
}
