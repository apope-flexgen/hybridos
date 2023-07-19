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
		return fmt.Sprintf("%s", e.Key)
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
	for _, accessor := range e.JsonLocation[0 : len(e.JsonLocation)-1] {
		str += fmt.Sprintf("\"%v\", ", accessor)
	}
	str += fmt.Sprintf("\"%v\"],\n        \"Error\": \"%v\"\n    }", e.JsonLocation[len(e.JsonLocation)-1], e.JsonError)
	return str
}

type ErrorLocations struct {
	ErrorLocs []ErrorLocation
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
					str += fmt.Sprintf("%s", accessor.Key)
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

var configErrorLocs ErrorLocations

func UnmarshalConfig(data []byte) {
	currentJsonLocation := make([]JsonAccessor, 0)
	configErrorLocs.ErrorLocs = make([]ErrorLocation, 0)
	MetricsConfig = *new(MetricsFile)
	MetricsConfig.Meta = make(map[string]interface{}, 0)
	MetricsConfig.Inputs = make(map[string]Input, 0)
	MetricsConfig.Attributes = make(map[string]Attribute, 0)
	MetricsConfig.Filters = make(map[string]interface{}, 0)
	MetricsConfig.Outputs = make(map[string]Output, 0)
	MetricsConfig.Metrics = make([]MetricsObject, 0)
	MetricsConfig.Echo = make([]EchoObject, 0)

	pj, err := simdjson.Parse(data, nil)
	if err != nil {
		log.Fatalf("%v", err)
	}

	// this is where the main config processing happens
	err = pj.ForEach(func(i simdjson.Iter) error {
		wasError := false
		if i.Type() != simdjson.TypeObject {
			log.Fatalf("Unexpected JSON format for config. Must be a json object containing meta data, inputs, filters (optional), outputs, and expressions.")
		}

		// handle meta data (map[string]interface{}; optional)
		element, internal_err := i.FindElement(nil, "meta")
		if internal_err == nil {
			currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "meta", JType: simdjson.TypeObject})
			metaObject, internal_err := element.Iter.Object(nil)
			if internal_err != nil {
				logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
				wasError = true
			} else {
				MetricsConfig.Meta, internal_err = metaObject.Map(MetricsConfig.Meta)
				if internal_err != nil { // not sure if we can technically get here...
					logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
					wasError = true
				}
			}
			currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
		}
		setupConfigLogging()

		// handle templating ([]Template; optional)
		element, internal_err = i.FindElement(nil, "templates")
		if internal_err == nil {
			MetricsConfig.Templates = make([]Template, 0)
			currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "templates", JType: simdjson.TypeArray})
			templateArray, internal_err := element.Iter.Array(nil)
			if internal_err != nil {
				logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
				wasError = true
			} else {
				// for each template object
				templateIndex := 0
				templateArray.ForEach(func(templateIter simdjson.Iter) {
					currentJsonLocation = append(currentJsonLocation, JsonAccessor{Index: templateIndex, JType: simdjson.TypeObject})
					template := Template{}
					_, internal_err = templateIter.Object(nil)
					if internal_err != nil {
						logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
						wasError = true
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove template index
						templateIndex += 1
						return
					} else { // handle the template object
						// get the value
						element, internal_err = templateIter.FindElement(nil, "type")
						currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "type", JType: simdjson.TypeString})
						if internal_err != nil {
							currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "type"
							// check if there's a "from" specifier, which indicates sequential templating
							element, internal_err = templateIter.FindElement(nil, "from")
							if internal_err != nil {
								// check if there's a "list" specifier, which indicates list templating
								element, internal_err = templateIter.FindElement(nil, "list")
								if internal_err != nil {
									logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("could not identify template type; need either from/to pair or list"))
									wasError = true
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove template index
									templateIndex += 1
									return
								} else {
									currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "list", JType: simdjson.TypeArray})
									var listArray *simdjson.Array
									listArray, internal_err = element.Iter.Array(nil)
									if internal_err != nil {
										logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
										wasError = true
										currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove list
										currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove template index
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
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
								}
							} else {
								currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "from", JType: simdjson.TypeInt})
								template.From, internal_err = element.Iter.Int()
								if internal_err != nil {
									logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
									wasError = true
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "from"
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove template index
									templateIndex += 1
									return
								} else {
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "from"
									element, internal_err = templateIter.FindElement(nil, "to")
									currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "to", JType: simdjson.TypeInt})
									if internal_err != nil {
										logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
										wasError = true
										currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "to"
										currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove template index
										templateIndex += 1
										return
									} else {
										template.To, internal_err = element.Iter.Int()
										if internal_err != nil {
											logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
											wasError = true
											currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "to"
											currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove template index
											templateIndex += 1
											return
										} else {
											currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "to"
											element, internal_err = templateIter.FindElement(nil, "step")
											currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "step", JType: simdjson.TypeInt})
											if internal_err == nil {
												template.Step, internal_err = element.Iter.Int()
												if internal_err != nil {
													logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
													wasError = true
													currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "step"
													currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove template index
													templateIndex += 1
													return
												} else if template.Step == 0 {
													logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("cannot have template step of 0; defaulting to a step of 1"))
													wasError = true
													template.Step = 1
												}
											} else {
												template.Step = 1
											}
											currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "step"
											element, internal_err = templateIter.FindElement(nil, "format")
											currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "format", JType: simdjson.TypeString})
											if internal_err == nil {
												template.Format, internal_err = element.Iter.StringCvt()
												if internal_err != nil {
													logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("Invalid format specifier %s; defaulting to %%d", template.Format))
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
													logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("Invalid format specifier %s; defaulting to %%d", template.Format))
													wasError = true
													template.Format = "%d"
												}
											} else {
												template.Format = "%d"
											}
											currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "format"
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
								logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
								wasError = true
								currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove type
								currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove template index
								templateIndex += 1
								return
							} else {
								if template.Type == "sequential" {
									element, internal_err = templateIter.FindElement(nil, "from")
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "type"
									currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "from", JType: simdjson.TypeInt})
									if internal_err != nil {
										logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
										wasError = true
										currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "from"
										currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove template index
										templateIndex += 1
										return
									} else {
										template.From, internal_err = element.Iter.Int()
										if internal_err != nil {
											logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
											wasError = true
											currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "from"
											currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove template index
											templateIndex += 1
											return
										} else {
											element, internal_err = templateIter.FindElement(nil, "to")
											currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "from"
											currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "to", JType: simdjson.TypeInt})
											if internal_err != nil {
												logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
												wasError = true
												currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "to"
												currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove template index
												templateIndex += 1
												return
											} else {
												template.To, internal_err = element.Iter.Int()
												if internal_err != nil {
													logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
													wasError = true
													currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "to"
													currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove template index
													templateIndex += 1
													return
												} else {
													element, internal_err = templateIter.FindElement(nil, "step")
													currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "to"
													currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "step", JType: simdjson.TypeInt})
													if internal_err == nil {
														template.Step, internal_err = element.Iter.Int()
														if internal_err != nil {
															logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
															wasError = true
															template.Step = 1
														} else if template.Step == 0 {
															logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("cannot have template step of 0; defaulting to a step of 1"))
															wasError = true
															template.Step = 1
														}
													} else {
														template.Step = 1
													}
													currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "step"
													element, internal_err = templateIter.FindElement(nil, "format")
													currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "format", JType: simdjson.TypeString})
													if internal_err == nil {
														template.Format, internal_err = element.Iter.StringCvt()
														if internal_err != nil {
															logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("Invalid format specifier %s; defaulting to %%d", template.Format))
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
															logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("Invalid format specifier %s; defaulting to %%d", template.Format))
															wasError = true
															template.Format = "%d"
														}
													} else {
														template.Format = "%d"
													}
													currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "format"
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
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "type"
									currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "list", JType: simdjson.TypeArray})
									if internal_err != nil {
										logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
										wasError = true
										currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "from"
										currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove template index
										templateIndex += 1
										return
									} else {
										var listArray *simdjson.Array
										listArray, internal_err = element.Iter.Array(nil)
										if internal_err != nil {
											logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
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
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "list"
								} else {
									logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("unexpected template type %v: need \"sequential\" or \"list\"", template.Type))
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "type"
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove template index
									templateIndex += 1
									return
								}
							}
						}

						// get the token (required)
						element, internal_err = templateIter.FindElement(nil, "token")
						currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "token", JType: simdjson.TypeString})
						if internal_err != nil {
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
							wasError = true
							currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "token"
							currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove template index
							templateIndex += 1
							return
						} else {
							template.Tok, internal_err = element.Iter.String()
							if internal_err != nil {
								logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
								wasError = true
								currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "token"
								currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove template index
								templateIndex += 1
								return
							} else if strings.Contains(template.Tok, "@") {
								logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("template tokens cannot contain '@' symbol; symbol is reserved for attributes"))
								wasError = true
								currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "token"
								currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove template index
								templateIndex += 1
								return
							} else if len(template.Tok) == 0 {
								logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("template tokens cannot be empty strings"))
								wasError = true
								currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "token"
								currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove template index
								templateIndex += 1
								return
							} else {
								for other_template_index, other_template := range MetricsConfig.Templates {
									if strings.Contains(other_template.Tok, template.Tok) {
										logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("template %v contains template %v's token in its entirety; note that neither template may behave as desired", other_template_index, templateIndex))
										wasError = true
									} else if strings.Contains(template.Tok, other_template.Tok) {
										logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("template %v contains template %v's token in its entirety; note that neither template may behave as desired", templateIndex, other_template_index))
										wasError = true
									}
								}
							}
						}
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "token"
					}
					MetricsConfig.Templates = append(MetricsConfig.Templates, template)
					templateIndex += 1
					currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove template index
				})
			}
			currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "templates"
		}

		// handle inputs (map[string]Input; technically optional, but it will probably be rare not to have any)
		element, internal_err = i.FindElement(nil, "inputs")
		if internal_err == nil {
			currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "inputs", JType: simdjson.TypeObject})
			inputObject, internal_err := element.Iter.Object(nil)
			if internal_err != nil {
				logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
				wasError = true
			} else {
				// for each input
				inputObject.ForEach(func(key []byte, inputIter simdjson.Iter) {
					currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: string(key), JType: simdjson.TypeObject})
					input := Input{}
					input.Name = string(key)

					// check for duplicate input names
					// this shouldn't happen in a valid json document but it's good to check
					if _, ok := MetricsConfig.Inputs[input.Name]; ok {
						// fatal error for input
						logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("duplicate input variable '%s'; only considering first occurence", input.Name))
						wasError = true
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "input_var_name"
						return
					}

					// get the uri
					element, internal_err = inputIter.FindElement(nil, "uri")
					currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "uri", JType: simdjson.TypeString})
					if internal_err != nil {
						// if there's no uri, check to see if it's an internal variable (we'll handle this later, so we're just checking for now...)
						element, internal_err = inputIter.FindElement(nil, "internal")
						if internal_err != nil {
							// fatal error for input
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
							wasError = true
							currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "uri"
							currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "input_var_name"
							return
						}
					} else {
						input.Uri, internal_err = element.Iter.String()
						if internal_err != nil {
							// fatal error for input
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
							wasError = true
							currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "uri"
							currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "input_var_name"
							return
						}
					}
					currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "uri"

					// check if internal variable
					element, internal_err = inputIter.FindElement(nil, "internal")
					currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "internal", JType: simdjson.TypeBool})
					if internal_err == nil {
						input.Internal, internal_err = element.Iter.Bool()
						if internal_err != nil {
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
							wasError = true
							if len(input.Uri) == 0 {
								// fatal error for input
								currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "internal"
								currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "input_var_name"
								return
							}
						}
						if input.Internal == false && len(input.Uri) == 0 {
							// fatal error for input
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("key 'internal' is specified as false but 'uri' field is empty; need one or the other"))
							wasError = true
							currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "internal"
							currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "input_var_name"
							return
						} else if input.Internal && len(input.Uri) > 0 {
							// currently, my thinking that this is a better default if both are specified
							// since someone might say "uri": "none"
							// (not fatal)
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("key 'internal' is specified as true but 'uri' field contains a uri; defaulting to internally calculated value"))
							wasError = true
							input.Uri = ""
						}
					}
					currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "internal"

					// get the data type
					element, internal_err = inputIter.FindElement(nil, "type")
					currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "type", JType: simdjson.TypeString})
					if internal_err != nil {
						// fatal error for input
						logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
						wasError = true
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "type"
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "input_var_name"
						return
					} else {
						input.Type, internal_err = element.Iter.String()
						if internal_err != nil {
							// fatal error for input
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
							wasError = true
							currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "type"
							currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "input_var_name"
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
						default:
							// fatal error for input
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("invalid data type %v specified for input; must be string, bool, float, int, or uint", input.Type))
							wasError = true
							currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "type"
							currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "input_var_name"
							return
						}
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "type"

						//the user can also specify an initial value
						element, internal_err = inputIter.FindElement(nil, "default")
						currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "default", JType: simdjson.TypeString})
						if internal_err == nil {
							elementInterface, internal_err := element.Iter.Interface()
							if internal_err != nil { // not sure if we can get here...
								logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
								wasError = true
							} else {
								input.Value = castValueToUnionType(elementInterface, input.Value.tag)
							}
						}
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "default"
					}

					// get any attributes (optional)
					element, internal_err = inputIter.FindElement(nil, "attributes")
					if internal_err == nil {
						var attributesArray *simdjson.Array
						attributesArray, internal_err = element.Iter.Array(nil)
						currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "attributes", JType: simdjson.TypeArray})
						if internal_err != nil {
							// not fatal
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
							wasError = true
						} else {
							input.Attributes = make([]string, 0)
							input.AttributesMap = make(map[string]string)
							attributeIndex := 0
							attributesArray.ForEach(func(attributeIter simdjson.Iter) {
								var attribute string
								attribute, internal_err = attributeIter.String()
								currentJsonLocation = append(currentJsonLocation, JsonAccessor{Index: attributeIndex, JType: simdjson.TypeString})
								if internal_err != nil {
									logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
									wasError = true
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
								currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove specific attribute
								attributeIndex += 1
							})
						}
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "attributes"
					}

					MetricsConfig.Inputs[string(key)] = input
					currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
				}, nil)
			}
			currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "inputs"
		}
		handleInputsTemplates()
		generateScope()
		verifyInputConfigLogging()

		// handle filters (map[string]interface{}; optional)
		element, internal_err = i.FindElement(nil, "filters")
		if internal_err == nil {
			currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "filters", JType: simdjson.TypeObject})
			metaObject, internal_err := element.Iter.Object(nil)
			if internal_err != nil {
				logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
				wasError = true
			} else {
				MetricsConfig.Filters, internal_err = metaObject.Map(MetricsConfig.Filters)
				if internal_err != nil { // don't know if you can technically end up in here..
					logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
					wasError = true
				}
			}
			currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
		}
		handleFiltersTemplates()
		if getAndParseFilters() { // getAndParseFilters returns true if there was an error
			wasError = true
		}
		verifyFilterConfigLogging()

		// handle outputs (map[string]Output; technically optional, but it will probably be rare not to have any)
		element, internal_err = i.FindElement(nil, "outputs")
		PublishUris = make(map[string][]string, 0)
		pubDataChanged = make(map[string]bool, 0)
		if internal_err == nil {
			currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "outputs", JType: simdjson.TypeObject})
			outputObject, internal_err := element.Iter.Object(nil)
			if internal_err != nil {
				logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
				wasError = true
			} else {
				// for each output
				outputObject.ForEach(func(key []byte, outputIter simdjson.Iter) {
					currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: string(key), JType: simdjson.TypeObject})
					output := Output{}

					// check for duplicate output names
					// this shouldn't happen in a valid json document but it's good to check
					if _, ok := MetricsConfig.Outputs[string(key)]; ok {
						// fatal error for output
						logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("duplicate output variable '%s'; only considering first occurence", string(key)))
						wasError = true
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "output_var_name"
						return
					}

					// get any flags (optional)
					element, internal_err = outputIter.FindElement(nil, "flags")
					output.Flags = make([]string, 0)
					if internal_err == nil {
						var flagsArr *simdjson.Array
						flagsArr, internal_err = element.Iter.Array(nil)
						currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "flags", JType: simdjson.TypeArray})
						if internal_err != nil {
							// non-fatal error; just skip adding flags
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
							wasError = true
						} else {
							flagIndex := 0
							flagsArr.ForEach(func(flagIter simdjson.Iter) {
								var flag string
								flag, internal_err = flagIter.String()
								currentJsonLocation = append(currentJsonLocation, JsonAccessor{Index: flagIndex, JType: simdjson.TypeString})
								if internal_err != nil {
									// non-fatal error; just skip adding this particular flag
									logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
									wasError = true
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove flag index
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
										logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("invalid output flag '%v'", flag))
										wasError = true
										currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove flag index
										return
									}
									output.Flags = append(output.Flags, flag)
								}
								currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove flag index
								flagIndex += 1
							})
						}
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "flags"
					}

					// get the uri (required)
					element, internal_err = outputIter.FindElement(nil, "uri")
					currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "uri", JType: simdjson.TypeString})
					if internal_err != nil {
						// fatal error for output
						logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
						wasError = true
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "uri"
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "output_var_name"
						return
					} else {
						output.Uri, internal_err = element.Iter.String()
						if internal_err != nil {
							// fatal error for output
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
							wasError = true
							currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "uri"
							currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "output_var_name"
							return
						}
					}
					currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "uri"

					// get the name if there is one
					element, internal_err = outputIter.FindElement(nil, "name")
					currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "name", JType: simdjson.TypeString})
					if internal_err == nil {
						output.Name, internal_err = element.Iter.String()
						if internal_err != nil {
							// non-fatal error; just log it if there is one
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
							wasError = true
						}
					}
					currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "name"

					// get the publishRate if there is one
					element, internal_err = outputIter.FindElement(nil, "publishRate")
					currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "publishRate", JType: simdjson.TypeInt})
					if internal_err == nil {
						output.PublishRate, internal_err = element.Iter.Int()
						if internal_err != nil {
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
							wasError = true
						} else if output.PublishRate <= 0 {
							// this default is applied later
							output.PublishRate = 0
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("publish rate must be greater than 0; defaulting to global publish rate"))
							wasError = true
						}
					}
					currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "publishRate"

					// add any attributes and their values
					element, internal_err = outputIter.FindElement(nil, "attributes")
					output.Attributes = make(map[string]interface{}, 0)
					if internal_err == nil {
						var attributesObj *simdjson.Object
						attributesObj, internal_err = element.Iter.Object(nil)
						currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "attributes", JType: simdjson.TypeObject})
						if internal_err != nil {
							// just ignore the attributes if there's an error reading them into an object
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
							wasError = true
						} else {
							attributesObj.ForEach(func(key []byte, objIter simdjson.Iter) {
								// if an error occurs, I believe that key will just show up as null in the message body
								output.Attributes[string(key)], _ = objIter.Interface()
							}, nil)
						}
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "attributes"
					}

					// determine if the output is an enum
					element, internal_err = outputIter.FindElement(nil, "enum")
					output.Enum = make([]EnumObject, 0)
					output.EnumMap = make(map[int]int, 0)
					if internal_err == nil && stringInSlice(output.Flags, "enum") {
						var enumArray *simdjson.Array
						enumArray, internal_err = element.Iter.Array(nil)
						currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "enum", JType: simdjson.TypeArray})
						if internal_err != nil {
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
							wasError = true
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
								currentJsonLocation = append(currentJsonLocation, JsonAccessor{Index: positionIndex, JType: simdjson.TypeObject})
								var enumObject EnumObject
								var simdjsonEnumObject *simdjson.Object
								simdjsonEnumObject, internal_err = enumIter.Object(nil)
								if internal_err == nil {
									element = simdjsonEnumObject.FindKey("value", nil)
									currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "value", JType: simdjson.TypeInt})
									if element != nil {
										enumObject.Value, internal_err = element.Iter.Int()
										if internal_err != nil {
											logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("%v; using default index of %d", internal_err, enumIndex))
											wasError = true
											enumObject.Value = enumIndex
										} else {
											enumIndex = enumObject.Value
										}
									} else {
										logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("path not found; using default index of %d", enumIndex))
										wasError = true
										enumObject.Value = enumIndex
									}
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)

									element = simdjsonEnumObject.FindKey("string", nil)
									currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "string", JType: simdjson.TypeString})
									if element != nil {
										enumObject.String, internal_err = element.Iter.StringCvt()
										if internal_err != nil {
											logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("%v; using default string of \"Unknown\"", internal_err))
											wasError = true
											enumObject.String = "Unknown"
										}
									} else {
										logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("path not found; using default string of \"Unknown\""))
										wasError = true
										enumObject.String = "Unknown"
									}
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
								} else {
									enumObject.String, internal_err = enumIter.StringCvt()
									enumObject.Value = enumIndex
									currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "string", JType: simdjson.TypeString})
									if internal_err != nil {
										logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("%v; using default index of %d and string of \"Unknown\"", internal_err, enumIndex))
										wasError = true
										enumObject.String = "Unknown"
									}
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
								}
								output.EnumMap[int(enumIndex)] = int(positionIndex)
								enumIndex = enumObject.Value + 1
								positionIndex += 1
								output.Enum = append(output.Enum, enumObject)
								currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
							})
						}
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
					} else if internal_err == nil && !stringInSlice(output.Flags, "enum") {
						logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("found 'enum' field, but no matching output flag"))
						wasError = true
					} else if stringInSlice(output.Flags, "enum") {
						logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("found 'enum' flag, but no matching 'enum' field"))
						wasError = true
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
						currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "bitfield", JType: simdjson.TypeArray})
						if internal_err != nil {
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
							wasError = true
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
								currentJsonLocation = append(currentJsonLocation, JsonAccessor{Index: positionIndex, JType: simdjson.TypeObject})
								var enumObject EnumObject
								var simdjsonEnumObject *simdjson.Object
								simdjsonEnumObject, internal_err = enumIter.Object(nil)
								if internal_err == nil {
									element = simdjsonEnumObject.FindKey("value", nil)
									currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "value", JType: simdjson.TypeInt})
									if element != nil {
										enumObject.Value, internal_err = element.Iter.Int()
										if internal_err != nil {
											logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("%v; using default index of %d", internal_err, enumIndex))
											wasError = true
											enumObject.Value = enumIndex
										} else if enumObject.Value != enumIndex {
											logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("%v; using default index of %d", "cannot skip values for bitfields", enumIndex))
											wasError = true
											enumObject.Value = enumIndex
										}
									} else {
										logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("path not found; using default index of %d", enumIndex))
										wasError = true
										enumObject.Value = enumIndex
									}
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)

									element = simdjsonEnumObject.FindKey("string", nil)
									currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "string", JType: simdjson.TypeString})
									if element != nil {
										enumObject.String, internal_err = element.Iter.StringCvt()
										if internal_err != nil {
											logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("%v; using default string of \"Unknown\"", internal_err))
											wasError = true
											enumObject.String = "Unknown"
										}
									} else {
										logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("path not found; using default string of \"Unknown\""))
										wasError = true
										enumObject.String = "Unknown"
									}
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
								} else {
									enumObject.String, internal_err = enumIter.StringCvt()
									enumObject.Value = enumIndex
									currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "string", JType: simdjson.TypeString})
									if internal_err != nil {
										logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("%v; using default index of %d and string of \"Unknown\"", internal_err, enumIndex))
										wasError = true
										enumObject.String = "Unknown"
									}
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
								}
								enumIndex = enumObject.Value + 1
								positionIndex += 1
								output.Bitfield = append(output.Bitfield, enumObject)
								currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
							})
						}
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
					} else if internal_err == nil && !stringInSlice(output.Flags, "bitfield") {
						logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("found 'bitfield' field, but no matching output flag"))
						wasError = true
					} else if stringInSlice(output.Flags, "bitfield") {
						logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("found 'bitfield' flag, but no matching 'bitfield' field"))
						wasError = true
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
					MetricsConfig.Outputs[string(key)] = output
					currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
				}, nil)

			}
			currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "outputs"
		}
		handleOutputsTemplates()
		for outputName, output := range MetricsConfig.Outputs {
			combineFlags(outputName, &output)
		}
		checkMixedFlags(currentJsonLocation)
		verifyOutputConfigLogging()

		// handle metrics ([]MetricsObject)
		element, internal_err = i.FindElement(nil, "metrics")
		if internal_err == nil {
			currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "metrics", JType: simdjson.TypeArray})
			metricsArr, internal_err := element.Iter.Array(nil)
			if internal_err != nil {
				logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
				wasError = true
			} else {
				// for each metrics object
				metricsIndex := 0
				internal_vars := make([]string, 0) // keep track of internal vars so we know if they're calculated before they're used
				metricsArr.ForEach(func(metricsIter simdjson.Iter) {
					currentJsonLocation = append(currentJsonLocation, JsonAccessor{Index: metricsIndex, JType: simdjson.TypeObject})
					metric := MetricsObject{}
					_, internal_err = metricsIter.Object(nil)
					if internal_err != nil {
						logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
						wasError = true
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "metricsIndex"
						metricsIndex += 1
						return
					} else { // handle the metrics object

						// get the id
						element, internal_err = metricsIter.FindElement(nil, "id")
						currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "id", JType: simdjson.TypeString})
						if internal_err != nil {
							metric.Id = fmt.Sprintf("%d", metricsIndex)
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("%v (warning only)", internal_err))
							wasError = true
						} else {
							metric.Id, internal_err = element.Iter.StringCvt()
							if internal_err != nil { // not sure if we can get here...
								logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
								wasError = true
							}
						}
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove id

						// get the value
						element, internal_err = metricsIter.FindElement(nil, "type")
						currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "type", JType: simdjson.TypeString})
						if internal_err != nil {
							metric.Type = NIL
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
							wasError = true
						} else {
							var typeString string
							typeString, internal_err = element.Iter.StringCvt()
							if internal_err != nil { // not sure if we can get here...
								logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
								wasError = true
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
								logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("unhandled output type %s; deferring to default output type", typeString))
								wasError = true
							}
						}
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove type

						// get the outputs (at least one is required)
						element, internal_err = metricsIter.FindElement(nil, "outputs")
						currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "outputs", JType: simdjson.TypeArray})
						if internal_err != nil {
							//we can also theoretically have an internal_output, so we can check for that if we don't find a regular output
							element, internal_err = metricsIter.FindElement(nil, "internal_output")
							if internal_err != nil {
								logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
								wasError = true
							}
						} else {
							var outputsArr *simdjson.Array
							outputsArr, internal_err = element.Iter.Array(nil)
							if internal_err != nil {
								var outputStr string
								outputStr, internal_err = element.Iter.StringCvt()
								if internal_err != nil { // not sure if we can get here...
									logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
									wasError = true
								} else {
									metric.Outputs = []string{outputStr}
								}
							} else {
								metric.Outputs = make([]string, 0)
								outputIndex := 0
								outputsArr.ForEach(func(outputIter simdjson.Iter) {
									var outputVar string
									outputVar, internal_err = outputIter.String()
									currentJsonLocation = append(currentJsonLocation, JsonAccessor{Index: outputIndex, JType: simdjson.TypeString})
									if internal_err != nil {
										logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
										wasError = true
									} else {
										metric.Outputs = append(metric.Outputs, outputVar)
									}
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
									outputIndex += 1
								})
							}
						}
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove outputs

						// check if there's an internal output
						element, internal_err = metricsIter.FindElement(nil, "internal_output")
						currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "internal_output", JType: simdjson.TypeString})
						if internal_err == nil {
							var internalOutput string
							internalOutput, internal_err = element.Iter.StringCvt()
							if internal_err != nil {
								logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
								wasError = true
							} else {
								metric.InternalOutput = internalOutput
							}
						}
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove internal_output

						// get the expression
						element, internal_err = metricsIter.FindElement(nil, "expression")
						currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "expression", JType: simdjson.TypeString})
						if internal_err != nil {
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
							wasError = true
							currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "expression"
							currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "metricsIndex"
							metricsIndex += 1
							return
						} else {
							var expr string
							expr, internal_err = element.Iter.String()
							if internal_err != nil {
								logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
								wasError = true
							} else {
								metric.Expression = expr
							}
						}
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "expression"

						// apply templating
						metricsObjects := handleMetricsTemplates(metric)

						// now that we've applied templating, check that the output vars are all valid
						// remove them if they're not
						currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "outputs", JType: simdjson.TypeArray})
						for idx, metric := range metricsObjects {
							outputIdx := 0
							metricOutputsCopy := make([]string, len(metric.Outputs))
							copy(metricOutputsCopy, metric.Outputs)
							for _, outputVar := range metricOutputsCopy {
								if _, ok := MetricsConfig.Outputs[outputVar]; !ok {
									if strings.Contains(outputVar, "@") {
										if _, ok := MetricsConfig.Outputs[strings.Split(outputVar, "@")[0]]; !ok {
											metricsObjects[idx].Outputs = append(metricsObjects[idx].Outputs[:outputIdx], metricsObjects[idx].Outputs[outputIdx+1:]...)
											outputIdx -= 1
											logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("output variable '%s' does not have a corresponding output config", outputVar))
											wasError = true
										}
									} else {
										metricsObjects[idx].Outputs = append(metricsObjects[idx].Outputs[:outputIdx], metricsObjects[idx].Outputs[outputIdx+1:]...)
										outputIdx -= 1
										logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("output variable '%s' does not have a corresponding output config", outputVar))
										wasError = true
									}
								}
								outputIdx += 1
							}
						}
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "outputs"

						// now that we've applied templating, check that the internal output is valid
						// remove it if not
						currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "internal_output", JType: simdjson.TypeString})
						for idx, metric := range metricsObjects {
							if len(metric.InternalOutput) > 0 {
								if _, ok := MetricsConfig.Inputs[metric.InternalOutput]; !ok {
									if strings.Contains(metric.InternalOutput, "@") {
										logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("cannot map internal_output variable to attribute"))
										wasError = true
									} else {
										logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("internal_output variable '%s' does not have a corresponding input config", metric.InternalOutput))
										wasError = true
									}
									metricsObjects[idx].InternalOutput = ""
								}
							}
						}
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // remove "internal_output"

						metricsObjectsIterationCopy := make([]MetricsObject, len(metricsObjects))
						copy(metricsObjectsIterationCopy, metricsObjects)
						idx := 0
						for _, metric := range metricsObjectsIterationCopy {
							if len(metric.InternalOutput) == 0 && len(metric.Outputs) == 0 {
								logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("after applying templating and variable checks, metric does not have valid internal_output or output variables; discarding metric"))
								wasError = true
								metricsObjects = append(metricsObjects[:idx], metricsObjects[idx+1:]...)
								idx -= 1
							}
							idx += 1
						}

						// preprocess all of the expressions in the metrics document
						// so that all we need to do is evaluate the expressions at runtime.
						// currently modeling after https://github.com/crsmithdev/goexpr
						currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "expression", JType: simdjson.TypeString})
						for _, metric := range metricsObjects {
							var warning string
							warning, internal_err = getExpression(&metric, internal_vars, len(MetricsConfig.Metrics))
							if len(warning) > 0 {
								logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("%v (warning only)", warning))
								wasError = true
							}
							if internal_err != nil {
								logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("%v; excluding this metric from calculations", internal_err))
								wasError = true
							} else {
								MetricsConfig.Metrics = append(MetricsConfig.Metrics, metric)
								internal_vars = append(internal_vars, metric.InternalOutput)
							}
						}
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
					}
					metricsIndex += 1
					currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete metrics index
				})

			}
			currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)

		}
		checkUnusedOutputs()

		// handle echos ([]EchoObject)
		element, internal_err = i.FindElement(nil, "echo")
		if internal_err == nil {
			currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "echo", JType: simdjson.TypeArray})
			echoArr, internal_err := element.Iter.Array(nil)
			if internal_err != nil {
				logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
				wasError = true
			} else {
				// for each echo object
				echoIndex := 0
				echoArr.ForEach(func(echoIter simdjson.Iter) {
					currentJsonLocation = append(currentJsonLocation, JsonAccessor{Index: echoIndex, JType: simdjson.TypeObject})
					echo := EchoObject{}

					_, internal_err = echoIter.Object(nil)
					if internal_err != nil {
						logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
						wasError = true
					} else { // handle the echo object
						// get the uri
						fatalErr := false
						echo.Inputs = make([]EchoInput, 0)
						echo.Echo = make(map[string]interface{}, 0)
						element, internal_err = echoIter.FindElement(nil, "uri")
						currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "uri", JType: simdjson.TypeString})
						if internal_err != nil {
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
							wasError = true
							fatalErr = true
						} else {
							echo.PublishUri, internal_err = element.Iter.StringCvt()
							if internal_err != nil {
								logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
								wasError = true
								fatalErr = true
							}
						}
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)

						// get the publish rate
						element, internal_err = echoIter.FindElement(nil, "publishRate")
						currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "publishRate", JType: simdjson.TypeString})
						if internal_err != nil {
							logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
							wasError = true
							fatalErr = true
						} else {
							echo.PublishRate, internal_err = element.Iter.Int()
							if internal_err != nil {
								logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
								wasError = true
								fatalErr = true
							}
						}
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)

						// get the heartbeat (optional)
						element, internal_err = echoIter.FindElement(nil, "heartbeat")
						currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "heartbeat", JType: simdjson.TypeString})
						if internal_err == nil {
							echo.Heartbeat, internal_err = element.Iter.StringCvt()
							if internal_err != nil {
								logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
								wasError = true
							}
						}
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)

						// get the format (optional)
						element, internal_err = echoIter.FindElement(nil, "format")
						currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "format", JType: simdjson.TypeString})
						if internal_err == nil {
							echo.Format, internal_err = element.Iter.StringCvt()
							if internal_err != nil {
								logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
								wasError = true
							}
						}
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)

						// get the inputs
						element, internal_err = echoIter.FindElement(nil, "inputs")
						if internal_err == nil {
							var inputsArr *simdjson.Array
							inputsArr, internal_err = element.Iter.Array(nil)
							currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "inputs", JType: simdjson.TypeArray})
							if internal_err != nil {
								logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
								wasError = true
							} else {
								echo.Inputs = make([]EchoInput, 0)
								inputIndex := 0
								inputsArr.ForEach(func(inputIter simdjson.Iter) {
									currentJsonLocation = append(currentJsonLocation, JsonAccessor{Index: inputIndex, JType: simdjson.TypeObject})
									var echoInput EchoInput
									echoInput.Registers = make(map[string]string, 0)
									element, internal_err = inputIter.FindElement(nil, "uri")
									currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "uri", JType: simdjson.TypeString})
									if internal_err != nil {
										logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
										wasError = true
										currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete uri
										currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete input index
										return
									} else {
										echoInput.Uri, internal_err = element.Iter.StringCvt()
										if internal_err != nil {
											logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
											wasError = true
											currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete uri
											currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete input index
											return
										}
									}
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete uri
									element, internal_err = inputIter.FindElement(nil, "registers")
									currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "registers", JType: simdjson.TypeObject})

									if internal_err != nil {
										logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
										wasError = true
										currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete uri
										currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete input index
										return
									} else {
										var temp interface{}
										temp, internal_err = element.Iter.Interface()

										if internal_err != nil {
											logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
											wasError = true
											currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete registers
											currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete input index
											return
										}
										var ok bool
										var tempInterface map[string]interface{}
										tempInterface, ok = temp.(map[string]interface{})
										if !ok {
											logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("could not convert registers to map[string]interface{}"))
											wasError = true
											currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete registers
											currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete input index
											return
										}

										for key, value := range tempInterface { // echo input registers can be either strings or objects with "source" and "default"
											switch value.(type) {
											case string:
												echoInput.Registers[key], _ = value.(string)
												echo.Echo[key] = nil
											case map[string]interface{}:
												sourceDefaultMap, _ := value.(map[string]interface{})
												source, okSource := sourceDefaultMap["source"]
												currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: key, JType: simdjson.TypeObject})
												currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "source", JType: simdjson.TypeString})
												if !okSource {
													logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("path not found"))
													wasError = true
													currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete register key
													currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete source
													continue
												}
												okStrSource := false
												echoInput.Registers[key], okStrSource = source.(string)
												if !okStrSource {
													logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("value is not string"))
													wasError = true
													currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete register key
													currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete source
													continue
												}
												currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete source
												currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "default", JType: simdjson.TypeNone})
												defaultVal, okDefault := sourceDefaultMap["default"]
												if !okDefault {
													logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("path not found"))
													wasError = true
													echo.Echo[key] = nil
												} else {
													echo.Echo[key] = defaultVal
												}
												currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete default
												currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete register key
											default:
												logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("could not convert registers to map[string]string"))
												wasError = true
												continue
											}
										}
										if len(echo.Heartbeat) > 0 {
											echo.Echo[echo.Heartbeat] = 0.0
										}
										for _, in := range echo.Inputs {
											for key, _ := range in.Registers {
												if _, ok = echoInput.Registers[key]; ok {
													logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("echo output key %v appears in more than one input register; excluding this input", key))
													wasError = true
													delete(echoInput.Registers, key)
												}
											}
										}
									}
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete registers
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete input index
									echo.Inputs = append(echo.Inputs, echoInput)
									inputIndex += 1
								})
							}
							currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1) // delete inputs

						}

						// get the echo vars
						element, internal_err = echoIter.FindElement(nil, "echo")
						currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "echo", JType: simdjson.TypeObject})
						if internal_err == nil {
							var echoInterface interface{}
							var echoMap map[string]interface{}
							echoInterface, internal_err = element.Iter.Interface()
							if internal_err != nil {
								logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, internal_err)
								wasError = true
							}
							var ok bool
							echoMap, ok = echoInterface.(map[string]interface{})
							if !ok {
								logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("could not convert echo object to map[string]interface{}"))
								wasError = true
							}
							for key, value := range echoMap {
								if _, ok := echo.Echo[key]; !ok {
									echo.Echo[key] = value
								} else {
									currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: key, JType: simdjson.TypeString})
									logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("found duplicate echo register; using the first occurence"))
									wasError = true
									currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
								}
							}
						}
						currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
						if !fatalErr {
							MetricsConfig.Echo = append(MetricsConfig.Echo, echo)
						}

					}

					echoIndex += 1
					currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
				})

			}
			currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
			handleEchoTemplates()

		}

		if wasError {
			return fmt.Errorf("")
		}
		return nil
	})

	var bytesDat interface{}
	err = json.Unmarshal(data, &bytesDat)
	if err == nil {
		bytesInterface, ok := bytesDat.(map[string]interface{})
		if ok {
			for key, _ := range bytesInterface {
				if !stringInSlice([]string{"meta", "templates", "inputs", "filters", "outputs", "metrics", "echo"}, key) {
					logError(&(configErrorLocs.ErrorLocs), []JsonAccessor{JsonAccessor{Key: configPathAndFile, JType: simdjson.TypeObject}}, fmt.Errorf("found unknown key '%s' in configuration document; ignoring config element", key))
				}
			}
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
	Mdo.Meta["config"] = ConfigFileLoc
	Mdo.Meta["timestamp"] = time.Now().Format(time.RFC3339)
	Mdo.Inputs = make(map[string]map[string]interface{}, len(MetricsConfig.Inputs))
	for key, _ := range MetricsConfig.Inputs {
		Mdo.Inputs[key] = make(map[string]interface{}, 1+len(MetricsConfig.Inputs[key].Attributes))
		Mdo.Inputs[key]["value"] = 0
		for _, attribute := range MetricsConfig.Inputs[key].Attributes {
			Mdo.Inputs[key][attribute] = 0
		}
	}
	Mdo.Filters = make(map[string][]string, len(MetricsConfig.Filters))
	for key, _ := range MetricsConfig.Filters {
		if len(dynamicFilterExpressions[key].DynamicInputs) > 0 {
			Mdo.Filters[key] = make([]string, len(dynamicFilterExpressions[key].DynamicInputs[0]))
		} else if len(staticFilterExpressions[key].DynamicInputs) > 0 {
			Mdo.Filters[key] = make([]string, len(staticFilterExpressions[key].DynamicInputs[0]))
		}
	}
	Mdo.Outputs = make(map[string]map[string]interface{}, len(MetricsConfig.Outputs))
	for key, _ := range MetricsConfig.Outputs {
		Mdo.Outputs[key] = make(map[string]interface{}, 1+len(MetricsConfig.Outputs[key].Attributes))
		Mdo.Outputs[key]["value"] = 0
		for attribute, attributeVal := range MetricsConfig.Outputs[key].Attributes {
			Mdo.Outputs[key][attribute] = attributeVal
		}
	}
	Mdo.Metrics = make(map[string]map[string]interface{}, len(MetricsConfig.Metrics))
	for i, _ := range MetricsConfig.Metrics {
		Mdo.Metrics[MetricsConfig.Metrics[i].Expression] = make(map[string]interface{}, 1)
		Mdo.Metrics[MetricsConfig.Metrics[i].Expression]["value"] = 0
	}
	Mdo.Echo = make(map[string]map[string]interface{}, len(MetricsConfig.Echo))
	for i, _ := range MetricsConfig.Echo {
		Mdo.Echo[MetricsConfig.Echo[i].PublishUri] = MetricsConfig.Echo[i].Echo
	}
	mdoBuf = new(bytes.Buffer)
	mdoEncoder = json.NewEncoder(mdoBuf)
	mdoEncoder.SetEscapeHTML(false)
	mdoEncoder.SetIndent("", "    ")
}

func delete_at_index(slice []JsonAccessor, index int) []JsonAccessor {

	// Append function used to append elements to a slice
	// first parameter as the slice to which the elements
	// are to be added/appended second parameter is the
	// element(s) to be appended into the slice
	// return value as a slice
	if index >= 0 && len(slice)-1 >= index {
		return append(slice[:index], slice[index+1:]...)
	} else {
		return slice
	}
}

func logError(errorLocs *[]ErrorLocation, currentJsonLocation []JsonAccessor, err error) {
	var errStr string
	if fmt.Sprintf("%v", err) == "path not found" {
		errStr = strings.Replace(fmt.Sprintf("%v", err), "path", fmt.Sprintf("key '%v'", currentJsonLocation[len(currentJsonLocation)-1]), -1)
		errStr = strings.ReplaceAll(errStr, "\"", "\\\"")
		*errorLocs = append(*errorLocs, ErrorLocation{JsonError: errStr})
		(*errorLocs)[len(*errorLocs)-1].JsonLocation = make([]JsonAccessor, len(currentJsonLocation)-1)
		copy(configErrorLocs.ErrorLocs[len(configErrorLocs.ErrorLocs)-1].JsonLocation, currentJsonLocation)
	} else if strings.Contains(fmt.Sprintf("%v", err), "value is not") {
		errStr = strings.Replace(fmt.Sprintf("%v", err), "value is not", "expected value to be", -1)
		errStr = strings.ReplaceAll(errStr, "\"", "\\\"")
		*errorLocs = append(*errorLocs, ErrorLocation{JsonError: errStr})
		(*errorLocs)[len(*errorLocs)-1].JsonLocation = make([]JsonAccessor, len(currentJsonLocation))
		copy(configErrorLocs.ErrorLocs[len(configErrorLocs.ErrorLocs)-1].JsonLocation, currentJsonLocation)
	} else {
		*errorLocs = append(*errorLocs, ErrorLocation{JsonError: strings.ReplaceAll(fmt.Sprintf("%v", err), "\"", "\\\"")})
		(*errorLocs)[len(*errorLocs)-1].JsonLocation = make([]JsonAccessor, len(currentJsonLocation))
		copy(configErrorLocs.ErrorLocs[len(configErrorLocs.ErrorLocs)-1].JsonLocation, currentJsonLocation)
	}

}

func handleInputsTemplates() {
	if allPossibleAttributes == nil {
		allPossibleAttributes = make(map[string][]string, 0)
	}
	for _, template := range MetricsConfig.Templates {
		for inputName, input := range MetricsConfig.Inputs {
			// it only really makes sense for inputs to be templated by their variable name
			// otherwise, we would have repeated variable names
			if strings.Contains(inputName, template.Tok) {
				for _, replacement := range template.List {
					newInput := Input{}
					newInput.Internal = input.Internal
					newInputName := strings.ReplaceAll(inputName, template.Tok, replacement)

					// check for duplicate input names
					// this is much more likely to happen if we are careless with templating
					if _, ok := MetricsConfig.Inputs[newInputName]; ok {
						// fatal error for input
						logError(&(configErrorLocs.ErrorLocs), []JsonAccessor{JsonAccessor{Key: "inputs", JType: simdjson.TypeObject}, JsonAccessor{Key: newInputName, JType: simdjson.TypeObject}}, fmt.Errorf("duplicate input variable '%s' when unraveling template; only considering first occurence", newInputName))
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
				}
				delete(MetricsConfig.Inputs, inputName)
			}
		}
	}
}

func handleFiltersTemplates() {
	for _, template := range MetricsConfig.Templates {
		for filterName, filter := range MetricsConfig.Filters {
			// it only really makes sense for filters to be templated by their variable name
			// otherwise, we would have repeated variable names
			if strings.Contains(filterName, template.Tok) {
				for _, replacement := range template.List {
					var wasErr bool
					newFilterName := strings.ReplaceAll(filterName, template.Tok, replacement)

					// check for duplicate filter/input names
					// this is much more likely to happen if we are careless with templating
					if _, ok := MetricsConfig.Inputs[newFilterName]; ok {
						// fatal error
						logError(&(configErrorLocs.ErrorLocs), []JsonAccessor{JsonAccessor{Key: "filters", JType: simdjson.TypeObject}, JsonAccessor{Key: newFilterName, JType: simdjson.TypeObject}}, fmt.Errorf("templated filter variable '%s' also occurs as input variable; only considering input variable", newFilterName))
						continue
					}
					if _, ok := MetricsConfig.Filters[newFilterName]; ok {
						// fatal error
						logError(&(configErrorLocs.ErrorLocs), []JsonAccessor{JsonAccessor{Key: "filters", JType: simdjson.TypeObject}, JsonAccessor{Key: newFilterName, JType: simdjson.TypeObject}}, fmt.Errorf("duplicate filter variable '%s' when unraveling template; only considering first occurence", newFilterName))
						continue
					}

					// copy over the templated filter with any templates removed
					switch filter.(type) {
					case string:
						strFilter, ok := filter.(string)
						if ok {
							newStrFilter := strings.ReplaceAll(strFilter, template.Tok, replacement)
							MetricsConfig.Filters[newFilterName] = newStrFilter
						} else {
							logError(&(configErrorLocs.ErrorLocs), []JsonAccessor{JsonAccessor{Key: "filters", JType: simdjson.TypeObject}, JsonAccessor{Key: newFilterName, JType: simdjson.TypeString}}, fmt.Errorf("could not convert filter to string"))
							wasErr = true
						}
					case []interface{}:
						filterArr, ok := filter.([]interface{})
						if !ok {
							logError(&(configErrorLocs.ErrorLocs), []JsonAccessor{JsonAccessor{Key: "filters", JType: simdjson.TypeObject}, JsonAccessor{Key: newFilterName, JType: simdjson.TypeArray}}, fmt.Errorf("could not convert filter to []string"))
							wasErr = true
						}
						newFilterArr := make([]interface{}, 0)
						for filterIndex, subFilter := range filterArr {
							strFilter, ok := subFilter.(string)
							if ok {
								newStrFilter := strings.ReplaceAll(strFilter, template.Tok, replacement)
								newFilterArr = append(newFilterArr, newStrFilter)
							} else {
								logError(&(configErrorLocs.ErrorLocs), []JsonAccessor{JsonAccessor{Key: "filters", JType: simdjson.TypeObject}, JsonAccessor{Key: newFilterName, JType: simdjson.TypeArray}, JsonAccessor{Index: filterIndex, JType: simdjson.TypeString}}, fmt.Errorf("could not convert subfilter to string"))
								wasErr = true
								break
							}
						}
						if !wasErr {
							MetricsConfig.Filters[newFilterName] = newFilterArr
						}
					case []string:
						filterArr, ok := filter.([]string)
						if !ok { // don't think I can technically get here, but it's for safety.
							logError(&(configErrorLocs.ErrorLocs), []JsonAccessor{JsonAccessor{Key: "filters", JType: simdjson.TypeObject}, JsonAccessor{Key: newFilterName, JType: simdjson.TypeArray}}, fmt.Errorf("could not convert filter to []string"))
							wasErr = true
						}
						newFilterArr := make([]string, 0)
						for _, strFilter := range filterArr {
							newStrFilter := strings.ReplaceAll(strFilter, template.Tok, replacement)
							newFilterArr = append(newFilterArr, newStrFilter)
						}
						if !wasErr {
							MetricsConfig.Filters[newFilterName] = newFilterArr
						}
					default:
						logError(&(configErrorLocs.ErrorLocs), []JsonAccessor{JsonAccessor{Key: "filters", JType: simdjson.TypeObject}, JsonAccessor{Key: newFilterName, JType: simdjson.TypeObject}}, fmt.Errorf("invalid filter type %v", reflect.TypeOf(filter)))
						continue
					}
				}
				delete(MetricsConfig.Filters, filterName)
			}
		}
	}
}

func handleOutputsTemplates() {
	for _, template := range MetricsConfig.Templates {
		for outputName, output := range MetricsConfig.Outputs {
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
							err = json.Unmarshal(b2, &(newOutput.Attributes))
						}
					}
					if len(output.Enum) > 0 {
						b, err := json.Marshal(output.Enum)
						if err == nil {
							b2 := bytes.ReplaceAll(b, []byte(template.Tok), []byte(replacement))
							err = json.Unmarshal(b2, &(newOutput.Enum))
						}
					}

					if len(output.Bitfield) > 0 {
						b, err := json.Marshal(output.Bitfield)
						if err == nil {
							b2 := bytes.ReplaceAll(b, []byte(template.Tok), []byte(replacement))
							err = json.Unmarshal(b2, &(newOutput.Bitfield))
						}
					}

					if len(output.Flags) > 0 {
						for p, flag := range output.Flags {
							newFlag := strings.ReplaceAll(flag, template.Tok, replacement)
							newOutput.Flags[p] = newFlag
						}
					}
					MetricsConfig.Outputs[newOutputName] = newOutput
				}
				delete(MetricsConfig.Outputs, outputName)
			}
		}
	}
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

