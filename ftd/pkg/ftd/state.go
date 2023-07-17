package ftd

import "sync/atomic"

// A value of 0 indicates primary, anything else is secondary, reads and writes to this variable must be atomic
var controllerState uint32

func ControllerStateIsPrimary() bool {
	return atomic.LoadUint32(&controllerState) == 0
}

func setControllerStatePrimary(setPrimary bool) {
	if setPrimary {
		atomic.StoreUint32(&controllerState, 0)
	} else {
		atomic.StoreUint32(&controllerState, 1)
	}
}
