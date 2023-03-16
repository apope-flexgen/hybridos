# Design Review

## Goals
As a support engineer, I want the controller to be able to determine when it should take over primary

## Affected Repos
cops

## Approach
Return a Total Health Score when ModeQuery is received from Other controller. The Total Health Score is comprised of a number of weighted Sub Health Scores.

Each health category keeps a Sub Health Status between 0 and 1. `Sub Health Score = Sub Health Status x Sub Health Weight`. The Sub Health Weights can be adjusted as desired.

The Sub Health Statuses track:
- Aggregate restarts per second across required processes for last 30 seconds. `Sub Health Status = 1 - Number of Restarts Per Second` with a status floor of zero.
- Aggregate average response time for required processes. Average less than or equal to heartbeat frequency considered a Sub Health Status of 1. Average greater than or equal to 5x heartbeat frequency considered a 0. Linear slope in between.
- Aggregate average memory usage. 2 percent or lower considered a Sub Health Status of 1. 10 percent or higher considered a 0. Linear slope in between.

When a controller sends a ModeQuery and gets the Other controller's Total Health Score in the response, it checks that score against a Total Health Threshold. If the Score falls below the Threshold, the controller takes over as primary and STONITHs the Other controller.

### External Examples
https://cloud.netapp.com/blog/azure-high-availability-basic-concepts-and-a-checklist
Define your availability metrics. These can include:
- Percentage of Uptime
- Mean Time to Recovery (MTTR)
- Mean Time between Failures (MTBR)
- Recovery Time Objective (RTO)
- Recovery Point Objective (RPO)

## Interface
Utilizes C2C communication interface to send totalHealthScore

## Testing
Unit tests for any new functions written that are unit testable

Manual tests:
- Manually kill a process and allow it to be restarted multiple times in a short period - verify primary takeover
- Manually kill a required process and keep it killed - verify primary takeover
- Temporarily make memory leak - verify primary takeover

## Backward Compatibility
Is backwards compatible

## Configuration
Add a boolean "totalHealthScoreWeight" field to each process