func handleEchoTemplates() {
	for _, template := range MetricsConfig.Templates {
		e := 0
		echoCopy := make([]EchoObject, len(MetricsConfig.Echo))
		copy(echoCopy, MetricsConfig.Echo)
		for _, echoObject := range echoCopy {
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
						switch value.(type) {
						case string:
							newValue = strings.ReplaceAll(value.(string), template.Tok, replacement)
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
								switch value.(type) {
								case string:
									newValue = strings.ReplaceAll(value.(string), template.Tok, replacement)
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
							switch value.(type) {
							case string:
								newValue = strings.ReplaceAll(value.(string), template.Tok, replacement)
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
func generateScope() {
	Scope = make(map[string][]Input, 0)
	for key, input = range MetricsConfig.Inputs {
		input.Name = key
		Scope[key] = []Input{input}
		if len(input.Attributes) > 0 {
			for attributeName, attributeLoc := range input.AttributesMap {
				Scope[attributeLoc] = []Input{Input{Name: attributeName, Value: MetricsConfig.Attributes[attributeLoc].Value}}
			}
		}
	}
}

func getExpression(metricsObject *MetricsObject, internal_vars []string, netIndex int) (string, error) {
	// preprocess all of the expressions in the metrics document
	// so that all we need to do is evaluate the expressions at runtime.
	// currently modeling after https://github.com/crsmithdev/goexpr
	var warning string
	if inputToMetricsExpression == nil {
		inputToMetricsExpression = make(map[string][]int, 0)
	}
	if expressionNeedsEval == nil {
		expressionNeedsEval = make(map[int]bool, len(MetricsConfig.Metrics))
	}
	if Scope == nil {
		Scope = make(map[string][]Input, 0)
	}

	exp, err := Parse((*metricsObject).Expression)
	if err != nil {
		return "", err
	}
	(*metricsObject).ParsedExpression = *exp

	containedInValChanged = make(map[string]bool, 0)
	for _, var_name := range exp.Vars {
		if _, ok := inputToMetricsExpression[var_name]; !ok {
			inputToMetricsExpression[var_name] = make([]int, 0)
			if MetricsConfig.Inputs[var_name].Internal && !stringInSlice(internal_vars, var_name) {
				warning = fmt.Sprintf("metrics expression uses internal_output var '%v' prior to its calculation; results displayed for this metric will lag behind '%v' by one cycle", var_name, var_name)
			}
			containedInValChanged[var_name] = false
		}
		inputToMetricsExpression[var_name] = append(inputToMetricsExpression[var_name], netIndex)

		if strings.Contains((*metricsObject).ParsedExpression.String, "ValueChanged") || strings.Contains((*metricsObject).ParsedExpression.String, "OverTimescale") {
			containedInValChanged[var_name] = true
		}
	}

	expressionNeedsEval[netIndex] = true
	(*metricsObject).State = make(map[string][]Union, 0)
	if strings.Contains((*metricsObject).ParsedExpression.String, "Integrate") ||
		strings.Contains((*metricsObject).ParsedExpression.String, "Time") ||
		strings.Contains((*metricsObject).ParsedExpression.String, "Milliseconds") ||
		strings.Contains((*metricsObject).ParsedExpression.String, "Pulse") {
		(*metricsObject).State["alwaysEvaluate"] = []Union{Union{tag: BOOL, b: true}}
	} else {
		(*metricsObject).State["alwaysEvaluate"] = []Union{Union{tag: BOOL, b: false}}
	}

	// populate the initialValues map
	outputUnion := Union{tag: (*metricsObject).Type}
	if (*metricsObject).ParsedExpression.ResultType != outputUnion.tag {
		if (*metricsObject).ParsedExpression.ResultType == STRING {
			containsFilter := false
			for _, varName := range (*metricsObject).ParsedExpression.Vars {
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
			if (*metricsObject).Type != NIL {
				warning = fmt.Sprintf("metrics expression produces possible result type %v but gets cast to %v", metricsObject.ParsedExpression.ResultType, outputUnion.tag)
			}
		}
	}
	if (*metricsObject).ParsedExpression.ResultType != NIL && outputUnion.tag == NIL {
		outputUnion = Union{tag: (*metricsObject).ParsedExpression.ResultType}
	}

	metricsObject.State["value"] = []Union{outputUnion}
	for _, outputVar := range (*metricsObject).Outputs { // metricsObject.Outputs is a list of output variable names
		output = MetricsConfig.Outputs[outputVar] // MetricsConfig.Outputs is a map[string]Output of output variable names to Output objects
		output.Value = outputUnion
		MetricsConfig.Outputs[outputVar] = output
	}
	return warning, err
}

func getAndParseFilters() bool {
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
	filters, wasError = ExtractFilters(MetricsConfig.Filters)
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
	if uriIsSet == nil {
		uriIsSet = make(map[string]bool, 0)
	}
	if uriToDirectSetActive == nil {
		uriToDirectSetActive = make(map[string]bool, 0)
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
	outputVarChanged[outputName] = true
	uriGroup := ""

	if len(output.Uri) > 0 {
		if stringInSlice(output.Flags, "lonely") {
			uriGroup = output.Uri + "[" + outputName + "]"
		} else if group, hasGroup := regexStringInSlice(output.Flags, `group\d+`); hasGroup {
			uriGroup = output.Uri + "[" + group + "]"
		} else {
			uriGroup = output.Uri
		}

		outputToUriGroup[outputName] = uriGroup

		if _, ok := PublishUris[uriGroup]; !ok {
			PublishUris[uriGroup] = make([]string, 0)
		}
		PublishUris[uriGroup] = append(PublishUris[uriGroup], outputName)

		if _, ok := PubUriFlags[uriGroup]; !ok {
			PubUriFlags[uriGroup] = make([]string, 0)
		}
		PubUriFlags[uriGroup] = append(PubUriFlags[uriGroup], output.Flags...)
		PubUriFlags[uriGroup] = removeDuplicateValues(PubUriFlags[uriGroup])
		if stringInSlice(PubUriFlags[uriGroup], "interval_set") {
			uriIsSet[uriGroup] = true
		} else {
			uriIsSet[uriGroup] = false
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
			uriToDirectSetActive[uriGroup] = false
		}
	}

}

func checkMixedFlags(currentJsonLocation []JsonAccessor) {
	uriGroup := ""
	currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "outputs", JType: simdjson.TypeObject})
	for outputName, output := range MetricsConfig.Outputs {
		currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: outputName, JType: simdjson.TypeObject})
		currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "flags", JType: simdjson.TypeArray})
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
				logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("output '%v' has format specified as both naked and clothed; defaulting to clothed", outputName))
			} else if stringInSlice(PubUriFlags[uriGroup], "clothed") && !stringInSlice(output.Flags, "clothed") && !stringInSlice(output.Flags, "naked") {
				logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("output '%v' has unspecified clothed/naked status; defaulting to clothed (warning only)", outputName))
			} else if stringInSlice(PubUriFlags[uriGroup], "naked") && !stringInSlice(output.Flags, "clothed") && !stringInSlice(output.Flags, "naked") {
				logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("output '%v' has unspecified clothed/naked status; defaulting to naked (warning only)", outputName))
			}
		}
		currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
		currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
	}
	currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
}

