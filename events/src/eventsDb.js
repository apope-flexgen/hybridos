/* eslint-disable no-console */
const fs = require('fs');
const path = require('path');
const mongoose = require('mongoose');

const { ObjectId } = mongoose.Schema.Types;

pathArg = "/usr/local/etc/config/events"
const configPath = path.resolve(pathArg);
const configPathAndFile = path.join(configPath, 'events.json');
if (!fs.existsSync(configPathAndFile)) {
    throw new Error(`events.json not found in ${configPath}`);
}

// eslint-disable-next-line import/no-dynamic-require
const fimsConfig = require(`${configPathAndFile}`);
const dbName = fimsConfig.dbName ? fimsConfig.dbName : 'hybridos';
const ipName = fimsConfig.mongo_ip ? fimsConfig.mongo_ip : 'localhost';
const portName = fimsConfig.mongo_port ? fimsConfig.mongo_port : '27017';

// Set up Mongo document schema
mongoose.connect(`mongodb://${ipName}:${portName}/${dbName}`, { useNewUrlParser: true, useUnifiedTopology: true });

/**
 * Schema definition for events table
 * @type {object}
 * @property {string} source where the event was emitted from, different from replyto
 * @property {Date} timestamp time when event was emitted
 * @property {string} message information about event
 * @property {number} severity enumerated severity level from 0 - 4
 * [DEBUG, INFO, STATUS, ALARM, FAULT]
 * @property {boolean} assert used in tandem with active to clear alarms/faults
 * @property {boolean} active set by events module when assert is true, cleared when assert is false
 * @property {boolean} clear marks event as a clear event for previous asserted events
 * @property {ObjectId} assertID id of previous asserted event to clear
 */
const sch = mongoose.Schema({
    source: String, // Who emitted the event, separate from replyto
    timestamp: Date,
    message: String, // Should this be validated/limited in length?
    severity: Number, // Will internationalize severity level here
    // [DEBUG, INFO, STATUS, ALARM, FAULT] 0..4
    assert: Boolean, // So callers can clear their alarms/fault, may auto clear after a time?
    active: Boolean, // Set by events module when "assert" is present, and cleared when
    // the commensurate clear event arrives
    clear: Boolean, // Mutually exclusive with "assert". Lets the event module know that this
    // event is meant to clear a previously asserted event
    assertId: ObjectId, // _id of previously asserted event to clear
});

// add composite index on severity and timestamp to Schema
sch.index({timestamp: -1, severity: 1, source: 1}, {background: true})

const Event = mongoose.model('events', sch);

/**
 * Query events module
 * @module eventsDB
 */
module.exports = {
    dbName,

    Event,

    /**
     * Queries the database and returns events
     * @param {object} body contains params for the query
     * @param {function} callback processing function for events queried
     */
    eventsQuery(body, callback) {
        if (body.count) {
            const q = Event.countDocuments();
            // .count was deprecated, use .countDocuments now
            if (body.severity) {
                q.where('severity').in(body.severity);
            }
            if (body.message) {
                q.where('message').regex(new RegExp(body.message, 'i'));
            }
            if (body.after) {
                q.where('timestamp').gte(body.after);
            }
            if (body.before) {
                q.where('timestamp').lte(body.before);
            }
            q.exec((err, docs) => {
                if (err) {
                    console.log(`mongoDB query error: ${err}`);
                    return false;
                }
                callback(docs);
                return true;
            });
        } else {
            const q = Event.find();
            if (body.severity) {
                q.where('severity').in(body.severity);
            }
            if (body.message) {
                q.where('message').regex(new RegExp(body.message, 'i'));
            }
            if (body.after) {
                q.where('timestamp').gte(body.after);
            }
            if (body.before) {
                q.where('timestamp').lte(body.before);
            }
            if (body.source) {
                const re = new RegExp(`^${body.source}`);
                q.find({ source: re });
            }
            if (body.active) {
                q.find({ active: body.active });
            }
            const limit = body.limit ? body.limit : 100;
            const skip = body.page ? (body.page - 1) * limit : 0;
            const sort = body.sort ? body.sort : { timestamp: 1 };
            q.sort(sort)
                .limit(limit)
                .skip(skip)
                .exec((err, docs) => {
                    if (err) {
                        console.log(`mongoDB query error: ${err}`);
                        return false;
                    }
                    callback(docs);
                    return true;
                });
        }
    },
    /**
     * Clears active events
     * @param {assertID} clearEvt event ID to clear
     * @param {function} callback post processing callback
     */
    clearActiveEventQuery(clearEvt, callback) {
        Event.findOneAndUpdate({ _id: clearEvt.assertId }, { active: false }, callback);
        return true;
    },
};
