Use the `start_setOutputOnUpdate_test.sh` script to run tests of the setOutputOnUpdate functionality. See the comments in that script for more information.

Use `stop_setOutputOnUpdate_test.sh` to stop the test and shut things down properly.

The `metrics_with_console_logs.js` file has a lot of console logging that can be illuminating while testing setOutputOnUpdate. Simply substitute the content of that file for the "real" `metrics.js` in `src`.

The testing described here focuses on the following metric in the `debug_metrics.json` file. Without the input the metric would not update and set its outputs. With this PR's code, now anything with param.setOutputOnUpdate will set on every publish whether or not the input is active. This functionality, of course, works with any metric, not just the following example.

                {
                    "id": "OF_slew_override_flag",
                    "inputs": [
                        {
                            "uri": "/components/constants",
                            "id": "FRRS_OF_slew_override_flag"
                        }
                    ],
                    "operation": "echo",
                    "initialInput": false,
                    "outputs": [
                        {
                            "uri": "/sites/odessa",
                            "id": "fr_OF_slew_override_flag"
                        }
                    ],
                    "param": {
                        "setOutputOnUpdate": true
                    }
                },
