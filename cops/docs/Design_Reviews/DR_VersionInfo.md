# Design Review
## Goals
As a UI user, I want to see process versions for each process.

## Affected Repos
site_controller, cops, config

## Approach
Site Controller utilizes existing Version object and loads version tag/build/commit during construction.

Add version tag, build, and commit to the reply SET that goes from Site Controller to COPS when COPS sends a GET to Site Controller.

In COPS, rewrite the statisticsReport thread that currently updates and publishes process statistics every 3 seconds to `/cops/processStats` to simply just update the statistics every 3 seconds. This statistics report can be accessed via a GET to `/cops/processStats`.

Make a new goroutine to publish a general briefing report every 5 seconds to `/cops`. For right now, it will only contain the process version info. More information can be added in the future.

## Interface
COPS<->Site Controller interface = FIMS

Publishes go out on FIMS

## Testing
- Manual FIMS listen to make sure briefing is being published.
- Manual FIMS GET to `/cops/processStats` to verify correct statistics are still updating.
- Make sure all unit tests are still passing.

## Backward Compatibility
This will not be compatible with older versions of COPS or Site Controller without updating the necessary config/binary files.

## Configuration
`"statsPubFrequencySeconds": 3,` changes to `"briefingPubFrequencyMS": 5000,` as shown below.

```json
{
    "ourUri": "/cops",
    "heartbeatFrequencyMS": 1000,
    "patrolFrequencyMS": 1000,
    "briefingPubFrequencyMS": 5000,
    "primaryIP": "192.168.1.1",
    "thisCtrlrStaticIP": "172.16.1.80",
    "otherCtrlrStaticIP": "172.16.2.82",
    "pduIP": "10.0.1.83",
    "processList": [
        {
            "name": "hybridos_controller",
            "uri": "/site",
            "killOnHang": true,
            "requiredForHealthyStatus": true,
            "hangTimeAllowanceMS": 3000
        }
    ]
}
```