# Metrics Functions
**Note**: When mixed types are entered into a function, type priority is assigned as follows: float64 > int64 > uint64 > bool. So, if an int64 and a uint64 are entered into a function, the result will default to an int64 result. To modify this behavior, you may use an explicit type-cast function to dictate the output type. Also note that the final result of ANY calculation is converted to the specified output type for that expression, regardless of input and output types.

**Also note**: Many of these functions have multiple aliases. Aliases are given in parentheses next to the function header in this document, e.g. addition can be `Add()` or `Sum()` depending on your preference. It is not difficult to put in a new alias, so if there is a preferred name for these functions, these can be added upon request.
## Math
### Add (Sum)
* Inputs:
  * Any number/type of inputs; order doesn't matter
* Functionality:
  * If only numeric types are entered, all inputs are converted to the same type and added.
  * Adding only bools is equivalent to applying || to the bools.
  * If a string is entered, all inputs are converted to strings and concatenated. Return value is a string.

### Subtract (Sub)
* Inputs:
  * `arg1`: the minuend
  * `arg2`: the subtrahend
* Functionality:
  * Inputs are converted to the same type, then `arg1 - arg2` is performed and returned. If `arg1` and `arg2` are uint64 and the result is negative, the result is converted to an int64, if possible.
  * Subtracting only bools is equivalent to applying != to the bools.
  * Strings cannot be subtracted.

### Multiply (Mult)
* Inputs:
  * Any number/type of inputs (excluding strings); order doesn't matter
* Functionality:
  * If only numeric types are entered, all inputs are converted to the same type and multiplied.
  * Multiplying only bools is equivalent to applying && to the bools.
  * Strings cannot be multiplied.

### Divide (Div)
* Inputs:
  * `arg1`: the dividend
  * `arg2`: the divisor
* Functionality:
  * Inputs are converted to the same type, then `arg1 / arg2` is performed and returned. Remember that integer division acts as floor division.
  * Dividing bools returns `arg1` if `arg2` is `true`.
  * Strings cannot be divided.

### Modulo (Mod)
* Inputs:
  * `arg1`: the dividend
  * `arg2`: the divisor
* Functionality:
  * Inputs are converted to the same type, then `arg1 % arg2` is performed and returned.
  * The `Mod` function cannot be applied to strings, bools, or floats.

### Root
* Inputs:
  * `arg1`: the radicand
  * `arg2`: the index
* Functionality:
  * Returns the `Nth` root of a number as a float. Equavalent to `Pow(arg1, 1/arg2)`.
  * Cannot take the root of a string or use a string as the index.

### SquareRoot (Sqrt)
* Inputs:
  * Any number/type of inputs (excluding strings); order doesn't matter
* Functionality:
  * Inputs are converted to floats, and then the square root of each input is calculated individually and independently. Returns an array of float Unions of len(inputs).
  * Cannot take the square root of a string or a negative number.

### Power (Pow)
* Inputs:
  * `arg1`: the base
  * `arg2`: the exponent
* Functionality:
  * Inputs are converted to floats, then `arg1` is raised to the `arg2` power. The result is converted back to the original result type (determined by both `arg1` and `arg2`).
  * Cannot take the power of a string or use a string as the exponent.

### Max
* Inputs:
  * Any number/type of inputs (excluding strings); order doesn't matter
* Functionality:
  * Finds the maximum numeric value among all inputs.
  * Return value will have the same type as the original maximum. (e.g. If `[int(5), float(4), uint(7)]` is input, then `uint(7)` will be output.)
  * Cannot find the maximum of a list of strings.

### Min
* Inputs:
  * Any number/type of inputs (excluding strings); order doesn't matter
* Functionality:
  * Finds the minimum numeric value among all inputs.
  * Return value will have the same type as the original minimum. (e.g. If `[int(5), float(4), uint(7)]` is input, then `float(4)` will be output.)
  * Cannot find the minimum of a list of strings.

### Average (Avg)
* Inputs:
  * Any number/type of inputs (excluding strings and bools); order doesn't matter
* Functionality:
  * If only numeric types are entered, all inputs are converted to the same type and averaged.
  * Cannot find the average of a list of bools or strings.

### Floor
* Inputs:
  * Any number/type of inputs (excluding strings); order doesn't matter
