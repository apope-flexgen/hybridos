package go_metrics

import (
	"bytes"
	"encoding/json"
	"fmt"
	"math"
	"os"
	"reflect"
	"regexp"
	"strings"
	"sync"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	simdjson "github.com/minio/simdjson-go"
)

type JsonAccessor struct {
	Key   string
	JType simdjson.Type
	Index int
}

func (e JsonAccessor) String() string {
	if len(e.Key) > 0 {
		return e.Key
	} else {
		return fmt.Sprintf("%d", e.Index)
	}
}

type ErrorLocation struct {
	JsonLocation []JsonAccessor
	JsonError    string
}

func (e ErrorLocation) String() string {
	str := "    {\n        \"Location\": ["
	if len(e.JsonLocation) > 0 {
		for _, accessor := range e.JsonLocation[0 : len(e.JsonLocation)-1] {
			str += fmt.Sprintf("\"%v\", ", accessor)
		}
		str += fmt.Sprintf("\"%v\"],\n        \"Error\": \"%v\"\n    }", e.JsonLocation[len(e.JsonLocation)-1], e.JsonError)
	} else {
		str += fmt.Sprintf("],\n        \"Error\": \"%v\"\n    }", e.JsonError)
	}
	return str
}

type ErrorLocations struct {
	ErrorLocs           []ErrorLocation
	currentJsonLocation []JsonAccessor
}

func (e ErrorLocations) String() string {
	allErrors := "\n"
	for _, err := range e.ErrorLocs {
		str := ""
		for _, accessor := range err.JsonLocation {
			if len(str) > 0 {
				str += " > "
			}
			switch accessor.JType {
			case simdjson.TypeArray:
				if len(accessor.Key) > 0 {
					str += fmt.Sprintf("[] %s", accessor.Key)
				} else {
					str += fmt.Sprintf("[] %d", accessor.Index)
				}
			case simdjson.TypeObject:
				if len(accessor.Key) > 0 {
					str += fmt.Sprintf("{} %s", accessor.Key)
				} else {
					str += fmt.Sprintf("{} %d", accessor.Index)
				}
			default:
				if len(accessor.Key) > 0 {
					str += accessor.Key
				} else {
					str += fmt.Sprintf("%d", accessor.Index)
				}
			}
		}
		str += fmt.Sprintf(": %v\n", err.JsonError)
		allErrors += str
	}

	return allErrors
}

func (e *ErrorLocations) initialize() {
	e.currentJsonLocation = make([]JsonAccessor, 0)
	e.ErrorLocs = make([]ErrorLocation, 0)
}

func (e *ErrorLocations) addKey(id string, simdjType simdjson.Type) {
	e.currentJsonLocation = append(e.currentJsonLocation, JsonAccessor{Key: id, JType: simdjType})
}

func (e *ErrorLocations) addIndex(index int, simdjType simdjson.Type) {
	e.currentJsonLocation = append(e.currentJsonLocation, JsonAccessor{Index: index, JType: simdjType})
}

func (e *ErrorLocations) removeLevel() {
	index := len(e.currentJsonLocation) - 1
	if index >= 0 && len(e.currentJsonLocation)-1 >= index {
		e.currentJsonLocation = append(e.currentJsonLocation[:index], e.currentJsonLocation[index+1:]...)
	}
}

func (e *ErrorLocations) logError(err error) {
	errStr := fmt.Sprintf("%v", err) // Store error message once since it's used multiple times
	if errStr == "path not found" && len(e.currentJsonLocation) > 0 {
		errStr = strings.Replace(errStr, "path", fmt.Sprintf("key '%v'", e.currentJsonLocation[len(e.currentJsonLocation)-1]), -1)
		errStr = strings.ReplaceAll(errStr, "\"", "\\\"")
		newError := ErrorLocation{JsonError: errStr, JsonLocation: make([]JsonAccessor, len(e.currentJsonLocation)-1)}
		copy(newError.JsonLocation, e.currentJsonLocation)
		e.ErrorLocs = append(e.ErrorLocs, newError)
	} else if strings.Contains(errStr, "value is not") {
		errStr = strings.Replace(errStr, "value is not", "expected value to be", -1)
		errStr = strings.ReplaceAll(errStr, "\"", "\\\"")
		newError := ErrorLocation{JsonError: errStr, JsonLocation: make([]JsonAccessor, len(e.currentJsonLocation))}
		copy(newError.JsonLocation, e.currentJsonLocation)
		e.ErrorLocs = append(e.ErrorLocs, newError)
	} else {
		errStr = strings.ReplaceAll(errStr, "\"", "\\\"")
		newError := ErrorLocation{JsonError: errStr, JsonLocation: make([]JsonAccessor, len(e.currentJsonLocation))}
		copy(newError.JsonLocation, e.currentJsonLocation)
		e.ErrorLocs = append(e.ErrorLocs, newError)
	}
}

func initializeMetricsConfig() {
	if MetricsConfig.Meta == nil {
		MetricsConfig.Meta = make(map[string]interface{}, 0)
	}

	MetricsConfig.Templates = make([]Template, 0)

	if MetricsConfig.Inputs == nil {
		MetricsConfig.Inputs = make(map[string]Input, 0)
	}
	if MetricsConfig.Attributes == nil {
		MetricsConfig.Attributes = make(map[string]Attribute, 0)
	}
	if MetricsConfig.Filters == nil {
		MetricsConfig.Filters = make(map[string]interface{}, 0)
	}
	if MetricsConfig.Outputs == nil {
		MetricsConfig.Outputs = make(map[string]Output, 0)
	}
	if MetricsConfig.Metrics == nil {
		MetricsConfig.Metrics = make([]MetricsObject, 0)
	}
	if MetricsConfig.Echo == nil {
		MetricsConfig.Echo = make([]EchoObject, 0)
	}
}

func parseMeta(i *simdjson.Iter) (tempMeta map[string]interface{}, wasWarning bool, wasError bool) {
	// handle meta data (map[string]interface{}; optional)
	element, internal_err := i.FindElement(nil, "meta")
	if internal_err == nil {
		configErrorLocs.addKey("meta", simdjson.TypeObject)
		metaObject, internal_err := element.Iter.Object(nil)
		if internal_err != nil {
			configErrorLocs.logError(internal_err)
			wasError = true
		} else {
			tempMeta, internal_err = metaObject.Map(tempMeta)
			if internal_err != nil { // not sure if we can technically get here...
				configErrorLocs.logError(internal_err)
				wasError = true
			} else {
				for key, value := range tempMeta {
					if _, ok := MetricsConfig.Meta[key]; !ok {
						MetricsConfig.Meta[key] = value
					} else if !stringInSlice([]string{"debug", "debug_inputs", "debug_outputs", "debug_filters"}, key) {
						configErrorLocs.logError(fmt.Errorf("received duplicate field '%s' in \"meta\" object of incoming config; defaulting to original value", key))
						wasWarning = true
					}
				}
			}
		}
		configErrorLocs.removeLevel()
	}
	return
}

var configErrorLocs ErrorLocations

