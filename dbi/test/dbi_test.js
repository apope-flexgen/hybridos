/* eslint-disable no-console */
const fs = require('fs');
// eslint-disable-next-line no-unused-vars
const colors = require('colors'); // this allows for addition of color to CLI,
// for example, `console.log('test'.yellow.bold)`

const { processEvent } = require('../src/dbi');

let testData;
const testDataFile = process.argv[2];
if (!testDataFile || !fs.existsSync(testDataFile)) {
    throw new Error('Please supply a path to the test data .json file. Usage: node dbi_test.js ./testData.json');
} else {
    // eslint-disable-next-line import/no-dynamic-require, global-require
    testData = require(testDataFile);
}

const theLength = testData.length;
let theMatches = 0;
let theNoMatches = 0;

function runTest(i) {
    if (i === 0) console.log(`\n\n\nBeginning Testing: ${new Date().toLocaleString()}`.yellow.bold);
    const {
        method, uri, replyto, body, expectedResponse, note,
    } = testData[i];
    const theMsg = {
        method,
        uri,
        replyto,
        body,
    };
    const expectedResponseStringified = JSON.stringify(expectedResponse);
    processEvent(theMsg, (res, err) => {
        if (res) {
            if (res === expectedResponseStringified) {
                theMatches += 1;
                console.log(`\nTest ${i + 1}: ${method.toUpperCase()}, request gets expected response:`.green.bold);
                console.log('Request: '.white, `${method}`.bold.white, `${uri} ${JSON.stringify(body)}`.white);
                console.log(`\nResponse: ${res}`.white);
            } else {
                theNoMatches += 1;
                console.log(`\nTest ${i + 1}: ${method.toUpperCase()}, request DOES NOT get expected response:`.red.bold);
                console.log('Request: '.white, `${method}`.bold.white, `${uri} ${JSON.stringify(body)}`.white);
                console.log('\nResponse expected:'.bold.white, `${expectedResponseStringified}`.white);
                console.log('Response received:'.bold.white, `${res}`.white);
            }
            if (note) console.log(`\nNOTE: ${note}`.cyan);
            console.log('\n-------'.white.bold);
            if (i === theLength - 1) {
                console.log(`\n\nTesting Complete: ${new Date().toLocaleString()}`.yellow.bold);
                if (theMatches > 0) console.log(`    ${theMatches} of ${theLength} tests passed`.green.bold);
                if (theNoMatches > 0) console.log(`    ${theNoMatches} of ${theLength} tests DID NOT pass`.red.bold, '\nYou may want to review the items that did not pass - sometimes they are expected to not pass.');
                console.log('\n===================');
                console.log('\n');
                process.exit();
            }
            runTest(i + 1);
        } else {
            console.log(err);
            process.exit();
        }
    });
}

setTimeout(() => {
    runTest(0);
}, 1500);
