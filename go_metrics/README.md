# Go Metrics
Author: Stephanie Reynolds

Updated: 4/25/2023
## What is Metrics?
Metrics is the current system for performing calculations on values that are collected at runtime. For example, if we are looking to calculate `power`, we could listen for `voltage` and `current`, and then, at run time, we could calculate the `power` as `voltage * current`. This calculated value is then fed back out to Fims on a specific output uri.

## The Need
Metrics files are icky!! The configs can be difficult to write and interpret, and they often require the creation of intermediate variables to perform calculations. This project is a complete overhaul of Metrics (written in Node JS) to a new version (written in Go) that is intended to have much simpler config documents with similar or more powerful capabilities.

## Quickstart

### Configuration document
The basic features of a config document are fairly straightforward, though you can do quite a lot beyond just the basics. To get you started, here is an example of a very simple config.
<details close>
<summary><b>Basic example of New Config Document</b></summary>
  
```
{
    "meta": { "publishRate": 2000 },
    "inputs": {
        "v1": { "uri": "/components/feeder_52m1/v1", "type": "float" }
    },
    "outputs": {
        "v1_times_5": { "uri": "/some/v1/output" }
		    },
    "metrics": [
        {
            "value": 0.0, 
            "outputs": "v1_times_5",
            "expression": "v1 * 5"
        }
    ]
}
```

</details><br>

The key details to highlight are that:
* You specify the publish rate (in milliseconds) in the Meta section. If not specified, the default publish rate is every 1000 milliseconds.
* Standard inputs require a uri and a type (bool, float, int, uint, string).
* Outputs require a uri.
* Metrics expressions require:
  * a default value (which determines the output data type of the expression),
  * an output or a list of outputs, and
  * an expression written as a string (which can be written in standard mathematical notation unless using specific functions).

Once you have your basic configuration document set up, you can run it by naming it `metrics.json` and placing it in the `configs` folder. To run the program with default settings, make sure Fims is running and call `./main` from the command line. You should begin to see traffic published to any output uris.