// Handle configuration processing based on the raw bytes received from file/dbi on startup or over fims
// data: the configuration data to process
// uniqueId: the uniqueId to use if there are collisions/duplicate values in the configuration
//
//	if empty, no collisions are allowed, else the unique id will help prevent collisions, and any that occur can safely use the latest value
//
// Return NewMetricsInfo struct containing updated values, as well as any warnings or errors that occurred
func UnmarshalConfig(data []byte, configId string) (NewMetricsInfo, bool, bool) {
	metricsConfigMutex.Lock()
	overwrite_conflicting_configs := false
	if len(configId) > 0 {
		overwrite_conflicting_configs = true
		// Use underscores in place of dashes throughout
		configId = strings.ReplaceAll(configId, "-", "_")
	}
	configErrorLocs.initialize()
	initializeMetricsConfig()
	new_inputs := []string{}
	new_filters := []string{}
	new_filters_starting_index := len(FiltersList)
	new_outputs := []string{}
	new_metrics_starting_index := len(MetricsConfig.Metrics)
	new_echo_starting_index := len(MetricsConfig.Echo)

	pj, err := simdjson.Parse(data, nil)
	if err != nil {
		log.Fatalf("%v", err)
	}

	wasWarning := false
	wasError := false
	// this is where the main config processing happens
	pj.ForEach(func(i simdjson.Iter) error {
		if i.Type() != simdjson.TypeObject {
			log.Fatalf("Unexpected JSON format for config. Must be a json object containing meta data, inputs, filters (optional), outputs, and expressions.")
		}

		tempMeta, tempWasWarning, tempWasError := parseMeta(&i)
		wasWarning = wasWarning || tempWasWarning
		wasError = wasError || tempWasError
		tempWasWarning, tempWasError = setupConfigLogging(tempMeta, configId)
		wasWarning = wasWarning || tempWasWarning
		wasError = wasError || tempWasError

		// handle templating ([]Template; optional)
		element, internal_err := i.FindElement(nil, "templates")
		if internal_err == nil {
			MetricsConfig.Templates = make([]Template, 0)
			configErrorLocs.addKey("templates", simdjson.TypeArray)
			templateArray, internal_err := element.Iter.Array(nil)
			if internal_err != nil {
				configErrorLocs.logError(internal_err)
				wasError = true
			} else {
				// for each template object
				templateIndex := 0
				templateArray.ForEach(func(templateIter simdjson.Iter) {
					configErrorLocs.addIndex(templateIndex, simdjson.TypeObject)
					template := Template{}
					_, internal_err = templateIter.Object(nil)
					if internal_err != nil {
						configErrorLocs.logError(internal_err)
						wasError = true
						configErrorLocs.removeLevel() // remove template index
						templateIndex += 1
						return
					} else { // handle the template object
						// get the value
						element, internal_err = templateIter.FindElement(nil, "type")
						configErrorLocs.addKey("type", simdjson.TypeString)
						if internal_err != nil {
							configErrorLocs.removeLevel() // remove "type"
							// check if there's a "from" specifier, which indicates sequential templating
							element, internal_err = templateIter.FindElement(nil, "from")
							if internal_err != nil {
								// check if there's a "list" specifier, which indicates list templating
								element, internal_err = templateIter.FindElement(nil, "list")
								if internal_err != nil {
									configErrorLocs.logError(fmt.Errorf("could not identify template type; need either from/to pair or list"))
									wasError = true
									configErrorLocs.removeLevel() // remove template index
									templateIndex += 1
									return
								} else {
									configErrorLocs.addKey("list", simdjson.TypeArray)
									var listArray *simdjson.Array
									listArray, internal_err = element.Iter.Array(nil)
									if internal_err != nil {
										configErrorLocs.logError(internal_err)
										wasError = true
										configErrorLocs.removeLevel() // remove list
										configErrorLocs.removeLevel() // remove template index
										templateIndex += 1
										return
									} else {
										template.List = make([]string, 0)
										listArray.ForEach(func(listIter simdjson.Iter) {
											var item interface{}
											item, internal_err = listIter.Interface()
											if internal_err == nil {
												template.List = append(template.List, fmt.Sprintf("%v", item))
											}
										})
									}
									configErrorLocs.removeLevel()
								}
							} else {
								configErrorLocs.addKey("from", simdjson.TypeInt)
								template.From, internal_err = element.Iter.Int()
								if internal_err != nil {
									configErrorLocs.logError(internal_err)
									wasError = true
									configErrorLocs.removeLevel() // remove "from"
									configErrorLocs.removeLevel() // remove template index
									templateIndex += 1
									return
								} else {
									configErrorLocs.removeLevel() // remove "from"
									element, internal_err = templateIter.FindElement(nil, "to")
									configErrorLocs.addKey("to", simdjson.TypeInt)
									if internal_err != nil {
										configErrorLocs.logError(internal_err)
										wasError = true
										configErrorLocs.removeLevel() // remove "to"
										configErrorLocs.removeLevel() // remove template index
										templateIndex += 1
										return
									} else {
										template.To, internal_err = element.Iter.Int()
										if internal_err != nil {
											configErrorLocs.logError(internal_err)
											wasError = true
											configErrorLocs.removeLevel() // remove "to"
											configErrorLocs.removeLevel() // remove template index
											templateIndex += 1
											return
										} else {
											configErrorLocs.removeLevel() // remove "to"
											element, internal_err = templateIter.FindElement(nil, "step")
											configErrorLocs.addKey("step", simdjson.TypeInt)
											if internal_err == nil {
												template.Step, internal_err = element.Iter.Int()
												if internal_err != nil {
													configErrorLocs.logError(internal_err)
													wasError = true
													configErrorLocs.removeLevel() // remove "step"
													configErrorLocs.removeLevel() // remove template index
													templateIndex += 1
													return
												} else if template.Step == 0 {
													configErrorLocs.logError(fmt.Errorf("cannot have template step of 0; defaulting to a step of 1"))
													wasError = true
													template.Step = 1
												}
											} else {
												template.Step = 1
											}
											configErrorLocs.removeLevel() // remove "step"
											element, internal_err = templateIter.FindElement(nil, "format")
											configErrorLocs.addKey("format", simdjson.TypeString)
											if internal_err == nil {
												template.Format, internal_err = element.Iter.StringCvt()
												if internal_err != nil {
													configErrorLocs.logError(fmt.Errorf("invalid format specifier %s; defaulting to %%d", template.Format))
													wasError = true
													template.Format = "%d"
												}
												format_specifiers := []string{"%d", `%(0??)(\d+)d`, "%c", "%U"}
												valid := false
												for _, format_specifier := range format_specifiers {
													if match, _ := regexp.MatchString(format_specifier, template.Format); match {
														valid = true
														break
													}
												}

												if !valid {
													configErrorLocs.logError(fmt.Errorf("invalid format specifier %s; defaulting to %%d", template.Format))
													wasError = true
													template.Format = "%d"
												}
											} else {
												template.Format = "%d"
											}
											configErrorLocs.removeLevel() // remove "format"
											template.List = make([]string, 0)
											if (template.From < template.To && template.Step > 0) || (template.From > template.To && template.Step < 0) {
												for p := template.From; p <= template.To; p += template.Step {
													template.List = append(template.List, fmt.Sprintf(template.Format, p))
												}
											}
										}
									}

								}
							}
						} else {
							template.Type, internal_err = element.Iter.StringCvt()
							if internal_err != nil {
								configErrorLocs.logError(internal_err)
								wasError = true
								configErrorLocs.removeLevel() // remove type
								configErrorLocs.removeLevel() // remove template index
								templateIndex += 1
								return
							} else {
								if template.Type == "sequential" {
									element, internal_err = templateIter.FindElement(nil, "from")
									configErrorLocs.removeLevel() // remove "type"
									configErrorLocs.addKey("from", simdjson.TypeInt)
									if internal_err != nil {
										configErrorLocs.logError(internal_err)
										wasError = true
										configErrorLocs.removeLevel() // remove "from"
										configErrorLocs.removeLevel() // remove template index
										templateIndex += 1
										return
									} else {
										template.From, internal_err = element.Iter.Int()
										if internal_err != nil {
											configErrorLocs.logError(internal_err)
											wasError = true
											configErrorLocs.removeLevel() // remove "from"
											configErrorLocs.removeLevel() // remove template index
											templateIndex += 1
											return
										} else {
											element, internal_err = templateIter.FindElement(nil, "to")
											configErrorLocs.removeLevel() // remove "from"
											configErrorLocs.addKey("to", simdjson.TypeInt)
											if internal_err != nil {
												configErrorLocs.logError(internal_err)
												wasError = true
												configErrorLocs.removeLevel() // remove "to"
												configErrorLocs.removeLevel() // remove template index
												templateIndex += 1
												return
											} else {
												template.To, internal_err = element.Iter.Int()
												if internal_err != nil {
													configErrorLocs.logError(internal_err)
													wasError = true
													configErrorLocs.removeLevel() // remove "to"
													configErrorLocs.removeLevel() // remove template index
													templateIndex += 1
													return
												} else {
													element, internal_err = templateIter.FindElement(nil, "step")
													configErrorLocs.removeLevel() // remove "to"
													configErrorLocs.addKey("step", simdjson.TypeInt)
													if internal_err == nil {
														template.Step, internal_err = element.Iter.Int()
														if internal_err != nil {
															configErrorLocs.logError(internal_err)
															wasError = true
															template.Step = 1
														} else if template.Step == 0 {
															configErrorLocs.logError(fmt.Errorf("cannot have template step of 0; defaulting to a step of 1"))
															wasError = true
															template.Step = 1
														}
													} else {
														template.Step = 1
													}
													configErrorLocs.removeLevel() // remove "step"
													element, internal_err = templateIter.FindElement(nil, "format")
													configErrorLocs.addKey("format", simdjson.TypeString)
													if internal_err == nil {
														template.Format, internal_err = element.Iter.StringCvt()
														if internal_err != nil {
															configErrorLocs.logError(fmt.Errorf("invalid format specifier %s; defaulting to %%d", template.Format))
															wasError = true
															template.Format = "%d"
														}
														format_specifiers := []string{"%d", `%(0??)(\d+)d`, "%c", "%U"}
														valid := false
														for _, format_specifier := range format_specifiers {
															if match, _ := regexp.MatchString(format_specifier, template.Format); match {
																valid = true
																break
															}
														}

														if !valid {
															configErrorLocs.logError(fmt.Errorf("invalid format specifier %s; defaulting to %%d", template.Format))
															wasError = true
															template.Format = "%d"
														}
													} else {
														template.Format = "%d"
													}
													configErrorLocs.removeLevel() // remove "format"
													template.List = make([]string, 0)
													if (template.From < template.To && template.Step > 0) || (template.From > template.To && template.Step < 0) {
														for p := template.From; p <= template.To; p += template.Step {
															template.List = append(template.List, fmt.Sprintf(template.Format, p))
														}
													}
												}
											}
										}
									}
								} else if template.Type == "list" {
									element, internal_err = templateIter.FindElement(nil, "list")
									configErrorLocs.removeLevel() // remove "type"
									configErrorLocs.addKey("list", simdjson.TypeArray)
									if internal_err != nil {
										configErrorLocs.logError(internal_err)
										wasError = true
										configErrorLocs.removeLevel() // remove "from"
										configErrorLocs.removeLevel() // remove template index
										templateIndex += 1
										return
									} else {
										var listArray *simdjson.Array
										listArray, internal_err = element.Iter.Array(nil)
										if internal_err != nil {
											configErrorLocs.logError(internal_err)
											wasError = true
										} else {
											template.List = make([]string, 0)
											listArray.ForEach(func(listIter simdjson.Iter) {
												var item interface{}
												item, internal_err = listIter.Interface()
												if internal_err == nil {
													template.List = append(template.List, fmt.Sprintf("%v", item))
												}
											})
										}
									}
									configErrorLocs.removeLevel() // remove "list"
								} else {
									configErrorLocs.logError(fmt.Errorf("unexpected template type %v: need \"sequential\" or \"list\"", template.Type))
									configErrorLocs.removeLevel() // remove "type"
									configErrorLocs.removeLevel() // remove template index
									templateIndex += 1
									return
								}
							}
						}

						// get the token (required)
						element, internal_err = templateIter.FindElement(nil, "token")
						configErrorLocs.addKey("token", simdjson.TypeString)
						if internal_err != nil {
							configErrorLocs.logError(internal_err)
							wasError = true
							configErrorLocs.removeLevel() // remove "token"
							configErrorLocs.removeLevel() // remove template index
							templateIndex += 1
							return
						} else {
							template.Tok, internal_err = element.Iter.String()
							if internal_err != nil {
								configErrorLocs.logError(internal_err)
								wasError = true
								configErrorLocs.removeLevel() // remove "token"
								configErrorLocs.removeLevel() // remove template index
								templateIndex += 1
								return
							} else if strings.Contains(template.Tok, "@") {
								configErrorLocs.logError(fmt.Errorf("template tokens cannot contain '@' symbol; symbol is reserved for attributes"))
								wasError = true
								configErrorLocs.removeLevel() // remove "token"
								configErrorLocs.removeLevel() // remove template index
								templateIndex += 1
								return
							} else if len(template.Tok) == 0 {
								configErrorLocs.logError(fmt.Errorf("template tokens cannot be empty strings"))
								wasError = true
								configErrorLocs.removeLevel() // remove "token"
								configErrorLocs.removeLevel() // remove template index
								templateIndex += 1
								return
							} else {
								for other_template_index, other_template := range MetricsConfig.Templates {
									if strings.Contains(other_template.Tok, template.Tok) {
										configErrorLocs.logError(fmt.Errorf("template %v contains template %v's token in its entirety; note that neither template may behave as desired", other_template_index, templateIndex))
										wasError = true
									} else if strings.Contains(template.Tok, other_template.Tok) {
										configErrorLocs.logError(fmt.Errorf("template %v contains template %v's token in its entirety; note that neither template may behave as desired", templateIndex, other_template_index))
										wasError = true
									}
								}
							}
						}
						configErrorLocs.removeLevel() // remove "token"
					}
					MetricsConfig.Templates = append(MetricsConfig.Templates, template)
					templateIndex += 1
					configErrorLocs.removeLevel() // remove template index
				})
			}
			configErrorLocs.removeLevel() // remove "templates"
		}

		// handle inputs (map[string]Input; technically optional, but it will probably be rare not to have any)
		element, internal_err = i.FindElement(nil, "inputs")
		if internal_err == nil {
			configErrorLocs.addKey("inputs", simdjson.TypeObject)
			inputObject, internal_err := element.Iter.Object(nil)
			if internal_err != nil {
				configErrorLocs.logError(internal_err)
				wasError = true
			} else {
				// for each input
				inputObject.ForEach(func(key []byte, inputIter simdjson.Iter) {
					configErrorLocs.addKey(string(key), simdjson.TypeObject)
					input := Input{}
					if overwrite_conflicting_configs {
						input.Name = string(key) + "_" + configId
					} else {
						input.Name = string(key)
					}

					// check for duplicate input names
					// this shouldn't happen in a valid json document but it's good to check
					// BUT now that we can load multiple configs, we really need to be sure the key doesn't already exist
					if _, ok := MetricsConfig.Inputs[input.Name]; ok && !overwrite_conflicting_configs {
						// fatal error for input
						configErrorLocs.logError(fmt.Errorf("duplicate input variable '%s'; only considering first occurence", input.Name))
						wasWarning = true
						configErrorLocs.removeLevel() // remove "input_var_name"
						return
					}

					// get the uri
					element, internal_err = inputIter.FindElement(nil, "uri")
					configErrorLocs.addKey("uri", simdjson.TypeString)
					if internal_err != nil {
						// if there's no uri, check to see if it's an internal variable (we'll handle this later, so we're just checking for now...)
						element, internal_err = inputIter.FindElement(nil, "internal")
						if internal_err != nil {
							// fatal error for input
							configErrorLocs.logError(internal_err)
							wasError = true
							configErrorLocs.removeLevel() // remove "uri"
							configErrorLocs.removeLevel() // remove "input_var_name"
							return
						}
					} else {
						input.Uri, internal_err = element.Iter.String()
						if internal_err != nil {
							// fatal error for input
							configErrorLocs.logError(internal_err)
							wasError = true
							configErrorLocs.removeLevel() // remove "uri"
							configErrorLocs.removeLevel() // remove "input_var_name"
							return
						}
					}
					configErrorLocs.removeLevel() // remove "uri"

					// check if internal variable
					element, internal_err = inputIter.FindElement(nil, "internal")
					configErrorLocs.addKey("internal", simdjson.TypeBool)
					if internal_err == nil {
						input.Internal, internal_err = element.Iter.Bool()
						if internal_err != nil {
							configErrorLocs.logError(internal_err)
							wasError = true
							if len(input.Uri) == 0 {
								// fatal error for input
								configErrorLocs.removeLevel() // remove "internal"
								configErrorLocs.removeLevel() // remove "input_var_name"
								return
							}
						}
						if !input.Internal && len(input.Uri) == 0 {
							// fatal error for input
							configErrorLocs.logError(fmt.Errorf("key 'internal' is specified as false but 'uri' field is empty; need one or the other"))
							wasError = true
							configErrorLocs.removeLevel() // remove "internal"
							configErrorLocs.removeLevel() // remove "input_var_name"
							return
						} else if input.Internal && len(input.Uri) > 0 {
							// currently, my thinking that this is a better default if both are specified
							// since someone might say "uri": "none"
							// (not fatal)
							configErrorLocs.logError(fmt.Errorf("key 'internal' is specified as true but 'uri' field contains a uri; defaulting to internally calculated value"))
							wasWarning = true
							input.Uri = ""
						}
					}
					configErrorLocs.removeLevel() // remove "internal"

					// get the data type
					element, internal_err = inputIter.FindElement(nil, "type")
					configErrorLocs.addKey("type", simdjson.TypeString)
					if internal_err != nil {
						// fatal error for input
						configErrorLocs.logError(internal_err)
						wasError = true
						configErrorLocs.removeLevel() // remove "type"
						configErrorLocs.removeLevel() // remove "input_var_name"
						return
					} else {
						input.Type, internal_err = element.Iter.String()
						if internal_err != nil {
							// fatal error for input
							configErrorLocs.logError(internal_err)
							wasError = true
							configErrorLocs.removeLevel() // remove "type"
							configErrorLocs.removeLevel() // remove "input_var_name"
							return
						}
						switch input.Type {
						case "string":
							input.Value.tag = STRING
						case "bool":
							input.Value.tag = BOOL
						case "float":
							input.Value.tag = FLOAT
						case "int":
							input.Value.tag = INT
						case "uint":
							input.Value.tag = UINT
						case "bitfield_int":
							input.Value.tag = UINT
						case "bitfield_string":
							input.Value.tag = STRING
						case "bitfield":
							input.Value.tag = UINT
						default:
							// fatal error for input
							configErrorLocs.logError(fmt.Errorf("invalid data type %v specified for input; must be string, bool, float, int, or uint", input.Type))
							wasError = true
							configErrorLocs.removeLevel() // remove "type"
							configErrorLocs.removeLevel() // remove "input_var_name"
							return
						}
						configErrorLocs.removeLevel() // remove "type"

						//the user can also specify an initial value
						element, internal_err = inputIter.FindElement(nil, "default")
						configErrorLocs.addKey("default", simdjson.TypeString)
						if internal_err == nil {
							elementInterface, internal_err := element.Iter.Interface()
							if internal_err != nil { // not sure if we can get here...
								configErrorLocs.logError(internal_err)
								wasError = true
							} else {
								input.Value = castValueToUnionType(elementInterface, input.Value.tag)
							}
						}
						configErrorLocs.removeLevel() // remove "default"
					}

					// get any attributes (optional)
					element, internal_err = inputIter.FindElement(nil, "attributes")
					if internal_err == nil {
						var attributesArray *simdjson.Array
						attributesArray, internal_err = element.Iter.Array(nil)
						configErrorLocs.addKey("attributes", simdjson.TypeArray)
						if internal_err != nil {
							// not fatal
							configErrorLocs.logError(internal_err)
							wasWarning = true
						} else {
							input.Attributes = make([]string, 0)
							input.AttributesMap = make(map[string]string)
							attributeIndex := 0
							attributesArray.ForEach(func(attributeIter simdjson.Iter) {
								var attribute string
								attribute, internal_err = attributeIter.String()
								configErrorLocs.addIndex(attributeIndex, simdjson.TypeString)
								if internal_err != nil {
									configErrorLocs.logError(internal_err)
									wasWarning = true
								} else {
									input.Attributes = append(input.Attributes, attribute)
									input.AttributesMap[attribute] = input.Name + "@" + attribute
									if attribute == "enabled" {
										MetricsConfig.Attributes[input.Name+"@"+attribute] = Attribute{Value: Union{tag: BOOL, b: true}, Name: attribute, InputVar: input.Name}
									} else {
										MetricsConfig.Attributes[input.Name+"@"+attribute] = Attribute{Value: Union{}, Name: attribute, InputVar: input.Name}
									}
									if allPossibleAttributes == nil {
										allPossibleAttributes = make(map[string][]string, 0)
									}
									if _, ok := allPossibleAttributes[attribute]; !ok {
										allPossibleAttributes[attribute] = make([]string, 0)
									}
									allPossibleAttributes[attribute] = append(allPossibleAttributes[attribute], input.Name+"@"+attribute)
								}
								configErrorLocs.removeLevel() // remove specific attribute
								attributeIndex += 1
							})
						}
						configErrorLocs.removeLevel() // remove "attributes"
					}

					// get the fims method
					element, internal_err = inputIter.FindElement(nil, "method")
					configErrorLocs.addKey("method", simdjson.TypeBool)
					if internal_err == nil {
						input.Method, internal_err = element.Iter.String()
						if internal_err != nil {
							configErrorLocs.logError(internal_err)
							wasWarning = true
							input.Method = ""
						} else if !(input.Method == "pub" || input.Method == "set" || input.Method == "both") {
							configErrorLocs.logError(fmt.Errorf("invalid fims input method; must be 'pub', 'set', or 'both'; defaulting to 'both'"))
							wasWarning = true
							input.Method = ""
						}
					} else {
						input.Method = ""
					}
					configErrorLocs.removeLevel() // remove "method"
					MetricsConfig.Inputs[input.Name] = input
					new_inputs = append(new_inputs, input.Name)
					configErrorLocs.removeLevel()
				}, nil)
			}
			configErrorLocs.removeLevel() // remove "inputs"
		}

		new_inputs, tempWasWarning, tempWasError = handleInputsTemplates(new_inputs, overwrite_conflicting_configs)
		wasWarning = wasWarning || tempWasWarning
		wasError = wasError || tempWasError
		generateScope(new_inputs)
		tempWasWarning, tempWasError = verifyInputConfigLogging()
		wasWarning = wasWarning || tempWasWarning
		wasError = wasError || tempWasError

		// handle filters (map[string]interface{}; optional)
		element, internal_err = i.FindElement(nil, "filters")
		if internal_err == nil {
			configErrorLocs.addKey("filters", simdjson.TypeObject)
			filterObject, internal_err := element.Iter.Object(nil)
			if internal_err != nil {
				configErrorLocs.logError(internal_err)
				wasError = true
			} else {
				if FiltersList == nil {
					FiltersList = []string{}
				}
				nextFilter := simdjson.Iter{}
				typ := simdjson.TypeObject
				var err error
				name := ""
				for {
					name, typ, err = filterObject.NextElement(&nextFilter)
					if typ == simdjson.TypeNone || err != nil {
						break
					}
					if overwrite_conflicting_configs {
						name = name + "_" + configId
					}
					configErrorLocs.addKey(name, simdjson.TypeObject)

					// remove filters so we don't repeat any in FiltersList (we'll add it back)
					removed_filter := false
					FiltersList, removed_filter = removeStringFromSlice(FiltersList, name)
					if removed_filter {
						new_filters_starting_index--
					}

					FiltersList = append(FiltersList, name) // todo: delete filter if not valid
					filterInterface, internal_err := nextFilter.Interface()
					if internal_err == nil {
						MetricsConfig.Filters[name] = filterInterface
						new_filters = append(new_filters, name)
					} else {
						configErrorLocs.logError(internal_err)
						wasError = true
					}
					configErrorLocs.removeLevel()
				}
			}
			configErrorLocs.removeLevel()
		}
		new_filters, tempWasWarning, tempWasError = handleFiltersTemplates(new_filters, overwrite_conflicting_configs)
		wasWarning = wasWarning || tempWasWarning
		wasError = wasError || tempWasError
		if getAndParseFilters(new_filters, new_filters_starting_index, new_inputs, overwrite_conflicting_configs, configId) { // getAndParseFilters returns true if there was an error
			wasError = true
		}
		if verifyFilterConfigLogging() {
			wasWarning = true
		}

		// handle outputs (map[string]Output; technically optional, but it will probably be rare not to have any)
		element, internal_err = i.FindElement(nil, "outputs")
		if PublishUris == nil {
			PublishUris = make(map[string][]string, 0)
		}
		if pubDataChanged == nil {
			pubDataChanged = make(map[string]bool, 0)
		}
		if internal_err == nil {
			configErrorLocs.addKey("outputs", simdjson.TypeObject)
			outputObject, internal_err := element.Iter.Object(nil)
			if internal_err != nil {
				configErrorLocs.logError(internal_err)
				wasError = true
			} else {
				// for each output
				outputObject.ForEach(func(key []byte, outputIter simdjson.Iter) {
					configErrorLocs.addKey(string(key), simdjson.TypeObject)
					output := Output{}
					outputName := string(key)
					if overwrite_conflicting_configs {
						outputName = outputName + "_" + configId
					}

					// check for duplicate output names
					// this shouldn't happen in a valid json document but it's good to check
					if _, ok := MetricsConfig.Outputs[outputName]; ok && !overwrite_conflicting_configs {
						// fatal error for output
						configErrorLocs.logError(fmt.Errorf("duplicate output variable '%s'; only considering first occurence", outputName))
						wasWarning = true
						configErrorLocs.removeLevel() // remove "output_var_name"
						return
					}

					// get any flags (optional)
					element, internal_err = outputIter.FindElement(nil, "flags")
					output.Flags = make([]string, 0)
					if internal_err == nil {
						var flagsArr *simdjson.Array
						flagsArr, internal_err = element.Iter.Array(nil)
						configErrorLocs.addKey("flags", simdjson.TypeArray)
						if internal_err != nil {
							// non-fatal error; just skip adding flags
							configErrorLocs.logError(internal_err)
							wasWarning = true
						} else {
							flagIndex := 0
							flagsArr.ForEach(func(flagIter simdjson.Iter) {
								var flag string
								flag, internal_err = flagIter.String()
								configErrorLocs.addIndex(flagIndex, simdjson.TypeString)
								if internal_err != nil {
									// non-fatal error; just skip adding this particular flag
									configErrorLocs.logError(internal_err)
									wasWarning = true
									configErrorLocs.removeLevel() // remove flag index
									return
								} else {
									validFlag := false
									for _, possibleFlag := range allOutputFlags {
										if matched, _ := regexp.MatchString(possibleFlag, flag); matched {
											validFlag = true
											break
										}
									}
									if !validFlag {
										// non-fatal error; just skip adding this particular flag
										configErrorLocs.logError(fmt.Errorf("invalid output flag '%v'", flag))
										wasWarning = true
										configErrorLocs.removeLevel() // remove flag index
										return
									}
									output.Flags = append(output.Flags, flag)
								}
								configErrorLocs.removeLevel() // remove flag index
								flagIndex += 1
							})
						}
						configErrorLocs.removeLevel() // remove "flags"
					}

					// get the uri (required)
					element, internal_err = outputIter.FindElement(nil, "uri")
					configErrorLocs.addKey("uri", simdjson.TypeString)
					if internal_err != nil {
						// fatal error for output
						configErrorLocs.logError(internal_err)
						wasError = true
						configErrorLocs.removeLevel() // remove "uri"
						configErrorLocs.removeLevel() // remove "output_var_name"
						return
					} else {
						output.Uri, internal_err = element.Iter.String()
						if internal_err != nil {
							// fatal error for output
							configErrorLocs.logError(internal_err)
							wasError = true
							configErrorLocs.removeLevel() // remove "uri"
							configErrorLocs.removeLevel() // remove "output_var_name"
							return
						}
					}
					configErrorLocs.removeLevel() // remove "uri"

					// get the name if there is one
					element, internal_err = outputIter.FindElement(nil, "name")
					configErrorLocs.addKey("name", simdjson.TypeString)
					if internal_err == nil {
						output.Name, internal_err = element.Iter.String()
						if internal_err != nil {
							// non-fatal error; just log it if there is one
							output.Name = string(key)
							configErrorLocs.logError(internal_err)
							wasWarning = true
						}
					} else {
						output.Name = string(key)
					}
					configErrorLocs.removeLevel() // remove "name"

					// get the publishRate if there is one
					element, internal_err = outputIter.FindElement(nil, "publishRate")
					configErrorLocs.addKey("publishRate", simdjson.TypeInt)
					if internal_err == nil {
						output.PublishRate, internal_err = element.Iter.Int()
						if internal_err != nil {
							configErrorLocs.logError(internal_err)
							wasWarning = true
						} else if output.PublishRate <= 0 {
							// this default is applied later
							output.PublishRate = 0
							configErrorLocs.logError(fmt.Errorf("publish rate must be greater than 0; defaulting to global publish rate"))
							wasWarning = true
						}
					}
					configErrorLocs.removeLevel() // remove "publishRate"

					// add any attributes and their values
					element, internal_err = outputIter.FindElement(nil, "attributes")
					output.Attributes = make(map[string]interface{}, 0)
					if internal_err == nil {
						var attributesObj *simdjson.Object
						attributesObj, internal_err = element.Iter.Object(nil)
						configErrorLocs.addKey("attributes", simdjson.TypeObject)
						if internal_err != nil {
							// just ignore the attributes if there's an error reading them into an object
							configErrorLocs.logError(internal_err)
							wasWarning = true
						} else {
							attributesObj.ForEach(func(attributeKey []byte, objIter simdjson.Iter) {
								// if an error occurs, I believe that key will just show up as null in the message body
								output.Attributes[string(attributeKey)], _ = objIter.Interface()
								OutputScope[outputName+"@"+string(attributeKey)] = []Union{getUnionFromValue(output.Attributes[string(attributeKey)])}
							}, nil)
						}
						configErrorLocs.removeLevel() // remove "attributes"
					}

					// determine if the output is an enum
					element, internal_err = outputIter.FindElement(nil, "enum")
					output.Enum = make([]EnumObject, 0)
					output.EnumMap = make(map[int]int, 0)
					if internal_err == nil && stringInSlice(output.Flags, "enum") {
						var enumArray *simdjson.Array
						enumArray, internal_err = element.Iter.Array(nil)
						configErrorLocs.addKey("enum", simdjson.TypeArray)
						if internal_err != nil {
							configErrorLocs.logError(internal_err)
							wasWarning = true
							flags_copy := make([]string, len(output.Flags))
							copy(flags_copy, output.Flags)
							fl_idx := 0
							for _, flag := range flags_copy {
								if flag == "enum" {
									output.Flags = append(output.Flags[:fl_idx], output.Flags[fl_idx+1:]...)
									fl_idx -= 1
								}
								fl_idx += 1
							}
						} else {
							var enumIndex int64
							var positionIndex int
							enumArray.ForEach(func(enumIter simdjson.Iter) {
								configErrorLocs.addIndex(positionIndex, simdjson.TypeObject)
								var enumObject EnumObject
								var simdjsonEnumObject *simdjson.Object
								simdjsonEnumObject, internal_err = enumIter.Object(nil)
								if internal_err == nil {
									element = simdjsonEnumObject.FindKey("value", nil)
									configErrorLocs.addKey("value", simdjson.TypeInt)
									if element != nil {
										enumObject.Value, internal_err = element.Iter.Int()
										if internal_err != nil {
											configErrorLocs.logError(fmt.Errorf("%v; using default index of %d", internal_err, enumIndex))
											wasWarning = true
											enumObject.Value = enumIndex
										} else {
											enumIndex = enumObject.Value
										}
									} else {
										configErrorLocs.logError(fmt.Errorf("path not found; using default index of %d", enumIndex))
										wasWarning = true
										enumObject.Value = enumIndex
									}
									configErrorLocs.removeLevel()

									element = simdjsonEnumObject.FindKey("string", nil)
									configErrorLocs.addKey("string", simdjson.TypeString)
									if element != nil {
										enumObject.String, internal_err = element.Iter.StringCvt()
										if internal_err != nil {
											configErrorLocs.logError(fmt.Errorf("%v; using default string of \"Unknown\"", internal_err))
											wasWarning = true
											enumObject.String = "Unknown"
										}
									} else {
										configErrorLocs.logError(fmt.Errorf("path not found; using default string of \"Unknown\""))
										wasWarning = true
										enumObject.String = "Unknown"
									}
									configErrorLocs.removeLevel()
								} else {
									enumObject.String, internal_err = enumIter.StringCvt()
									enumObject.Value = enumIndex
									configErrorLocs.addKey("string", simdjson.TypeString)
									if internal_err != nil {
										configErrorLocs.logError(fmt.Errorf("%v; using default index of %d and string of \"Unknown\"", internal_err, enumIndex))
										wasWarning = true
										enumObject.String = "Unknown"
									}
									configErrorLocs.removeLevel()
								}
								output.EnumMap[int(enumIndex)] = int(positionIndex)
								enumIndex = enumObject.Value + 1
								positionIndex += 1
								output.Enum = append(output.Enum, enumObject)
								configErrorLocs.removeLevel()
							})
						}
						configErrorLocs.removeLevel()
					} else if internal_err == nil && !stringInSlice(output.Flags, "enum") {
						configErrorLocs.logError(fmt.Errorf("found 'enum' field, but no matching output flag"))
						wasWarning = true
					} else if stringInSlice(output.Flags, "enum") {
						configErrorLocs.logError(fmt.Errorf("found 'enum' flag, but no matching 'enum' field"))
						wasWarning = true
						flags_copy := make([]string, len(output.Flags))
						copy(flags_copy, output.Flags)
						fl_idx := 0
						for _, flag := range flags_copy {
							if flag == "enum" {
								output.Flags = append(output.Flags[:fl_idx], output.Flags[fl_idx+1:]...)
								fl_idx -= 1
							}
							fl_idx += 1
						}
					}

					// determine if the output is a bitfield
					element, internal_err = outputIter.FindElement(nil, "bitfield")
					output.Bitfield = make([]EnumObject, 0)
					if internal_err == nil && stringInSlice(output.Flags, "bitfield") {
						var enumArray *simdjson.Array
						enumArray, internal_err = element.Iter.Array(nil)
						configErrorLocs.addKey("bitfield", simdjson.TypeArray)
						if internal_err != nil {
							configErrorLocs.logError(internal_err)
							wasWarning = true
							flags_copy := make([]string, len(output.Flags))
							copy(flags_copy, output.Flags)
							fl_idx := 0
							for _, flag := range flags_copy {
								if flag == "bitfield" {
									output.Flags = append(output.Flags[:fl_idx], output.Flags[fl_idx+1:]...)
									fl_idx -= 1
								}
								fl_idx += 1
							}
						} else {
							var enumIndex int64
							var positionIndex int
							enumArray.ForEach(func(enumIter simdjson.Iter) {
								configErrorLocs.addIndex(positionIndex, simdjson.TypeObject)
								var enumObject EnumObject
								var simdjsonEnumObject *simdjson.Object
								simdjsonEnumObject, internal_err = enumIter.Object(nil)
								if internal_err == nil {
									element = simdjsonEnumObject.FindKey("value", nil)
									configErrorLocs.addKey("value", simdjson.TypeInt)
									if element != nil {
										enumObject.Value, internal_err = element.Iter.Int()
										if internal_err != nil {
											configErrorLocs.logError(fmt.Errorf("%v; using default index of %d", internal_err, enumIndex))
											wasWarning = true
											enumObject.Value = enumIndex
										} else if enumObject.Value != enumIndex {
											configErrorLocs.logError(fmt.Errorf("%v; using default index of %d", "cannot skip values for bitfields", enumIndex))
											wasWarning = true
											enumObject.Value = enumIndex
										}
									} else {
										configErrorLocs.logError(fmt.Errorf("path not found; using default index of %d", enumIndex))
										wasWarning = true
										enumObject.Value = enumIndex
									}
									configErrorLocs.removeLevel()

									element = simdjsonEnumObject.FindKey("string", nil)
									configErrorLocs.addKey("string", simdjson.TypeString)
									if element != nil {
										enumObject.String, internal_err = element.Iter.StringCvt()
										if internal_err != nil {
											configErrorLocs.logError(fmt.Errorf("%v; using default string of \"Unknown\"", internal_err))
											wasWarning = true
											enumObject.String = "Unknown"
										}
									} else {
										configErrorLocs.logError(fmt.Errorf("path not found; using default string of \"Unknown\""))
										wasWarning = true
										enumObject.String = "Unknown"
									}
									configErrorLocs.removeLevel()
								} else {
									enumObject.String, internal_err = enumIter.StringCvt()
									enumObject.Value = enumIndex
									configErrorLocs.addKey("string", simdjson.TypeString)
									if internal_err != nil {
										configErrorLocs.logError(fmt.Errorf("%v; using default index of %d and string of \"Unknown\"", internal_err, enumIndex))
										wasWarning = true
										enumObject.String = "Unknown"
									}
									configErrorLocs.removeLevel()
								}
								enumIndex = enumObject.Value + 1
								positionIndex += 1
								output.Bitfield = append(output.Bitfield, enumObject)
								configErrorLocs.removeLevel()
							})
						}
						configErrorLocs.removeLevel()
					} else if internal_err == nil && !stringInSlice(output.Flags, "bitfield") {
						configErrorLocs.logError(fmt.Errorf("found 'bitfield' field, but no matching output flag"))
						wasWarning = true
					} else if stringInSlice(output.Flags, "bitfield") {
						configErrorLocs.logError(fmt.Errorf("found 'bitfield' flag, but no matching 'bitfield' field"))
						wasWarning = true
						flags_copy := make([]string, len(output.Flags))
						copy(flags_copy, output.Flags)
						fl_idx := 0
						for _, flag := range flags_copy {
							if flag == "bitfield" {
								output.Flags = append(output.Flags[:fl_idx], output.Flags[fl_idx+1:]...)
								fl_idx -= 1
							}
							fl_idx += 1
						}
					}
					MetricsConfig.Outputs[outputName] = output
					new_outputs = append(new_outputs, outputName)
					configErrorLocs.removeLevel()
				}, nil)

			}
			configErrorLocs.removeLevel() // remove "outputs"
		}
		new_outputs = handleOutputsTemplates(new_outputs)
		for _, outputName := range new_outputs {
			output := MetricsConfig.Outputs[string(outputName)]
			combineFlags(outputName, &output)
		}
		if checkMixedFlags() {
			wasWarning = true
		}
		if verifyOutputConfigLogging() {
			wasWarning = true
		}

		// handle metrics ([]MetricsObject)
		element, internal_err = i.FindElement(nil, "metrics")
		if internal_err == nil {
			configErrorLocs.addKey("metrics", simdjson.TypeArray)
			metricsArr, internal_err := element.Iter.Array(nil)
			if internal_err != nil {
				configErrorLocs.logError(internal_err)
				wasError = true
			} else {
				// for each metrics object
				metricsIndex := 0
				internal_vars := make([]string, 0) // keep track of internal vars so we know if they're calculated before they're used
				metricsArr.ForEach(func(metricsIter simdjson.Iter) {
					configErrorLocs.addIndex(metricsIndex, simdjson.TypeObject)
					metric := MetricsObject{}
					_, internal_err = metricsIter.Object(nil)
					if internal_err != nil {
						configErrorLocs.logError(internal_err)
						wasError = true
						configErrorLocs.removeLevel() // remove "metricsIndex"
						metricsIndex += 1
						return
					} else { // handle the metrics object

						// get the id
						element, internal_err = metricsIter.FindElement(nil, "id")
						configErrorLocs.addKey("id", simdjson.TypeString)
						if internal_err != nil {
							metric.Id = fmt.Sprintf("%d", metricsIndex)
							configErrorLocs.logError(fmt.Errorf("%v (warning only)", internal_err))
							wasWarning = true
						} else {
							metric.Id, internal_err = element.Iter.StringCvt()
							if internal_err != nil { // not sure if we can get here...
								configErrorLocs.logError(internal_err)
								wasWarning = true
							}
						}
						configErrorLocs.removeLevel() // remove id

						// get the value
						element, internal_err = metricsIter.FindElement(nil, "type")
						configErrorLocs.addKey("type", simdjson.TypeString)
						if internal_err != nil {
							metric.Type = NIL
							configErrorLocs.logError(internal_err)
							wasWarning = true
						} else {
							var typeString string
							typeString, internal_err = element.Iter.StringCvt()
							if internal_err != nil { // not sure if we can get here...
								configErrorLocs.logError(internal_err)
								wasWarning = true
							}
							switch typeString {
							case "string":
								metric.Type = STRING
							case "bool":
								metric.Type = BOOL
							case "uint":
								metric.Type = UINT
							case "int":
								metric.Type = INT
							case "float":
								metric.Type = FLOAT
							case "default":
								metric.Type = NIL
							default:
								metric.Type = NIL
								configErrorLocs.logError(fmt.Errorf("unhandled output type %s; deferring to default output type", typeString))
								wasWarning = true
							}
						}
						configErrorLocs.removeLevel() // remove type

						// Determine whether this is an alert (default false)
						element, internal_err = metricsIter.FindElement(nil, "alert")
						configErrorLocs.addKey("alert", simdjson.TypeString)
						if internal_err != nil {
							// alert field is optional, set false and continue
							metric.Alert = false
						} else {
							// error if alert field was found but does not contain a boolean value
							alertVal, internal_err := element.Iter.Bool()
							if internal_err != nil {
								configErrorLocs.logError(internal_err)
								wasError = true
							} else {
								metric.Alert = alertVal
							}
						}
						configErrorLocs.removeLevel() // remove "alert"

						if metric.Alert {
							element, internal_err = metricsIter.FindElement(nil, "messages")
							// Messages are required for alerts, optional (unused) otherwise
							if internal_err != nil || element == nil {
								configErrorLocs.logError(fmt.Errorf("message array required for alerts: %w", internal_err))
								wasError = true
							} else {
								var messageArray *simdjson.Array
								messageArray, internal_err = element.Iter.Array(nil)
								configErrorLocs.addKey("messages", simdjson.TypeArray)
								if internal_err != nil {
									configErrorLocs.logError(fmt.Errorf("failed to get alert message array: %w", internal_err))
									wasError = true
								} else {
									metric.Messages = make(map[string]string)
									var positionIndex int
									messageArray.ForEach(func(messageItr simdjson.Iter) {
										configErrorLocs.addIndex(positionIndex, simdjson.TypeObject)
										var messageObj *simdjson.Object
										messageObj, internal_err = messageItr.Object(nil)
										if internal_err != nil {
											configErrorLocs.logError(fmt.Errorf("failed to get message entry %d from alert message array: %w", positionIndex, internal_err))
											wasError = true
										} else {
											messageMap := make(map[string]interface{})
											_, internal_err = messageObj.Map(messageMap)
											if internal_err != nil || len(messageMap) == 0 {
												configErrorLocs.logError(fmt.Errorf("failed to get message expression %d from alert message array: %w", positionIndex, internal_err))
												wasError = true
											} else {
												for messageExpression, messageInterface := range messageMap {
													messageString, ok := messageInterface.(string)
													if !ok {
														configErrorLocs.logError(fmt.Errorf("failed to get message string %d from alert message array: %w", positionIndex, internal_err))
														wasError = true
													} else {
														metric.Messages[messageExpression] = messageString
													}
												}
											}
										}
										positionIndex += 1
										configErrorLocs.removeLevel() // remove positionIndex
									})
								}
								configErrorLocs.removeLevel() // remove "messages"
							}
						}

						// get the outputs (at least one is required)
						element, internal_err = metricsIter.FindElement(nil, "outputs")
						configErrorLocs.addKey("outputs", simdjson.TypeArray)
						if internal_err != nil {
							//we can also theoretically have an internal_output, so we can check for that if we don't find a regular output
							element, internal_err = metricsIter.FindElement(nil, "internal_output")
							if internal_err != nil {
								configErrorLocs.logError(internal_err)
								wasError = true
							}
						} else {
							var outputsArr *simdjson.Array
							outputsArr, internal_err = element.Iter.Array(nil)
							if internal_err != nil {
								var outputStr string
								outputStr, internal_err = element.Iter.StringCvt()
								if internal_err != nil { // not sure if we can get here...
									configErrorLocs.logError(internal_err)
									wasWarning = true
								} else {
									if overwrite_conflicting_configs {
										outputStr = outputStr + "_" + configId
									}
									metric.Outputs = []string{outputStr}
								}
							} else {
								metric.Outputs = make([]string, 0)
								outputIndex := 0
								outputsArr.ForEach(func(outputIter simdjson.Iter) {
									var outputVar string
									outputVar, internal_err = outputIter.String()
									configErrorLocs.addIndex(outputIndex, simdjson.TypeString)
									if internal_err != nil {
										configErrorLocs.logError(internal_err)
										wasWarning = true
									} else {
										if overwrite_conflicting_configs {
											outputVar = outputVar + "_" + configId
										}
										metric.Outputs = append(metric.Outputs, outputVar)
									}
									configErrorLocs.removeLevel()
									outputIndex += 1
								})
							}
						}
						configErrorLocs.removeLevel() // remove outputs

						// check if there's an internal output
						element, internal_err = metricsIter.FindElement(nil, "internal_output")
						configErrorLocs.addKey("internal_output", simdjson.TypeString)
						if internal_err == nil {
							var internalOutput string
							internalOutput, internal_err = element.Iter.StringCvt()
							if internal_err != nil {
								configErrorLocs.logError(internal_err)
								wasWarning = true
							} else {
								if overwrite_conflicting_configs {
									internalOutput = internalOutput + "_" + configId
								}
								metric.InternalOutput = internalOutput
							}
						}
						configErrorLocs.removeLevel() // remove internal_output

						// get the expression
						element, internal_err = metricsIter.FindElement(nil, "expression")
						configErrorLocs.addKey("expression", simdjson.TypeString)
						if internal_err != nil {
							configErrorLocs.logError(internal_err)
							wasError = true
							configErrorLocs.removeLevel() // remove "expression"
							configErrorLocs.removeLevel() // remove "metricsIndex"
							metricsIndex += 1
							return
						} else {
							// expression can be string or array
							// if array, concatenate all strings together
							// else, just use the expression directly
							var expr string
							expr_array, internal_err := element.Iter.Array(nil)
							if internal_err != nil {
								expr, internal_err = element.Iter.String()
								if internal_err != nil {
									configErrorLocs.logError(internal_err)
									wasError = true
								} else {
									metric.Expression = expr
								}
							} else {
								expr_index := 0
								expr_array.ForEach(func(expr_iter simdjson.Iter) {
									configErrorLocs.addIndex(expr_index, simdjson.TypeString)
									temp_expr, internal_err := expr_iter.String()
									if internal_err != nil {
										configErrorLocs.logError(internal_err)
										wasError = true
									} else {
										expr += temp_expr
									}
									expr_index++
								})
								if len(expr) > 0 {
									metric.Expression = expr
								}
							}

						}
						configErrorLocs.removeLevel() // remove "expression"

						// apply templating
						metricsObjects := handleMetricsTemplates(metric)

						// now that we've applied templating, check that the output vars are all valid
						// remove them if they're not
						configErrorLocs.addKey("outputs", simdjson.TypeArray)
						for idx := range metricsObjects {
							outputIdx := 0
							metricOutputsCopy := make([]string, len(metricsObjects[idx].Outputs))
							copy(metricOutputsCopy, metricsObjects[idx].Outputs)
							for _, outputVar := range metricOutputsCopy {
								if _, ok := MetricsConfig.Outputs[outputVar]; !ok {
									if strings.Contains(outputVar, "@") {
										if _, ok := MetricsConfig.Outputs[strings.Split(outputVar, "@")[0]]; !ok {
											metricsObjects[idx].Outputs = append(metricsObjects[idx].Outputs[:outputIdx], metricsObjects[idx].Outputs[outputIdx+1:]...)
											outputIdx -= 1
											configErrorLocs.logError(fmt.Errorf("output variable '%s' does not have a corresponding output config", outputVar))
											wasError = true
										}
									} else {
										metricsObjects[idx].Outputs = append(metricsObjects[idx].Outputs[:outputIdx], metricsObjects[idx].Outputs[outputIdx+1:]...)
										outputIdx -= 1
										configErrorLocs.logError(fmt.Errorf("output variable '%s' does not have a corresponding output config", outputVar))
										wasError = true
									}
								}
								outputIdx += 1
							}
						}
						configErrorLocs.removeLevel() // remove "outputs"

						// now that we've applied templating, check that the internal output is valid
						// remove it if not
						configErrorLocs.addKey("internal_output", simdjson.TypeString)
						for idx, metric := range metricsObjects {
							if len(metric.InternalOutput) > 0 {
								if _, ok := MetricsConfig.Inputs[metric.InternalOutput]; !ok {
									if strings.Contains(metric.InternalOutput, "@") {
										configErrorLocs.logError(fmt.Errorf("cannot map internal_output variable to attribute"))
										wasError = true
									} else {
										configErrorLocs.logError(fmt.Errorf("internal_output variable '%s' does not have a corresponding input config", metric.InternalOutput))
										wasError = true
									}
									metricsObjects[idx].InternalOutput = ""
								}
							}
						}
						configErrorLocs.removeLevel() // remove "internal_output"

						metricsObjectsIterationCopy := make([]MetricsObject, len(metricsObjects))
						copy(metricsObjectsIterationCopy, metricsObjects)
						idx := 0
						for _, metric := range metricsObjectsIterationCopy {
							if len(metric.InternalOutput) == 0 && len(metric.Outputs) == 0 {
								configErrorLocs.logError(fmt.Errorf("after applying templating and variable checks, metric does not have valid internal_output or output variables; discarding metric"))
								wasError = true
								metricsObjects = append(metricsObjects[:idx], metricsObjects[idx+1:]...)
								idx -= 1
							}
							idx += 1
						}

						// preprocess all of the expressions in the metrics document
						// so that all we need to do is evaluate the expressions at runtime.
						// currently modeling after https://github.com/crsmithdev/goexpr
						configErrorLocs.addKey("expression", simdjson.TypeString)
						for _, metric := range metricsObjects {
							var warning string
							warning, internal_err = metric.getExpression(internal_vars, len(MetricsConfig.Metrics), configId)
							if len(warning) > 0 {
								configErrorLocs.logError(fmt.Errorf("%v (warning only)", warning))
								wasWarning = true
							}
							if overwrite_conflicting_configs {
								messages_copy := make(map[string]string, 0)
								for metricsSubexpression, messageString := range metric.Messages {
									messages_copy[metricsSubexpression] = messageString
								}
								metric.Messages = make(map[string]string, 0)
								for metricsSubexpression, messageString := range messages_copy {
									for _, varName := range metric.ParsedExpression.Vars {
										if !stringInSlice([]string{"true", "false", "nil", "attribute", "value"}, varName) {
											varName = strings.TrimSuffix(varName, "_"+configId)
											isAttribute := false
											for attributeName := range allPossibleAttributes {
												if attributeName == varName {
													isAttribute = true
												}
											}
											if !isAttribute {
												regexpression := "\\b" + varName + "\\b"
												compiled_expression, err := regexp.Compile(regexpression)
												if err == nil {
													expression_bytes := compiled_expression.ReplaceAll([]byte(metricsSubexpression), []byte(varName+"_"+configId))
													metricsSubexpression = string(expression_bytes)
													messageString = strings.ReplaceAll(messageString, "{"+varName+"}", "{"+varName+"_"+configId+"}")
													metric.Messages[metricsSubexpression] = messageString
												}
											}
										}
									}
								}
							}
							if internal_err != nil {
								configErrorLocs.logError(fmt.Errorf("%v; excluding this metric from calculations", internal_err))
								wasError = true
							} else {
								if overwrite_conflicting_configs {
									for i, metricObject := range MetricsConfig.Metrics {
										if metricObject.Id == metric.Id && new_metrics_starting_index > 0 {
											MetricsConfig.Metrics = append(MetricsConfig.Metrics[:i], MetricsConfig.Metrics[i+1:]...)
											if len(metricObject.InternalOutput) > 0 {
												internal_vars, _ = removeStringFromSlice(internal_vars, metricObject.InternalOutput)
											}
											new_metrics_starting_index--
											break
										}
									}
								}
								MetricsConfig.Metrics = append(MetricsConfig.Metrics, metric)
								internal_vars = append(internal_vars, metric.InternalOutput)
							}
						}
						configErrorLocs.removeLevel()
					}
					metricsIndex += 1
					configErrorLocs.removeLevel() // delete metrics index
				})

			}
			configErrorLocs.removeLevel()

		}
		if checkUnusedOutputs() {
			wasError = true
		}
		configErrorLocs.addKey("metrics", simdjson.TypeArray)
		if new_metrics_starting_index < 0 {
			new_metrics_starting_index = 0
		}
		warnings, errs := mapOutputsToMetrics(new_metrics_starting_index)
		for j, warning := range warnings {
			if len(warning) > 0 {
				configErrorLocs.addIndex(j, simdjson.TypeObject)
				configErrorLocs.addKey("expression", simdjson.TypeString)
				configErrorLocs.logError(fmt.Errorf("had an issue mapping outputs to metrics: %v (warning only)", warning))
				wasWarning = true
				configErrorLocs.removeLevel()
				configErrorLocs.removeLevel()
			}
		}
		j := 0
		for j, internal_err = range errs {
			if internal_err != nil {
				configErrorLocs.addIndex(j, simdjson.TypeObject)
				configErrorLocs.logError(fmt.Errorf("had an issue mapping outputs to metrics: %v; excluding this metric from calculations", internal_err))
				wasError = true
				configErrorLocs.removeLevel()
			}
		}
		configErrorLocs.removeLevel()

		// handle echos ([]EchoObject)
		element, internal_err = i.FindElement(nil, "echo")
		if internal_err == nil {
			configErrorLocs.addKey("echo", simdjson.TypeArray)
			echoArr, internal_err := element.Iter.Array(nil)
			if internal_err != nil {
				configErrorLocs.logError(internal_err)
				wasError = true
			} else {
				// for each echo object
				echoIndex := 0
				echoArr.ForEach(func(echoIter simdjson.Iter) {
					configErrorLocs.addIndex(echoIndex, simdjson.TypeObject)
					echo := EchoObject{}

					_, internal_err = echoIter.Object(nil)
					if internal_err != nil {
						configErrorLocs.logError(internal_err)
						wasError = true
					} else { // handle the echo object
						// get the uri
						fatalErr := false
						echo.Inputs = make([]EchoInput, 0)
						echo.Echo = make(map[string]interface{}, 0)
						element, internal_err = echoIter.FindElement(nil, "uri")
						configErrorLocs.addKey("uri", simdjson.TypeString)
						if internal_err != nil {
							configErrorLocs.logError(internal_err)
							wasError = true
							fatalErr = true
						} else {
							echo.PublishUri, internal_err = element.Iter.StringCvt()
							if internal_err != nil {
								configErrorLocs.logError(internal_err)
								wasError = true
								fatalErr = true
							}
						}
						configErrorLocs.removeLevel()

						// get the publish rate
						element, internal_err = echoIter.FindElement(nil, "publishRate")
						configErrorLocs.addKey("publishRate", simdjson.TypeString)
						if internal_err != nil {
							configErrorLocs.logError(internal_err)
							wasError = true
							fatalErr = true
						} else {
							echo.PublishRate, internal_err = element.Iter.Int()
							if internal_err != nil {
								configErrorLocs.logError(internal_err)
								wasError = true
								fatalErr = true
							}
						}
						configErrorLocs.removeLevel()

						// get the heartbeat (optional)
						element, internal_err = echoIter.FindElement(nil, "heartbeat")
						configErrorLocs.addKey("heartbeat", simdjson.TypeString)
						if internal_err == nil {
							echo.Heartbeat, internal_err = element.Iter.StringCvt()
							if internal_err != nil {
								configErrorLocs.logError(internal_err)
								wasWarning = true
							}
						}
						configErrorLocs.removeLevel()

						// get the format (optional)
						element, internal_err = echoIter.FindElement(nil, "format")
						configErrorLocs.addKey("format", simdjson.TypeString)
						if internal_err == nil {
							echo.Format, internal_err = element.Iter.StringCvt()
							if internal_err != nil {
								configErrorLocs.logError(internal_err)
								wasWarning = true
							}
						}
						configErrorLocs.removeLevel()

						// get the null_value_default (optional)
						var null_value_default interface{}
						element, internal_err = echoIter.FindElement(nil, "null_value_default")
						configErrorLocs.addKey("null_value_default", simdjson.TypeString)
						if internal_err == nil {
							null_value_default, internal_err = element.Iter.Interface()
							if internal_err != nil {
								configErrorLocs.logError(internal_err)
								wasError = true
							}
						}
						configErrorLocs.removeLevel()

						// get the inputs
						element, internal_err = echoIter.FindElement(nil, "inputs")
						if internal_err == nil {
							var inputsArr *simdjson.Array
							inputsArr, internal_err = element.Iter.Array(nil)
							configErrorLocs.addKey("inputs", simdjson.TypeArray)
							if internal_err != nil {
								configErrorLocs.logError(internal_err)
								wasError = true
							} else {
								echo.Inputs = make([]EchoInput, 0)
								inputIndex := 0
								inputsArr.ForEach(func(inputIter simdjson.Iter) {
									configErrorLocs.addIndex(inputIndex, simdjson.TypeObject)
									var echoInput EchoInput
									echoInput.Registers = make(map[string]string, 0)
									element, internal_err = inputIter.FindElement(nil, "uri")
									configErrorLocs.addKey("uri", simdjson.TypeString)
									if internal_err != nil {
										configErrorLocs.logError(internal_err)
										wasError = true
										configErrorLocs.removeLevel() // delete uri
										configErrorLocs.removeLevel() // delete input index
										return
									} else {
										echoInput.Uri, internal_err = element.Iter.StringCvt()
										if internal_err != nil {
											configErrorLocs.logError(internal_err)
											wasError = true
											configErrorLocs.removeLevel() // delete uri
											configErrorLocs.removeLevel() // delete input index
											return
										}
									}
									configErrorLocs.removeLevel() // delete uri
									element, internal_err = inputIter.FindElement(nil, "registers")
									configErrorLocs.addKey("registers", simdjson.TypeObject)

									if internal_err != nil {
										configErrorLocs.logError(internal_err)
										wasError = true
										configErrorLocs.removeLevel() // delete uri
										configErrorLocs.removeLevel() // delete input index
										return
									} else {
										var temp interface{}
										temp, internal_err = element.Iter.Interface()

										if internal_err != nil {
											configErrorLocs.logError(internal_err)
											wasError = true
											configErrorLocs.removeLevel() // delete registers
											configErrorLocs.removeLevel() // delete input index
											return
										}
										var ok bool
										var tempInterface map[string]interface{}
										tempInterface, ok = temp.(map[string]interface{})
										if !ok {
											configErrorLocs.logError(fmt.Errorf("could not convert registers to map[string]interface{}"))
											wasError = true
											configErrorLocs.removeLevel() // delete registers
											configErrorLocs.removeLevel() // delete input index
											return
										}

										for key, value := range tempInterface { // echo input registers can be either strings or objects with "source" and "default"
											switch x := value.(type) {
											case string:
												echoInput.Registers[key] = x
												echo.Echo[key] = null_value_default
											case map[string]interface{}:
												source, okSource := x["source"]
												configErrorLocs.addKey(key, simdjson.TypeObject)
												configErrorLocs.addKey("source", simdjson.TypeString)
												if !okSource {
													configErrorLocs.logError(fmt.Errorf("path not found"))
													wasError = true
													configErrorLocs.removeLevel() // delete register key
													configErrorLocs.removeLevel() // delete source
													continue
												}
												okStrSource := false
												echoInput.Registers[key], okStrSource = source.(string)
												if !okStrSource {
													configErrorLocs.logError(fmt.Errorf("value is not string"))
													wasError = true
													configErrorLocs.removeLevel() // delete register key
													configErrorLocs.removeLevel() // delete source
													continue
												}
												configErrorLocs.removeLevel() // delete source
												configErrorLocs.addKey("default", simdjson.TypeNone)
												defaultVal, okDefault := x["default"]
												if !okDefault {
													configErrorLocs.logError(fmt.Errorf("path not found"))
													wasError = true
													echo.Echo[key] = null_value_default
												} else {
													echo.Echo[key] = defaultVal
												}
												configErrorLocs.removeLevel() // delete default
												configErrorLocs.removeLevel() // delete register key
											default:
												configErrorLocs.logError(fmt.Errorf("could not convert registers to map[string]string"))
												wasError = true
												continue
											}
										}
										if len(echo.Heartbeat) > 0 {
											echo.Echo[echo.Heartbeat] = 0.0
										}
										for _, in := range echo.Inputs {
											for key := range in.Registers {
												if _, ok = echoInput.Registers[key]; ok {
													configErrorLocs.logError(fmt.Errorf("echo output key %v appears in more than one input register; excluding this input", key))
													wasWarning = true
													delete(echoInput.Registers, key)
												}
											}
										}
									}
									configErrorLocs.removeLevel() // delete registers
									configErrorLocs.removeLevel() // delete input index
									echo.Inputs = append(echo.Inputs, echoInput)
									inputIndex += 1
								})
							}
							configErrorLocs.removeLevel() // delete inputs

						}

						// get the echo vars
						element, internal_err = echoIter.FindElement(nil, "echo")
						configErrorLocs.addKey("echo", simdjson.TypeObject)
						if internal_err == nil {
							var echoInterface interface{}
							var echoMap map[string]interface{}
							echoInterface, internal_err = element.Iter.Interface()
							if internal_err != nil {
								configErrorLocs.logError(internal_err)
								wasError = true
							}
							var ok bool
							echoMap, ok = echoInterface.(map[string]interface{})
							if !ok {
								configErrorLocs.logError(fmt.Errorf("could not convert echo object to map[string]interface{}"))
								wasError = true
							}
							for key, value := range echoMap {
								if echoValue, ok := echo.Echo[key]; !ok {
									echo.Echo[key] = value
								} else {
									if echoValue == nil {
										echo.Echo[key] = value // set the default based on this register
									} else {
										configErrorLocs.addKey(key, simdjson.TypeString)
										configErrorLocs.logError(fmt.Errorf("found duplicate echo register; using the first occurence"))
										wasWarning = true
										configErrorLocs.removeLevel()
									}
								}
							}
						}

						if null_value_default == nil {
							for key, value := range echo.Echo {
								if value == nil {
									configErrorLocs.addKey(key, simdjson.TypeString)
									configErrorLocs.logError(fmt.Errorf("default value for echo input register '%s' was not specified and echo object does not contain field 'null_value_default' to override the null register value; setting default value of register to 0 (warning only)", key))
									wasWarning = true
									configErrorLocs.removeLevel()
									echo.Echo[key] = int64(0)
								}
							}
						}

						configErrorLocs.removeLevel()
						if !fatalErr {
							MetricsConfig.Echo = append(MetricsConfig.Echo, echo)
						}

					}

					echoIndex += 1
					configErrorLocs.removeLevel()
				})

			}
			configErrorLocs.removeLevel()
			handleEchoTemplates(new_echo_starting_index)

		}

		if wasError || wasWarning {
			return fmt.Errorf("")
		}
		return nil
	})

	var bytesDat interface{}
	err = json.Unmarshal(data, &bytesDat)
	if err == nil {
		bytesInterface, ok := bytesDat.(map[string]interface{})
		if ok {
			configErrorLocs.addIndex(0, simdjson.TypeObject)
			for key := range bytesInterface {
				if !stringInSlice([]string{"meta", "templates", "inputs", "filters", "outputs", "metrics", "echo"}, key) {
					wasWarning = true
					configErrorLocs.logError(fmt.Errorf("found unknown key '%s' in configuration document; ignoring config element", key))
				}
			}
			configErrorLocs.removeLevel()
		}
	}

	if len(ConfigErrorsFile) > 0 {
		var str string
		var fd *os.File
		if len(configErrorLocs.ErrorLocs) > 0 {
			str = "[\n"
			for _, errMsg := range configErrorLocs.ErrorLocs[0 : len(configErrorLocs.ErrorLocs)-1] {
				str += fmt.Sprintf("%v,\n", errMsg)
			}
			str += fmt.Sprintf("%v\n]", configErrorLocs.ErrorLocs[len(configErrorLocs.ErrorLocs)-1])
		} else {
			str = "[ ]"
		}
		if strings.Contains(ConfigErrorsFile, ".json") {
			fd, _ = os.OpenFile(ConfigErrorsFile, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0644)
		} else {
			fd, _ = os.OpenFile(ConfigErrorsFile+".json", os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0644)
		}
		defer fd.Close()
		fd.WriteString(str)
	}
	if len(configErrorLocs.ErrorLocs) > 0 {
		log.Warnf("Error unmarshaling json file: %v", configErrorLocs)
	}

	// get the MDO ready for later
	Mdo.Meta = make(map[string]interface{}, 3) // to hold: config file, process name, time published
	Mdo.Meta["name"] = ProcessName
	Mdo.Meta["config"] = ConfigSource
	Mdo.Meta["timestamp"] = time.Now().Format(time.RFC3339)
	Mdo.Inputs = make(map[string]map[string]interface{}, len(MetricsConfig.Inputs))
	for key := range MetricsConfig.Inputs {
		Mdo.Inputs[key] = make(map[string]interface{}, 1+len(MetricsConfig.Inputs[key].Attributes))
		Mdo.Inputs[key]["value"] = 0
		for _, attribute := range MetricsConfig.Inputs[key].Attributes {
			Mdo.Inputs[key][attribute] = 0
		}
	}
	Mdo.Filters = make(map[string][]string, len(MetricsConfig.Filters))
	for key := range MetricsConfig.Filters {
		if len(dynamicFilterExpressions[key].DynamicInputs) > 0 {
			Mdo.Filters[key] = make([]string, len(dynamicFilterExpressions[key].DynamicInputs[0]))
		} else if len(staticFilterExpressions[key].DynamicInputs) > 0 {
			Mdo.Filters[key] = make([]string, len(staticFilterExpressions[key].DynamicInputs[0]))
		}
	}
	Mdo.Outputs = make(map[string]map[string]interface{}, len(MetricsConfig.Outputs))
	for key := range MetricsConfig.Outputs {
		Mdo.Outputs[key] = make(map[string]interface{}, 1+len(MetricsConfig.Outputs[key].Attributes))
		Mdo.Outputs[key]["value"] = 0
		for attribute, attributeVal := range MetricsConfig.Outputs[key].Attributes {
			Mdo.Outputs[key][attribute] = attributeVal
		}
	}
	Mdo.Metrics = make(map[string]map[string]interface{}, len(MetricsConfig.Metrics))
	for i := range MetricsConfig.Metrics {
		Mdo.Metrics[MetricsConfig.Metrics[i].Expression] = make(map[string]interface{}, 1)
		Mdo.Metrics[MetricsConfig.Metrics[i].Expression]["value"] = 0
	}
	Mdo.Echo = make(map[string]map[string]interface{}, len(MetricsConfig.Echo))
	for i := range MetricsConfig.Echo {
		Mdo.Echo[MetricsConfig.Echo[i].PublishUri] = MetricsConfig.Echo[i].Echo
	}
	mdoBuf = new(bytes.Buffer)
	mdoEncoder = json.NewEncoder(mdoBuf)
	mdoEncoder.SetEscapeHTML(false)
	mdoEncoder.SetIndent("", "    ")
	new_metrics_info := NewMetricsInfo{
		new_inputs,
		new_filters,
		new_filters_starting_index,
		new_outputs,
		new_metrics_starting_index,
		new_echo_starting_index,
		0,
	}
	metricsConfigMutex.Unlock()
	return new_metrics_info, wasWarning, wasError
}

