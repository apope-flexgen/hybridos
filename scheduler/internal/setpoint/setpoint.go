package setpoint

import (
	"fims"
	"fmt"
	"strings"

	"github.com/flexgen-power/go_flexgen/parsemap"
)

type Setpoint struct {
	Id      string      `json:"id"`
	Name    string      `json:"name"`
	Unit    string      `json:"unit"`
	Uri     string      `json:"uri"`
	VarType string      `json:"type"`
	Value   interface{} `json:"value"`
}

// Ensures the given type string is a valid option and that the value matches the configured type.
// Performs type casting for integers.
func (sp *Setpoint) Validate() error {
	switch sp.VarType {
	case "Float":
		if _, ok := sp.Value.(float64); !ok {
			return fmt.Errorf("type is %s but received a %T", sp.VarType, sp.Value)
		}
	case "Int":
		valInt, err := parsemap.CastToInt(sp.Value)
		if err != nil {
			return fmt.Errorf("type is Int but failed to cast received %T value to int: %w", sp.Value, err)
		}
		sp.Value = valInt
	case "Bool":
		if _, ok := sp.Value.(bool); !ok {
			return fmt.Errorf("type is %s but received a %T", sp.VarType, sp.Value)
		}
	case "String":
		if _, ok := sp.Value.(string); !ok {
			return fmt.Errorf("type is %s but received a %T", sp.VarType, sp.Value)
		}
	default:
		return fmt.Errorf("invalid variable type %s", sp.VarType)
	}
	return nil
}

// Sends a value to the setpoint's URI via a FIMS SET.
// Uses the passed-in parameter instead of the Value setpoint field,
// because the Value setpoint field is only necessarily valid for
// constant setpoints, not variable setpoints.
func (sp *Setpoint) SendSet(f *fims.Fims, val interface{}, clothed bool) {
	// if scheduler events are disabled, do not send setpoints
	if f == nil {
		return
	}
	if clothed {
		val = map[string]interface{}{"value": val}
	}
	f.SendSet(sp.Uri, "", val)

}

// Creates an ID for the mode using its name.
func (sp *Setpoint) GenerateId() {
	sp.Id = strings.ToLower(sp.Name)
	sp.Id = strings.ReplaceAll(sp.Id, " ", "_")
	sp.Id = strings.ReplaceAll(sp.Id, "/", "_")
}