* Functionality:
  * "Rounds down" floats to the nearest whole number. Floats are left as float-type values.
  * Ints, uints, and bools are left as-is.
  * Returns an array of floored Unions of len(inputs) with their original types.
  * Cannot find the floor of a string.

### Ceiling (Ceil)
* Inputs:
  * Any number/type of inputs (excluding strings); order doesn't matter
* Functionality:
  * "Rounds up" floats to the nearest whole number. Floats are left as float-type values.
  * Ints, uints, and bools are left as-is.
  * Returns an array of ceiling-ed Unions of len(inputs) with their original types.
  * Cannot find the ceiling of a string.
### Round
* Inputs:
  * Any number/type of inputs (excluding strings); order doesn't matter
* Functionality:
  * Rounds floats to the nearest whole number (round down if less than 0.5, round up if greater than or equal to 0.5). Floats are left as float-type values.
  * Ints, uints, and bools are left as-is.
  * Returns an array of rounded Unions of len(inputs) with their original types.
  * Cannot round the value of a string.

### Percent (Pct)
* Inputs:
  * `arg1`: the dividend
  * `arg2`: the divisor
* Functionality:
  * Inputs are converted to floats, then `arg1 / arg2 * 100` is performed and returned.
  * Cannot find the percent of a string.

### FloorDiv
* Inputs:
  * `arg1`: the dividend
  * `arg2`: the divisor
* Functionality:
  * Inputs are converted to the same type, then `Floor(arg1 / arg2)` is performed and returned.
  * Dividing bools returns `arg1` if `arg2` is `true`.
  * Strings cannot be divided.

### AbsoluteValue (Abs)
* Inputs:
  * Any number/type of inputs (excluding strings); order doesn't matter
* Functionality:
  * Finds the absolute value of each individual input value, maintaining its data type. Returns an array of Unions of len(inputs) with their original types.
  * Cannot find the absolute value of a string.

### Residual Sum of Squares (Rss)
* Inputs:
  * Any number/type of inputs (excluding strings); order doesn't matter
* Functionality:
  * All inputs are converted to floats, then each input is squared and all squared inputs are added. The square root of the resulting sum is returned.
  * Cannot use RSS on strings.

## Bit Manipulation
### BitwiseAnd
* Inputs:
  * Any number/type of inputs (excluding strings and floats); order doesn't matter
* Functionality:
  * Inputs are converted to the same type, then the bitwise and of the bits is computed and returned.
  * The `BitwiseAnd` function cannot be applied to strings or floats.

### BitwiseOr
* Inputs:
  * Any number/type of inputs (excluding strings and floats); order doesn't matter
* Functionality:
  * Inputs are converted to the same type, then the bitwise or of the bits is computed and returned.
  * The `BitwiseOr` function cannot be applied to strings or floats.

### BitwiseXor
* Inputs:
  * Any number/type of inputs (excluding strings and floats); order doesn't matter
* Functionality:
  * Inputs are converted to the same type, then the bitwise exclusive or of the bits is computed and returned.
  * The `BitwiseXor` function cannot be applied to strings or floats.

### BitwiseAndNot
* Inputs:
  * Two inputs of type uint, int, or bool; order doesn't matter
* Functionality:
  * Inputs are converted to the same type, then the bitwise and not of the bits is computed and returned. (Note: `and not` is effectively `a && !b`.)
  * The `BitwiseAndNot` function cannot be applied to strings or floats.

### LeftShift
* Inputs:
  * `arg1`: the number to apply the shift to
  * `arg2`: the shift to apply; max of `63`
* Functionality:
  * Inputs are converted to the same type, then bits are shifted left by `arg2` bits.
  * `arg2` must be small enough to keep `arg1` within the 64-bit limit. Overflow will return an error.
  * The `LeftShift` function cannot be applied to strings, bools, or floats.

### RightShift
* Inputs:
  * `arg1`: the number to apply the shift to
  * `arg2`: the shift to apply
* Functionality:
  * Inputs are converted to the same type, then bits are shifted right by `arg2` bits.
  * The result will be `0` if `arg1` is shifted by 63 or more bits.
  * The `RightShift` function cannot be applied to strings, bools, or floats.