func handleInputsTemplates(new_inputs []string, overwrite_conflicting_configs bool) ([]string, bool, bool) {
	wasWarning := false
	wasError := false
	if allPossibleAttributes == nil {
		allPossibleAttributes = make(map[string][]string, 0)
	}
	for _, template := range MetricsConfig.Templates {
		new_inputs_copy := make([]string, len(new_inputs))
		copy(new_inputs_copy, new_inputs)
		i := 0
		for _, inputName := range new_inputs_copy {
			input := MetricsConfig.Inputs[inputName]
			// it only really makes sense for inputs to be templated by their variable name
			// otherwise, we would have repeated variable names
			if strings.Contains(inputName, template.Tok) {
				for _, replacement := range template.List {
					newInput := Input{}
					newInput.Internal = input.Internal
					newInputName := strings.ReplaceAll(inputName, template.Tok, replacement)

					// check for duplicate input names
					// this is much more likely to happen if we are careless with templating
					if _, ok := MetricsConfig.Inputs[newInputName]; ok && !overwrite_conflicting_configs {
						// fatal error for input
						configErrorLocs.addKey("inputs", simdjson.TypeObject)
						configErrorLocs.addKey(newInputName, simdjson.TypeObject)
						configErrorLocs.logError(fmt.Errorf("duplicate input variable '%s' when unraveling template; only considering first occurence", newInputName))
						configErrorLocs.removeLevel()
						configErrorLocs.removeLevel()
						wasWarning = true
						continue
					}

					newInput.Name = newInputName
					newInput.Uri = strings.ReplaceAll(input.Uri, template.Tok, replacement)
					newInput.Type = input.Type
					newInput.Value = input.Value
					if newInput.Value.tag == STRING {
						newInput.Value.s = strings.ReplaceAll(newInput.Value.s, template.Tok, replacement)
					}
					newInput.Attributes = make([]string, len(input.Attributes))
					newInput.AttributesMap = make(map[string]string, len(input.AttributesMap))
					if len(input.Attributes) > 0 {
						for p, attribute := range input.Attributes {
							newAttribute := strings.ReplaceAll(attribute, template.Tok, replacement)
							newInput.Attributes[p] = newAttribute
							newInput.AttributesMap[newAttribute] = newInputName + "@" + newAttribute
							delete(MetricsConfig.Attributes, input.Name+"@"+attribute)
							if attribute == "enabled" {
								MetricsConfig.Attributes[newInputName+"@"+newAttribute] = Attribute{Value: Union{tag: BOOL, b: true}, Name: newAttribute, InputVar: newInputName}
							} else {
								MetricsConfig.Attributes[newInputName+"@"+newAttribute] = Attribute{Value: Union{}, Name: newAttribute, InputVar: newInputName}
							}
							if _, ok := allPossibleAttributes[newAttribute]; !ok {
								allPossibleAttributes[newAttribute] = make([]string, 0)
							}
							allPossibleAttributes[newAttribute] = append(allPossibleAttributes[newAttribute], newInputName+"@"+newAttribute)

							indexToRemove := -1
							for oldIndex, inputATattribute := range allPossibleAttributes[attribute] {
								if inputATattribute == fmt.Sprintf("%s@%s", inputName, attribute) {
									indexToRemove = oldIndex
									break
								}
							}
							if indexToRemove >= 0 {
								allPossibleAttributes[attribute] = append(allPossibleAttributes[attribute][:indexToRemove], allPossibleAttributes[attribute][indexToRemove+1:]...)
								if len(allPossibleAttributes[attribute]) == 0 {
									delete(allPossibleAttributes, attribute)
								}
							}
						}
					}
					MetricsConfig.Inputs[newInputName] = newInput
					new_inputs = append(new_inputs, newInputName)
				}
				delete(MetricsConfig.Inputs, inputName)
				new_inputs = delete_string_at_index(new_inputs, i)
				i--
			}
			i++
		}
	}
	return new_inputs, wasWarning, wasError
}

