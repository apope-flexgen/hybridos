/* eslint-disable prefer-promise-reject-errors */
/* eslint-disable no-console */
const mongoose = require('mongoose');
const { Event } = require('./eventsDb');
const { Alert } = require('./alerts/alertsDb');
const { AlertOrganization } = require('./alerts/alertsOrgDb');
const { AlertTemplate } = require('./alerts/alertsTemplateDb');

const dbName = 'standalone_storage';

async function clearDatabase() {
    await mongoose.connect(`mongodb://localhost:27017/${dbName}`, { useNewUrlParser: true, useUnifiedTopology: true });

    const d = new Date().toISOString();
    const eventPromise = new Promise((resolve, reject) => {
        Event.deleteMany({ timestamp: { $lt: d } }).exec((err, evt) => {
            if (err) {
                console.log("Something didn't work: ", err);
                reject(false);
            }
            if (process.env.NODE_ENV !== 'test') console.log(`${evt.n} events deleted before ${d}`);
            resolve(true);
        });
    });
    const alertPromise = new Promise((resolve, reject) => {
        Alert.deleteMany().exec((err, evt) => {
            if (err) {
                console.log("Something didn't work: ", err);
                reject(false);
            }
            if (process.env.NODE_ENV !== 'test') console.log(`${evt.n} alerts deleted before ${d}`);
            resolve(true);
        });
    });
    const alertTemplatesPromise = new Promise((resolve, reject) => {
        AlertTemplate.deleteMany().exec((err, evt) => {
            if (err) {
                console.log("Something didn't work: ", err);
                reject(false);
            }
            if (process.env.NODE_ENV !== 'test') console.log(`${evt.n} alertTemplates deleted before ${d}`);
            resolve(true);
        });
    });
    const alertOrgsPromise = new Promise((resolve, reject) => {
        AlertOrganization.deleteMany().exec((err, evt) => {
            if (err) {
                console.log("Something didn't work: ", err);
                reject(false);
            }
            if (process.env.NODE_ENV !== 'test') console.log(`${evt.n} alertOrgs deleted before ${d}`);
            resolve(true);
        });
    });
    await Promise.all([eventPromise, alertPromise, alertTemplatesPromise, alertOrgsPromise]);
}

if (!['loadData', 'test'].includes(process.env.NODE_ENV)) {
    clearDatabase();
    console.log('Database has been cleared');
}

module.exports = {
    clearDatabase,
};
