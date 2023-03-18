# Metrics Operators

## Extents

### min or max

* `inputs` - any numbers or booleans, will compute the minimum or maximum. false == 0, true == 1. Returns NaN if invalid inputs like strings are provided
* `param` - none
* `state` - none

## Aggregates 

### integrate

* `inputs` - one number, rectangle rule numerical integration of current input and time since last update
* `param`
  * `timescale` - number of hours to time scale the integration, e.g. if `input` is in *kW*, and `timescale` is 1, the result is in *kWh*
  * `abs` - boolean for whether the `abs()` function should be applied to the input when integrating
  * `minuteReset` - clock minute multiple at which the integral resets. 0 or empty for no reset
  * `minuteOffset` - minutes ahead of minuteReset multiples at which the integral resets
* `state`
  * `timestamp` - last time this metric was updated
  * `value` - the running integral
  * `minute` - last minute on the clock this metric was updated

## Math

### sum

* `inputs` - any, by default sums all inputs unless modified by params
* `param`
  * `operations` - a string of the same length as `inputs` composed of `+` or `-` characters indicating if the corresponding input should be added or subtracted from the total
  * `offset` - a fixed number to add to the total, may be negative
* `state` - none

### product

* `inputs` - any, by default multiplies all inputs unless modified by params
* `param`
  * `operations` - a string of the same length as `inputs` composed of `*` or `/` characters indicating if the corresponding input should multiply or divide the total
  * `gain` - a fixed number to multiply the total, may be negative
  * `upperLimit` and `lowerLimit` - limits to keep operation in range when dividing by small numbers or zero
* `state` - none

### average

* `inputs` - any, simple unweighted average af inputs
* `param` - none
* `state` - none

### rss

* `inputs` - any, a root sum square calculation. `y = sqrt(x[0]^2 + x[1]^2 + ... + x[n]^2)`
* `param` - none
* `state` - none

## Bit Manipulation

### and

* `inputs` - bitfields, defaults to return a bitwise AND of the input bitfields and converting to an integer output. e.g. `inputs = [[{"value":0,"string":"fault"},{"value":1,"string":"other"}],[{"value":0","string":"faulty"},{"value":3,"string":"nada"},{"value":8,"string":"final"}]]` returns 267
* `param`
  * `bitfield` - when true, returns the AND combined bitfield of the inputs. i.e. for the above inputs `[{"value":0,"string":"faulty"},{"value":1,"string":"other"},{"value":3,"string":"nada"},{"value":8,"string":"final"}]`
* `state` - none

### or

* `inputs` - any, bitwise or of numbers, strings, booleans or anything otherwise truthy
* `param` - none
* `state` - none

### enum

* `inputs` - not implemented
* `param` - none
* `state` - none

### bitfield

* `inputs` - any booleans or truthy values, packs them into a bitfield with values defaulting to the position in the input list
* `param`
  * `pos` - list of numbers corresponding to the `"value"` you want associated with the particular position in the input list
  * `string` - as `pos`, but the `"string"` association
  * `invertMask` - list of true/false values to selectively invert the inputs
* `state` - none

### length

* `inputs` - bitfield input, sums how many values are present in each bitfield, useful for counting number of alarms or faults
* `param` - none
* `state` - none

### bitfieldpositioncount

* `inputs` - any bitfields, by default counts how many objects with `"value" == 0` in the input bitfield
* `param`
  * `pos` - number to count a different bitfield position
  * `invert` - if true, count if the position is missing
* `state` - none

## Logic

### srff

Implementation of an SR flip-flop (S dominated), with the truth table:

| S   | R   | Output (Q) |
| --- | --- | ---------- |
| 0   | 0   | No change  |
| 0   | 1   | 0          |
| 1   | 0   | 1          |
| 1   | 1   | 1          |

* `inputs` - 2, the first input is the *S* while the second is the *R*. Inputs may be any
 type, and use javascript's truthiness rules. Non-blank strings, any array or object (even
 empty), non-zero numbers, and explicit boolean true are all truthy.
* `param` - none
* `state`
  * `q` - the last output value, defaults to *false*

### compare

Greater than, less than, equal to, in various combinations in order across inputs

* `inputs` - any, will proceed with checks until any comes up false. So `gte` on inputs [8,8,3] will return true after checking 8 >= 8, then 8 >= 3. [3,8,8] would return false after checking 3 >= 8. Will return false if only 0 or 1 inputs.
* `param`
  * `operation` - ['gt', 'gte', 'lt', 'lte', 'eq', 'ne'], like MongoDB comparison operators. Default *eq*
  * `reference` - when present, will be appended to the end of the inputs and compared as usual

### compareand

Compare each input to the reference
* `inputs` - any, will check each input against the reference, return true if all inputs satisfy the condition
* `param`
  * `operation` - ['gt', 'gte', 'lt', 'lte', 'eq', 'ne'], like MongoDB comparison operators. Default *eq*
  * `reference` - required, all inputs checked against this

### compareor