func handleFiltersTemplates(new_filters []string, overwrite_conflicting_configs bool) ([]string, bool, bool) {
	wasWarning := false
	wasError := false
	for _, template := range MetricsConfig.Templates {
		new_filters_copy := make([]string, len(new_filters))
		copy(new_filters_copy, new_filters)
		i := 0
		for _, filterName := range new_filters_copy {
			filter := MetricsConfig.Filters[filterName]
			// it only really makes sense for filters to be templated by their variable name
			// otherwise, we would have repeated variable names
			if strings.Contains(filterName, template.Tok) {
				for _, replacement := range template.List {
					var wasErr bool
					newFilterName := strings.ReplaceAll(filterName, template.Tok, replacement)

					// check for duplicate filter/input names
					// this is much more likely to happen if we are careless with templating
					if _, ok := MetricsConfig.Inputs[newFilterName]; ok && !overwrite_conflicting_configs {
						// fatal error
						configErrorLocs.addKey("filters", simdjson.TypeObject)
						configErrorLocs.addKey(newFilterName, simdjson.TypeObject)
						configErrorLocs.logError(fmt.Errorf("templated filter variable '%s' also occurs as input variable; only considering input variable", newFilterName))
						configErrorLocs.removeLevel()
						configErrorLocs.removeLevel()
						wasError = true
						continue
					}
					if _, ok := MetricsConfig.Filters[newFilterName]; ok && !overwrite_conflicting_configs {
						// fatal error
						configErrorLocs.addKey("filters", simdjson.TypeObject)
						configErrorLocs.addKey(newFilterName, simdjson.TypeObject)
						configErrorLocs.logError(fmt.Errorf("duplicate filter variable '%s' when unraveling template; only considering first occurence", newFilterName))
						configErrorLocs.removeLevel()
						configErrorLocs.removeLevel()
						wasWarning = true
						continue
					}

					// copy over the templated filter with any templates removed
					switch x := filter.(type) {
					case string:
						newStrFilter := strings.ReplaceAll(x, template.Tok, replacement)
						MetricsConfig.Filters[newFilterName] = newStrFilter
						new_filters = append(new_filters, newFilterName)
					case []interface{}:
						newFilterArr := make([]interface{}, 0)
						for filterIndex, subFilter := range x {
							strFilter, ok := subFilter.(string)
							if ok {
								newStrFilter := strings.ReplaceAll(strFilter, template.Tok, replacement)
								newFilterArr = append(newFilterArr, newStrFilter)
							} else {
								configErrorLocs.addKey("filters", simdjson.TypeObject)
								configErrorLocs.addKey(newFilterName, simdjson.TypeArray)
								configErrorLocs.addIndex(filterIndex, simdjson.TypeString)
								configErrorLocs.logError(fmt.Errorf("could not convert subfilter to string"))
								wasErr = true
								configErrorLocs.removeLevel()
								configErrorLocs.removeLevel()
								configErrorLocs.removeLevel()
								wasError = true
								break
							}
						}
						if !wasErr {
							MetricsConfig.Filters[newFilterName] = newFilterArr
							new_filters = append(new_filters, newFilterName)
						}
					case []string:
						newFilterArr := make([]string, 0)
						for _, strFilter := range x {
							newStrFilter := strings.ReplaceAll(strFilter, template.Tok, replacement)
							newFilterArr = append(newFilterArr, newStrFilter)
						}
						if !wasErr {
							MetricsConfig.Filters[newFilterName] = newFilterArr
							new_filters = append(new_filters, newFilterName)
						}
					default:
						configErrorLocs.addKey("filters", simdjson.TypeObject)
						configErrorLocs.addKey(newFilterName, simdjson.TypeObject)
						configErrorLocs.logError(fmt.Errorf("invalid filter type %v", reflect.TypeOf(filter)))
						configErrorLocs.removeLevel()
						configErrorLocs.removeLevel()
						wasError = true
						continue
					}
				}
				delete(MetricsConfig.Filters, filterName)
				new_filters = delete_string_at_index(new_filters, i)
				i--
			}
			i++
		}
	}
	return new_filters, wasWarning, wasError
}

