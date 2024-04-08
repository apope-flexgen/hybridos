/* eslint-disable no-console */
const alertExamples = require('./alerts/alertExamples.js');
const { Alert } = require('./alerts/alertsDb.js');
const { Event } = require('./eventsDb.js');
const mongoose = require('mongoose');

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
    await mongoose.connect(`mongodb://localhost:27017/standalone_storage`, { useNewUrlParser: true, useUnifiedTopology: true });
    const alertEntries = [
        ...alertExamples.databaseEntries
    ];

    await Promise.all(alertEntries.map(async (entry) => {
        const alert = Alert(entry);
        alert.save((err) => {
            if (err) {
                console.log('problem ', err);
            } else {
                if (process.env.NODE_ENV !== "test") {
                    console.log('yay');
                }
            }
            return true;
        });
        return true;
    }));
}

// if we're not running a test we want this stuff to just happen
if (process.env.NODE_ENV !== "test") {
    entries.map(loadEmUp);
    loadAlerts();
}

module.exports = {
    loadAlerts,
}