Compare each input to the reference
* `inputs` - any, will check each input against the reference, return true if any inputs satisfy the condition
* `param`
  * `operation` - ['gt', 'gte', 'lt', 'lte', 'eq', 'ne'], like MongoDB comparison operators. Default *eq*
  * `reference` - required, all inputs checked against this

## Utility

### echo

Takes a single input, either `pub`ed from or `set` to the target URI
defined for this metric's input.

* `inputs` - 1
* `param` - none

### select

Takes at least a single input, and using Javascript's truthiness rules,
 determines whether to return the *true* case or the *false* case. The 
 true case and false case can either be static by using the param field, 
 or they can be dynamic using the inputs field. The param field takes 
 precedent if it exists.

To determine what the true case is...
* If `param.trueCase` is present, then it is the true case.
* If `param.trueCase` is not present, then `input[1]` is the true case.

To determine what the false case is...
* If `param.falseCase` is present, then it is the false case.
* If `param.falseCase` is not present but `param.trueCase` is, `input[1]` is the false case.
* If `param.falseCase` and `param.trueCase` are both not present, `input[2]` is the false case.

Computation fields:
* `inputs` - 3
  * `0` - the input used as the selector boolean
  * `1` - the true case input if `param.trueCase` is not present, or the false case input if `param.trueCase` is present, but `param.falseCase` is not
  * `2` - the false case input if neither `param.trueCase` nor `param.falseCase` are present
* `param`
  * `trueCase` - the value you want to send if `input[0]` is truthy. This can be false, 0, or empty string (falsy values), omit if you want to use dynamic values from inputs for the select metric
  * `falseCase` - like above, but if `input[0]` is not truthy

### selectn
Acts as a multiplexer to forward one of many input options to the output.
* `inputs` - 2+
  * `0` - the index of the input that should be forwarded to the output. If less than 1 or greater than the max index, defaults to the max index.
  * `1+` - the many inputs that can be selected to be forwarded to the output.

Example:
```jsonc
{
    "id": "UF_slew_rate",
    "inputs": [
        {
            "uri": "/metrics/internal_northfork",
            "id": "selectorn_output"
        },
        {
            "uri": "/metrics/TX100_constants",
            "id": "ffr_uf_slew_rate" // selectorn_output == 1
        },
        {
            "uri": "/metrics/TX100_constants",
            "id": "frrs_uf_slew_rate" // selectorn_output == 2
        },
        {
            "uri": "/metrics/TX100_constants",
            "id": "rrs_gen_uf_slew_rate" // selectorn_output == 3
        },
        {
            "uri": "/metrics/TX100_constants",
            "id": "rrs_load_uf_slew_rate" // selectorn_output == 4
        },
        {
            "uri": "/metrics/TX100_constants",
            "id": "pfr_uf_slew_rate" // selectorn_output == 5
        }
    ],
    "operation": "selectn"
}
```

In the example case, `selectorn_output` is the selector that should hold a value between 1 and 5, inclusive. If `selectorn_output` is 1, then `ffr_uf_slew_rate`'s value will be published to the output (`uf_slew_rate`). If `selectorn_output` is 5, then `pfr_uf_slew_rate`'s value will be published to the output. If `selectorn_output` is less than 1 or greater than 5, it is an invalid input so `selectn` will default to the max index (5) and `pfr_uf_slew_rate`'s value will be published to the output.

### selectorn
Iterates across an array of inputs and outputs the index (starting from 1) of the input that evaluates to JavaScript "truthy" (true, non-zero number, etc.).

If all inputs are false, then the output will be 0.
* `inputs` - 1+
  * `0+` - the many inputs that could be "truthy" and cause their respective index to be the output.

Example:
```jsonc
{
    "id": "selectorn_output",
    "inputs": [
        {
            "uri": "/metrics/internal_northfork",
            "id": "ffr_enable_flag" // if true, output will be 1
        },
        {
            "uri": "/metrics/internal_northfork",
            "id": "frrs_enable_flag" // if true and ffr_enable_flag is false, output will be 2
        },
        {
            "uri": "/metrics/internal_northfork",
            "id": "rrs_gen_enable_flag" // if true and all above inputs are false, output will be 3
        },
        {
            "uri": "/metrics/internal_northfork",
            "id": "rrs_load_enable_flag" // if true and all above inputs are false, output will be 4
        },
        {
            "uri": "/metrics/internal_northfork",
            "id": "uf_events_disable_flag" // if true and all above inputs are false, output will be 5
        }
    ],
    "operation": "selectorn"
}
```

## Special

### runtime

* `inputs` - special order of inputs [charge energy, discharge energy, power output]. Calculates time charge or discharge (depending on sign of power output) time remaining in hours if inputs are given in *kWh* and *kW*
* `param`
  * `gain` - multiplies output in case you want time in minutes or seconds
  * `upper` - limits the runtime calculation in case output power is very low. Applied after `gain`
  * `minP` - if output power input is below `minP`, runtime is calculated at `minP`
  * `defaultP` - overrides `minP` in runtime calculation when output power is below `minP`