For more example configs, see the [Examples folder](https://github.com/flexgen-power/go_metrics/tree/initial_setup/examples) 

### Build
* First, navigate to the `./src` directory.

* `main.go` contains the main run script for go_metrics. It depends strongly on the `go_metrics` package found in `pkg/go_metrics`. You will first need to copy the `config` and `go_metrics` packages to GOROOT (typically `/usr/local/go` or `usr/lib/golang`):
  ```
  \cp -r ../pkg/go_metrics [GOPATH]/src
  cp -a ../pkg/go_metrics  [GOPATH]/src
  ```

* To create a build of the package, run the following command in the terminal:
  ```
  go build -o /usr/local/bin/go_metrics
  ```

* Make sure Fims is running.
* Then, to run Go metrics:
  ```
  go_metrics [path/to/config]
  ```

## More details about Metrics v2
But wait! That doesn't seem like it's very useful. I want to do more! Well...lucky for you, there's much MUCH more!!
<details close>
<summary><b>Detailed Example of New Config Document</b></summary>
<br>

```
{
    "meta": {
		"note": "all big fields (templates, inputs, filters, outputs, metrics, echo) are optional",
		"publishRate": 2000
    },
	"templates": [
		{
			"type": "sequential",
			"from": 1,
			"to": 16,
			"step": 3,
			"token": "##"
        },
		{
			"type": "list",
			"list": ["bobcat", "cheetah", "lion"],
			"token": "qq"
        }
	],
    "inputs": {
        "var_name1": { "uri": "/components/bms_74b/vnom", "type": "float", "default": 5.0            },
        "var_name2": { "uri": "/components/feeder_52m1/v1", "type": "float"            },
        "var_name3": { "uri": "/components/feeder_52m1/id", "type": "string"            },
        "var_name4": { "uri": "/components/feeder_52u1/pmax", "type": "bool"            },
        "var_name5": { "uri": "/components/bms_74b/id", "type": "string", "attributes":["enabled", "scale"]          },
		"intermediate_input":{"internal":true, "type":"int"}
    },
    "filters": {
        "all_vars_enabled": "regex(var_name*) | attribute(enabled == true)",
        "all_float_vars": "regex(var_name*) | type(float)"
    },
    "outputs": {
        "output##_qq": { "uri": "/some/output##", "flags": [ "clothed", "group1","clothed"], "publishRate": 1000, "attributes": {"scale": 1000, "units": "deg C", "ui_type": "none"}},
        "output##": { "name":"timestamp","uri": "/some/output##", "flags": ["group2"] },
		"level2_output": { "uri": "/some/level2"},
		"enum_output": {
						"name": "status", "uri": "/some/status/output", "flags": ["enum"],
						"enum": [
							"Power up",
							"Initialization",
							{"value": 10,"string": "Off", "note": "the next enum string has an implicit value of 11", "note2": "can skip values for enums but not bitfields"},
							"Precharge",
							{"value":20, "string":"some other value"}
						]
					},
		"bitfield_output": {
			"name": "status2", "uri": "/some/status/output","flags": ["bitfield"],
			"bitfield": [
				"Power up",
				"Initialization",
				"Off",
				"Precharge"
			]
		}
		    },
    "metrics": [
        {
            "value": 0.0, 
            "outputs": ["output##_qq@scale"],
            "expression": "If(var_name5@enabled < 5, 100, 150)"
        },
        {
            "value": "", 
            "outputs": "output##",
            "expression": "MillisecondsToRFC3339(Time())"
        },
		{
            "value": 0, 
            "outputs": "enum_output",
            "expression": "3"
        },
		{
            "value": 0, 
            "outputs": "bitfield_output",
            "expression": "true | false << 1 | true << 2 | true << 3"
        },
		{
            "value": 0, 
            "internal_output": "intermediate_input",
            "expression": "5"
        },
		{
            "value": 0, 
            "outputs": "level2_output",
            "expression": "intermediate_input*5"
        }
    ],
    "echo": [
		{
			"uri": "/components/sel_735",
			"publishRate": 1000,
			"format": "naked",
			"inputs": [
				{
					"uri": "/components/feeder",
					"registers": {
						"f": {"source": "frequency", "default": 60},
						"p": {"source":"active_power", "default":100},
						"pf": "power_factor",
						"q": "reactive_power",
						"v": "voltage_l1",
						"v1": "voltage_l2",
						"v2": "voltage_l3",
						"s1": {"source":"string_uri_element", "default": "some value for the string"},
						"b1": {"source":"bool_uri_element", "default": true}
					}
				}
			],
			"echo": {
				"apparent_power": 0,
				"current_l1": 0,
				"current_l2": 0,
				"current_l3": 0,
				"current_n": 0,
				"kvarh_delivered": 0,
				"kvarh_received": 0,
				"kwh_delivered": 0,
				"kwh_received": 0,
				"thd_i_l1": 0,
				"thd_i_l2": 0,
				"thd_i_l3": 0,
				"thd_v_l1": 0,
				"thd_v_l2": 0,
				"thd_v_l3": 0,
				"voltage_l1_l2": 0,
				"voltage_l2_l3": 0,
				"voltage_l3_l1": 0
			}
		}
	]
}
```
</details>
<br>
Obviously, there's still a lot of stuff in this config document!! Let's break it down by section.

<details close>
<summary><b>Meta</b></summary>

* As of right now, all fields are optional.
* Metadata information will be included in any output config document (e.g. MDO).
* Currently, the only metadata field that is processed is `"publishRate"`.
  * `"publishRate"` should be an integer.
  * This represents the global publish rate (in milliseconds) for any outputs that do not have their own publish rate specified.
  * If publish rate is not specified globally, the default global publish rate is 1000 milliseconds.
</details>
<details close>
<summary><b>Templates</b></summary>

* Templates come in two types:
  * Sequential
  * List
  <details close>
  <summary>Sequential Templates</summary>

  * Fields:
    * `"type":"sequential"` (optional): really just there for your own information
    * `"from"` (required for sequential templating): an integer that specifies the value to start counting from
    * `"to"` (required for sequential templating): an integer that specifies the value to stop counting at
    * `"step"` (optional): an integer that specifies the interval to count by; defaults to 1 if not specified
    * `"token"` (required): anything except `@`, since that is reserved for attribute specifiers. Overlapping templates will throw an error and will prioritize templates earlier in the document.
  * Example:
    ```
    {
      "type": "sequential",
      "from": 1,
      "to": 9,
      "step": 2,
      "token": "##"
    }
    ```
    * The above example will replace all instances of `##` with each of the values `1, 3, 5, 7, and 9`.
  </details>
  <details close>
  <summary>List templates</summary>

  * Fields:
    * `"type":"list"` (optional): really just there for your own information
    * `"list"` (required for list templating): a list of string values to replace the token with
    * `"token"` (required): anything except `@`, since that is reserved for attribute specifiers. Overlapping templates will throw an error and will prioritize templates earlier in the document.
  * Example
    ```
    {
      "type": "list",
      "list": ["cougar", "bobcat", "lion"]
      "token": "##"
    }
    ```
    * The above example will replace all instances of `##` with each of the values `"cougar", "bobcat", and "lion"`.
  </details>
  <details close>
  <summary>How templates are applied</summary>
  
    <details close>
    <summary>Input Templating </summary>

    * Inputs are single-level templated.
      * The input name **must** contain a template token if the input is to be templated.
      * Any fields within the input containing a template token will match the input name's template token replacement.
      * Example:

        ```
        "ess##_var": {"uri": "/components/ess_##/vnom", "type": "float"}
        ```

        In the above input variable, if using the token `##` with values `1, 2, and 3` yields the following input variables:

        ```
        "ess1_var": {"uri": "/components/ess_1/vnom", "type": "float"}
        "ess2_var": {"uri": "/components/ess_2/vnom", "type": "float"}
        "ess3_var": {"uri": "/components/ess_3/vnom", "type": "float"}
        ```
        
        These input variables can then be used either in templated expressions or in independent, non-templated expressions.
  </details>
  <details close>
  <summary>Filter Templating</summary>

  * Filters are single-level templated.
    * Filter variable name must contain template token. See above description for inputs.
    </details>
    <details close>
    <summary>Output Templating</summary>

    * Outputs are single-level templated.
      * Output variable name must contain template token. See above description for inputs.
    </details>
    <details close>
    <summary>Metrics Expression Templating</summary>

    * Metrics Expressions have two possible levels for templating.
      <details close>
      <summary>Level 1 templating: Outputs only</summary>

        * If an output value contains templating but the metrics expression does not, the same value will be sent to multiple output variables.
        * Example:
          ```
          {
            "value": 0.
            "outputs": "output##",
            "expression": "MillisecondsToRFC3339(Time())"
          }
          ```
          Replacing the token `##` with the values `1, 2, and 3` will result in the following metrics expression:
          ```
          {
            "value": 0.
            "outputs": ["output1", "output2","output3"],
            "expression": "MillisecondsToRFC3339(Time())"
          }
          ```
          In this example, the current zulu time will be published to each of the three outputs using only one metrics calculation.
        </details>
        <details close>
        <summary>Level 2 templating: Expressions and outputs</summary>

        * If an expression contains templating alongside it's corresponding output, the metrics object will be duplicated and sent to its corresponding output variable(s).
        * Example:
          ```
          {
            "value": 0.
            "outputs": "output##",
            "expression": "input##"
          }
          ```
          Replacing the token `##` with the values `1, 2, and 3` will result in the following 3 metrics expressions:
          ```
          {
            "value": 0.
            "outputs": "output1",
            "expression": "input1"
          },
          {
            "value": 0.
            "outputs": "output2",
            "expression": "input2"
          },
          {
            "value": 0.
            "outputs": "output3",
            "expression": "input3"
          }
          ```
          In this example, `input1` will be echo'ed to `output1`, `input2` will be echo'ed to `output2`, and `input3` will be echo'ed to `output3`.
        </details>
    </details>
    <details close>
    <summary>Echo Templating</summary>

    * Echo objects have three possible levels of templating.
      <details close>
      <summary>Level 1 templating: Echo registers only</summary>
       
        * If neither the echo object nor the echo object's inputs have templating, the individual registers will be templated accordingly.
        * Example:
          ```
          {
            "uri": ...,
              .
              .
              .
            "inputs": [ ... ]
            "echo": {
              "f": 0,
              "v##": 0,
              "c": 0
            }
          }
          ```
          Replacing the token `##` with the values `1, 2, and 3` will result in the following echo object in place of the original:
          ```
          {
            "uri": ...,
              .
              .
              .
            "inputs": [ ... ]
            "echo": {
              "f": 0,
              "v1": 0,
              "v2": 0,
              "v3": 0,
              "c": 0
            }
          }
          ```
        </details>
        <details close>
      <summary>Level 2 templating: Echo Inputs</summary>

        * If the echo object does not have templating but the echo object's input uris have templating, the input objects will be templated accordingly.
          * Note that each input register name must contain a templating token.
        * Example:
          ```
          {
            "uri": ...,
              .
              .
              .
            "inputs": [
              {
                "uri": "/components/feeder##",
                "registers": {
                    "f##": {"source": "frequency", "default": 60},
                    "p##": {"source":"active_power", "default":100},
                    "pf##": "power_factor",
                    "q##": "reactive_power"
                  }
                }
              ]
            "echo": {
              .
              .
              .
            }
          }
          ```
          Replacing the token `##` with the values `1, 2, and 3` will result in the following echo object in place of the original:
          ```
          {
            "uri": ...,
              .
              .
              .
            "inputs": [
              {
                "uri": "/components/feeder1",
                "registers": {
                    "f1": {"source": "frequency", "default": 60},
                    "p1": {"source":"active_power", "default":100},
                    "pf1": "power_factor",
                    "q1": "reactive_power"
                  }
                },
                {
                "uri": "/components/feeder2",
                "registers": {
                    "f2": {"source": "frequency", "default": 60},
                    "p2": {"source":"active_power", "default":100},
                    "pf2": "power_factor",
                    "q2": "reactive_power"
                  }
                },
                {
                "uri": "/components/feeder3",
                "registers": {
                    "f3": {"source": "frequency", "default": 60},
                    "p3": {"source":"active_power", "default":100},
                    "pf3": "power_factor",
                    "q3": "reactive_power"
                  }
                }
              ]
            "echo": {
              .
              .
              .
            }
          }
          ```
        </details>
        <details close>
        <summary>Level 3 templating: Echo Objects</summary>

        * An echo object will be templated if its output uri contains a templating token.
          * Note that, because echo registers can have the same name if they are in different echo objects, you do not need to apply templating to any of the registers (unless you want to).
        * Example:
          ```
          {
            "uri": "/components/ess_##",
              .
              .
              .
            "inputs": [
              {
                "uri": "/components/feeder##",
                "registers": {
                    "ess_##_f": {"source": "frequency", "default": 60},
                    "p": {"source":"active_power", "default":100},
                    "pf": "power_factor",
                    "q": "reactive_power"
                  }
                }
              ]
            "echo": {
              .
              .
              .
            }
          }
          ```
          Replacing the token `##` with the values `1, 2, and 3` will result in the following echo objects in place of the original:
          ```
          {
            "uri": "/components/ess_1",
              .
              .
              .
            "inputs": [
              {
                "uri": "/components/feeder1",
                "registers": {
                    "ess_1_f": {"source": "frequency", "default": 60},
                    "p": {"source":"active_power", "default":100},
                    "pf": "power_factor",
                    "q": "reactive_power"
                  }
                }
              ]
            "echo": {
              .
              .
              .
            }
          },
          {
            "uri": "/components/ess_2",
              .
              .
              .
            "inputs": [
              {
                "uri": "/components/feeder2",
                "registers": {
                    "ess_2_f": {"source": "frequency", "default": 60},
                    "p": {"source":"active_power", "default":100},
                    "pf": "power_factor",
                    "q": "reactive_power"
                  }
                }
              ]
            "echo": {
              .
              .
              .
            }
          },
          {
            "uri": "/components/ess_3",
              .
              .
              .
            "inputs": [
              {
                "uri": "/components/feeder3",
                "registers": {
                    "ess_3_f": {"source": "frequency", "default": 60},
                    "p": {"source":"active_power", "default":100},
                    "pf": "power_factor",
                    "q": "reactive_power"
                  }
                }
              ]
            "echo": {
              .
              .
              .
            }
          }
          ```
        </details>
    </details>
  </details>
</details>
<details close>
<summary><b>Inputs</b></summary>

Input variables are used as either inputs to filter expressions or as inputs to metrics expressions. They are effectively the "variables" of the new metrics expressions.
  * There are two types of inputs:
    * Standard inputs
    * Internal inputs
  * Standard Inputs:
    * It is anticipated that most inputs will be standard inputs. Standard inputs consist of the following:
      * `Input variable name` (required): These are the unique identifiers/variable names to be used in the metrics expressions
      * `"uri"` (required for all standard inputs): This is where the data will come from. Also dictates what URIs to subscribe to.
      * `"type"` (required for all inputs): The anticipated data type that will be received from the specified uri. These are used to validate each metrics equation using static type-checking.
        * Must be one of the following: `"string"`, `"int"`, `"uint"`, `"bool"`, `"float"`
      * `"attributes"` (optional): Any same-level uri elements that we want to relate directly to the input. Typically will be used for clothed input values. These are not included in static type-checking.
      * `"default"` (optional): The initial value of the variable, prior to receiving any messages from the input uri.
  * Internal inputs:
    * These are inputs that come directly from metrics expressions as outputs. Internal inputs consist of the following:
      * `"internal": true` (required for internal input)
      * `"type"` (required for all inputs; same as for standard inputs): The anticipated data type that will be received from the specified uri. These are used to validate each metrics equation using static type-checking.
        * Must be one of the following: `"string"`, `"int"`, `"uint"`, `"bool"`, `"float"`
      * `"default"` (optional): The initial value of the variable, prior to receiving any messages from the corresponding metrics expression.
</details>
<details close>
<summary><b>Filters</b></summary>
Filters are intended to act as a sieve for input variables. They can be used to select variables using regex, to select variables with similar data types, or to select variables with certain attributes that match given conditions. Once selected, they variables are referred to collectively using the filter variable name, though they act as multiple separate inputs. For now, they are relatively restricted in their functionality, but we can definitely add more features in the future.

* Each filter must have:
    * `Filter variable name` (required): This acts as the identifier/variable name and can be used in metrics expressions just as input variables are used.
    * Corresponding filter(s) to run, separated by a pipe `" | "` operator.
* Current filters are:
  * `regex`: These apply to input variable names and use standard [Golang regex](https://github.com/google/re2/wiki/Syntax). This filter is applied prior to runtime and does not get re-evaluated.
    * For example, `var_name*` would select all input variables that begin with `var_name` (e.g. `var_name1`, `var_name2`, `var_name3`, etc.).
  * `type`: These select all of the input variables with a given type (int, uint, float, string, bool). This filter is applied prior to runtime and does not get re-evaluated.
  * `attribute`: This selects inputs based on the values of their attributes. They are relatively simple for right now and must take the format of:
    ```
    attribute([attribute name] [comparison operator] [comparison value])
    ```
    For example, a valid attribute filter takes the form of `attribute(enabled == true)`. As of right now, the attribute name *must* be on the left and a non-variable basic literal (string, int, float, bool, uint) *must* be on the right. Comparison operators include `==, !=, >, <, >=, <=`. Attribute filters are "dynamic" filters, in that they get applied during runtime (as attribute values change).
</details>
<details close>
<summary><b>Outputs</b></summary>
Outputs are the variables published to the outside world. They consist of the following:

  * `Output variable name` (required): These are the identifiers/variable names for the values that come from metrics expressions. By default, this variable name will be the string identifier in the published message body.
  * `uri` (required): This is where the data will be published to.
  * `flags` (optional): These must be in the form of a list `[]`. They act as controls for how to publish the variable and/or message body. currently valid flags are:
    * `"naked"`: to specify an output that does not have any attached attributes in the message body. For example, the message body:
      ```
      "output_var": 5
      ```
    * `"clothed"`: to specify an output that *does* have attached attributes in the message body. For example, the message body:
      ```
      "output_var": {"value": 5, "scale": 1, "ui_type": "none", "units": "deg C"}
      ```
        * Without any flags, the default output format is `naked`
        * However, if a naked/clothed flag type is specified, the remaining variables in that publish group will take on that publish type.
        * If, for some reason, you want to mix naked and clothed variables, you can do that by specifying each output flag explicitly.
    * `group#` (where `#` can be replaced by any number): If multiple output variables have the same output URI, but we want to publish them in separate message bodies (possibly with different naked/clothed formats), we can assign them a group number.
      * All variables within a group will be published in a single message.
      * All variables in separate groups will be published in separate messages.
      * Without a group number, variables are grouped within the "base" uri group (no group number).
    * `lonely`: To specify that we only ever want to publish a variable by itself, we can make a variable lonely. This will not be in a group and will abide by its own rules.
    * `"interval_set"`: If we want to send values out as a "set" rather than as a "pub", we can specify this flag. Values will be sent out at regular intervals using the "set" method.
    * `"enum"`: This is a type check for an enum output. If an output is to be an enum, it must contain this flag.
    * `"bitfield"`: This is a type check for a bitfield output. If an output is to be a bitfield, it must contain this flag.
  * `attributes` (optional): If an output is clothed, attributes will be published alongside the value. For example, if we want the message body for `ouput_var` to be:
      ```
      "output_var": {"value": 5, "scale": 1, "ui_type": "none", "units": "deg C"}
      ```
    we would specify the attributes as follows:
    ```
    "attributes": {"scale": 1, "ui_type": "none", "units": "deg C"}
    ```
    The value of the output variable is automatically assigned to the key `value` in the message body.
  * `publishRate` (optional): An integer to specify the publish rate (in milliseconds) for a specific URI/URI group. This publish rate takes priority over the global publish rate.
  * `name` (optional): The key to correspond with the output variable. Overrides the variable name for the output.
    * For example, if we have the following output variables:
      ```
      "output_var1": { "name": "timestamp", "uri": "/some/uri_1" },
      "output_var2": { "name": "timestamp", "uri": "/some/uri_2" }
      ```
      the message body for each will appear as:
      ```
      "timestamp": "[some timestamp]"
      ```
      as opposed to being identified by `"output_var1"` and `"output_var2"`.
</details>
<details close>
<summary><b>Metrics</b></summary>
Metrics objects dictate the calculations that are performed. Each metrics object consists of:

  * `value` (required): determines the output value type (string, int, uint, float, bool) and initial value
  * `outputs` (required, unless an internal_output is specified): a list or a single string that specifies the output variables that will store the corresponding output value from the metric expression.
  * `internal_output` (optional; can replace having an output or list of outputs): must point to an input variable that has been deemed "internal". This input variable will take on the value from this particular metrics expression, and can then be reused in other metrics expressions.
    * Note: metrics expressions are typically calculated in the order specified in the config document, so it is important that any internal outputs are calculated before they are used. Otherwise, the calculations will "lag" behind the internal output by 1 publish cycle.
  * `expression` (required): a string representation of what you want to calculate.
    * Expressions support "basic" expressions using most binary operators (+, -, *, /, %, &, |, ^, &&, ||, &^, >>, <<, >, <, >=, <=, ==, !=) and they follow standard order of operations.
    * Expressions support the unary operators `!` and `-`. (I tried to build in `++` and `--` but that caused issues in Go, so those got scratched.)
    * Expressions can contain parentheses `()` to help control evaluation order.
    * There are also quite a few functions that are supported.
      * These have not yet been documented, but they include all of the [old metrics functions](https://github.com/flexgen-power/metrics/blob/dev/documentation/Metrics_Operators.md), with the exception of `runtime`, `unicompare`, `bitfieldPositionCount`, and `combineInputsToArray`.
      * Inputs are simply put into the parentheses of the functions.
      * For most functions, if the old metrics had parameters, the parameters can instead be supplied via unary operators (e.g. `!` for `invert`).
    </details>
<details close>
<summary><b>Echo</b></summary>

Echo behaves almost identically to the [old echo](https://github.com/flexgen-power/echo). It is designed to "echo" incoming values to a different URI. While "echo" can be achieved using metrics expressions as well, this was added as a convenience, since many variables might be echoed to the same URI from the same input URI. More documentation on this to come, but, for now, use the old echo documentation.
</details>

Once a config has been constructed, you need to make sure you have the necessary traffic being generated. Use fims_server, twins, etc. to get your traffic up and going.

To run the latest version of the new metrics echo, navigate to `/src` and run `go build -o main` to generate the object file. Then, run `./main /path/to/metrics/config` to begin calculations!

## Old documentation that needs editing
<details close>
<summary>Equation parsing</summary>
<br>

### You can parse JSONs into equations!!
* `parse.go` can take any basic mathematical expression with variables and parse it into an expression that can be evaluated at runtime.
* **ParseToString():**
    * This method is technically optional. If you have a JSON with an equation in it, this method allows you to extract it as a string expression, prior to breaking it down into an abstract syntax tree.
    * The currently supported format is an array of strings. An example to give to ParseToString() would be `["var1", "+", "var2", {"MULT": ["var3","var4","var5"]}]`. This would get parsed into a string equation of `var1+var2+Mult(var3,var4,var5)`.
    * You can (optionally) start with the string version of the equation, as a single element in an array: `["var1 + var2 + Mult(var3, var4, var5)"]`.
    * White space is ignored, so add as many spaces as you want to make it look nice!!
* **Parse():**
    * Once a string has been properly formatted for parsing, you can pass a string expression into `Parse()`. This function breaks down the string into an Expression struct, which consists of 1) the original string expression, 2) a list of runtime args to be treated as variables, and 3) an abstract syntax tree used for evaluation at runtime.
    * This function is based off of [goexpr](https://github.com/crsmithdev/goexpr). See goexpr or the source code of `parse.go` for the specific details.

### You can evaluate equations at runtime!!
* `eval.go` can take a previously parsed Expression struct, give it values for each of the variables, and evaluate the expression using those values.
* The currently supported functions are:
    * Basic math: Add, subtract, multiply, divide, modulo
    * Bitwise functions: bitwise and, bitwise or, bitwise xor, left shift, right shift, bitwise and_not
    * Logical operators: and, or, not
    * Relational operators: Equal, not equal, less than, greater than, less than or equal, greater than or equal
    * Math functions: root, power, maximum, minimum, average, floor, ceiling, square root, percentage of, floor division, absolute value, rounding
    * Type conversion: bool, int, uint, float, string
* The functions (previously in Metrics) that need to be implemented but that have not been yet:
    * integrate, enum, bitfield, count, bitfield position count, srff, select, runtime, unicompare, pulse, currentTimeMilliseconds, compareMillisecondsToCurrentTime, millisecondsToRFC3389, combineInputsToArray
* Inputs and outputs are in this weird struct called a Union, which is meant to mimic the union-type found in C++. The struct consists of an enum tag that specifies the data type (INT, BOOL, UINT, FLOAT, STRING), and fields that represent each of those data types. The actual value for the data type is expressed in the corresponding field (e.g. i for INT).

### Currently working on reading in FIMS data and evaluating expressions at runtime
* I've gotten a very very basic proof-of-concept going in `main.go`, but it definitely still needs work.
</details>