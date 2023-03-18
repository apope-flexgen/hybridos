/**
 *
 * types.go
 *
 * Any data types that are used throughout the code and are thus not owned by any one file.
 *
 */
package main

type editingMethod int

// Object editing methods.
//
// SET means to overwrite the entire old object with the new object.
//
// POST means to add the new object to the old object, editing the old object if there is overlap.
//
// DEL means to subtract the old object by the new object.
const (
	SET = iota
	POST
	DEL
)

type editingInterface int

// Entities that have editing abilities.
//
// CALLER: The editor is coming through a higher-up function to make multiple edits.
//
// INTERNAL: An internal process made an edit and now external entities should be notified.
//
// UI: HTTPS->FIMS.
//
// DBI: Database->FIMS.
//
// SCADA: Modbus->FIMS.
//
// FLEET-SITE: Direct HTTPS websocket.
//
// CONTROLLING_PROCESS: Any service editing over FIMS
//
// COPS: Raw TCP socket for redundant failover.
const (
	CALLER = iota
	INTERNAL
	UI
	DBI
	SCADA
	FLEET_SITE
	CONTROLLING_PROCESS
	COPS
)
