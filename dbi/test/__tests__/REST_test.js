const { processEvent } = require('../../src/dbi');
const testData = require('./REST_test_data.json');

const theLength = testData.length;

function runTest(i, callback) {
    const {
        method, uri, replyto, body, expectedResponse,
    } = testData[i];
    const theMsg = {
        method,
        uri,
        replyto,
        body,
    };
    const expectedResponseStringified = JSON.stringify(expectedResponse);
    processEvent(theMsg, (res) => {
        if (res && res === expectedResponseStringified) {
            callback(true);
        } else {
            callback(false);
        }
    });
}

function runJestTestsAndExit(callback2) {
    for (let i = 0; i < theLength; i += 1) {
        test(`Test ${i}: ${testData[i].method} ${testData[i].uri} is handled properly`, (done) => {
            function callback(res) {
                try {
                    expect(res).toBe(true);
                    done();
                } catch (error) {
                    done(error);
                }
            }
            runTest(i, callback);
        });
        if (i === theLength - 1) {
            callback2();
            // sending this back as a callback, as opposed to just doing the timeout
            // and process.exit here, avoids reporting a jest error in the console.
        }
    }
}

runJestTestsAndExit(() => {
    setTimeout(() => {
        process.exit(0);
    }, 3000);
});
