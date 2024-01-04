package go_metrics

import (
	"fmt"
	"go/ast"
	"math"
	"regexp"
	"strconv"
	"strings"
	"time"
)

// data types represented with fims
// bool, int64, uint64, float64, string
type DataType uint8

const (
	NIL = iota
	BOOL
	INT
	UINT
	FLOAT
	STRING
)

func (dataType DataType) String() string {
	switch dataType {
	case STRING:
		return "string"
	case BOOL:
		return "bool"
	case INT:
		return "int"
	case UINT:
		return "uint"
	case FLOAT:
		return "float"
	default:
		return "nil"
	}
}

func (dataType *DataType) MarshalJSON() ([]byte, error) {
	switch *dataType {
	case STRING:
		return []byte("\"string\""), nil
	case BOOL:
		return []byte("\"bool\""), nil
	case INT:
		return []byte("\"int\""), nil
	case UINT:
		return []byte("\"uint\""), nil
	case FLOAT:
		return []byte("\"float\""), nil
	default:
		return []byte("\"nil\""), nil
	}
}

// the layout of the metrics file
type MetricsFile struct {
	Meta       map[string]interface{} `json:"meta"` // metadata for the file
	Templates  []Template             `json:"templates,omitempty"`
	Inputs     map[string]Input       `json:"inputs"`            // a map of variable names to the uri locations they will come from
	Attributes map[string]Attribute   `json:"-"`                 // an attribute can belong to an input or an output
	Filters    map[string]interface{} `json:"filters,omitempty"` // a map of variable names to apply 1) run-time changes to metrics or 2) execute regular expressions on variable names
	Outputs    map[string]Output      `json:"outputs"`           // a map of variable names that get sent to specific uris with optional flags
	Metrics    []MetricsObject        `json:"metrics"`           // an array of the calculations that we want to perform by default
	Echo       []EchoObject           `json:"echo"`              // an array of things to echo; basically the same config as the original echo
}

// metrics data object
type MDO struct {
	Meta    map[string]interface{}            `json:"meta,omitempty"`
	Inputs  map[string]map[string]interface{} `json:"inputs,omitempty"`
	Filters map[string][]string               `json:"filters,omitempty"`
	Outputs map[string]map[string]interface{} `json:"outputs,omitempty"`
	Metrics map[string]map[string]interface{} `json:"metrics,omitempty"`
	Echo    map[string]map[string]interface{} `json:"echo,omitempty"`
}

// To prevent using interfaces, we use a Union
// structure instead. It contains a "tag" that identifies
// what data type the struct value stores.
type Union struct {
	tag DataType
	b   bool
	i   int64
	ui  uint64
	f   float64
	s   string
}

// func (union Union) String() string {
// 	switch union.tag {
// 	case STRING:
// 		return union.s
// 	case BOOL:
// 		return fmt.Sprintf("%t", union.b)
// 	case INT:
// 		return fmt.Sprintf("%d", union.i)
// 	case UINT:
// 		return fmt.Sprintf("%d", uint64(union.ui))
// 	case FLOAT:
// 		return fmt.Sprintf("%f", union.f)
// 	default:
// 		return fmt.Sprintf("nil")
// 	}
// }

type Template struct {
	Type   string   `json:"type"`
	From   int64    `json:"from,omitempty"`
	To     int64    `json:"to,omitempty"`
	Step   int64    `json:"step,omitempty"`
	Format string   `json:"format,omitempty"`
	List   []string `json:"list,omitempty"`
	Tok    string   `json:"token"`
}

// the uris that we will look for in incoming fims data
type Input struct {
	Uri           string            `json:"uri,omitempty"`
	Internal      bool              `json:"internal,omitempty"`
	Type          string            `json:"type,omitempty"`
	Attributes    []string          `json:"attributes,omitempty"`
	AttributesMap map[string]string `json:"-"`
	Name          string            `json:"name,omitempty"`
	Value         Union             `json:"-"`
}

