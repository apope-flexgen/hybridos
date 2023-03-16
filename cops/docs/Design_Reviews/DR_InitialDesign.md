# Design Review

## Goals
Develop a supervising process that monitors other processes to see if they are dead or hung using heartbeats.

## Affected Repos
New repo: cops (Central Operating Process Supervisor)
config
Any repos that need a heartbeat counter added to them

## Approach

Each process has its own heartbeat value.

There are three threads that run in COPS: sendHeartbeat, processFIMS, and patrolProcesses. sendHeartbeat and patrolProcesses execute at a configurable frequency, wherease processFIMS is always available to process newly received FIMS messages.

- sendHeartbeat sends a process's COPS heartbeat value to it via a FIMS SET, then follows the SET with sending a FIMS GET requesting the process's stored heartbeat value.

- When processFIMS receives the reply SET, it checks the received heartbeat value against the process's COPS heartbeat value. If the two heartbeats are equal, COPS will update that process's timestamp for `lastConfirmedAlive` and increment that process's COPS heartbeat. If the received PID is different than the stored PID, COPS will update the stored PID.

- patrolProcesses checks each process's `hangTimeAllowance` against the difference between the current time and the process's `lastConfirmedAlive`. If the process has been unresponsive for too long, COPS will execute failure actions.

Failure actions include printing warnings to the console, sending alarm notifications to the events module, and killing the process if it is hung and not fully dead.

## Interface
COPS will be started alongside the rest of the processes. It will interface with other processes via FIMS and with the user via the event notifications.

## Testing
COPS can be tested by purposefully killing processes and making sure COPS responds as expected.

## Backward Compatibility
COPS is not backwards compatible.

## Configuration
Example of new way to configure COPS:

```json
{
    "ourUri": "/cops",
    "heartbeatFrequencyMS": 1000,
    "patrolFrequencyMS": 1000,
    "processList": [
        {
            "name": "hybridos_controller",
            "uri": "/site",
            "killOnHang": true,
            "hangTimeAllowanceMS": 3000
        },
        {
            "name": "modbus_client",
            "uri": "/components/modbus_test_client",
            "killOnHang": true,
            "hangTimeAllowanceMS": 3000
        }
    ]
}
```