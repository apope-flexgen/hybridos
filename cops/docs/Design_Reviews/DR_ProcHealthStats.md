# Design Review

## Goals

As a support engineer I want statistics on how stable running processes are

## Affected Repos

cops

## Approach

COPS will report the timestamp of when it began running along with the following statistics for each process:
- Process alive or dead
- Timestamp of last restart. If no restarts, it will equal COPS time of start
- Cumulative restarts
- Average heartbeat response time
- Max heartbeat response time
- Latest heartbeat response time
- Average % memory usage
- Max % memory usage
- Latest % memory usage
- Average % CPU usage

Use exec.Command() function and give it the command `ps -o pid,%cpu,%mem p 4855,4916,5223` to get output data about relevant processes which COPS can then parse to get CPU and memory usage. In this example, `4855,4916,5223` is a list of the PIDs. The output data of the Linux command can be returned to a bytes.Buffer variable (see example in reference "Running Linux commands with Go") and converted to a string for parsing.

Publish the statistics on the URI `/cops/process_stats` at a configurable frequency

### Example Statistics Publish
```json
{
    "COPSTimeOfStart": "2021-01-20 12:00:00",
    "procStats": 
    [
        {
            "procName": "hybridos_controller",
            "alive": true,
            "lastRestart": "2021-01-20 12:00:00",
            "totalRestarts": 0,
            "avgResponseTimeMS": 123,
            "maxResponseTimeMS": 321,
            "lastResponseTimeMS": 231,
            "avgMemUsagePct": 2.3,
            "maxMemUsagePct": 3.2,
            "lastMemUsagePct": 2.2,
            "avgCpuUsagePct": 8.9
        },
        {
            "procName": "metrics",
            "alive": true,
            "lastRestart": "2021-02-03 10:41:28",
            "totalRestarts": 3,
            "avgResponseTimeMS": 123,
            "maxResponseTimeMS": 321,
            "lastResponseTimeMS": 231,
            "avgMemUsagePct": 2.3,
            "maxMemUsagePct": 3.2,
            "lastMemUsagePct": 2.2,
            "avgCpuUsagePct": 8.9
        },
        {
            "procName": "modbus_client",
            "alive": true,
            "lastRestart": "2021-01-26 05:52:46",
            "totalRestarts": 3,
            "avgResponseTimeMS": 123,
            "maxResponseTimeMS": 321,
            "lastResponseTimeMS": 231,
            "avgMemUsagePct": 2.3,
            "maxMemUsagePct": 3.2,
            "lastMemUsagePct": 2.2,
            "avgCpuUsagePct": 8.9
        }
    ]
}
```

### References

Timestamps - https://golang.org/pkg/time/#Time

Running Linux commands with Go - https://golang.org/pkg/os/exec/#Command

Linux "top" command - https://www.geeksforgeeks.org/top-command-in-linux-with-examples/#:~:text=top%20command%20is%20used%20to,managed%20by%20the%20Linux%20Kernel

Getting specific processes in "top" output - https://electrictoolbox.com/linux-top-only-one-process/

## Interface

COPS will periodically publish process statistics on FIMS. Anyone viewing this FIMS traffic can see the statistics.

## Testing

If the statistics are visible and previous COPS tests are still passing, the feature works.

## Backward Compatibility

Update required

## Configuration

New configuration variable required to set stat publish frequency

```json
{
    "ourUri": "/cops",
    "heartbeatFrequencyMS": 1000,
    "patrolFrequencyMS": 1000,
    "statsPublishFrequencySeconds": 3,
    "processList": [
        {
            "hybridos_controller": {
                "uri": "/site",
                "killOnHang": true,
                "hangTimeAllowanceMS": 3000
            }
        },
        {
            "modbus_client": {
                "uri": "/components/modbus_test_client",
                "killOnHang": true,
                "hangTimeAllowanceMS": 3000
            }
        }
    ]
}
```