func handleOutputsTemplates(new_outputs []string) []string {
	for _, template := range MetricsConfig.Templates {
		new_outputs_copy := make([]string, len(new_outputs))
		copy(new_outputs_copy, new_outputs)
		i := 0
		for _, outputName := range new_outputs_copy {
			output := MetricsConfig.Outputs[outputName]
			// it only really makes sense for outputs to be templated by their variable name
			// otherwise, we would have repeated variable names
			// we also need to be aware that the templating also needs to occur in the
			// metrics objects simultaneously
			if strings.Contains(outputName, template.Tok) {
				for _, replacement := range template.List {
					newOutput := Output{}
					newOutput.PublishRate = output.PublishRate
					newOutputName := strings.ReplaceAll(outputName, template.Tok, replacement)
					newOutput.Uri = strings.ReplaceAll(output.Uri, template.Tok, replacement)
					newOutput.Name = strings.ReplaceAll(output.Name, template.Tok, replacement)
					newOutput.Attributes = make(map[string]interface{}, len(output.Attributes))
					newOutput.Enum = make([]EnumObject, len(output.Enum))
					newOutput.Bitfield = make([]EnumObject, len(output.Bitfield))
					newOutput.Flags = make([]string, len(output.Flags))
					if len(output.Attributes) > 0 {
						b, err := json.Marshal(output.Attributes)
						if err == nil {
							b2 := bytes.ReplaceAll(b, []byte(template.Tok), []byte(replacement))
							json.Unmarshal(b2, &(newOutput.Attributes))
						}
					}
					if len(output.Enum) > 0 {
						b, err := json.Marshal(output.Enum)
						if err == nil {
							b2 := bytes.ReplaceAll(b, []byte(template.Tok), []byte(replacement))
							json.Unmarshal(b2, &(newOutput.Enum))
						}
					}

					if len(output.Bitfield) > 0 {
						b, err := json.Marshal(output.Bitfield)
						if err == nil {
							b2 := bytes.ReplaceAll(b, []byte(template.Tok), []byte(replacement))
							json.Unmarshal(b2, &(newOutput.Bitfield))
						}
					}

					if len(output.Flags) > 0 {
						for p, flag := range output.Flags {
							newFlag := strings.ReplaceAll(flag, template.Tok, replacement)
							newOutput.Flags[p] = newFlag
						}
					}
					MetricsConfig.Outputs[newOutputName] = newOutput
					new_outputs = append(new_outputs, newOutputName)
				}
				delete(MetricsConfig.Outputs, outputName)
				new_outputs = delete_string_at_index(new_outputs, i)
				i--
			}
			i++
		}
	}
	return new_outputs
}

func handleMetricsTemplates(metricTmp MetricsObject) []MetricsObject {
	metricsObjects := make([]MetricsObject, 0)
	metricsObjects = append(metricsObjects, metricTmp)
	for _, template := range MetricsConfig.Templates {
		m := 0
		metricsCopy := make([]MetricsObject, len(metricsObjects))
		copy(metricsCopy, metricsObjects)
		for _, metric := range metricsCopy {
			// metrics can be templated by expression
			// (e.g. if you want the same expression to apply to multiple inputs - then sent to multiple outputs)
			// or by output (e.g. if you want the same value to get sent to multiple outputs)
			if strings.Contains(metric.Expression, template.Tok) {
				for _, replacement := range template.List {
					newMetric := MetricsObject{}
					if strings.Contains(metric.Id, template.Tok) {
						newMetric.Id = strings.ReplaceAll(metric.Id, template.Tok, replacement)
					} else {
						newMetric.Id = metric.Id + "_" + replacement
					}
					newMetric.InternalOutput = strings.ReplaceAll(metric.InternalOutput, template.Tok, replacement)
					newMetric.Expression = strings.ReplaceAll(metric.Expression, template.Tok, replacement)
					newMetric.Type = metric.Type
					newMetric.Outputs = make([]string, len(metric.Outputs))
					for p, output := range metric.Outputs {
						newOutput := strings.ReplaceAll(output, template.Tok, replacement)
						newMetric.Outputs[p] = newOutput
					}
					metricsObjects = append(metricsObjects, newMetric)
				}
				metricsObjects = append(metricsObjects[:m], metricsObjects[m+1:]...)
				m -= 1
			} else {
				o := 0
				for _, output := range metric.Outputs {
					if strings.Contains(output, template.Tok) {
						for _, replacement := range template.List {
							newOutput := strings.ReplaceAll(output, template.Tok, replacement)
							metricsObjects[m].Outputs = append(metricsObjects[m].Outputs, newOutput)
						}
						metricsObjects[m].Outputs = append(metricsObjects[m].Outputs[:o], metricsObjects[m].Outputs[o+1:]...)
						o -= 1
					}
					o += 1
				}
			}
			m += 1
		}
	}
	return metricsObjects
}

