# Missing/Altered Features from Metrics v1
**Note**: These were the differences I found upon first pass. If any of these functionalities need to be reverted back to their original functioning (or need to be included), please let us know!

## Functions
### Integrate
* `abs` parameter can be replaced by a call to `Abs()`

### Length
* Not implemented.
* To count alarms or faults, can instead do: `Sum(Int(all_alarms))`, i.e. convert the boolean alarm values to integers (`0`/`1`) and add them together.

### BitfieldPositionCount
* Not implemented.
* Currently would have to do something manually, e.g. `Sum(Int(val1 == 0, val2 == 0, val3 == 0))` or `Sum(Int(Not(all_vals)))`

### Compare
* For single arguments, just use comparison operators, e.g. `a < b`.
* If wanting to compare multiple values in order, use the functional form, e.g. `LessThan(a, b, c, d, e)`.
* The `Compare` function can be used to produce an array of bool values from individual comparison to the reference. Can then perform other functions on the result.

### Select
* Replaced by `If` function. Takes the form of `If(condition, trueCase, falseCase)`

### SelectN
* Previous behavior: If selection index is less than 1 or greater than the max index, defaults to the max index.
* Current behavior: If selection index is less than 1 or greater than the max index, defaults to the last valid output value.

### SelectorN
* Previous behavior: If all inputs are false, then output will be 0.
* Current behavior: If all inputs are false, then output will be -1.

### CombineInputsToArray
* Not implemented.
* Output values cannot be arrays. However, "arrays" of values can be handled WITHIN a metrics expression (e.g. filter variables that reference more than one value)