/* eslint-disable no-console */
const mongoose = require('mongoose');

const dbName = process.argv[2];
if (!dbName) {
    throw new Error('Please supply the database name. Usage: node clearDatabase.js databaseName');
}

mongoose.connect(`mongodb://localhost:27017/${dbName}`, { useNewUrlParser: true });
const { ObjectId } = mongoose.Schema.Types;
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
const Event = mongoose.model('events', sch);

const readline = require('readline');

const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout,
});

rl.question('Are you sure you want to want to clear the database?\nThis will delete every entry in the events collection. Type DELETE to confirm. ', (answer) => {
    if (answer === 'DELETE') {
        const d = new Date().toISOString();
        Event.deleteMany({ timestamp: { $lt: d } }).exec((err, evt) => {
            if (err) {
                console.log("Something didn't work: ", err);
                return false;
            }
            console.log(`${evt.n} events deleted before ${d}`);
            return true;
        });
    }
    rl.close();
});