## Logic
### And
* Inputs:
  * Any number/type of inputs; order doesn't matter
* Functionality:
  * Inputs are converted to booleans with the following behaviors:
    * Uints, ints, and floats are treated as `true` if non-zero and `false` if and only if they are `0`.
    * Strings are treated as `true` if they are parsed from `"true"` and `false` if they are parsed from `"false"`. If they do not parse to either `true` or `false` directly then strings' truth value is determined by `len(string) > 0`.
  * The logical AND of the arguments is returned as a bool.

### Or
* Inputs:
  * Any number/type of inputs; order doesn't matter
* Functionality:
  * Inputs are converted to booleans with the following behaviors:
    * Uints, ints, and floats are treated as `true` if non-zero and `false` if and only if they are `0`.
    * Strings are treated as `true` if they are parsed from `"true"` and `false` if they are parsed from `"false"`. If they do not parse to either `true` or `false` directly then strings' truth value is determined by `len(string) > 0`.
  * The logical OR of the arguments is returned as a bool.

### Not
* Inputs:
  * Any number/type of inputs; order doesn't matter
* Functionality:
  * Inputs are converted to booleans with the following behaviors:
    * Uints, ints, and floats are treated as `true` if non-zero and `false` if and only if they are `0`.
    * Strings are treated as `true` if they are parsed from `"true"` and `false` if they are parsed from `"false"`. If they do not parse to either `true` or `false` directly then strings' truth value is determined by `len(string) > 0`.
  * Each input is individually and independently logically negated and an array of Unions of len(inputs) is returned.

### Equal
* Inputs:
  * Any number/type of inputs; order doesn't matter
* Functionality:
  * Inputs are converted to the same type with the following behaviors:
    * `true` evaluates to `1` and `false` evaluates to `0` when compared against numeric data types.
    * Strings are NEVER converted to other data types and will yield `false` if compared to numeric types. (e.g. `"5" == 5` and `"true" == true` will both return `false`)
  * All inputs must be equal to return `true`. A single inequivalency will return `false`.

### NotEqual
* Inputs:
  * Any number/type of inputs; order doesn't matter
* Functionality:
  * Inputs are converted to the same type with the following behaviors:
    * `true` evaluates to `1` and `false` evaluates to `0` when compared against numeric data types.
    * Strings are NEVER converted to other data types and will yield `false` if compared to numeric types. (e.g. `"5" == 5` and `"true" == true` will both return `false`)
  * All inputs must be not equal to return `true`. A single equivalency will return `false`.

### LessThan
* Inputs:
  * Any number/type of inputs; operands are compared in the order that they appear (i.e. `arg1 < arg2 < ... < argN`)
* Functionality:
  * Inputs are converted to the same type with the following behaviors:
    * `true` evaluates to `1` and `false` evaluates to `0` when compared against numeric data types.
    * Strings can only be compared against other strings and will yield an error if compared against other types.
  * The inequality must hold true all the way through from left to right for the function to return `true`.

### GreaterThan
* Inputs:
  * Any number/type of inputs; operands are compared in the order that they appear (i.e. `arg1 > arg2 > ... > argN`)
* Functionality:
  * Inputs are converted to the same type with the following behaviors:
    * `true` evaluates to `1` and `false` evaluates to `0` when compared against numeric data types.
    * Strings can only be compared against other strings and will yield an error if compared against other types.
  * The inequality must hold true all the way through from left to right for the function to return `true`.

### LessThanOrEqual
* Inputs:
  * Any number/type of inputs; operands are compared in the order that they appear (i.e. `arg1 <= arg2 <= ... <= argN`)
* Functionality:
  * Inputs are converted to the same type with the following behaviors:
    * `true` evaluates to `1` and `false` evaluates to `0` when compared against numeric data types.
    * Strings can only be compared against other strings and will yield an error if compared against other types.
  * The inequality must hold true all the way through from left to right for the function to return `true`.

### GreaterThan
* Inputs:
  * Any number/type of inputs; operands are compared in the order that they appear (i.e. `arg1 >= arg2 >= ... >= argN`)
