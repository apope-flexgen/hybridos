package go_metrics

import (
	"fmt"
	"regexp"
	"strings"

	log "github.com/flexgen-power/go_flexgen/logger"
	simdjson "github.com/minio/simdjson-go"
)

// extract filters
func ExtractFilters(filters map[string]interface{}) (map[string]Filter, bool) {
	wasError := false
	discardFilter := false
	output := make(map[string]Filter, len(filters))
	if FilterScope == nil {
		FilterScope = make(map[string][]string, 0)
	}
	currentJsonLocation := make([]JsonAccessor, 0)
	currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: "filters", JType: simdjson.TypeObject})
	// get the filter expressions
	for filter_name, filter := range filters {
		currentJsonLocation = append(currentJsonLocation, JsonAccessor{Key: filter_name, JType: simdjson.TypeObject})
		if _, ok := InputScope[filter_name]; ok {
			logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("filter variable name '%s' already exists in scope; discarding filter", filter_name))
			wasError = true
			continue
		}
		inputs := make([]string, 0)
		for key := range InputScope {
			inputs = append(inputs, key)
		}
		var filterObject Filter
		filterObject.StaticFilterExpressions = make([]Expression, 0)
		filterObject.DynamicFilterExpressions = make([]Expression, 0)
		switch x := filter.(type) {
		case string:
			stringExpressions := strings.Split(x, "|")
			err := getFilterExpressions(stringExpressions, &filterObject)
			if err != nil {
				discardFilter = true
				logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("%v; discarding filter", err))
				wasError = true
			}
		case []string:
			err := getFilterExpressions(x, &filterObject)
			if err != nil {
				discardFilter = true
				logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("%v; discarding filter", err))
				wasError = true
			}
		default:
			discardFilter = true
			logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("unhandled filter expression; discarding filter"))
			wasError = true

		} // end switch

		if discardFilter {
			delete(MetricsConfig.Filters, filter_name)
			discardFilter = false
			continue
		}

		intermediateInputs := inputs
		var err error
		// apply static filters where we can
		for _, expression := range filterObject.StaticFilterExpressions {
			if expression.IsRegex {
				intermediateInputs, err = EvaluateRegexFilter(expression.String, intermediateInputs)
				if err != nil {
					discardFilter = true
					logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("%v; discarding filter", err))
					wasError = true
					break
				}
			} else if expression.IsTypeFilter {
				intermediateInputs, err = EvaluateTypeFilter(expression.String, intermediateInputs)
				if err != nil {
					logError(&(configErrorLocs.ErrorLocs), currentJsonLocation, fmt.Errorf("%v; discarding filter", err))
					wasError = true
					discardFilter = true
					break
				}
			}
		}

		if debug {
			if stringInSlice(debug_filters, filter_name) {
				log.Debugf("Filter [%s] contains input vars %v", filter_name, intermediateInputs)
			}
		}

		// if the first dynamic filter is an attribute filter
		// we can do some minor pre-filtering to find only variables with that attribute
		if len(filterObject.DynamicFilterExpressions) > 0 && strings.Contains(filterObject.DynamicFilterExpressions[0].String, "attribute") {
			tmp := make([]string, 0)
			for _, varName := range filterObject.DynamicFilterExpressions[0].Vars {
				if attributesList, ok := allPossibleAttributes[varName]; ok {
					for _, attribute := range attributesList {
						if stringInSlice(intermediateInputs, MetricsConfig.Attributes[attribute].InputVar) {
							tmp = append(tmp, MetricsConfig.Attributes[attribute].InputVar)
						}
					}
				}
			}
			intermediateInputs = tmp
		}

		if discardFilter {
			delete(MetricsConfig.Filters, filter_name)
			discardFilter = false
			continue
		}

		// set the intermediate inputs as the first round of dynamic inputs
		filterObject.DynamicInputs = make([][]string, len(filterObject.DynamicFilterExpressions)+1)
		for i:= range filterObject.DynamicFilterExpressions {
			filterObject.DynamicInputs[i] = make([]string, len(intermediateInputs))
		}
		filterObject.DynamicInputs[0] = intermediateInputs
		output[filter_name] = filterObject
		InputScope[filter_name] = make([]Union, 0)
		for _, inputName := range intermediateInputs {
			InputScope[filter_name] = append(InputScope[filter_name], InputScope[inputName]...)
		}
		FilterScope[filter_name] = make([]string, len(intermediateInputs))
		copy(FilterScope[filter_name], intermediateInputs)
		currentJsonLocation = delete_at_index(currentJsonLocation, len(currentJsonLocation)-1)
	}

	return output, wasError
}

// get expressions
func getFilterExpressions(stringExpressions []string, filterObject *Filter) error {
	staticFilter := true
	for _, exp := range stringExpressions {
		var expression *Expression
		if strings.Contains(strings.ToLower(exp), "regex") {
			staticFilter = true
			start := strings.Index(exp, "(")
			end := strings.Index(exp, ")")
			expression = &Expression{
				String:  exp[start+1 : end],
				IsRegex: true,
			}
		} else if strings.Contains(strings.ToLower(exp), "type") {
			staticFilter = true
			start := strings.Index(exp, "(")
			end := strings.Index(exp, ")")
			expression = &Expression{
				String:       exp[start+1 : end],
				IsTypeFilter: true,
			}
		} else {
			staticFilter = false
			var err error
			expression, err = Parse(exp)
			if err != nil {
				return err
			}
		}
		if staticFilter {
			filterObject.StaticFilterExpressions = append(filterObject.StaticFilterExpressions, *expression)
		} else {
			filterObject.DynamicFilterExpressions = append(filterObject.DynamicFilterExpressions, *expression)
		}
	}
	return nil
}

// string filters are for regexp only right now...
func EvaluateRegexFilter(filter string, inputs []string) ([]string, error) {
	output := make([]string, 0)
	regEx, err := regexp.Compile(filter)
	if err != nil {
		return nil, fmt.Errorf("could not parse regular expression. See https://github.com/google/re2/wiki/Syntax for help constructing")
	}

	for _, input := range inputs {
		if match := regEx.FindString(input); len(match) > 0 {
			if _, ok := MetricsConfig.Attributes[input]; !ok {
				output = append(output, input)
			}
		}
	}

	return output, nil
}

// determine which inputs have the correct type according to the specified filter
func EvaluateTypeFilter(filter string, inputs []string) ([]string, error) {
	output := make([]string, 0)
	filter = strings.ToLower(filter)

	// double check that the filter is a vaild filter
	if !stringInSlice([]string{"string", "int", "uint", "float", "bool"}, filter) {
		return nil, fmt.Errorf("invalid type filter; must be one of string, int, uint, float, or bool")
	}
	for _, input := range inputs {
		if MetricsConfig.Inputs[input].Type == filter {
			output = append(output, MetricsConfig.Inputs[input].Name)
		}
	}

	return output, nil
}