func handleEchoTemplates(new_echo_starting_index int) {
	if len(MetricsConfig.Echo) <= new_echo_starting_index {
		return
	}
	for _, template := range MetricsConfig.Templates {
		e := new_echo_starting_index
		echoCopy := make([]EchoObject, len(MetricsConfig.Echo))
		copy(echoCopy, MetricsConfig.Echo)
		for _, echoObject := range echoCopy[new_echo_starting_index:] {
			// metrics can be templated by expression
			// (e.g. if you want the same expression to apply to multiple inputs - then sent to multiple outputs)
			// or by output (e.g. if you want the same value to get sent to multiple outputs)
			var templateInput bool
			for _, input := range echoObject.Inputs {
				if strings.Contains(input.Uri, template.Tok) {
					templateInput = true
					break
				}
			}
			if strings.Contains(echoObject.PublishUri, template.Tok) {
				for _, replacement := range template.List {
					newEchoObject := EchoObject{PublishRate: echoObject.PublishRate, Format: echoObject.Format}
					newEchoObject.Echo = make(map[string]interface{}, len(echoObject.Echo))
					newEchoObject.PublishUri = strings.ReplaceAll(echoObject.PublishUri, template.Tok, replacement)
					newEchoObject.PublishRate = echoObject.PublishRate
					newEchoObject.Heartbeat = strings.ReplaceAll(echoObject.Heartbeat, template.Tok, replacement)
					newEchoObject.Format = strings.ReplaceAll(echoObject.Format, template.Tok, replacement)
					newEchoObject.Inputs = make([]EchoInput, len(echoObject.Inputs))
					for i, input := range echoObject.Inputs {
						newEchoObject.Inputs[i].Uri = strings.ReplaceAll(input.Uri, template.Tok, replacement)
						newEchoObject.Inputs[i].Registers = make(map[string]string, len(input.Registers))
						for key, value := range input.Registers {
							newKey := strings.ReplaceAll(key, template.Tok, replacement)
							newValue := strings.ReplaceAll(value, template.Tok, replacement)
							newEchoObject.Inputs[i].Registers[newKey] = newValue
							newEchoObject.Echo[newKey] = nil
						}
					}
					for key, value := range echoObject.Echo {
						newKey := strings.ReplaceAll(key, template.Tok, replacement)
						var newValue interface{}
						switch x := value.(type) {
						case string:
							newValue = strings.ReplaceAll(x, template.Tok, replacement)
						default:
							newValue = value
						}
						newEchoObject.Echo[newKey] = newValue
					}
					MetricsConfig.Echo = append(MetricsConfig.Echo, newEchoObject)
				}
				MetricsConfig.Echo = append(MetricsConfig.Echo[:e], MetricsConfig.Echo[e+1:]...)
				e -= 1
			} else if templateInput {
				o := 0
				echoKeysToDelete := make([]string, 0)
				for _, input := range echoObject.Inputs {
					if strings.Contains(input.Uri, template.Tok) {
						for _, replacement := range template.List {
							newEchoInput := EchoInput{}
							newEchoInput.Uri = strings.ReplaceAll(input.Uri, template.Tok, replacement)
							newEchoInput.Registers = make(map[string]string, len(input.Registers))
							for key, value := range input.Registers {
								newKey := strings.ReplaceAll(key, template.Tok, replacement)
								newValue := strings.ReplaceAll(value, template.Tok, replacement)
								newEchoInput.Registers[newKey] = newValue
							}
							MetricsConfig.Echo[e].Inputs = append(MetricsConfig.Echo[e].Inputs, newEchoInput)

							for key, value := range echoObject.Echo {
								if strings.Contains(key, template.Tok) && !stringInSlice(echoKeysToDelete, key) {
									echoKeysToDelete = append(echoKeysToDelete, key)
								}
								newKey := strings.ReplaceAll(key, template.Tok, replacement)
								var newValue interface{}
								switch x := value.(type) {
								case string:
									newValue = strings.ReplaceAll(x, template.Tok, replacement)
								default:
									newValue = value
								}
								MetricsConfig.Echo[e].Echo[newKey] = newValue
							}
						}
						for _, key := range echoKeysToDelete {
							delete(MetricsConfig.Echo[e].Echo, key)
						}
						MetricsConfig.Echo[e].Inputs = append(MetricsConfig.Echo[e].Inputs[:o], MetricsConfig.Echo[e].Inputs[o+1:]...)
						o -= 1
					}
					o += 1
				}
			} else {
				for i, input := range echoObject.Inputs {
					for key, value := range input.Registers {
						if strings.Contains(key, template.Tok) {
							for _, replacement := range template.List {
								newKey := strings.ReplaceAll(key, template.Tok, replacement)
								newValue := strings.ReplaceAll(value, template.Tok, replacement)
								MetricsConfig.Echo[e].Inputs[i].Registers[newKey] = newValue
							}
							delete(MetricsConfig.Echo[e].Inputs[i].Registers, key)
						}
					}
				}
				for key, value := range echoObject.Echo {
					if strings.Contains(key, template.Tok) {
						for _, replacement := range template.List {
							newKey := strings.ReplaceAll(key, template.Tok, replacement)
							var newValue interface{}
							switch x := value.(type) {
							case string:
								newValue = strings.ReplaceAll(x, template.Tok, replacement)
							default:
								newValue = value
							}
							MetricsConfig.Echo[e].Echo[newKey] = newValue
						}
						delete(MetricsConfig.Echo[e].Echo, key)
					}
				}
			}
			e += 1
		}

	}
}

// first pass at generating the scope; excludes filters
func generateScope(new_inputs []string) {
	if InputScope == nil {
		InputScope = make(map[string][]Union, 0)
	}
	if OutputScope == nil {
		OutputScope = make(map[string][]Union, 0)
	}
	for _, key := range new_inputs {
		input := MetricsConfig.Inputs[key]
		if _, ok := InputScope[key]; !ok {
			InputScope[key] = []Union{input.Value}
			if len(input.Attributes) > 0 {
				for _, attributeLoc := range input.AttributesMap {
					InputScope[attributeLoc] = []Union{MetricsConfig.Attributes[attributeLoc].Value}
				}
			}
		}
	}
}

// Configure the MetricsObject state and outputs with default values. Will hold the metricsMutex[netIndex]
// until configuration has completed. This function is used both during initial configuration, and during
// runtime when an expression is manually reset.
// metricsObject: The MetricsObject to configure
// netIndex: The index of the MetricsObject within the list of MetricsObjects
// return: any configuration warnings or errors that occurred
func (metricsObject *MetricsObject) configureStateAndOutputs() (warning string, err error) {
	// Setup locks
	netIndex := metricsObject.Idx
	metricsAlreadyConfigured := len(metricsMutex) > 0 && netIndex < len(metricsMutex)
	// Only lock if reconfiguring. Otherwise the lock will not have been configured and will not be usable yet
	if metricsAlreadyConfigured {
		metricsMutex[netIndex].Lock()
	}
	expressionNeedsEvalMutex.RLock()
	defer func() {
		expressionNeedsEvalMutex.RUnlock()
		if metricsAlreadyConfigured {
			metricsMutex[netIndex].Unlock()
		}
	}()

	// Mark the expression for reevaluation
	expressionNeedsEval[netIndex] = true

	// (Re)configure state
	metricsObject.State = make(map[string][]Union, 0)
	// Time-based operations that need regular evaluation to ensure they get the updates they need
	if strings.Contains(metricsObject.ParsedExpression.String, "Integrate") ||
		strings.Contains(metricsObject.ParsedExpression.String, "Time") ||
		strings.Contains(metricsObject.ParsedExpression.String, "Milliseconds") ||
		strings.Contains(metricsObject.ParsedExpression.String, "Pulse") ||
		strings.Contains(metricsObject.ParsedExpression.String, "Duration") {
		metricsObject.State["alwaysEvaluate"] = []Union{{tag: BOOL, b: true}}
	} else {
		metricsObject.State["alwaysEvaluate"] = []Union{{tag: BOOL, b: false}}
	}

	// populate the initialValues map
	outputUnion := Union{tag: metricsObject.Type}
	if metricsObject.ParsedExpression.ResultType != outputUnion.tag {
		if metricsObject.ParsedExpression.ResultType == STRING {
			containsFilter := false
			for _, varName := range metricsObject.ParsedExpression.Vars {
				if _, ok := MetricsConfig.Filters[varName]; ok {
					containsFilter = true
				}
			}
			if containsFilter { // filters only provide warnings because I'm not confident in the type-checking
				warning = fmt.Sprintf("metrics expression produces possible result type %v but gets cast to %v", metricsObject.ParsedExpression.ResultType, outputUnion.tag)
			} else { // if there are no filters, the type checking is pretty much certain - and we can't cast strings to other data types
				err = fmt.Errorf("metrics expression produces possible result type %v but gets cast to %v", metricsObject.ParsedExpression.ResultType, outputUnion.tag)
			}
		} else { // we can do an implicit type cast, but this may not be what we want to do...
			if metricsObject.Type != NIL {
				warning = fmt.Sprintf("metrics expression produces possible result type %v but gets cast to %v", metricsObject.ParsedExpression.ResultType, outputUnion.tag)
			}
		}
	}
	if metricsObject.ParsedExpression.ResultType != NIL && outputUnion.tag == NIL {
		outputUnion = Union{tag: metricsObject.ParsedExpression.ResultType}
	}

	metricsObject.State["value"] = []Union{outputUnion}
	// For alerts, track the pair of subexpressions to their associated messages
	// For example, when soc < 10, the user might configure "SOC {soc} is too low" to be reported by the metric
	// the messageExpression would then be "soc < 10" and the messageString would be "SOC {soc} is too low"
	// The messageStrings are raw string literals that will later have their {variables} substituted with their actual values in the parsedMessageStrings
	if metricsObject.Alert {
		metricsObject.State["activeMessages"] = make([]Union, 0)
		metricsObject.State["activeTimestamps"] = make([]Union, 0)
		metricsObject.State["previousExpressionValues"] = make([]Union, 0)
		metricsObject.State["messageExpressions"] = make([]Union, 0)
		metricsObject.State["messageStrings"] = make([]Union, 0)
		metricsObject.State["valueChanged"] = make([]Union, 0)
		for messageExpression, messageString := range metricsObject.Messages {
			// State holds a slice for each key rather than a map, so split the expressions, their values, and associated strings into separate
			// slices with corresponding indices
			metricsObject.State["previousExpressionValues"] = append(metricsObject.State["previousExpressionValues"], Union{tag: NIL})
			metricsObject.State["messageExpressions"] = append(metricsObject.State["messageExpressions"], Union{tag: STRING, s: messageExpression})
			metricsObject.State["messageStrings"] = append(metricsObject.State["messageStrings"], Union{tag: STRING, s: messageString})
			metricsObject.State["activeMessages"] = append(metricsObject.State["activeMessages"], Union{tag: STRING, s: ""})
			metricsObject.State["activeTimestamps"] = append(metricsObject.State["activeTimestamps"], Union{tag: STRING, s: ""})
			metricsObject.State["valueChanged"] = append(metricsObject.State["valueChanged"], Union{tag: BOOL, b: false})
		}
	}

	for _, outputVar := range metricsObject.Outputs { // metricsObject.Outputs is a list of output variable names
		output := MetricsConfig.Outputs[outputVar] // MetricsConfig.Outputs is a map[string]Output of output variable names to Output objects
		output.Value = outputUnion
		OutputScope[outputVar] = []Union{outputUnion}

		MetricsConfig.Outputs[outputVar] = output
		if stringInSlice(MetricsConfig.Outputs[outputVar].Flags, "direct_set") ||
			stringInSlice(MetricsConfig.Outputs[outputVar].Flags, "post") {
			for _, var_name := range metricsObject.ParsedExpression.Vars {
				inputYieldsDirectMsg[var_name] = true
				for input_name, filter_names := range inputToFilterExpression {
					for _, filter_name := range filter_names {
						if filter_name == var_name {
							inputYieldsDirectMsg[input_name] = true
						}
					}
				}
			}
		}
	}
	return
}

// Preprocess all of the expressions in the metrics document
// so that all we need to do is evaluate the expressions at runtime.
// currently modeling after https://github.com/crsmithdev/goexpr
func (metricsObject *MetricsObject) getExpression(internal_vars []string, netIndex int, configId string) (string, error) {
	var warning string
	if inputToMetricsExpression == nil {
		inputToMetricsExpression = make(map[string][]int, 0)
	}
	if expressionNeedsEval == nil {
		expressionNeedsEval = make(map[int]bool, len(MetricsConfig.Metrics))
	}
	if InputScope == nil {
		InputScope = make(map[string][]Union, 0)
		OutputScope = make(map[string][]Union, 0)
	}

	exp, err := Parse((*metricsObject).Expression, configId)
	if err != nil {
		return "", fmt.Errorf("could not parse expression: %w", err)
	}
	(*metricsObject).ParsedExpression = *exp

	if containedInValChanged == nil {
		containedInValChanged = make(map[string]bool, 0)
	}

	if inputYieldsDirectMsg == nil {
		inputYieldsDirectMsg = make(map[string]bool, 0)
	}

	for _, var_name := range exp.Vars {
		if _, ok := inputToMetricsExpression[var_name]; !ok {
			inputToMetricsExpression[var_name] = make([]int, 0)
			if MetricsConfig.Inputs[var_name].Internal && !stringInSlice(internal_vars, var_name) {
				warning = fmt.Sprintf("metrics expression uses internal_output var '%v' prior to its calculation; results displayed for this metric will lag behind '%v' by one cycle", var_name, var_name)
			}
			containedInValChanged[var_name] = false
			inputYieldsDirectMsg[var_name] = false
		}
		inputToMetricsExpression[var_name] = append(inputToMetricsExpression[var_name], netIndex)

		if strings.Contains((*metricsObject).ParsedExpression.String, "ValueChanged") || strings.Contains((*metricsObject).ParsedExpression.String, "OverTimescale") {
			containedInValChanged[var_name] = true
		}
	}

	return warning, err
}

func getAndParseFilters(new_filters []string, new_filters_starting_index int, new_inputs []string, overwrite_conflicting_configs bool, configId string) bool {
	wasError := false
	if inputToFilterExpression == nil {
		inputToFilterExpression = make(map[string][]string, 0)
	}
	if filterNeedsEval == nil {
		filterNeedsEval = make(map[string]bool, 0)
	}
	if staticFilterExpressions == nil {
		staticFilterExpressions = make(map[string]Filter, 0)
	}
	if dynamicFilterExpressions == nil {
		dynamicFilterExpressions = make(map[string]Filter, 0)
	}

	var filters map[string]Filter
	filters, wasError = ExtractFilters(MetricsConfig.Filters, new_filters, new_filters_starting_index, new_inputs, overwrite_conflicting_configs, configId)
	for key, filter := range filters {
		if len(filter.StaticFilterExpressions) == 0 && len(filter.DynamicFilterExpressions) == 0 {
			continue
		}
		if len(filter.StaticFilterExpressions) > 0 {
			staticFilterExpressions[key] = filter
		}
		if len(filter.DynamicFilterExpressions) > 0 {
			dynamicFilterExpressions[key] = filter
		}
		for _, dynamicInput := range filter.DynamicInputs[0] {
			if _, ok := inputToFilterExpression[dynamicInput]; !ok {
				inputToFilterExpression[dynamicInput] = make([]string, 0)
			}
			inputToFilterExpression[dynamicInput] = append(inputToFilterExpression[dynamicInput], key)
		}
		filterNeedsEval[key] = true
	}
	PutFiltersInOrder()
	return wasError
}

func combineFlags(outputName string, output *Output) {
	if PubUriFlags == nil {
		PubUriFlags = make(map[string][]string, 0)
	}
	if PublishUris == nil {
		PublishUris = make(map[string][]string, 0)
	}
	if outputToUriGroup == nil {
		outputToUriGroup = make(map[string]string, 0)
	}
	if uriIsIntervalSet == nil {
		uriIsIntervalSet = make(map[string]bool, 0)
	}
	if uriIsDirect == nil {
		uriIsDirect = make(map[string]map[string]bool, 0)
		uriIsDirect["set"] = make(map[string]bool)
		uriIsDirect["post"] = make(map[string]bool)
	}
	if uriToDirectMsgActive == nil {
		uriToDirectMsgActive = make(map[string]map[string]bool, 0)
		uriToDirectMsgActive["set"] = make(map[string]bool)
		uriToDirectMsgActive["post"] = make(map[string]bool)
	}
	if uriIsSparse == nil {
		uriIsSparse = make(map[string]bool, 0)
	}
	if uriHeartbeat == nil {
		uriHeartbeat = make(map[string]bool, 0)
	}
	if outputVarChanged == nil {
		outputVarChanged = make(map[string]bool, 0)
	}
	if noGetResponse == nil {
		noGetResponse = make(map[string]bool, 0)
	}
	if uriIsLonely == nil {
		uriIsLonely = make(map[string]bool, 0)
	}
	if _, ok := outputVarChanged[outputName]; !ok {
		outputVarChanged[outputName] = true
	}
	uriGroup := ""

	if len(output.Uri) > 0 {
		if stringInSlice(output.Flags, "lonely") {
			uriGroup = output.Uri + "[" + outputName + "]"
			uriIsLonely[uriGroup] = true
		} else if group, hasGroup := regexStringInSlice(output.Flags, `group\d+`); hasGroup {
			uriGroup = output.Uri + "[" + group + "]"
			uriIsLonely[uriGroup] = false
		} else {
			uriGroup = output.Uri
			uriIsLonely[uriGroup] = false
		}

		outputToUriGroup[outputName] = uriGroup

		if _, ok := PublishUris[uriGroup]; !ok {
			PublishUris[uriGroup] = make([]string, 0)
		}
		if !stringInSlice(PublishUris[uriGroup], outputName) {
			PublishUris[uriGroup] = append(PublishUris[uriGroup], outputName)
		}

		if _, ok := PubUriFlags[uriGroup]; !ok {
			PubUriFlags[uriGroup] = make([]string, 0)
		}
		PubUriFlags[uriGroup] = append(PubUriFlags[uriGroup], output.Flags...)
		PubUriFlags[uriGroup] = removeDuplicateValues(PubUriFlags[uriGroup])
		if stringInSlice(PubUriFlags[uriGroup], "interval_set") {
			uriIsIntervalSet[uriGroup] = true
			noGetResponse[output.Uri] = true
			noGetResponse[output.Uri+"/"+output.Name] = true
		} else {
			uriIsIntervalSet[uriGroup] = false
		}
		if stringInSlice(PubUriFlags[uriGroup], "sparse") {
			uriIsSparse[uriGroup] = true
		} else {
			uriIsSparse[uriGroup] = false
		}
		if stringInSlice(PubUriFlags[uriGroup], "no_heartbeat") {
			uriHeartbeat[uriGroup] = false
		} else {
			uriHeartbeat[uriGroup] = true
		}
		if stringInSlice(PubUriFlags[uriGroup], "direct_set") {
			uriIsDirect["set"][uriGroup] = true
			uriToDirectMsgActive["set"][uriGroup] = false
			noGetResponse[output.Uri] = true
			noGetResponse[output.Uri+"/"+output.Name] = true
		}
		if stringInSlice(PubUriFlags[uriGroup], "post") {
			uriIsDirect["post"][uriGroup] = true
			uriToDirectMsgActive["post"][uriGroup] = false
			noGetResponse[output.Uri] = true
			noGetResponse[output.Uri+"/"+output.Name] = true
		}
	}

}