// the uris that we will publish to for outgoing fims data
type Output struct {
	Name          string                 `json:"name,omitempty"`
	Uri           string                 `json:"uri"`
	Flags         []string               `json:"flags,omitempty"`
	Value         Union                  `json:"-"`
	AttributesMap map[string]string      `json:"-"`
	Attributes    map[string]interface{} `json:"attributes,omitempty"`
	PublishRate   int64                  `json:"publishRate,omitempty"`
	EnumMap       map[int]int            `json:"-"`
	Enum          []EnumObject           `json:"enum,omitempty"`
	Bitfield      []EnumObject           `json:"bitfield,omitempty"`
}

type Attribute struct {
	Name      string
	InputVar  string
	OutputVar string
	Value     Union
}

type EnumObject struct {
	Value  int64  `json:"value"`
	String string `json:"string"`
}

// Expression is a mathematic expression that can be evaluated, given a scope
type Expression struct {
	String       string   // the expression string
	Vars         []string // the variable names in the expression
	Ast          ast.Node `json:"-"` // the root of the expression AST ("abstract syntax tree")
	ResultType   DataType // the result type of the expression
	IsRegex      bool     `json:"-"` // special type of expression
	IsTypeFilter bool     `json:"-"` // special type of expression
}

type Filter struct {
	StaticFilterExpressions  []Expression // the list of static filters to apply (in order)
	DynamicFilterExpressions []Expression // the list of dynamic filters to apply (in order)
	DynamicInputs            [][]string   // the list of inputs to the dynamic filter expressions
}

type MetricsObject struct {
	Id               string             `json:"id"`                        // the identifier for the metric; necessary for proper update tool functionality
	Type             DataType           `json:"type"`                      // the default value for an output - also specifies the data type
	Outputs          []string           `json:"outputs,omitempty"`         // the output variable to publish to (e.g. "output_1" which would have been mapped to an Output struct)
	InternalOutput   string             `json:"internal_output,omitempty"` // reuse the output value as an input variable (specified in inputs)
	Expression       string             `json:"expression"`                // a string that represents an expression to evaluate
	ParsedExpression Expression         `json:"-"`                         // an expression object after we've parsed everything
	State            map[string][]Union `json:"-"`                         // values created after runtime that are necessary for certain functions to evaluate
}

type EchoObject struct {
	PublishUri  string                 `json:"uri"`                 // the uri to publish to
	PublishRate int64                  `json:"publishRate"`         // the rate to publish at
	Heartbeat   string                 `json:"heartbeat,omitempty"` // still unsure what this does...I think it's just a name for the heartbeat var
	Format      string                 `json:"format,omitempty"`    // naked, clothed, sparse -- applies to ALL output values
	Inputs      []EchoInput            `json:"inputs,omitempty"`    // where to look for values
	Echo        map[string]interface{} `json:"echo,omitempty"`      // values to give to variables by default; overwritten if found in inputs (but only if a value comes in)
	Ticker      *time.Ticker           `json:"-"`                   //keeps track of publishes for this echo object
}

type EchoInput struct {
	Uri       string            `json:"uri"`       // the uri to look at
	Registers map[string]string `json:"registers"` // a map of new variable names (published to output uri) to old variable names (found in the input uri)
}

// given an arbitrary value (of unknown type), assign it
// to an appropriately tagged Union struct
func getUnionFromValue(value interface{}) Union {
	switch (value).(type) {
	case string:
		return Union{
			tag: STRING,
			s:   (value).(string),
		}
	case bool:
		return Union{
			tag: BOOL,
			b:   (value).(bool),
		}
	case uint64:
		return Union{
			tag: UINT,
			ui:  (value).(uint64),
		}
	case int64:
		return Union{
			tag: INT,
			i:   (value).(int64),
		}
	case float64:
		return Union{
			tag: FLOAT,
			f:   (value).(float64),
		}
	default:
		return Union{
			tag: NIL,
			s:   "",
		}
	}
}

// convert any Union value to its string representation
func unionValueToString(union *Union) string {
	switch union.tag {
	case STRING:
		return union.s
	case BOOL:
		return fmt.Sprintf("%t", union.b)
	case INT:
		return fmt.Sprintf("%d", union.i)
	case UINT:
		return fmt.Sprintf("%d", uint64(union.ui))
	case FLOAT:
		return fmt.Sprintf("%f", union.f)
	default:
		return "null"
	}
}

