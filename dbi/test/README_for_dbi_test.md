To use `dbi_test.js`, first run `./start_for_testing.sh`, then, in a separate terminal session, run `node dbi_test.js ./testData.json`.

`dbi_test.js` will exit once the testing is complete. Remember to control-c your `start_for_testing.sh` session and then run `./stop_for_testing.sh` to clean up any remaining processes when you are finished using `dbi_test.js`.

In the example above, `dbi_test.js` uses the data in `testData.json`. You can change that command line argument to point to other test data if you wish.

`testData.json` contains an array of JSON objects in the following format. Each JSON object provides 2-3 things for testing, 1) the standard parts of a FIMS message that will be fed to the `dbi.js` function that handles incoming FIMS messages (`processEvent`), 2) an "expected response" that will be compared to what's returned from `dbi.js` via a callback, and 3) an optional "note" that will appear in `dbi_test`'s terminal output.

```
    {
        "method": [the method, either "set", "get", or "del"],
        "uri": [the uri, starting with "/dbi..."],
        "replyto": [either null or a standard FIMS replyto],
        "body": [a JSON object or a single string, number, or boolean],
        "expectedResponse": [the JSON object you expect to be returned],
        "note": [(optional) any note that you want to appear under the test in the terminal output]
    }
```

Here is an example of a complete test object including the array that wraps it/them:

```
[
    {
        "method": "set",
        "uri": "/dbi/test_site_controller/assets",
        "replyto": null,
        "body": {
            "nom_vol": 220,
            "nom_freq": 80
        },
        "expectedResponse": {
            "assets": {
                "nom_vol": 220,
                "nom_freq": 80
            }
        },
        "note": "creates a new record"
    }
]
```

-DM February 2021