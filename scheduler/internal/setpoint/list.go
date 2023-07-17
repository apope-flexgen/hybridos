package setpoint

import "fmt"

type List []Setpoint

// Searches the list for a setpoint with the given ID and returns its index.
// If not found, returns -1.
func (list *List) Find(id string) (index int) {
	for i, sp := range *list {
		if id == sp.Id {
			return i
		}
	}
	return -1
}

// Validates every setpoint in the list. If the list is nil, sets it to an empty list.
func (list *List) Validate() error {
	if *list == nil {
		*list = make(List, 0)
		return nil
	}

	idSet := make(map[string]struct{})
	for i, sp := range *list {
		if _, ok := idSet[sp.Id]; ok {
			return fmt.Errorf("found duplicate ID %s", sp.Id)
		}
		idSet[sp.Id] = struct{}{}
		if err := sp.Validate(); err != nil {
			return fmt.Errorf("invalid setpoint in index %d: %w", i, err)
		}
	}
	return nil
}

// Adds a new setpoint to the list. Will return an error if there is already a setpoint with the same ID.
func (list *List) Append(newSetpoint Setpoint) error {
	for _, sp := range *list {
		if sp.Id == newSetpoint.Id {
			return fmt.Errorf("setpoint with ID %s already exists", newSetpoint.Id)
		}
	}
	*list = append(*list, newSetpoint)
	return nil
}

// Returns if diffList has any setpoints that are not in existingList based on ID.
func (existingList *List) IsMissingSetpointsThatAreIn(diffList List) bool {
	for _, sp := range diffList {
		if existingList.Find(sp.Id) == -1 {
			return true
		}
	}
	return false
}

// Returns a deep copy of the list.
func (list *List) Copy() List {
	cp := make(List, len(*list))
	copy(cp, *list)
	return cp
}

func (list *List) Delete(id string) {
	index := list.Find(id)
	if index < 0 {
		return
	}
	*list = append((*list)[:index], (*list)[index+1:]...)
}