// cast a union from one time to another
func castUnionType(union *Union, newType DataType) error {
	if union.tag == newType {
		return nil
	} else if newType == NIL {
		*union = Union{}
	}
	switch union.tag {
	case STRING:
		var err error
		if newType == BOOL {
			var boolVal bool
			boolVal, err = strconv.ParseBool(union.s)
			if err != nil {
				boolVal = len(union.s) > 0
			}
			union.b = boolVal
		} else if newType == INT {
			var intVal int64
			intVal, err = strconv.ParseInt(union.s, 10, 64)
			if err != nil {
				return err
			}
			union.i = intVal
		} else if newType == UINT {
			var intVal uint64
			intVal, err = strconv.ParseUint(union.s, 10, 64)
			if err != nil {
				return err
			}
			union.ui = intVal
		} else if newType == FLOAT {
			var floatVal float64
			floatVal, err = strconv.ParseFloat(union.s, 64)
			if err != nil {
				return err
			}
			union.f = floatVal
		}
		union.s = ""
		union.tag = newType
	case INT:
		if newType == BOOL {
			union.tag = newType
			if union.i != 0 {
				union.b = true
			} else {
				union.b = false
			}
			union.i = 0
			return nil
		} else if newType == UINT {
			if union.i < 0 {
				return fmt.Errorf("cannot convert negative int64 to uint64")
			} else {
				union.tag = newType
				union.ui = uint64(union.i)
				union.i = 0
				return nil
			}
		} else if newType == FLOAT {
			union.tag = newType
			union.f = float64(union.i)
			union.i = 0
			return nil
		} else if newType == STRING {
			union.tag = STRING
			union.s = fmt.Sprintf("%d", union.i)
			union.i = 0
			return nil
		}
	case UINT:
		if newType == BOOL {
			union.tag = newType
			if union.ui != 0 {
				union.b = true
			} else {
				union.b = false
			}
			union.ui = 0
			return nil
		} else if newType == INT {
			if union.ui > math.MaxInt64 {
				return fmt.Errorf("uint64 value is too large to convert to int64")
			} else {
				union.tag = newType
				union.i = int64(union.ui)
				union.ui = 0
				return nil
			}
		} else if newType == FLOAT {
			union.tag = newType
			union.f = float64(union.ui)
			union.ui = 0
			return nil
		} else if newType == STRING {
			union.tag = STRING
			union.s = fmt.Sprintf("%d", union.ui)
			union.ui = 0
			return nil
		}
	case FLOAT:
		if newType == BOOL {
			union.tag = newType
			if union.f != 0 {
				union.b = true
			} else {
				union.b = false
			}
			union.f = 0.0
			return nil
		} else if newType == INT {
			if union.f >= math.MaxInt64 || union.f < math.MinInt64 {
				return fmt.Errorf("float64 value is too large to convert to int64")
			} else {
				union.tag = newType
				if union.f < 0 {
					union.f = -math.Floor(math.Abs(union.f))
					union.i = int64(union.f)
				} else {
					union.f = math.Floor(union.f)
					union.i = int64(union.f)
				}
				union.f = 0.0
				return nil
			}
		} else if newType == UINT {
			if union.f > math.MaxUint64 {
				return fmt.Errorf("float64 value is too large to convert to uint64")
			} else if union.f < 0 {
				return fmt.Errorf("cannot convert negative float64 to uint64")
			} else {
				union.tag = newType
				union.f = math.Floor(union.f)
				union.ui = uint64(union.f)
				union.f = 0.0
				return nil
			}
		} else if newType == STRING {
			union.tag = STRING
			union.s = fmt.Sprintf("%f", union.f)
			union.f = 0.0
			return nil
		}
	case BOOL:
		if newType == STRING {
			union.tag = STRING
			union.s = fmt.Sprintf("%t", union.b)
			union.b = false
			return nil
		} else if newType == FLOAT || newType == INT || newType == UINT {
			union.tag = newType
			if union.b {
				switch newType {
				case FLOAT:
					union.f = 1.0
				case INT:
					union.i = 1
				case UINT:
					union.ui = 1
				}
				union.b = false
			} else {
				switch newType {
				case FLOAT:
					union.f = 0.0
				case INT:
					union.i = 0
				case UINT:
					union.ui = 0
				}
			}
			return nil
		}
	default:
		union.i = 0
		union.ui = 0
		union.f = 0.0
		union.b = false
		union.s = ""
		union.tag = newType
	}
	return nil
}