* Functionality:
  * Inputs are converted to the same type with the following behaviors:
    * `true` evaluates to `1` and `false` evaluates to `0` when compared against numeric data types.
    * Strings can only be compared against other strings and will yield an error if compared against other types.
  * The inequality must hold true all the way through from left to right for the function to return `true`.

### Set-Reset Flip-Flop (SRFF)
* Inputs:
  * `arg1`: set S
  * `arg2`: reset R
* Functionality:
  * Inputs are converted to booleans with the following behaviors:
    * Uints, ints, and floats are treated as `true` if non-zero and `false` if and only if they are `0`.
    * Strings are treated as `true` if they are parsed from `"true"` and `false` if they are parsed from `"false"`. If they do not parse to either `true` or `false` directly then strings' truth value is determined by `len(string) > 0`.
  * SRFF behaves according to the following truth table:
    | S | R | Output (Q) |
    |:-:|:-:|:----------:|
    | false | false | No change |
    | false | true | false |
    | true | false | true |
    | true | true | true |

### If (IfThen, IfThenElse)
* Inputs:
  * `arg0`: the condition to evaluate
  * `arg1`: the branch to evaluate if `arg0` is true.
  * `arg2` (optional): the branch to evaluate if `arg0` is false.
* Functionality:
  * Works just like an `if` statement. If `arg0` then return `arg1`. If `arg0` is false then return `arg2`.
  * Note: This is one of the only functions where capitalization matters. The `If` MUST be capitalized for the expression to parse correctly.

### SelectN
* Inputs:
  * `arg0`: the index of the input that should be selected
  * `arg1, arg2, arg3,..., argN`: one of the inputs that will be selected. Arg number shown above is the matching index. (i.e. the first item you can select is `arg1` when `arg0 == 1`)
* Functionality:
  * Acts as a multiplexer to forward one of many input options to the output.
  * When `arg0 < 1` or `arg0 > N`, the last previously valid output is shown.

### Enum
* Inputs:
  * `arg0`: the enum value of the input that should be selected
  * `enumVal1, enumString1, enumVal2, enumString2,...,...,enumValN, enumStringN`: the enum value-string pairs. (i.e. if `enumVal5` is `20` and `enumString5` is `"I love puppies"`, then `"I love puppies"` will be outputted when `arg0 == 20`)
* Functionality:
  * Based on `arg0`, select one of the `enumStrings` to output.
  * Each `enumString_i` has a preceding `enumVal_i`. If `arg0 == enumVal_i`, then the output is `enumString_i`.
  * If `arg0` does not match any of the `enumVals`, the output will be `"Unknown"`
  * The input list must be of length `2N + 1`.

### SelectorN
* Inputs:
  * Any number/type of inputs
* Functionality:
  * Iterates across the inputs, then outputs the index (starting from 1) of the input that evaluates to `true` using the conversion rules below.
  * Inputs are converted to booleans with the following behaviors:
    * Uints, ints, and floats are treated as `true` if non-zero and `false` if and only if they are `0`.
    * Strings are treated as `true` if they are parsed from `"true"` and `false` if they are parsed from `"false"`. If they do not parse to either `true` or `false` directly then strings' truth value is determined by `len(string) > 0`.
  * If the first argument is true, `1` will be output (and so on).
  * If no outputs are true, `-1` will be output.

### CompareOr
* Inputs:
  * `comparison operator`: any of the comparison operators as a string (`"==", "!=", "<", ">", "<=", ">="`)
  * `reference`: the value to compare the input(s) to; will be the RIGHT operand of the comparison expression
  * `inputs`: Any number/type of inputs
* Functionality:
  * Individually and independently compare each input to the `reference` using the specified `comparison operator`. The `input` will be the left operand and the `reference` will be the right operand, e.g. `input < reference`.
  * Once all comparisons have been made, take the resulting truth values and `Or` them together.
  * Example usage: `CompareOr(\"<\", 5, int1)`. Note that the quotes surround the comparison operator are required and will need to be escaped with backslashes, as shown.

### CompareAnd
* Inputs:
  * `comparison operator`: any of the comparison operators as a string (`"==", "!=", "<", ">", "<=", ">="`)
  * `reference`: the value to compare the input(s) to; will be the RIGHT operand of the comparison expression
  * `inputs`: Any number/type of inputs
