// TODO: fix this rule
/* eslint-disable max-lines */
const { faker } = require('@faker-js/faker');
const fs = require('fs');

/**
 * Usage:
 *
 * node generateFakeEvents.ts
 * mongoimport --uri=mongodb://localhost:27017/standalone_storage --collection events --jsonArray --type json --file fake_events.json
 *
 * Note: 1M records is about ~100MB +/-10MB (dependent on random data generator)
 * Note: Profile query in mongo
 *   - use standalone_storage
 *   - db.setProfilingLevel(2)
 *   - db.system.profile.find({"op": "query", "ns" : "standalone_storage.events"}).sort({"ts":-1}).pretty().limit(1)
 */

const MS_IN_DAY = 86400000;
const DAYS_BEHIND = 7;
const NUMBER_OF_RECORDS = 1000;
const sources = ['Assets', 'Site', 'Modbus Client', 'Modbus Server', 'COPS'];
var records = [];

var minDate = Date.now() - MS_IN_DAY * DAYS_BEHIND;
var maxDate = Date.now();

const generateRecord = () => {
  return {
    source: faker.helpers.arrayElement(sources),
    message: faker.datatype.string(10),
    timestamp: { $date: faker.datatype.datetime({ min: minDate, max: maxDate }) },
    severity: faker.datatype.number({ min: 1, max: 4 }),
  };
};

for (let i = 0; i < NUMBER_OF_RECORDS; i++) {
  records.push(generateRecord());
}

var json = JSON.stringify(records);
fs.writeFile('fake_events.json', json, 'utf8', () =>
  console.log(`generated ${NUMBER_OF_RECORDS} events`),
);