* `state` - none

### unicompare

* `inputs` - 1 or 2 numbers, first is the `base` number for the comparison, the second is the optional number to `compare` to (if not included, compare to 0). Returns how much of `base` is supplied by `compare` 
* `param`
  * `invert` - if true, inverts the `base` input
  * `balance` - How much of `base` is not supplied by `compare`
* `state` - none
  
| base | compare      | invert | balance | return | comment                                          |
| ---- | ------------ | ------ | ------- | ------ | ------------------------------------------------ |
| -150 | 600          | true   | false   | 150    | charging from solar, excess available            |
| -150 | 600          | true   | true    | 0      |                                                  |
| -150 | 600          | false  | false   | 0      |                                                  |
| -150 | 600          | false  | true    | 0      |                                                  |
| -150 | 50           | true   | false   | 50     | how much charge from solar, not enough                  |
| -150 | 50           | true   | true    | 100    | how much charge from grid, excess not supplied by solar |
| 150  | not supplied | false  | true   | 150    | discharging                                      |

### quadtosigned

* `inputs` - 1 number 0-3.999 that represents quadrant integer + cos(phi) of power factor within that quadrant, returns signed power factor
* `param` - none
* `state` - none

### signedtoquad

* `inputs` - 1 number, signed power factor and returns 0-1.999. Incomplete inversion of `quadtosigned`
* `param` - none
* `state` - none

### pulse

* `inputs` - 1 or 2 booleans, first is the `trigger` bool for beginning a pulse, the second is the optional bool to `reset` the output.
* `param`
  * `invert` - if true, inverts the output so a pulse goes from high to low then back to high, and reset makes the output high
  * `time` - controls how long the pulse will last, in milliseconds
* `initialValue` - this is optional and IS A PRE-INVERT VALUE. That means if invert is false, then the output will start at the value given by `initialValue`. If invert is true, the output will start at the opposite of `initialValue`.
* `initialInput` - optional. Can be true/false or 1/0. Code can handle either.

### currentTimeMilliseconds

Returns the current time in milliseconds, e.g., 1605715762167. Commonly used to send a heartbeat to determine latency and/or connected status.
* `inputs` - none, but must be included as an empty array: `"inputs": [],`
* `param` - none
* `state` - none

### compareMillisecondsToCurrentTime

Compares the input (usually a milliseconds timestamp) to the current time in milliseconds. If no params are supplied, it returns `Date.now() - [input]`, the difference of the input and the current time. If params are supplied, it returns `(Date.now() - [input]) [param.operation] [param.reference]` as a boolean. For example, `(1605715762000 - 1605715761500) < 1000` returns `true`. In comparing an input to current time, a positive number means that the input is older than the current time. This will usually be the case. Commonly used to determine connected status by checking if a timestamp returned from a remote process is less than X milliseconds older than the current time.

* `inputs` - an integer representing a time in milliseconds, e.g., 1605715762167
* `param`
  * `operation` - ['gt', 'gte', 'lt', 'lte', 'eq', 'ne'], like MongoDB comparison operators. Default *lt*
  * `reference` - reference number for comparing the difference of input and current time: Current time minus input time = X, `X [operation] [reference]` will be true or false.
* `state` - none

### millisecondsToRFC3339

Returns the input (usually a milliseconds timestamp) as a human-readable RFC3339 timestamp string. By default, the string returned shows Zulu (or "UTC", or "GMT") time. Using an operation parameter and an optional reference parameter, you can return the string in local time of the server, or in the time of any timezone.
* `inputs` - an integer representing a time in milliseconds, e.g., 1605715762167
* `param`
  * `operation` - ['zulu', 'localTime', 'timezone'] Default *zulu*
  * `reference` - a number of hours before Zulu time (negative numbers) or after Zulu time (positive numbers). For reference, during Standard Time, the east coast of the US is -5 hours from Zulu time. For example, a reference (an offset) of "-4" will result in a time like "2020-11-21T23:36:46.727-04:00". The default is 0 (which is the same as Zulu time).
* `state` - none

Examples:
  operation = 'zulu':                     2020-11-21T05:50:23.463Z
  operation = 'localTime':                2020-11-21T00:50:23.463-05:00
  operation = 'timezone', reference = -4: 2020-11-21T01:50:23.463-04:00
  operation = 'timezone', reference = 0:  2020-11-21T05:50:23.463+00:00

### combineInputsToArray

Returns inputs, in order, in an array. Usually used for controlling enabled status in the UI by sending the control object's value and enabled status in an array. When used for that purpose, only two inputs are used: the first element of the array is the value that the UI will display for that object, the second element of the array is a boolean controlling whether the object is enabled (true, not grayed out) or not (false, grayed out). When used for the UI, designate the UI endpoint as the output. You will need to set the `controlled_by_metric` value in the UI metadata in `web_ui.json` to `true` so that the value and enabled status will only be changed when this array arrives over FIMS, overriding the normal publish. 
* `inputs` - any values