* Functionality:
  * Individually and independently compare each input to the `reference` using the specified `comparison operator`. The `input` will be the left operand and the `reference` will be the right operand, e.g. `input < reference`.
  * Once all comparisons have been made, take the resulting truth values and `And` them together.
  * Example usage: `CompareAnd(\"<\", 5, int1)`. Note that the quotes surround the comparison operator are required and will need to be escaped with backslashes, as shown.

## Type Conversion
### Bool
* Inputs:
  * Any number/type of inputs
* Functionality:
  * Each input is converted to a boolean value. Output is an array of Unions of len(inputs).
    * Bool → bool: value remains the same
    * Uint → bool: returns `val != 0`
    * Int → bool: returns `val != 0`
    * Float → bool: returns `val != 0.0`
    * String → bool: If `string == "true"` returns `true`; if `string == "false"` returns `false`; else returns `len(string) > 0`

### Uint
* Inputs:
  * Any number/type of inputs
* Functionality:
  * Each input is converted to an unsigned integer value. Output is an array of Unions of len(inputs).
    * Bool → uint: `false` → `0`, `true` → `1`
    * Uint → uint: value remains the same
    * Int → uint: If `val >= 0`, value remains the same. If `val < 0`, returns an error.
    * Float → uint: If `val >= 0`, returns `Floor(val)`. If `val < 0` or `val > math.MaxUint64`, returns an error.
    * String → uint: Attempt to parse an unsigned integer. Return the parsed value or an error.

### Int
* Inputs:
  * Any number/type of inputs
* Functionality:
  * Each input is converted to a signed integer value. Output is an array of Unions of len(inputs).
    * Bool → int: `false` → `0`, `true` → `1`
    * Uint → int: If `val <= math.MaxInt64`, value remains the same. If `val > math.MaxInt64`, returns an error.
    * Int → int: value remains the same.
    * Float → int: If `0 <= val <= math.MaxInt64`, returns `Floor(val)`. If `math.MinInt64 <= val < 0`, returns `-Floor(Abs(val))` (i.e. truncates the decimal). If `val < math.MinInt64` or `val > math.MaxInt64`, returns an error.
    * String → int: Attempt to parse a signed integer. Return the parsed value or an error.

### Float
* Inputs:
  * Any number/type of inputs
* Functionality:
  * Each input is converted to a float value. Output is an array of Unions of len(inputs).
    * Bool → float: `false` → `0.0`, `true` → `1.0`
    * Uint → float: Direct type cast to float. May lose precision.
    * Int → float: Direct type cast to float. May lose precision.
    * Float → float: value remains the same.
    * String → float: Attempt to parse a float value. Return the parsed value or an error.

### String
* Inputs:
  * Any number/type of inputs
* Functionality:
  * Each input is converted to a string value. Output is an array of Unions of len(inputs).
    * Bool → string: returns `"true"` or `"false"`
    * Uint → string: returns unpadded integer value as a string
    * Int → string: returns unpadded integer value as a string
    * Float → string: returns `%f` string-formatted value
    * String → string: value remains the same.

## Special
### ValueChanged
* Input:
  * A single input of any type
* Functionality
  * Returns `true` if the input has changed since the last received value. Returns `false` if the value has not changed.
### QuadToSigned
* Inputs:
  * `arg1`: a single number in the range of `[0, 4)` that represents a quadrant integer + cos(ϕ) of power factor within that quadrant.
* Functionality:
  * Returns signed power factor of the input.
    * If `arg1 < 0 || arg.f >= 4`: return `0` 
      * else if `arg1 < 1`: return `arg1`
	      * else if `arg1 < 2`: return `-(arg1 - 1)` 
	        * else if `arg1 < 3`: return `arg1 - 2`
              * else: return `-(arg1 - 3)` 
### SignedToQuad
* Inputs:
  * `arg1`: A signed power factor.
* Functionality:
  * Returns an incomplete inversion of `QuadToSigned`.
    * If `arg1 <= -1`: return `-1`
      * else if `arg1 < 0`: return `1 - arg1`
        * else if `arg1 < 1`: return `arg1`
          * else: return `1`

## Time-based Functions
### Integrate
* Inputs:
  * `arg1`: the value to integrate with respect to time
