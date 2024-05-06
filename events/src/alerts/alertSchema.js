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
    organizationId: String,
    severity: Number,
    title: String,
    aliases: [Object],
    messages: [Object],
};

const baseIncidentSchema = {
    id: String,
    version: Number,
    details: [{
        message: String,
        timestamp: Date,
    }],
};

/**
 * Schema definition for alerts table
 * @type {object}
 *
 * primary fields
 * @property {string} id (also lives in go_metrics. Used to cross reference)
 * @property {string} organizationId (string)
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
        resolution_time: Date,
    }],
    history: [baseAlertSchema],
});

/**
 * Schema definition for alert template entries
 * @type {object}
 *
 * @property {string} id Unique identifier
 */
const alertTemplateSchema = mongoose.Schema({
    id: String,
    type: String,
    list: [String],
    from: Number,
    to: Number,
    minWidth: Number,
    token: String,
});

/**
 * Schema definition for alert org enum table
 * @type {object}
 *
 * @property {string} id
 * @property {string} name
 */
const alertOrganizationSchema = mongoose.Schema({
    id: String,
    name: String,
});

module.exports = {
    alertSchema,
    alertTemplateSchema,
    alertOrganizationSchema,
};
