/* eslint-disable camelcase */
/* eslint-disable no-console */
const mongoose = require('mongoose');
const alertExamples = require('./alerts/alertExamples');
const { Alert } = require('./alerts/alertsDb');
const { Event } = require('./eventsDb');
const { AlertOrganization } = require('./alerts/alertsOrgDb');
const { AlertTemplate } = require('./alerts/alertsTemplateDb');
const { clearDatabase } = require('./clearDatabase');

function loadEmUp(body) {
    const evt = Event(body);
    evt.save((err) => {
        if (err) {
            console.log('problem ', err);
        } else {
            console.log('yay');
        }
        return true;
    });
    return true;
}

const entries = [
    {
        source: '/assets/features/arbitrage',
        message: 'Entering Energy Arbitrage mode',
        timestamp: '2018-10-24T09:59:00.000Z',
        severity: 2,
    },
    {
        source: '/assets/ess/ess1',
        message: 'Energy Storage System 1 has shut down due to EPO',
        timestamp: '2018-10-24T10:00:00.000Z',
        severity: 4,
        assert: true,
        active: true,
    },
    {
        _id: '5bd1298ededcf50a275b9e08',
        source: '/assets/ess/ess2',
        message: 'Energy Storage System 2 has shut down due to EPO',
        timestamp: '2018-10-24T10:01:00.000Z',
        severity: 4,
        assert: true,
        active: false,
    },
    {
        source: '/site',
        message: "It's a new day",
        timestamp: '2018-10-24T10:02:00.000Z',
        severity: 1,
    },
    {
        source: '/assets/ess/ess2',
        message: 'Energy Storage System 2 has shut down due to EPO',
        timestamp: '2018-10-24T10:03:00.000Z',
        severity: 4,
        clear: true,
        assertId: '5bd1298ededcf50a275b9e08',
    },
    {
        source: '/assets/feeders/52-m1',
        message: 'Line undervoltage L1',
        timestamp: '2018-10-24T10:04:00.000Z',
        severity: 3,
    },
];

async function loadAlerts() {
    await mongoose.connect('mongodb://localhost:27017/standalone_storage', { useNewUrlParser: true, useUnifiedTopology: true });
    const alertEntries = [
        ...alertExamples.alertDatabaseEntries,
    ];

    await clearDatabase();

    await Promise.all(alertEntries.map(async (entry) => {
        const alert = Alert(entry);
        alert.save((err) => {
            if (err) {
                console.log('problem ', err);
            } else if (process.env.NODE_ENV !== 'test') {
                console.log('yay');
            }
            return true;
        });
        return true;
    }));

    const templateEntries = [
        ...alertExamples.templateEntries,
    ];

    await Promise.all(templateEntries.map(async (entry) => {
        const alert = AlertTemplate(entry);
        alert.save((err) => {
            if (err) {
                console.log('problem ', err);
            } else if (process.env.NODE_ENV !== 'test') {
                console.log('yay');
            }
            return true;
        });
        return true;
    }));

    const orgEntries = [
        ...alertExamples.organizationEntries,
    ];

    await Promise.all(orgEntries.map(async (entry) => {
        const org = AlertOrganization(entry);
        org.save((err) => {
            if (err) {
                console.log('problem ', err);
            } else if (process.env.NODE_ENV !== 'test') {
                console.log('yay');
            }
            return true;
        });
        return true;
    }));

    if (process.argv.length > 3 && process.argv[3] === 'stress') {
        const n = 50000;
        console.log(`Inserting ${n} resolved incident entries`);
        const alert = Alert(alertEntries[0]);

        const trigger_time = alertExamples.times.hours_ago(5);
        const resolution_time = alertExamples.times.hours_ago(3);
        for (let i = 0; i < n; i += 1) {
            alert.resolved.push({
                id: `stress-test-resolved-${i}`,
                version: 3,
                resolution_message: `Resolution for stress test ${i}`,
                resolution_time,
                details: [{
                    message: `Stress test ${i}`,
                    timestamp: trigger_time,
                }],
            });
        }
        alert.save((err) => {
            if (err) {
                console.log('problem ', err);
            } else if (process.env.NODE_ENV !== 'test') {
                console.log('stressful yay');
            }
            return true;
        });
    }
}

// if we're not running a test we want this stuff to just happen
async function loadDatabase() {
    await loadAlerts();
    entries.map(loadEmUp);
}
if (process.env.NODE_ENV !== 'test') {
    loadDatabase();
    if (process.argv.length > 3 && process.argv[3] === 'stress') {
        setTimeout(process.exit, 10000);
    } else {
        setTimeout(process.exit, 300);
    }
}

module.exports = {
    loadAlerts,
};