func GetPubTickers() {
	tickers = make([](*time.Ticker), 0)
	pubTickers = make(map[string]int, 0)
	globalPubRate := int64(1000)
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

	uriGroup := ""
	for outputName, output := range MetricsConfig.Outputs {
		if len(output.Uri) > 0 {
			if stringInSlice(output.Flags, "lonely") {
				PublishUris[output.Uri+"["+outputName+"]"] = []string{outputName}
				pubDataChanged[output.Uri+"["+outputName+"]"] = true
				uriGroup = output.Uri + "[" + outputName + "]"
			} else if group, hasGroup := regexStringInSlice(output.Flags, `group\d+`); hasGroup {
				if _, ok := PublishUris[output.Uri+"["+group+"]"]; ok {
					PublishUris[output.Uri+"["+group+"]"] = append(PublishUris[output.Uri+"["+group+"]"], outputName)
					uriGroup = output.Uri + "[" + group + "]"
				} else {
					PublishUris[output.Uri+"["+group+"]"] = []string{outputName}
					pubDataChanged[output.Uri+"["+group+"]"] = true
					uriGroup = output.Uri + "[" + group + "]"
				}
			} else {
				if _, ok := PublishUris[output.Uri]; !ok {
					PublishUris[output.Uri] = make([]string, 0)
				}
				PublishUris[output.Uri] = append(PublishUris[output.Uri], outputName)
				pubDataChanged[output.Uri] = true
				uriGroup = output.Uri
			}
			if _, ok := uriToDirectSetActive[uriGroup]; !ok { // if it's not a direct set, then we need a pub ticker for it
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
	tickerPubs = make(map[int][]string)
	for pubUri, tickerIndex := range pubTickers {
		if _, ok := tickerPubs[tickerIndex]; !ok {
			tickerPubs[tickerIndex] = make([]string, 0)
		}
		tickerPubs[tickerIndex] = append(tickerPubs[tickerIndex], pubUri)
	}

	// not directly related, but this will allow for multithreading with different metrics expressions
	metricsMutex = make([]sync.RWMutex, len(MetricsConfig.Metrics))
	for i, _ := range MetricsConfig.Metrics {
		metricsMutex[i] = *new(sync.RWMutex)
	}

	for echoIndex, _ := range MetricsConfig.Echo {
		MetricsConfig.Echo[echoIndex].Ticker = time.NewTicker(time.Duration(MetricsConfig.Echo[echoIndex].PublishRate) * time.Millisecond)
	}
	EvaluateExpressions()
}

func GetSubscribeUris() {
	if MetricsConfig.Meta == nil { // shouldn't happen if things are properly initialized, but just in case...
		MetricsConfig.Meta = make(map[string]interface{}, 0)
	}
	SubscribeUris = make([]string, 0)
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

	// extract parent uris (so we know what to subscribe to)
	// and also make an easy way to "fetch" data into our input Unions
	uriToInputNameMap = make(map[string][]string, len(MetricsConfig.Inputs))
	for key, input = range MetricsConfig.Inputs {
		if len(input.Uri) > 0 {
			SubscribeUris = append(SubscribeUris, GetParentUri(input.Uri))
			if _, ok := uriToInputNameMap[input.Uri]; !ok {
				uriToInputNameMap[input.Uri] = make([]string, 0)
			}
			uriToInputNameMap[input.Uri] = append(uriToInputNameMap[input.Uri], key)
		}
	}

	// each echo input should have a uri to subscribe to
	uriToEchoObjectInputMap = make(map[string]map[int]int, 0)
	for echoIndex, echoObject := range MetricsConfig.Echo {
		for inputIndex, echoInput := range echoObject.Inputs {
			if len(echoInput.Uri) > 0 {
				SubscribeUris = append(SubscribeUris, echoInput.Uri)
				if _, ok := uriToEchoObjectInputMap[echoInput.Uri]; !ok {
					uriToEchoObjectInputMap[echoInput.Uri] = make(map[int]int, 0)
				}
				uriToEchoObjectInputMap[echoInput.Uri][echoIndex] = inputIndex
			}
		}
	}

	// do the same for outputs so that we can respond to "gets"
	uriToOutputNameMap = make(map[string]string, len(MetricsConfig.Outputs))
	for outputName, output = range MetricsConfig.Outputs {
		if len(output.Uri) > 0 {
			if len(output.Name) > 0 {
				uriToOutputNameMap[output.Uri+"/"+output.Name] = outputName
				uriToOutputNameMap[output.Uri+"/"+outputName] = outputName
			} else {
				uriToOutputNameMap[output.Uri+"/"+outputName] = outputName
			}
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
	UriElements = make(map[string][][]string, 0)
	for _, input = range MetricsConfig.Inputs {
		uriFrags := strings.Split(input.Uri, "/")
		for i := 1; i < len(uriFrags)-1; i++ {
			if _, ok := UriElements[strings.Join(uriFrags[:i+1], "/")]; !ok {
				UriElements[strings.Join(uriFrags[:i+1], "/")] = make([][]string, 0)
			}
			UriElements[strings.Join(uriFrags[:i+1], "/")] = append(UriElements[strings.Join(uriFrags[:i+1], "/")], uriFrags[i+1:len(uriFrags)])
		}
		UriElements[input.Uri] = make([][]string, 0)
		if len(input.Attributes) > 0 {
			for _, attributeName := range input.Attributes {
				UriElements[input.Uri+"/"+attributeName] = make([][]string, 0)
				UriElements[input.Uri+"@"+attributeName] = make([][]string, 0)
			}
			for _, attributeName := range input.Attributes {
				if _, ok := UriElements[strings.Join(uriFrags[:len(uriFrags)-1], "/")+"/"+attributeName]; !ok {
					UriElements[strings.Join(uriFrags[:len(uriFrags)-1], "/")+"/"+attributeName] = make([][]string, 0)
					UriElements[strings.Join(uriFrags[:len(uriFrags)-1], "/")+"@"+attributeName] = make([][]string, 0)
				}
			}
		}
	}

	// do the same for echo inputs
	for _, echoObject := range MetricsConfig.Echo {
		for _, echoInput := range echoObject.Inputs {
			if len(echoInput.Uri) > 0 {
				uriFrags := strings.Split(echoInput.Uri, "/")
				for _, oldVar := range echoInput.Registers {
					uriFrags2 := append(uriFrags, oldVar)
					for i := 1; i < len(uriFrags2)-1; i++ {
						if _, ok := UriElements[strings.Join(uriFrags2[:i+1], "/")]; !ok {
							UriElements[strings.Join(uriFrags2[:i+1], "/")] = make([][]string, 0)
						}
						UriElements[strings.Join(uriFrags2[:i+1], "/")] = append(UriElements[strings.Join(uriFrags2[:i+1], "/")], uriFrags2[i+1:len(uriFrags2)])
					}
					UriElements[strings.Join(uriFrags2, "/")] = make([][]string, 0)
				}
			}
		}
	}

	// do the same for outputs
	// map each parent uri to its children components that we'll need to fetch
	// from that parent uri
	OutputUriElements = make(map[string][][]string, 0)
	for outputName, output = range MetricsConfig.Outputs {
		uriFrags := strings.Split(output.Uri, "/")
		if len(output.Name) > 0 {
			uriFrags = append(uriFrags, output.Name)
		} else {
			uriFrags = append(uriFrags, outputName)
		}
		for i := 1; i < len(uriFrags)-1; i++ {
			if _, ok := OutputUriElements[strings.Join(uriFrags[:i+1], "/")]; !ok {
				OutputUriElements[strings.Join(uriFrags[:i+1], "/")] = make([][]string, 0)
			}
			OutputUriElements[strings.Join(uriFrags[:i+1], "/")] = append(OutputUriElements[strings.Join(uriFrags[:i+1], "/")], uriFrags[i+1:len(uriFrags)])
		}
		outputUri := strings.Join(uriFrags, "/")
		OutputUriElements[outputUri] = make([][]string, 0)
		if len(output.Attributes) > 0 {
			for attributeName, _ := range output.Attributes {
				OutputUriElements[outputUri+"/"+attributeName] = make([][]string, 0)
				OutputUriElements[outputUri+"@"+attributeName] = make([][]string, 0)
			}
			for attributeName, _ := range output.Attributes {
				if _, ok := OutputUriElements[outputUri+"/"+attributeName]; !ok {
					OutputUriElements[strings.Join(uriFrags[:len(uriFrags)-1], "/")+"/"+attributeName] = make([][]string, 0)
					OutputUriElements[strings.Join(uriFrags[:len(uriFrags)-1], "/")+"@"+attributeName] = make([][]string, 0)
				}
			}
		}
	}

	// //leaving this here in case we need it again (maybe a command-line option or query?)
	// fmt.Println("Looking for elements:")
	// for key, _ := range UriElements {
	// 	fmt.Printf("\t%s :", key)
	// 	for _, element := range UriElements[key] {
	// 		fmt.Printf("  %v", element)
	// 	}
	// 	fmt.Println()
	// }
}

func setupConfigLogging() {
	if logFileLoc, ok := MetricsConfig.Meta["log_file"]; ok {
		if len(log.ConfigFile) == 0 { // if it the location hasn't been set via command line options
			log.ConfigFile, ok = logFileLoc.(string)
			if !ok {
				logError(&(configErrorLocs.ErrorLocs), []JsonAccessor{JsonAccessor{Key: "meta", JType: simdjson.TypeObject}, JsonAccessor{Key: "log_file", JType: simdjson.TypeString}}, fmt.Errorf("value is not string"))
			}
		}
	}

	if debug_interface, ok := MetricsConfig.Meta["debug"]; ok {
		log.SetLogLevel([]interface{}{"debug"})
		debug, ok = debug_interface.(bool)
		if !ok {
			logError(&(configErrorLocs.ErrorLocs), []JsonAccessor{JsonAccessor{Key: "meta", JType: simdjson.TypeObject}, JsonAccessor{Key: "debug", JType: simdjson.TypeBool}}, fmt.Errorf("value is not bool"))
		}
	}

	if debug_interface, ok := MetricsConfig.Meta["debug_inputs"]; ok {
		debug_inputs_interface, ok := debug_interface.([]interface{})
		if !ok {
			logError(&(configErrorLocs.ErrorLocs), []JsonAccessor{JsonAccessor{Key: "meta", JType: simdjson.TypeObject}, JsonAccessor{Key: "debug_inputs", JType: simdjson.TypeArray}}, fmt.Errorf("value is not []string"))
			MetricsConfig.Meta["debug_inputs"] = []string{}
		} else {
			debug_inputs = []string{}
			for i, debug_input := range debug_inputs_interface {
				input_str, ok_str := debug_input.(string)
				if !ok_str {
					logError(&(configErrorLocs.ErrorLocs), []JsonAccessor{JsonAccessor{Key: "meta", JType: simdjson.TypeObject}, JsonAccessor{Key: "debug_inputs", JType: simdjson.TypeArray}, JsonAccessor{Index: i, JType: simdjson.TypeString}}, fmt.Errorf("value is not string"))
				} else {
					debug_inputs = append(debug_inputs, input_str)
				}
			}
		}
	}

	if debug_interface, ok := MetricsConfig.Meta["debug_filters"]; ok {
		debug_filters_interface, ok := debug_interface.([]interface{})
		if !ok {
			logError(&(configErrorLocs.ErrorLocs), []JsonAccessor{JsonAccessor{Key: "meta", JType: simdjson.TypeObject}, JsonAccessor{Key: "debug_filters", JType: simdjson.TypeArray}}, fmt.Errorf("value is not []string"))
			MetricsConfig.Meta["debug_filters"] = []string{}
		} else {
			debug_filters = []string{}
			for i, debug_filter := range debug_filters_interface {
				filter_str, ok_str := debug_filter.(string)
				if !ok_str {
					logError(&(configErrorLocs.ErrorLocs), []JsonAccessor{JsonAccessor{Key: "meta", JType: simdjson.TypeObject}, JsonAccessor{Key: "debug_filters", JType: simdjson.TypeArray}, JsonAccessor{Index: i, JType: simdjson.TypeString}}, fmt.Errorf("value is not string"))
				} else {
					debug_filters = append(debug_filters, filter_str)
				}
			}
		}
	}

	if debug_interface, ok := MetricsConfig.Meta["debug_outputs"]; ok {
		debug_outputs_interface, ok := debug_interface.([]interface{})
		if !ok {
			logError(&(configErrorLocs.ErrorLocs), []JsonAccessor{JsonAccessor{Key: "meta", JType: simdjson.TypeObject}, JsonAccessor{Key: "debug_outputs", JType: simdjson.TypeArray}}, fmt.Errorf("value is not []string"))
			MetricsConfig.Meta["debug_outputs"] = []string{}
		} else {
			debug_outputs = []string{}
			for i, debug_output := range debug_outputs_interface {
				output_str, ok_str := debug_output.(string)
				if !ok_str {
					logError(&(configErrorLocs.ErrorLocs), []JsonAccessor{JsonAccessor{Key: "meta", JType: simdjson.TypeObject}, JsonAccessor{Key: "debug_outputs", JType: simdjson.TypeArray}, JsonAccessor{Index: i, JType: simdjson.TypeString}}, fmt.Errorf("value is not string"))
				} else {
					debug_outputs = append(debug_outputs, output_str)
				}
			}
		}
	}
}

func verifyInputConfigLogging() {
	if _, ok := MetricsConfig.Meta["debug_inputs"]; ok && debug {
		for i, debug_input := range debug_inputs {
			if _, inputExists := MetricsConfig.Inputs[debug_input]; !inputExists {
				logError(&(configErrorLocs.ErrorLocs), []JsonAccessor{JsonAccessor{Key: "meta", JType: simdjson.TypeObject}, JsonAccessor{Key: "debug_inputs", JType: simdjson.TypeArray}, JsonAccessor{Index: i, JType: simdjson.TypeString}}, fmt.Errorf("debug input '%s' does not exist; no debug info for this variable will be displayed", debug_input))
				debug_inputs = append(debug_inputs[:i], debug_inputs[i+1:]...)
			}
		}
	}
}

func verifyFilterConfigLogging() {
	if _, ok := MetricsConfig.Meta["debug_filters"]; ok && debug {
		for i, debug_filter := range debug_filters {
			if _, inputExists := MetricsConfig.Filters[debug_filter]; !inputExists {
				logError(&(configErrorLocs.ErrorLocs), []JsonAccessor{JsonAccessor{Key: "meta", JType: simdjson.TypeObject}, JsonAccessor{Key: "debug_filters", JType: simdjson.TypeArray}, JsonAccessor{Index: i, JType: simdjson.TypeString}}, fmt.Errorf("debug filter '%s' does not exist; no debug info for this variable will be displayed", debug_filter))
				debug_filters = append(debug_filters[:i], debug_filters[i+1:]...)
			}
		}
	}
}

func verifyOutputConfigLogging() {
	if _, ok := MetricsConfig.Meta["debug_outputs"]; ok && debug {
		for i, debug_output := range debug_outputs {
			if _, outputExists := MetricsConfig.Outputs[debug_output]; !outputExists {
				logError(&(configErrorLocs.ErrorLocs), []JsonAccessor{JsonAccessor{Key: "meta", JType: simdjson.TypeObject}, JsonAccessor{Key: "debug_outputs", JType: simdjson.TypeArray}, JsonAccessor{Index: i, JType: simdjson.TypeString}}, fmt.Errorf("debug output '%s' does not exist; no debug info for this variable will be displayed", debug_output))
				debug_outputs = append(debug_outputs[:i], debug_outputs[i+1:]...)
			}
		}
	}
}

func checkUnusedOutputs() {
	for outputName, _ := range MetricsConfig.Outputs {
		used := false
		for i, _ := range MetricsConfig.Metrics {
			if stringInSlice(MetricsConfig.Metrics[i].Outputs, outputName) {
				used = true
				break
			}
		}
		if !used {
			logError(&(configErrorLocs.ErrorLocs), []JsonAccessor{JsonAccessor{Key: "outputs", JType: simdjson.TypeObject}, JsonAccessor{Key: outputName, JType: simdjson.TypeObject}}, fmt.Errorf("output '%v' is never set by a metrics expression; excluding from published outputs", outputName))
			delete(MetricsConfig.Outputs, outputName)
		}
	}
}