// Get the result data type of a binary operation between two data
// types. The "precedence" of data types is as follows:
// STRING > FLOAT > INT > UINT > BOOL
// For example, INT + UINT --> INT
// For example, FLOAT + BOOL --> FLOAT
func getResultType(tag1, tag2 DataType) DataType {
	if tag1 == STRING || tag2 == STRING {
		return STRING
	} else if tag1 == FLOAT || tag2 == FLOAT {
		return FLOAT
	} else if tag1 == INT || tag2 == INT {
		return INT
	} else if tag1 == UINT || tag2 == UINT {
		return UINT
	} else if tag1 == BOOL || tag2 == BOOL {
		return BOOL
	} else {
		return NIL
	}
}

// given an arbitrary value, cast it to a Union of a specific data type
func castValueToUnionType(value interface{}, newType DataType) Union {
	switch newType {
	case STRING:
		var str string
		switch value.(type) {
		case string:
			str = fmt.Sprintf("%s", value)
		case bool:
			str = fmt.Sprintf("%t", value)
		case int64:
			str = fmt.Sprintf("%d", value)
		case int:
			str = fmt.Sprintf("%d", value)
		case uint64:
			str = fmt.Sprintf("%d", value)
		case float64:
			str = fmt.Sprintf("%f", value)
		default:
			str = ""
		}
		return Union{
			tag: STRING,
			s:   str,
		}
	case BOOL:
		switch x := (value).(type) {
		case string:
			if x == "false" {
				return Union{
					tag: newType,
					b:   false,
				}
			}
			return Union{
				tag: newType,
				b:   len(x) > 0,
			}
		case uint64:
			return Union{
				tag: newType,
				b:   x != 0,
			}
		case int64:
			return Union{
				tag: newType,
				b:   x != 0,
			}
		case int:
			return Union{
				tag: newType,
				b:   x != 0,
			}
		case float64:
			return Union{
				tag: newType,
				b:   x != 0,
			}
		case bool:
			return Union{
				tag: newType,
				b:   x,
			}
		default:
			return Union{tag: BOOL}
		}
	case INT:
		switch x := (value).(type) {
		case string:
			str, _ := strconv.ParseInt(x, 10, 64)
			return Union{
				tag: INT,
				i:   str,
			}
		case uint64:
			if x > uint64(math.MaxInt64) {
				return Union{
					tag: newType,
					i:   math.MaxInt64,
				}
			}
			return Union{
				tag: newType,
				i:   int64(x),
			}
		case int64:
			return Union{
				tag: newType,
				i:   int64(x),
			}
		case int:
			return Union{
				tag: newType,
				i:   int64(x),
			}
		case float64:
			if x > float64(math.MaxInt64) {
				return Union{
					tag: newType,
					i:   math.MaxInt64,
				}
			} else if x < float64(math.MinInt64) {
				return Union{
					tag: newType,
					i:   math.MinInt64,
				}
			}
			if x < 0 {
				return Union{
					tag: newType,
					i:   int64(-math.Floor(math.Abs(x))),
				}
			}
			return Union{
				tag: newType,
				i:   int64(math.Floor(x)),
			}
		case bool:
			if x {
				return Union{
					tag: newType,
					i:   1,
				}
			} else {
				return Union{
					tag: newType,
					i:   0,
				}
			}
		default:
			return Union{tag: INT}
		}
	case UINT:
		switch x := (value).(type) {
		case string:
			str, _ := strconv.ParseUint(x, 10, 64)
			return Union{
				tag: UINT,
				ui:  str,
			}
		case uint64:
			return Union{
				tag: newType,
				ui:  uint64(x),
			}
		case int64:
			if x < 0 {
				return Union{
					tag: newType,
					ui:  0,
				}
			}
			return Union{
				tag: newType,
				ui:  uint64(x),
			}
		case int:
			if x < 0 {
				return Union{
					tag: newType,
					ui:  0,
				}
			}
			return Union{
				tag: newType,
				ui:  uint64(x),
			}
		case float64:
			if x < 0 {
				return Union{
					tag: newType,
					ui:  0,
				}
			} else if x > math.MaxUint64 {
				return Union{
					tag: UINT,
					ui:  math.MaxUint64,
				}
			}
			return Union{
				tag: newType,
				ui:  uint64(math.Floor(x)),
			}
		case bool:
			if x {
				return Union{
					tag: newType,
					ui:  1,
				}
			} else {
				return Union{
					tag: newType,
					ui:  0,
				}
			}
		default:
			return Union{tag: UINT}
		}
	case FLOAT:
		switch x := (value).(type) {
		case string:
			str, _ := strconv.ParseFloat(x, 64)
			return Union{
				tag: FLOAT,
				f:   str,
			}
		case uint64:
			return Union{
				tag: newType,
				f:   float64(x),
			}
		case int64:
			return Union{
				tag: newType,
				f:   float64(x),
			}
		case int:
			return Union{
				tag: newType,
				f:   float64(x),
			}
		case float64:
			return Union{
				tag: newType,
				f:   float64(x),
			}
		case bool:
			if x {
				return Union{
					tag: newType,
					f:   1.0,
				}
			} else {
				return Union{
					tag: newType,
					f:   0.0,
				}
			}
		default:
			return Union{tag: FLOAT}
		}
	default:
		return Union{}
	}
}