func checkMixedFlags() (wasWarning bool) {
	uriGroup := ""
	configErrorLocs.addKey("outputs", simdjson.TypeObject)
	for outputName, output := range MetricsConfig.Outputs {
		configErrorLocs.addKey(outputName, simdjson.TypeObject)
		configErrorLocs.addKey("flags", simdjson.TypeArray)
		if len(output.Uri) > 0 {
			if stringInSlice(output.Flags, "lonely") {
				uriGroup = output.Uri + "[" + outputName + "]"
			} else if group, hasGroup := regexStringInSlice(output.Flags, `group\d+`); hasGroup {
				uriGroup = output.Uri + "[" + group + "]"
			} else {
				uriGroup = output.Uri
			}

			if stringInSlice(output.Flags, "naked") && stringInSlice(output.Flags, "clothed") {
				// this default is applied in main at runtime (because "clothed" appears first in the if-else statement)
				wasWarning = true
				configErrorLocs.logError(fmt.Errorf("output '%v' has format specified as both naked and clothed; defaulting to clothed", outputName))
			} else if stringInSlice(PubUriFlags[uriGroup], "clothed") && !stringInSlice(output.Flags, "clothed") && !stringInSlice(output.Flags, "naked") {
				wasWarning = true
				configErrorLocs.logError(fmt.Errorf("output '%v' has unspecified clothed/naked status; defaulting to clothed (warning only)", outputName))
			} else if stringInSlice(PubUriFlags[uriGroup], "naked") && !stringInSlice(output.Flags, "clothed") && !stringInSlice(output.Flags, "naked") {
				wasWarning = true
				configErrorLocs.logError(fmt.Errorf("output '%v' has unspecified clothed/naked status; defaulting to naked (warning only)", outputName))
			}
			if stringInSlice(output.Flags, "flat") && !stringInSlice(output.Flags, "lonely") {
				wasWarning = true
				configErrorLocs.logError(fmt.Errorf("output '%v' has missing flag 'lonely'; when using 'flat' the output must be 'lonely' as well; defaulting to nested formatting (warning only)", outputName))
			}
		}
		configErrorLocs.removeLevel()
		configErrorLocs.removeLevel()
	}
	configErrorLocs.removeLevel()
	return
}

func GetPubTickers(new_metrics_info NewMetricsInfo) NewMetricsInfo {
	metricsConfigMutex.Lock()
	new_outputs := new_metrics_info.new_outputs
	new_echo_starting_index := new_metrics_info.new_echo_starting_index
	first_initialization := false
	if tickers == nil {
		first_initialization = true
		tickers = make([](*time.Ticker), 0)
	}

	new_metrics_info.new_tickers_starting_index = len(tickers)

	if pubTickers == nil {
		pubTickers = make(map[string]int, 0)
	}
	globalPubRate := int64(1000)
	if first_initialization {
		if pubRate, okPubRate := MetricsConfig.Meta["publishRate"]; okPubRate {
			intPubRate, okInt := pubRate.(int64)
			if !okInt {
				floatPubRate, okFloat := pubRate.(float64)
				if okFloat && floatPubRate > 0.0 {
					globalPubTicker := time.NewTicker(time.Duration(int64(math.Round(floatPubRate * 1000000))))
					globalPubRate = int64(math.Round(floatPubRate * 1000000))
					tickers = append(tickers, globalPubTicker)
				} else {
					globalPubTicker := time.NewTicker(time.Duration(1000) * time.Millisecond)
					tickers = append(tickers, globalPubTicker)
				}
			} else if intPubRate > 0 {
				globalPubTicker := time.NewTicker(time.Duration(intPubRate) * time.Millisecond)
				globalPubRate = intPubRate
				tickers = append(tickers, globalPubTicker)
			} else {
				globalPubTicker := time.NewTicker(time.Duration(1000) * time.Millisecond)
				tickers = append(tickers, globalPubTicker)
			}
		} else {
			globalPubTicker := time.NewTicker(time.Duration(1000) * time.Millisecond)
			tickers = append(tickers, globalPubTicker)
		}
	}

	uriGroup := ""
	for _, outputName := range new_outputs {
		output := MetricsConfig.Outputs[outputName]
		if len(output.Uri) > 0 {
			if stringInSlice(output.Flags, "lonely") {
				PublishUris[output.Uri+"["+outputName+"]"] = []string{outputName}
				pubDataChanged[output.Uri+"["+outputName+"]"] = true
				uriGroup = output.Uri + "[" + outputName + "]"
			} else if group, hasGroup := regexStringInSlice(output.Flags, `group\d+`); hasGroup {
				if _, ok := PublishUris[output.Uri+"["+group+"]"]; ok {
					uriGroup = output.Uri + "[" + group + "]"
					if !stringInSlice(PublishUris[uriGroup], outputName) {
						PublishUris[uriGroup] = append(PublishUris[uriGroup], outputName)
					}
				} else {
					PublishUris[output.Uri+"["+group+"]"] = []string{outputName}
					pubDataChanged[output.Uri+"["+group+"]"] = true
					uriGroup = output.Uri + "[" + group + "]"
				}
			} else {
				if _, ok := PublishUris[output.Uri]; !ok {
					PublishUris[output.Uri] = make([]string, 0)
				}
				if !stringInSlice(PublishUris[output.Uri], outputName) {
					PublishUris[output.Uri] = append(PublishUris[output.Uri], outputName)
				}
				pubDataChanged[output.Uri] = true
				uriGroup = output.Uri
			}
			_, set_ok := uriIsDirect["set"][uriGroup]
			_, post_ok := uriIsDirect["post"][uriGroup]
			if !set_ok && !post_ok { // if it's not a direct message, then we need a pub ticker for it
				if stringInSlice(output.Flags, "lonely") && output.PublishRate > 0 && output.PublishRate != globalPubRate { // if it's lonely, create its own ticker
					pubTicker := time.NewTicker(time.Duration(output.PublishRate) * time.Millisecond)
					tickers = append(tickers, pubTicker)
					pubTickers[uriGroup] = len(tickers) - 1
				} else if output.PublishRate > 0 && output.PublishRate != globalPubRate {
					if _, ok := pubTickers[uriGroup]; !ok { // if it's in a group, use the group's ticker or create a new one if the group doesn't have one yet
						pubTicker := time.NewTicker(time.Duration(output.PublishRate) * time.Millisecond)
						tickers = append(tickers, pubTicker)
						pubTickers[uriGroup] = len(tickers) - 1
					}
				} else {
					if _, ok := pubTickers[uriGroup]; !ok {
						pubTickers[uriGroup] = 0 // use the "master" pub ticker
					}
				}
			}
		}
	}
	if tickerPubs == nil {
		tickerPubs = make(map[int][]string)
	}
	for pubUri, tickerIndex := range pubTickers {
		if _, ok := tickerPubs[tickerIndex]; !ok {
			tickerPubs[tickerIndex] = make([]string, 0)
		}
		if !stringInSlice(tickerPubs[tickerIndex], pubUri) {
			tickerPubs[tickerIndex] = append(tickerPubs[tickerIndex], pubUri)
		}
	}

	// not directly related, but this will allow for multithreading with different metrics expressions
	if metricsMutex == nil {
		metricsMutex = make([]sync.RWMutex, 0)
	}
	for len(metricsMutex) < len(MetricsConfig.Metrics) {
		metricsMutex = append(metricsMutex, *new(sync.RWMutex))
	}

	if new_echo_starting_index < len(MetricsConfig.Echo) {
		echoIndex := new_echo_starting_index
		for range MetricsConfig.Echo[new_echo_starting_index:] {
			MetricsConfig.Echo[echoIndex].Ticker = time.NewTicker(time.Duration(MetricsConfig.Echo[echoIndex].PublishRate) * time.Millisecond)
			echoIndex++
		}
	}

	EvaluateExpressions()

	for directMsgUriGroup := range uriToDirectMsgActive {
		directMsgMutex.Lock()
		uriToDirectMsgActive["set"][directMsgUriGroup] = false
		uriToDirectMsgActive["post"][directMsgUriGroup] = false
		directMsgMutex.Unlock()
	}
	metricsConfigMutex.Unlock()
	return new_metrics_info
}

func joinMaps(interface1, interface2 interface{}) map[string]interface{} {
	joinedMaps := map[string]interface{}{}
	map1, ok1 := interface1.(map[string]interface{})
	map2, ok2 := interface2.(map[string]interface{})
	if ok1 && ok2 {
		for key, value1 := range map1 {
			if value2, ok := map2[key]; !ok {
				joinedMaps[key] = value1
			} else {
				map1next, ok1 := value1.(map[string]interface{})
				map2next, ok2 := value2.(map[string]interface{})
				if ok1 && ok2 {
					joinedMaps[key] = joinMaps(map1next, map2next)
				} else if ok1 {
					map1next[""] = value2
					joinedMaps[key] = map1next
				} else if ok2 {
					map2next[""] = value1
					joinedMaps[key] = map2next
				}
			}
		}
		for key, value2 := range map2 {
			if _, ok := map1[key]; !ok {
				joinedMaps[key] = value2
			}
		}
		return joinedMaps
	} else if ok1 {
		map1[""] = interface2
		return map1
	} else if ok2 {
		map2[""] = interface1
		return map2
	} else {
		joinedMaps[""] = []interface{}{interface1, interface2}
		return joinedMaps
	}
}

func addUriFrags(completeUri string, uriFragList []string, uriMap map[string]interface{}) map[string]interface{} {
	uriFrag := uriFragList[0]
	if uriFrag == "" && len(uriFragList) > 1 {
		uriFragList = uriFragList[1:]
		uriFrag = uriFragList[0]
	}
	completeUri += "/" + uriFrag
	if _, ok := uriMap[uriFrag]; !ok {
		uriMap[uriFrag] = map[string]interface{}{}
	}

	uriMap2, ok := uriMap[uriFrag].(map[string]interface{})
	if ok && len(uriFragList) > 1 {
		uriMap2 = addUriFrags(completeUri, uriFragList[1:], uriMap2)
	}
	uriMap[uriFrag] = uriMap2
	if _, ok := UriElements[completeUri]; !ok {
		UriElements[completeUri] = uriMap2
	} else {
		UriElements[completeUri] = joinMaps(UriElements[completeUri], uriMap2)
	}
	return uriMap
}