* Functionality:
  * Uses rectangle-rule integration of the input integrated over the time since the last update. 
  * Timescale is 1 hour, e.g. if `input` is in kW, the result is in kWh
  * Note: This function is missing the previous parameters for `abs`, `minuteReset`, and `minuteOffset`. The `abs` parameter can be applied using the `Abs()` function, but `minuteReset` and `minuteOffset` have no real equivalent. If this functionality is needed, it can be added back in; just let us know!

### IntegrateOverTimescale
* Inputs:
  * `arg1`: the value to integrate with respect to time
  * `arg2`: the timescale to integrate over, in hours
* Functionality:
  * Uses rectangle-rule integration of the input integrated over the time since the last update. 
  * Timescale is 1 hour, e.g. if `input` is in kW, the result is in kWh
  * Note: This function is missing the previous parameters for `abs`, `minuteReset`, and `minuteOffset`. The `abs` parameter can be applied using the `Abs()` function, but `minuteReset` and `minuteOffset` have no real equivalent. If this functionality is needed, it can be added back in; just let us know!

### CurrentTimeMilliseconds (Time)
* Inputs:
  * None
* Functionality:
  * Returns the current time in milliseconds, e.g., 1605715762167. Commonly used to send a heartbeat to determine latency and/or connected status.

### MillisecondsSince
* Inputs:
  * `arg1`: an int or uint representing a time in milliseconds
* Functionality:
  * Returns `time.Now() - arg1`.
  * If you want to _compare_ `time.Now()` to the specified time, you can use the `Time()` function and a comparison operator _instead_ of the old metrics function `compareMillisecondsToCurrentTime`, e.g. `Time() > arg1`. 

### MillisecondsToRFC3339 (RFC3339)
* Inputs:
  * `arg1`: an int or uint representing a time in milliseconds
* Functionality:
  * Returns the input as a human-readable RFC3339 timestamp string. By default, the string returned shows Zulu (or "UTC", or "GMT") time.
  * Note: The `localTime` and `timezone` parameters, and the `reference` parameter were not included but can be upon request.

### Pulse
* Inputs:
  * `trigger`: bool for beginning a pulse
  * `reset`: bool to reset the output (required, but can be a constant `true` or `false` instead of a variable)
  * `timeout`: controls how long the pulse will last, in milliseconds
* Functionality:
  * When `trigger` occurs, set the output to `true` for `timeout` milliseconds.
  * If `reset` is true, set the output to `false`, regardless of trigger state. Setting the `reset` back to false will allow the `trigger` to "work" again.
  * To invert the output, use the unary operator `!`, e.g. `!Pulse(trigger, reset, timeout)`.

### MaxOverTimescale
* Inputs:
  * `input`: the value to be compared over time
  * `timeframe`: the time period in milliseconds to compare `input`
* Functionality:
  * Finds the max value of `input` in the last `timeframe` milliseconds.
  * If no values appear within timeframe, returns the last received value of `input`.

### MinOverTimescale
* Inputs:
  * `input`: the value to be compared over time
  * `timeframe`: the time period in milliseconds to compare `input`
* Functionality:
  * Finds the min value of `input` in the last `timeframe` milliseconds.
  * If no values appear within timeframe, returns the last received value of `input`.

### AvgOverTimescale
* Inputs:
  * `input`: the value to be averaged over time
  * `timeframe`: the time period in milliseconds to average `input`
* Functionality:
  * Finds the average of all received values of `input` in the last `timeframe` milliseconds.
  * If no values appear within timeframe, returns the last received value of `input`.

### SumOverTimescale
* Inputs:
  * `input`: the value to be summed over time
  * `timeframe`: the time period in milliseconds to sum `input`
* Functionality:
  * Finds the sum of all received values of `input` in the last `timeframe` milliseconds.
  * If no values appear within timeframe, returns the last received value of `input`.

### ValueChangedOverTimescale
* Inputs:
  * `input`: the value to be compared over time
  * `timeframe`: the time period in milliseconds to compare `input`
* Functionality:
  * If the `input` value changes at all within the `timeframe`, returns true. (e.g. If it takes on the values of [1, 2, 3, 2, 1] in the time frame, the function will return true.)
  * Potentially useful for rapidly changing values that may not be displayed in regular go_metrics publishes.
