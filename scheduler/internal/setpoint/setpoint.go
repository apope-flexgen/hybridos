package setpoint

import (
	"fims"
	"fmt"
	"sort"
	"strconv"
	"strings"

	"github.com/flexgen-power/go_flexgen/parsemap"
)

type Setpoint struct {
	Id          string        `json:"id"`
	Name        string        `json:"name"`
	Unit        string        `json:"unit"`
	Uri         string        `json:"uri"`
	VarType     string        `json:"type"`
	Value       interface{}   `json:"value"`
	IsTemplate  bool          `json:"is_template"`
	BatchPrefix string        `json:"batch_prefix"`
	BatchRange  []interface{} `json:"batch_range"`
	BatchValue  []interface{} `json:"batch_value"`
}

// Ensures that all the templating variables are present and make sense
func (sp *Setpoint) checkForTemplatedVariables() error {
	if sp.BatchPrefix == "" || sp.BatchValue == nil || sp.BatchRange == nil || !sp.IsTemplate {
		return fmt.Errorf("not all templated variables provided when at least one was present and non nil")
	}
	if len(sp.BatchValue) == 0 || len(sp.BatchRange) == 0 {
		return fmt.Errorf("batch_value or batch_range is an empty array while using a batched setpoint. invalid setpoint")
	}

	return nil
}

// Validates a variable with ".." syntax
// ie. BatchRange and BatchPrefix
func validateRange(rangedVar string) ([]int, error) {
	if !strings.Contains(rangedVar, "..") {
		return nil, fmt.Errorf("rangedVar did not contain \"..\"")
	}
	splitStrings := strings.Split(rangedVar, "..")
	if len(splitStrings) != 2 {
		return nil, fmt.Errorf("batch support only functions for string with format '\"<num>..<num>\"' but this string has multiple instances of \"..\"")
	}
	if strings.Contains(splitStrings[0], ".") || strings.Contains(splitStrings[1], ".") {
		return nil, fmt.Errorf("decimal found in ranged value only ints supported. Values were \"%s\" and \"%s\". Did you not use this format '\"<num>..<num>\"'", splitStrings[0], splitStrings[1])
	}
	var toInt []int
	for _, str := range splitStrings {
		asInt, err := strconv.Atoi(str)
		if err != nil {
			return nil, fmt.Errorf("failed to parse int from string: %w", err)
		}
		toInt = append(toInt, asInt)
	}
	if toInt[0] >= toInt[1] {
		sort.Ints(toInt)
	}
	return toInt, nil
}

