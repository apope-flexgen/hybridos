# Data to FIMS (DTF)

This module's job is to take data from a CSV file and get it into FIMS. The first revision design goals are:

* Process the timestamp information in the input data in order to play back data in real time or accelerated time
* Choose whether each series of data is transmitted as a `pub` or `set`
* Publish or set to any number of configurable URIs
* Reply to a `get` with latest information
* Interpolate between data points if necessary, otherwise zero-order hold
* Process a single CSV file per program instance
* Expose configuration to FIMS to start/stop/reset playback

Later design goals include:

* Disabling automatic time stepping, accepting an external "clock"
* Process multiple CSV files in a single instance
* Play back data from web APIs such as real time market prices or weather data
* Play back data from locally stored data in InfluxDB

## Configuration

```javascript
{
    // Series name from CSV header to process for timestamp information
    "timestamp":"Timestamp", 
    // Multiplier on real time of how quickly to step through data 
    "timeMultiplier":100, 
    // How often, in ms, to `set` data
    "setRate": 500,
    // List of series to set, and their destinations
    "sets":[
        {
            // Series name from CSV header
            "src":"NMW",
            // Fully qualified URI to field to set
            "dest":"/components/fixed_load/pcmd"
        }
    ],
    // How oftien, in ms to `pub` data
    "pubRate": 500,
    // List of pub endpoints
    "pubs":[
        {
            // URI to publish as. Take care not to overload an endpoint
            "uri":"/components/gcs",
            // List of series to pub and the field to pub as
            "fields": [
                {
                    // Series name from CSV header
                    "src":"LMP",
                    // This will fully resolve to /components/gcs/price
                    "dest":"price"
                }
            ]
        }
    ]
}
```