func getValueFromUnion(union *Union) interface{} {
	switch (*union).tag {
	case STRING:
		return (*union).s
	case BOOL:
		return (*union).b
	case INT:
		return (*union).i
	case UINT:
		return (*union).ui
	case FLOAT:
		return (*union).f
	default:
		return nil
	}
}

// why does this not exist in the standard string library yet????
func stringInSlice(s []string, str string) bool {
	for _, v := range s {
		if v == str {
			return true
		}
	}
	return false
}

/*
 * Gets one level above for URI
 * returns original URI, if no parent
 */
func GetParentUri(str string) string {
	ind := strings.LastIndex(str, "/")
	if ind < 0 {
		if len(str) > 0 {
			return "/" + str
		}
		return str
	} else if ind == 0 {
		return str
	}
	if ind > 0 && strings.Index(str, "/") != 0 {
		return "/" + str[0:ind]
	}
	return str[0:ind]
}

/*
 * Gets one level above for URI
 * returns original URI, if no parent
 */
func GetUriElement(str string) string {
	ind := strings.LastIndex(str, "/")
	if ind < 0 {
		return str
	}
	return str[ind+1:]
}

//remove duplicate values for a string slice
func removeDuplicateValues(slice []string) []string {
	keys := make(map[string]bool)
	list := []string{}

	// If the key(values of the slice) is not equal
	// to the already present value in new slice (list)
	// then we append it. else we jump on another element.
	for _, entry := range slice {
		if _, value := keys[entry]; !value {
			keys[entry] = true
			list = append(list, entry)
		}
	}
	return list
}

func unionListsMatch(list1, list2 []Union) bool {
	if len(list1) != len(list2) {
		return false
	}
	for i, union1 := range list1 { // order matters
		if union1 != list2[i] {
			return false
		}
	}
	return true
}

func regexStringInSlice(s []string, regexString string) (string, bool) {
	for _, v := range s {
		if match, _ := regexp.MatchString(regexString, v); match {
			return v, true
		}
	}
	return "", false
}

func squashUris(uris []string) []string {
	uriOutput := []string{}
	uriLength := 1
	for {
		uris2 := make([]string, len(uris))
		copy(uris2, uris)
		i := 0
		for _, uri := range uris {
			uriUriFrags := strings.Split(uri, "/")
			if len(uriUriFrags) == uriLength {
				if uriLength > 1 && stringInSlice(uriOutput, strings.Join(uriUriFrags[0:uriLength-1], "/")) {
					if i < len(uris2)-1 {
						uris2 = append(uris2[:i], uris2[i+1:]...)
						i -= 1
					} else {
						uris2 = uris2[:i]
					}
				} else {
					uriOutput = append(uriOutput, uri)
					if i < len(uris2)-1 {
						uris2 = append(uris2[:i], uris2[i+1:]...)
						i -= 1
					} else {
						uris2 = uris2[:i]
					}
				}
			}
			i += 1
		}
		uris = uris2
		if len(uris) == 0 {
			break
		}
		uriLength += 1
	}
	return uriOutput
}
