/* eslint-disable no-console */
const mongoose = require('mongoose');
const { Event } = require('./eventsDb');
const { Alert } = require('./alerts/alertsDb');
const readline = require('readline');

const dbName = "standalone_storage";

async function clearDatabase() {
    await mongoose.connect(`mongodb://localhost:27017/${dbName}`, { useNewUrlParser: true, useUnifiedTopology: true });

    const d = new Date().toISOString();
    const eventPromise = new Promise((resolve, reject) => {
        Event.deleteMany({ timestamp: { $lt: d } }).exec((err, evt) => {
            if (err) {
                console.log("Something didn't work: ", err);
                reject(false);
            }
            if (process.env.NODE_ENV !== "test") console.log(`${evt.n} events deleted before ${d}`);
            resolve(true);
        });
    })
    const alertPromise = new Promise((resolve, reject) => {
        Alert.deleteMany().exec((err, evt) => {
            if (err) {
                console.log("Something didn't work: ", err);
                reject(false);
            }
            if (process.env.NODE_ENV !== "test") console.log(`${evt.n} events deleted before ${d}`);
            resolve(true);
        });
    });
    await Promise.all([eventPromise, alertPromise]);
}

if (process.env.NODE_ENV !== "test") {
    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout,
    });
    rl.question('Are you sure you want to want to clear the database?\nThis will delete every entry in the events collection. Type DELETE to confirm. ', (answer) => {
        if (answer === 'DELETE') {
            clearDatabase();
        }
        rl.close();
    });
}

module.exports = {
    clearDatabase,
}