// Vibe checks for templating configuration
func (sp *Setpoint) validateTemplatingIfPresent() error {
	if sp.IsTemplate {
		if err := sp.checkForTemplatedVariables(); err != nil {
			return fmt.Errorf("attempting to vaidate setpoint when IsTemplate was true when: %w", err)
		}
	}
	if sp.BatchPrefix != "" {
		// Technically allowed
		if strings.Contains(sp.BatchPrefix, "#") {
		}
	}
	if sp.BatchRange != nil {
		if err := sp.checkForTemplatedVariables(); err != nil {
			return fmt.Errorf("attempting to vaidate setpoint when BatchRange is not nil, but : %w", err)
		}

		for _, inter := range sp.BatchRange {
			switch inter.(type) {
			case int, int8, int16, int32, int64, float32, float64:
				// do nothing we are happy
			case string:
				// make sure the string parses to something useful
				if _, err := validateRange(inter.(string)); err != nil {
					return fmt.Errorf("failed to parse string into legible range. String was: \"%s\". Err: %w", inter.(string), err)
				}
			default:
				return fmt.Errorf("unsupported type found in batched variable. Type was %T", inter)
			}
		}
	}
	if sp.BatchValue != nil {
		if err := sp.checkForTemplatedVariables(); err != nil {
			return fmt.Errorf("attempting to vaidate setpoint when BatchValue is not nil, but : %w", err)
		}

		for _, inter := range sp.BatchRange {
			switch inter.(type) {
			case int, int8, int16, int32, int64, float32, float64:
				// do nothing we are happy
			case string:
				// make sure the string parses to something useful
				if _, err := validateRange(inter.(string)); err != nil {
					return fmt.Errorf("failed to parse string into legible range. String was: \"%s\". Err: %w", inter.(string), err)
				}
			default:
				return fmt.Errorf("unsupported type found in batched variable. Type was %T", inter)
			}
		}
	}
	return nil
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

	return sp.validateTemplatingIfPresent()
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

// Validates and converts batch value to a list of ints.
// Batch value can contain numeric values or strings of the
// specific format "<some_num>..<some_other_num>"
func batchValueToIntArray(batch []interface{}) ([]int, error) {
	var assetNums []int
	if len(batch) == 0 {
		return nil, fmt.Errorf("error in batch_value_to_int_array: batch was empty and therefore nothing useful occured")
	}
	for _, elem := range batch {
		switch elem.(type) {
		case int, int8, int16, int32, int64:
			assetNums = append(assetNums, elem.(int))
		case float32:
			assetNums = append(assetNums, int(elem.(float32)))
		case float64:
			assetNums = append(assetNums, int(elem.(float64)))
		case string:
			asInts, err := validateRange(elem.(string))
			if err != nil {
				return nil, fmt.Errorf("error when validating range during conversion to int slice: err: %w", err)
			}

			for asInts[0] <= asInts[1] {
				assetNums = append(assetNums, asInts[0])
				asInts[0] = asInts[0] + 1
			}
		default:
			return nil, fmt.Errorf("unexpected type in BatchValue mixed slice. Type was %T", elem)
		}
	}
	return assetNums, nil
}

// Takes in a number and converts the setpoint prefix and uri to a
// finalized_uri with templating replaced by said number with correct
// 0 padding
func (sp *Setpoint) generateTemplatedUri(num int) (string, error) {
	if strings.Contains(sp.Uri, "#") {
		// Get the number of # which translates to how many zeros to pad
		poundIt := strings.Index(sp.Uri, "#")
		poundCt := 0
		for sp.Uri[poundIt] == '#' {
			poundCt = poundCt + 1
			poundIt = poundIt + 1
		}
		format := "%0" + strconv.Itoa(poundCt) + "d"
		paddedNum := fmt.Sprintf(format, num)
		finalizedURI := sp.BatchPrefix + strings.Replace(sp.Uri, "#", paddedNum, 1)
		return finalizedURI, nil
	}
	return "", fmt.Errorf("\"BatchValue\" is not a []int. Rejecting templated variable")
}

// The templated version of SendSet. Will take BatchPrefix + Uri
// substituting in the numbers garnered from BatchValue
func (sp *Setpoint) SendTemplatedSet(f *fims.Fims, val interface{}, clothed bool) error {
	// If scheduler events are disabled, do not send setpoints
	if f == nil {
		return nil
	}

	// If a variable is templated then it can be a naked value or an object (ex. "start": true)
	// If it's an object then it must contain a "batch_value" key and a "value" key (ex. "start": {"value": true, "batch_value":[1,2])
	// When naked use the modes default "batch_value" configured during mode creation
	// When an object, override the aforementioned "batch_value"
	switch v := val.(type) {
	case map[string]interface{}:
		value, valOk := v["value"]
		batchVal, batchValOk := v["batch_value"]
		if !valOk || !batchValOk {
			return fmt.Errorf("could not parse \"value\" or \"batch_value\" from templated variable")
		}
		if _, ok := batchVal.([]interface{}); !ok {
			return fmt.Errorf("did not provide []interface{} for batch_value")
		}
		assetNums, err := batchValueToIntArray(batchVal.([]interface{}))
		if err != nil {
			return fmt.Errorf("error checking batchVal type is []interface{}: %w", err)
		}
		for _, num := range assetNums {
			finalizedURI, err := sp.generateTemplatedUri(num)
			if err != nil {
				return fmt.Errorf("error when generating templated uri: %w", err)
			}

			if clothed {
				value = map[string]interface{}{"value": value}
			}

			f.SendSet(finalizedURI, "", value)
		}
	case int, int8, int16, int32, int64, float32, float64, string, bool:
		assetNums, err := batchValueToIntArray(sp.BatchValue)
		if err != nil {
			return fmt.Errorf("error while converting batch_value to int array: %w", err)
		}
		for _, num := range assetNums {
			finalizedURI, err := sp.generateTemplatedUri(num)
			if err != nil {
				return fmt.Errorf("error while generating templated uri: %w", err)
			}
			if clothed {
				val = map[string]interface{}{"value": val}
			}

			f.SendSet(finalizedURI, "", val)
		}
	default:
		return fmt.Errorf("unable to determine value type when executing event. Problematic variable value %w", val)
	}
	return nil
}

// Creates an ID for the mode using its name.
func (sp *Setpoint) GenerateId() {
	sp.Id = strings.ToLower(sp.Name)
	sp.Id = strings.ReplaceAll(sp.Id, " ", "_")
	sp.Id = strings.ReplaceAll(sp.Id, "/", "_")
}
