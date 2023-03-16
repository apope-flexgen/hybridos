# Design Review

## Goals

As a site operator, I want hung processes to get automatically restarted.

## Affected Repos

cops, config, hybridos_controller, modbus_interface, any other repos that want to be checked by COPS

## Approach

### Heartbeat Reply Ticket
Each process has a single URI of the form `/<process name>/cops` that is a unique endpoint for FIMS messages. When a GET is sent to this URI, the process replies with a json containing not only its COPS heartbeat, but also its PID. Both of these values should be naked values regardless of how the process handles other FIMS messages. When a SET is sent to `/<process name>/cops/cops_heartbeat`, the process will read the naked value and update its heartbeat value. This strategy will require additional development in each repo that has a process to be checked by COPS.

### Kill Action Ticket
COPS will parse the latest PID associated with each process from the SET messages. COPS will use the latest PID to update its process pointer for that process only if the read PID is different than the stored PID for that process. Use `func FindProcess(pid int) (*Process, error)` from the `os` package and pass in a new PID to update the process pointer.

If a process is not responding, COPS executes a `kill` command as a part of its failure actions so that services can restart the failed process. To kill the process, pass the process pointer to `func (p *Process) Kill() error` which is also from the `os` package.

References:
https://golang.org/pkg/os/#Process

## Interface

COPS interfaces with other repos via FIMS
This design will have COPS interface with the system OS to kill other processes

## Testing

Testing will require hanging processes without killing them. For hybridos_controller, this is accomplished by adding a FIMS hook at the URI `/site/debug/hang`. When a SET is sent to this URI, Site Controller will enter an infinite loop and hang. This should cause the COPS heartbeat check to eventually fail. COPS should kill hybridos_controller, and services or MCP (MCP for twins testing) should start it back up. Something similar could be done to test other processes.

## Backward Compatibility

This will not be compatible with older versions of COPS or Site Controller without updating the necessary config files.

## Configuration

The below json is an example of how COPS would be configured with this new feature. Each process must specify with `killOnHang` whether or not it should be killed if detected as hung.

```json
{
    "ourUri": "/cops",
    "heartbeatFreq": 1000,
    "replyToGetDeadline": 900,
    "heartbeatErrorAllowance": 5,
    "processList": [
        {
            "name": "hybridos_controller",
            "uri": "/site",
            "killOnHang": true
        },
        {
            "name": "modbus_client",
            "uri": "/components/modbus_test_client",
            "killOnHang": true
        }
    ]
}