func GetSubscribeUris(new_metrics_info NewMetricsInfo) {
	metricsConfigMutex.Lock()
	new_inputs := new_metrics_info.new_inputs
	new_echo_starting_index := new_metrics_info.new_echo_starting_index
	new_outputs := new_metrics_info.new_outputs
	first_initialization := false

	if MetricsConfig.Meta == nil { // shouldn't happen if things are properly initialized, but just in case...
		MetricsConfig.Meta = make(map[string]interface{}, 0)
	}

	if SubscribeUris == nil {
		first_initialization = true
		SubscribeUris = make([]string, 0)
	}
	if first_initialization {

		if len(ProcessName) > 0 {
			if ProcessName[0] == '/' {
				ProcessName = ProcessName[1:]
			}
		} else if metrics_name_interface, ok := MetricsConfig.Meta["name"]; ok {
			metrics_name, okStr := metrics_name_interface.(string)
			if okStr && metrics_name[0] == '/' {
				ProcessName = metrics_name[1:]
			} else if okStr {
				ProcessName = metrics_name
			} else {
				ProcessName = "go_metrics"
			}
		} else {
			ProcessName = "go_metrics"
		}
		MetricsConfig.Meta["name"] = ProcessName
		SubscribeUris = append(SubscribeUris, "/"+ProcessName)

	}

	// extract parent uris (so we know what to subscribe to)
	// and also make an easy way to "fetch" data into our input Unions
	if uriToInputNameMap == nil {

		uriToInputNameMap = make(map[string][]string, len(MetricsConfig.Inputs))

	}
	for _, key := range new_inputs {

		input := MetricsConfig.Inputs[key]

		if len(input.Uri) > 0 {
			SubscribeUris = append(SubscribeUris, GetParentUri(input.Uri))
			if _, ok := uriToInputNameMap[input.Uri]; !ok {
				uriToInputNameMap[input.Uri] = make([]string, 0)
			}
			uriToInputNameMap[input.Uri] = append(uriToInputNameMap[input.Uri], key)
		}
	}

	// each echo input should have a uri to subscribe to
	if uriToEchoObjectInputMap == nil {
		uriToEchoObjectInputMap = make(map[string]map[int]int, 0)
	}
	if echoPublishUristoEchoNum == nil {
		echoPublishUristoEchoNum = make(map[string]int, 0)
	}
	if echoOutputToInputNum == nil {
		echoOutputToInputNum = make(map[string]int, 0)
	}

	if new_echo_starting_index < len(MetricsConfig.Echo) {
		for i, echoObject := range MetricsConfig.Echo[new_echo_starting_index:] {
			echoIndex := new_echo_starting_index + i
			for inputIndex, echoInput := range echoObject.Inputs {
				if len(echoInput.Uri) > 0 {
					SubscribeUris = append(SubscribeUris, echoInput.Uri)
					if _, ok := uriToEchoObjectInputMap[echoInput.Uri]; !ok {
						uriToEchoObjectInputMap[echoInput.Uri] = make(map[int]int, 0)
					}
					uriToEchoObjectInputMap[echoInput.Uri][echoIndex] = inputIndex
					for echoRegister := range echoInput.Registers {
						echoOutputToInputNum[echoObject.PublishUri+"/"+echoRegister] = inputIndex
					}
				}
			}
			if len(echoObject.PublishUri) > 0 {
				SubscribeUris = append(SubscribeUris, echoObject.PublishUri)
				echoPublishUristoEchoNum[echoObject.PublishUri] = echoIndex
			}
		}
	}

	// do the same for outputs so that we can respond to "gets"
	if uriToOutputNameMap == nil {
		uriToOutputNameMap = make(map[string][]string, len(MetricsConfig.Outputs))
	}
	for _, outputName := range new_outputs {
		output := MetricsConfig.Outputs[outputName]

		if len(output.Uri) > 0 {
			if len(output.Name) > 0 {
				// It's possible for multiple output names to map to a single uri.
				// Store all of the collisions in a list so they can still be tracked rather than overwritten
				if _, ok := uriToOutputNameMap[output.Uri+"/"+output.Name]; !ok {
					uriToOutputNameMap[output.Uri+"/"+output.Name] = make([]string, 0)
				}
				uriToOutputNameMap[output.Uri+"/"+output.Name] = append(uriToOutputNameMap[output.Uri+"/"+output.Name], outputName)
			}
			// output.Name itself might be repeated, so we may overwrite another entry with this one due to the same URI being produced
			if output.Name != outputName {
				if _, ok := uriToOutputNameMap[output.Uri+"/"+outputName]; !ok {
					uriToOutputNameMap[output.Uri+"/"+outputName] = make([]string, 0)
				}
				uriToOutputNameMap[output.Uri+"/"+outputName] = append(uriToOutputNameMap[output.Uri+"/"+outputName], outputName)
			}

			// parent uri
			if _, ok := uriToOutputNameMap[output.Uri]; !ok {
				uriToOutputNameMap[output.Uri] = make([]string, 0)
			}
			uriToOutputNameMap[output.Uri] = append(uriToOutputNameMap[output.Uri], outputName)

			SubscribeUris = append(SubscribeUris, output.Uri)
		}
	}

	// remove duplicates so we don't double subscribe (not sure if this is necessary)
	SubscribeUris = removeDuplicateValues(SubscribeUris)
	SubscribeUris = squashUris(SubscribeUris)
	base_level := false
	for len(SubscribeUris) > 64 && !base_level { // current max number of subscriptions; may change over time, but there's currently no constant for it
		base_level = true
		subscribeUris2 := make([]string, 0)
		for _, uri := range SubscribeUris {
			subscribeUris2 = append(subscribeUris2, GetParentUri(uri))
		}
		for _, uri := range subscribeUris2 {
			if uri != GetParentUri(uri) {
				base_level = false
				break
			}
		}
		SubscribeUris = removeDuplicateValues(subscribeUris2)
		SubscribeUris = squashUris(SubscribeUris)
	}
	if base_level && len(SubscribeUris) > 64 {
		SubscribeUris = []string{"/"}
		log.Warnf("Attempted to subscribe to %d uris (max is 64). Subscribing to / instead.", len(SubscribeUris))
	}

	// map each parent uri to its children components that we'll need to fetch
	// from that parent uri

	if UriElements == nil {
		UriElements = make(map[string]interface{}, 0)
	}

	// Add hard-coded configuration endpoint
	configFrags := strings.Split("/configuration", "/")
	UriElements = addUriFrags("", configFrags, UriElements)

	// TODO: fix this
	// // quick add for quick startup
	// for _, inputName := range new_inputs {
	// 	input := MetricsConfig.Inputs[inputName]
	// 	parentUri := GetParentUri(input.Uri)
	// 	uriElement := GetUriElement(input.Uri)
	// 	if parentUriElement, ok := UriElements[parentUri]; !ok {
	// 		UriElements[parentUri] = map[string]interface{}{
	// 			uriElement: map[string]interface{}{},
	// 		}
	// 	} else if parentMap, ok := parentUriElement.(map[string]interface{}); ok {
	// 		if _, ok := parentMap[uriElement]; !ok {
	// 			parentMap[uriElement] = map[string]interface{}{}
	// 			UriElements[parentUri] = parentMap
	// 		}
	// 	}
	// }
	//

	// slow add because this can take awhile
	// go func(new_metrics_inputs []string) {
	for _, inputName := range new_inputs {
		input := MetricsConfig.Inputs[inputName]
		uriFrags := strings.Split(input.Uri, "/")
		UriElements = addUriFrags("", uriFrags, UriElements)

		if len(input.Attributes) > 0 {
			for _, attributeName := range input.Attributes {
				uriFrags2 := append(uriFrags, attributeName)
				UriElements = addUriFrags("", uriFrags2, UriElements)
			}
			for _, attributeName := range input.Attributes {
				uriFrags := strings.Split(input.Uri, "/")
				uriFrags[len(uriFrags)-1] = uriFrags[len(uriFrags)-1] + "@" + attributeName
				UriElements = addUriFrags("", uriFrags, UriElements)
			}
		}
	}
	// }(new_inputs)

	// do the same for echo inputs
	if new_echo_starting_index < len(MetricsConfig.Echo) {

		// TODO: Fix this
		// // quick add for quick startup
		// for _, echoObject := range MetricsConfig.Echo[new_echo_starting_index:] {
		// 	parentUri := echoObject.PublishUri
		// 	if parentUriElement, ok := UriElements[parentUri]; !ok {
		// 		tempMap := map[string]interface{}{}
		// 		for _, echoInput := range echoObject.Inputs {
		// 			if len(echoInput.Uri) > 0 {
		// 				for newVar := range echoInput.Registers {
		// 					tempMap[newVar] = map[string]interface{}{}
		// 				}
		// 			}
		// 		}
		// 		for echoVar := range echoObject.Echo {
		// 			tempMap[echoVar] = map[string]interface{}{}
		// 		}
		// 		UriElements[parentUri] = tempMap
		// 	} else if parentMap, ok := parentUriElement.(map[string]interface{}); ok {
		// 		for _, echoInput := range echoObject.Inputs {
		// 			if len(echoInput.Uri) > 0 {
		// 				for newVar := range echoInput.Registers {
		// 					if _, ok := parentMap[newVar]; !ok {
		// 						parentMap[newVar] = map[string]interface{}{}
		// 					}
		// 				}
		// 			}
		// 		}
		// 		for echoVar := range echoObject.Echo {
		// 			if _, ok := parentMap[echoVar]; !ok {
		// 				parentMap[echoVar] = map[string]interface{}{}
		// 			}
		// 		}
		// 		UriElements[parentUri] = parentMap

		// 	}
		// }

		// slow add because this can take awhile
		// go func(new_metrics_echo_starting_index int) {
		for _, echoObject := range MetricsConfig.Echo[new_echo_starting_index:] {
			publishUriFrags := strings.Split(echoObject.PublishUri, "/")
			for _, echoInput := range echoObject.Inputs {
				if len(echoInput.Uri) > 0 {
					uriFrags := strings.Split(echoInput.Uri, "/")
					for newVar, oldVar := range echoInput.Registers {
						uriFrags2 := append(uriFrags, oldVar)

						UriElements = addUriFrags("", uriFrags2, UriElements)

						uriFrags3 := append(publishUriFrags, newVar)
						UriElements = addUriFrags("", uriFrags3, UriElements)
					}
				}
			}
			for register := range echoObject.Echo {
				uriFrags2 := append(publishUriFrags, register)
				UriElements = addUriFrags("", uriFrags2, UriElements)
			}
		}
		// }(new_echo_starting_index)
	}

	// TODO: Fix this
	// do the same for outputs
	// map each parent uri to its children components that we'll need to fetch
	// from that parent uri
	// quick add for quick startup
	// for _, outputName := range new_outputs {
	// 	output := MetricsConfig.Outputs[outputName]
	// 	parentUri := output.Uri
	// 	uriElement := ""
	// 	if len(output.Name) > 0 {
	// 		uriElement = output.Name
	// 	} else {
	// 		uriElement = outputName
	// 	}
	// 	if parentUriElement, ok := UriElements[parentUri]; !ok {
	// 		UriElements[parentUri] = map[string]interface{}{
	// 			uriElement: map[string]interface{}{},
	// 		}
	// 	} else if parentMap, ok := parentUriElement.(map[string]interface{}); ok {
	// 		if _, ok := parentMap[uriElement]; !ok {
	// 			parentMap[uriElement] = map[string]interface{}{}
	// 			UriElements[parentUri] = parentMap
	// 		}
	// 	}
	// }

	// slow add because this can take awhile
	// go func(new_metrics_outputs []string) {
	for _, outputName := range new_outputs {
		output := MetricsConfig.Outputs[outputName]
		if len(output.Uri) > 0 {
			uriFrags := strings.Split(output.Uri, "/")
			if len(output.Name) > 0 {
				uriFrags = append(uriFrags, output.Name)
			} else {
				uriFrags = append(uriFrags, outputName)
			}
			UriElements = addUriFrags("", uriFrags, UriElements)

			if len(output.Attributes) > 0 {
				for attributeName := range output.Attributes {
					uriFrags = append(uriFrags, attributeName)
					UriElements = addUriFrags("", uriFrags, UriElements)
				}
			}
		}
	}
	// }(new_outputs)

	if first_initialization {
		iterMap = make(map[string]*simdjson.Iter, 0)
		objMap = make(map[string]*simdjson.Object, 0)
		elemMap = make(map[string]*simdjson.Iter, 0)
	}
	metricsConfigMutex.Unlock()
}

func setupConfigLogging(tempMeta map[string]interface{}, configId string) (wasWarning bool, wasError bool) {
	if logFileLoc, ok := MetricsConfig.Meta["log_file"]; ok {
		if len(log.ConfigFile) == 0 { // if it the location hasn't been set via command line options
			log.ConfigFile, ok = logFileLoc.(string)
			if !ok {
				configErrorLocs.addKey("meta", simdjson.TypeObject)
				configErrorLocs.addKey("log_file", simdjson.TypeString)
				configErrorLocs.logError(fmt.Errorf("value is not string"))
				configErrorLocs.removeLevel()
				configErrorLocs.removeLevel()
				wasWarning = true
			}
		}
	}

	if debug_interface, ok := tempMeta["debug"]; ok {
		debug, ok = debug_interface.(bool)
		if !ok {
			configErrorLocs.addKey("meta", simdjson.TypeObject)
			configErrorLocs.addKey("debug", simdjson.TypeBool)
			configErrorLocs.logError(fmt.Errorf("value is not bool"))
			configErrorLocs.removeLevel()
			configErrorLocs.removeLevel()
			wasWarning = true
		} else {
			MetricsConfig.Meta["debug"] = debug
			if debug {
				log.SetLogLevel([]interface{}{"debug"})
			}
		}
	}

	if debug_inputs == nil {
		debug_inputs = []string{}
	}
	if debug_interface, ok := tempMeta["debug_inputs"]; ok {
		debug_inputs_interface, ok := debug_interface.([]interface{})
		if !ok {
			configErrorLocs.addKey("meta", simdjson.TypeObject)
			configErrorLocs.addKey("debug_inputs", simdjson.TypeArray)
			configErrorLocs.logError(fmt.Errorf("value is not []string"))
			configErrorLocs.removeLevel()
			configErrorLocs.removeLevel()
			wasWarning = true
		} else {
			for i, debug_input := range debug_inputs_interface {
				input_str, ok_str := debug_input.(string)
				if !ok_str {
					configErrorLocs.addKey("meta", simdjson.TypeObject)
					configErrorLocs.addKey("debug_inputs", simdjson.TypeArray)
					configErrorLocs.addIndex(i, simdjson.TypeString)
					configErrorLocs.logError(fmt.Errorf("value is not string"))
					configErrorLocs.removeLevel()
					configErrorLocs.removeLevel()
					configErrorLocs.removeLevel()
					wasWarning = true
				} else {
					if len(configId) > 0 {
						input_str = input_str + "_" + configId
					}
					debug_inputs = append(debug_inputs, input_str)
				}
			}
			MetricsConfig.Meta["debug_inputs"] = debug_inputs
		}
	}

	if debug_filters == nil {
		debug_filters = []string{}
	}
	if debug_interface, ok := tempMeta["debug_filters"]; ok {
		debug_filters_interface, ok := debug_interface.([]interface{})
		if !ok {
			configErrorLocs.addKey("meta", simdjson.TypeObject)
			configErrorLocs.addKey("debug_filters", simdjson.TypeArray)
			configErrorLocs.logError(fmt.Errorf("value is not []string"))
			configErrorLocs.removeLevel()
			configErrorLocs.removeLevel()
			wasWarning = true
		} else {
			for i, debug_input := range debug_filters_interface {
				input_str, ok_str := debug_input.(string)
				if !ok_str {
					configErrorLocs.addKey("meta", simdjson.TypeObject)
					configErrorLocs.addKey("debug_filters", simdjson.TypeArray)
					configErrorLocs.addIndex(i, simdjson.TypeString)
					configErrorLocs.logError(fmt.Errorf("value is not string"))
					configErrorLocs.removeLevel()
					configErrorLocs.removeLevel()
					configErrorLocs.removeLevel()
					wasWarning = true
				} else {
					if len(configId) > 0 {
						input_str = input_str + "_" + configId
					}
					debug_filters = append(debug_filters, input_str)
				}
			}
			MetricsConfig.Meta["debug_filters"] = debug_filters
		}
	}

	if debug_outputs == nil {
		debug_outputs = []string{}
	}
	if debug_interface, ok := tempMeta["debug_outputs"]; ok {
		debug_outputs_interface, ok := debug_interface.([]interface{})
		if !ok {
			configErrorLocs.addKey("meta", simdjson.TypeObject)
			configErrorLocs.addKey("debug_outputs", simdjson.TypeArray)
			configErrorLocs.logError(fmt.Errorf("value is not []string"))
			configErrorLocs.removeLevel()
			configErrorLocs.removeLevel()
			wasWarning = true
		} else {
			for i, debug_input := range debug_outputs_interface {
				input_str, ok_str := debug_input.(string)
				if !ok_str {
					configErrorLocs.addKey("meta", simdjson.TypeObject)
					configErrorLocs.addKey("debug_outputs", simdjson.TypeArray)
					configErrorLocs.addIndex(i, simdjson.TypeString)
					configErrorLocs.logError(fmt.Errorf("value is not string"))
					configErrorLocs.removeLevel()
					configErrorLocs.removeLevel()
					configErrorLocs.removeLevel()
					wasWarning = true
				} else {
					if len(configId) > 0 {
						input_str = input_str + "_" + configId
					}
					debug_outputs = append(debug_outputs, input_str)
				}
			}
			MetricsConfig.Meta["debug_outputs"] = debug_outputs
		}
	}
	return
}

func verifyInputConfigLogging() (wasWarning bool, wasError bool) {
	if _, ok := MetricsConfig.Meta["debug_inputs"]; ok && debug {
		copy_debug_inputs := make([]string, len(debug_inputs))
		copy(copy_debug_inputs, debug_inputs)
		i := 0
		for _, debug_input := range copy_debug_inputs {
			if _, inputExists := MetricsConfig.Inputs[debug_input]; !inputExists {
				configErrorLocs.addKey("meta", simdjson.TypeObject)
				configErrorLocs.addKey("debug_inputs", simdjson.TypeArray)
				configErrorLocs.addIndex(i, simdjson.TypeString)
				configErrorLocs.logError(fmt.Errorf("debug input '%s' does not exist; no debug info for this variable will be displayed", debug_input))
				configErrorLocs.removeLevel()
				configErrorLocs.removeLevel()
				configErrorLocs.removeLevel()
				wasWarning = true
				if i >= 0 && len(debug_inputs)-1 >= i {
					debug_inputs = append(debug_inputs[:i], debug_inputs[i+1:]...)
					i -= 1
				}
			}
			i += 1
		}
	}
	debug_inputs = removeDuplicateValues(debug_inputs)
	MetricsConfig.Meta["debug_inputs"] = debug_inputs
	return
}

func verifyFilterConfigLogging() (wasWarning bool) {
	if _, ok := MetricsConfig.Meta["debug_filters"]; ok && debug {
		copy_debug_filters := make([]string, len(debug_filters))
		copy(copy_debug_filters, debug_filters)
		i := 0
		for _, debug_filter := range copy_debug_filters {
			if _, inputExists := MetricsConfig.Filters[debug_filter]; !inputExists {
				configErrorLocs.addKey("meta", simdjson.TypeObject)
				configErrorLocs.addKey("debug_filters", simdjson.TypeArray)
				configErrorLocs.addIndex(i, simdjson.TypeString)
				configErrorLocs.logError(fmt.Errorf("debug filter '%s' does not exist; no debug info for this variable will be displayed", debug_filter))
				configErrorLocs.removeLevel()
				configErrorLocs.removeLevel()
				configErrorLocs.removeLevel()
				wasWarning = true
				if i >= 0 && len(debug_filters)-1 >= i {
					debug_filters = append(debug_filters[:i], debug_filters[i+1:]...)
					i -= 1
				}
			}
			i += 1
		}
	}
	debug_filters = removeDuplicateValues(debug_filters)
	MetricsConfig.Meta["debug_filters"] = debug_filters
	return
}

func verifyOutputConfigLogging() (wasWarning bool) {
	if _, ok := MetricsConfig.Meta["debug_outputs"]; ok && debug {
		copy_debug_outputs := make([]string, len(debug_outputs))
		copy(copy_debug_outputs, debug_outputs)
		i := 0
		for _, debug_output := range copy_debug_outputs {
			if _, outputExists := MetricsConfig.Outputs[debug_output]; !outputExists {

				configErrorLocs.addKey("meta", simdjson.TypeObject)
				configErrorLocs.addKey("debug_outputs", simdjson.TypeArray)
				configErrorLocs.addIndex(i, simdjson.TypeString)
				configErrorLocs.logError(fmt.Errorf("debug output '%s' does not exist; no debug info for this variable will be displayed", debug_output))
				configErrorLocs.removeLevel()
				configErrorLocs.removeLevel()
				configErrorLocs.removeLevel()
				wasWarning = true
				if i >= 0 && len(debug_outputs)-1 >= i {
					debug_outputs = append(debug_outputs[:i], debug_outputs[i+1:]...)
					i -= 1
				}
			}
			i += 1
		}
	}
	debug_outputs = removeDuplicateValues(debug_outputs)
	MetricsConfig.Meta["debug_outputs"] = debug_outputs
	return
}

func checkUnusedOutputs() (wasError bool) {
	for outputName := range MetricsConfig.Outputs {
		used := false
		for i := range MetricsConfig.Metrics {
			if stringInSlice(MetricsConfig.Metrics[i].Outputs, outputName) {
				used = true
				break
			}
		}
		if !used {
			configErrorLocs.addKey("outputs", simdjson.TypeObject)
			configErrorLocs.addKey(outputName, simdjson.TypeObject)
			configErrorLocs.logError(fmt.Errorf("output '%v' is never set by a metrics expression; excluding from published outputs", outputName))
			configErrorLocs.removeLevel()
			configErrorLocs.removeLevel()
			wasError = true
			delete(MetricsConfig.Outputs, outputName)
		}
	}
	return
}

// Map the Output Variables to their associated metrics
// Return any warning strings or errors that occurred
func mapOutputsToMetrics(new_metrics_starting_index int) (warnings []string, errs []error) {
	if len(MetricsConfig.Metrics) == new_metrics_starting_index {
		return
	}

	outputToMetricsObject = make(map[string][]*MetricsObject, 0)
	for i, metricsObject := range MetricsConfig.Metrics {
		MetricsConfig.Metrics[i].Idx = i
		metricsObjectPointer := &MetricsConfig.Metrics[i]
		// Map the expression to all of its associated outputs
		for _, output := range metricsObject.Outputs {
			// Use the associated expression index (netIndex) to point to the expression
			// An expression can have multiple outputs and each output can have multiple expressions (but only one is used currently)
			outputToMetricsObject[output] = append(outputToMetricsObject[output], metricsObjectPointer)
		}
	}

	// only configure the state for the new metrics
	for i := range MetricsConfig.Metrics[new_metrics_starting_index:] {
		idx := new_metrics_starting_index + i
		metricsObjectPointer := &MetricsConfig.Metrics[idx]
		warning, err := metricsObjectPointer.configureStateAndOutputs()
		warnings = append(warnings, warning)
		errs = append(errs, err)
	}
	return
}
