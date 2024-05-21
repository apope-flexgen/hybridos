package go_metrics

import (
	"fmt"
	"regexp"
	"sort"
	"strings"

	log "github.com/flexgen-power/go_flexgen/logger"
	simdjson "github.com/minio/simdjson-go"
)

func delete_string_at_index(slice []string, index int) []string {

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

// extract filters
func ExtractFilters(filters map[string]interface{}, new_filters []string, new_filters_starting_index int, new_inputs []string, overwrite_conflicting_configs bool, configId string) (map[string]Filter, bool) {
	if len(FiltersList) == new_filters_starting_index {
		return nil, false
	}
	wasError := false
	discardFilter := false
	output := make(map[string]Filter, len(filters))
	if FilterScope == nil {
		FilterScope = make(map[string][]string, 0)
	}
	for _, filter := range new_filters {
		FilterScope[filter] = []string{}
	}
	filtersListCopy := make([]string, len(FiltersList)-new_filters_starting_index)
	copy(filtersListCopy, FiltersList[new_filters_starting_index:])
	configErrorLocs.addKey("filters", simdjson.TypeObject)
	// get the filter expressions
	for i, filter_name := range filtersListCopy {
		filter := filters[filter_name]
		configErrorLocs.addKey(filter_name, simdjson.TypeObject)
		if _, ok := InputScope[filter_name]; ok && !overwrite_conflicting_configs {
			configErrorLocs.logError(fmt.Errorf("filter variable name '%s' already exists in scope; discarding filter", filter_name))
			wasError = true
			configErrorLocs.removeLevel()
			continue
		}
		inputs := make([]string, len(new_inputs))
		copy(inputs, new_inputs)

		for key := range FilterScope {
			if filter_name != key && !stringInSlice(inputs, key) {
				inputs = append(inputs, key)
			}
		}
		var filterObject Filter
		filterObject.StaticFilterExpressions = make([]Expression, 0)
		filterObject.DynamicFilterExpressions = make([]Expression, 0)
		switch x := filter.(type) {
		case string:
			stringExpressions := strings.Split(x, " | ")
			err := getFilterExpressions(stringExpressions, &filterObject, configId)
			if err != nil {
				discardFilter = true
				configErrorLocs.logError(fmt.Errorf("%v; discarding filter", err))
				wasError = true
			}
		case []string:
			err := getFilterExpressions(x, &filterObject, configId)
			if err != nil {
				discardFilter = true
				configErrorLocs.logError(fmt.Errorf("%v; discarding filter", err))
				wasError = true
			}
		default:
			discardFilter = true
			configErrorLocs.logError(fmt.Errorf("unhandled filter expression; discarding filter"))
			wasError = true

		} // end switch

		if discardFilter {
			delete(MetricsConfig.Filters, filter_name)
			delete(FilterScope, filter_name)
			FiltersList = delete_string_at_index(FiltersList, i+new_filters_starting_index)
			discardFilter = false
			configErrorLocs.removeLevel()
			continue
		}

		intermediateInputs := inputs
		var err error
		// apply static filters where we can
		for p, expression := range filterObject.StaticFilterExpressions {
			if expression.IsRegex {
				intermediateInputs, err = EvaluateRegexFilter(&expression, intermediateInputs, configId)
				if err != nil {
					discardFilter = true
					configErrorLocs.logError(fmt.Errorf("%v; discarding filter", err))
					wasError = true
					break
				}
			} else if expression.IsTypeFilter {
				intermediateInputs, err = EvaluateTypeFilter(&expression, intermediateInputs)
				if err != nil {
					configErrorLocs.logError(fmt.Errorf("%v; discarding filter", err))
					wasError = true
					discardFilter = true
					break
				}
			}
			filterObject.StaticFilterExpressions[p] = expression
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
			delete(FilterScope, filter_name)
			FiltersList = delete_string_at_index(FiltersList, i+new_filters_starting_index)
			discardFilter = false
			configErrorLocs.removeLevel()
			continue
		}

		// set the intermediate inputs as the first round of dynamic inputs
		filterObject.DynamicInputs = make([][]string, len(filterObject.DynamicFilterExpressions)+1)
		for i := range filterObject.DynamicFilterExpressions {
			filterObject.DynamicInputs[i] = make([]string, len(intermediateInputs))
		}
		filterObject.DynamicInputs[0] = intermediateInputs
		sort.Strings(filterObject.DynamicInputs[0])
		output[filter_name] = filterObject
		InputScope[filter_name] = make([]Union, 0)
		for _, inputName := range intermediateInputs {
			InputScope[filter_name] = append(InputScope[filter_name], InputScope[inputName]...)
		}
		FilterScope[filter_name] = make([]string, len(intermediateInputs))
		copy(FilterScope[filter_name], intermediateInputs)
		configErrorLocs.removeLevel()
	}
	configErrorLocs.removeLevel()

	return output, wasError
}

// get expressions
func getFilterExpressions(stringExpressions []string, filterObject *Filter, configId string) error {
	staticFilter := true
	for _, exp := range stringExpressions {
		var expression *Expression
		if strings.Contains(strings.ToLower(exp), "regex(") {
			staticFilter = true
			start := strings.Index(exp, "(")
			end := strings.Index(exp, ")")
			string_expression := exp[start+1 : end]
			expression = &Expression{
				String:  string_expression,
				IsRegex: true,
			}
		} else if strings.Contains(strings.ToLower(exp), "type(") {
			staticFilter = true
			start := strings.Index(exp, "(")
			end := strings.Index(exp, ")")
			string_expression := exp[start+1 : end]
			expression = &Expression{
				String:       string_expression,
				IsTypeFilter: true,
			}
		} else {
			staticFilter = false
			var err error
			expression, err = Parse(exp, configId)
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
func EvaluateRegexFilter(expression *Expression, inputs []string, configId string) ([]string, error) {
	output := make([]string, 0)
	regEx, err := regexp.Compile(expression.String)
	if err != nil {
		return nil, fmt.Errorf("could not parse regular expression. See https://github.com/google/re2/wiki/Syntax for help constructing")
	}

	for _, input := range inputs {
		inputWithoutSuffix := input
		if len(configId) > 0 {
			inputWithoutSuffix = strings.TrimSuffix(input, "_"+configId)
		}
		if match := regEx.FindString(inputWithoutSuffix); len(match) > 0 {
			if _, ok := MetricsConfig.Attributes[input]; !ok {
				output = append(output, input)
			}
		}
	}
	expression.Vars = make([]string, len(output))
	copy(expression.Vars, output)

	return output, nil
}

// determine which inputs have the correct type according to the specified filter
func EvaluateTypeFilter(expression *Expression, inputs []string) ([]string, error) {
	output := make([]string, 0)
	expression.String = strings.ToLower(expression.String)

	// double check that the filter is a vaild filter
	if !stringInSlice([]string{"string", "int", "uint", "float", "bool"}, expression.String) {
		return nil, fmt.Errorf("invalid type filter; must be one of string, int, uint, float, or bool")
	}
	for _, input := range inputs {
		if MetricsConfig.Inputs[input].Type == expression.String {
			output = append(output, input)
		}
	}
	expression.Vars = make([]string, len(output))
	copy(expression.Vars, output)

	return output, nil
}

// If one filter relies on another, it needs to be evaluated after that filter
// This function needs to get called after we've parsed the filter expressions
func PutFiltersInOrder() {
	FiltersList = make([]string, len(MetricsConfig.Filters))

	// Helper function to perform depth-first search to resolve dependencies
	var dfs func(string, map[string]bool, map[string]bool)
	dfs = func(filterName string, visited map[string]bool, path map[string]bool) {
		// If already visited or in the current path, return
		if visited[filterName] || path[filterName] {
			return
		}

		// Add to current path
		path[filterName] = true

		// Get the dependencies of the current filter
		filter, ok := staticFilterExpressions[filterName]
		if ok && len(filter.StaticFilterExpressions) > 0 { // we have static filters
			for _, expr := range filter.StaticFilterExpressions {
				for _, v := range expr.Vars {
					if _, ok := MetricsConfig.Filters[v]; ok {
						dfs(v, visited, path)
					}
				}
			}
		}
		filter, ok = dynamicFilterExpressions[filterName]
		if ok && len(filter.DynamicFilterExpressions) > 0 { // we have dynamic filters
			for _, expr := range filter.DynamicFilterExpressions {
				for _, v := range expr.Vars {
					if v == "value" { // do all other filters first
						for filterName2 := range MetricsConfig.Filters {
							if filterName2 == filterName {
								continue
							}
							dfs(filterName2, visited, path)
						}
					}
					if _, ok := MetricsConfig.Filters[v]; ok {
						dfs(v, visited, path)
					}
				}
			}
		}

		// Mark as visited and add to the result
		visited[filterName] = true
		FiltersList = append(FiltersList, filterName)

		// Remove from current path
		delete(path, filterName)
	}

	// Initialize the result and visited maps
	visited := make(map[string]bool)
	path := make(map[string]bool)

	// Start depth-first search for each filter
	for filterName := range MetricsConfig.Filters {
		dfs(filterName, visited, path)
	}
}
