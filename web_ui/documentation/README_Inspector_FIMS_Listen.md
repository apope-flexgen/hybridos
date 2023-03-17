# UI FIMS Listen on the FIMS page in Inspector

## Goals

* Allow user to drill down to any index of any FIMS message and see realtime display of the value(s) at that index. Make it easy to find pertinent URIs and keys/IDs to quickly get what you want. Allow any number of values to be displayed at once in various formats. Allow realtime calculations on values returned.


## Modules/Repos Affected

* web_ui
* web_server


## Assumptions

* Accessing specific values from FIMS messages in realtime is valuable.
* Doing calculations on specific values from FIMS messages in realtime is valuable.


## Interface

* User enters standard URIs and key/ID names to drill down into FIMS message objects to see value(s) they are interested in. If using multiple index levels, they are written using JavaScript standard "dot" notation, e.g., `/assets/ess/ess_1 maint_mode.options.0.name`. Query results are displayed with the URI and keys of your query: "**/assets/ess/ess_1** active_power_setpoint.value: 0".
* Multiple queries can be performed at the same time, simply separate them with a comma.
* Multiple queries can be wrapped in straight brackets to display the values in neat columns in the order they appear in your query.
* You can apply simple or complex calculations to queries wrapped in straight brackets (even a single query). If you have, for example, three queries returning three values, you can enter a math equation to perform on those queries. Your first query will be represented by "a", your second by "b" and so on up to ten queries ("j"). Simply enter a space and then your calculation after the closing bracket of your queries, for example, `[/assets/ess/ess_1 active_power_setpoint.value, /assets/ess/ess_2 active_power_setpoint.value] MAX(a, b)` or `... (a+b)/2` or `... (a+(b*2))/4` or `... MEAN(SQRT(b), a, a+b)` or whatever you like. (Note that I am not a mathematician, these examples may be uninteresting.)
* When doing calculations on an array of queries, you can add reference queries *before* your bracketed queries. These "references" can then be compared visually to calculated values.
* The URI and key/value object for anything displayed in the UI can be determined by simply clicking in the display area for that URI/key/value *when the Inspector pages are visible*. For example, clicking on "Features/Active Power Management/Site Load Active Power" (click somewhere between the text and the numerical display) shows `/features/active_power '{"site_kW_load": 1398.7576904296875}'` in the browser's console. You can use this info to formulate a FIMS Send or a FIMS Listen.


## Interface Details

* ARRAYS: Put brackets around any query to make it display values only, in neat columns. With an array, you can do calculations like this: "[/assets/ess/ess_1 active_power_setpoint.value, /components/sungrow_ess_1 active_power_setpoint, /site/summary ess_kW_cmd.value] (a+b)/c". Your first query item is represented by "a", your second by "b", etc. You can do any sort of calculation and can use up to ten variables (a through j).
* REFERENCES: When using brackets to do calculations on numbers in an array, you can put additional queries before (outside) the bracketed queries. These "references" can then be compared visually to calculated values.
* FLAGS: The available flags are "-keys", "-uris", "-exact", and "-csv". They can be used in any order and must be separated from other parts of your query with a space.
* KEYS FLAG: Type "-keys" after a URI to see the keys or IDs available at that URI. e.g., "/assets/ess/ess_1 -keys". The query `/assets/ess/ess_1` will return "name,active_power,active_power_setpoint,alarms,apparent_power,...". Once you know the key of the data you want to see, enter a space and the key/ID you want to display, after the URI e.g., "/assets/ess/ess_1 maint_mode". You can enter multiple keys using JavaScript-style dot notation like this: "/assets/ess/ess_1 maint_mode.options.0.name".
* URIS FLAG: Type "-uris" after a URI to see the full URIs available at that URI root. The query `/assets -uris` will return "/assets/ess/ess_1,/assets/ess/ess_2,/assets/ess/summary,/assets/feeders/feed_1,...". A partial match like `/assets/ess -uris` will return only those URIs starting with "/assets/ess", in this example, "/assets/ess/ess_1,/assets/ess/ess_2,/assets/ess/summary".
* EXACT FLAG: Type "-exact" to view only your exact query, e.g. use "/components/bess_aux -exact" if you don't want to see data from "/components/bess_aux_load".
* CSV FLAG: Type "-csv" to output your results in the display area as comma-separated values. These can easily be copy-and-pasted into Excel. In Excel, after pasting, select "Data>Text to Columns..." to separate the CSV into Excel columns. NOTE: Any references, the result of any calculation, and the calculation itself are included in CSV output. Additionally, date, time, and milliseconds columns are added to CSV output for sorting and time reference.
* THROTTLE FLAG: Type "-throttle[any number of milliseconds]" to throttle the results. The default is no throttling; the results are displayed as soon as they arrive.


## Reference

### Operators

*   `+` - Add / Unary Plus
*   `-` - Subtract / Unary Minus
*   `*` - Multiply
*   `/` - Divide
*   `^` - Power
*   `%` - Modulo
*   `(` - Begin Group
*   `)` - End Group
*   `,` - Separate Argument

### Constants

*   `E` - Euler's constant and the base of natural logarithms.
*   `LN2` - Natural logarithm of 2.
*   `LN10` - Natural logarithm of 10.
*   `LOG2E` - Base 2 logarithm of E.
*   `LOG10E` - Base 10 logarithm of E.
*   `PHI` - Golden ratio.
*   `PI` - Ratio of the circumference of a circle to its diameter.
*   `SQRT1_2` - Square root of 1/2.
*   `SQRT2` - Square root of 2.
*   `TAU` - Ratio of the circumference of a circle to its radius.

### Methods

*   `ABS(x)` - Returns the absolute value of a number.
*   `ACOS(x)` - Returns the arccosine of a number.
*   `ACOSH(x)` - Returns the hyperbolic arccosine of a number.
*   `ADD(x, y)` - Returns the total of two numbers.
*   `ASIN(x)` - Returns the arcsine of a number.
*   `ASINH(x)` - Returns the hyperbolic arcsine of a number.
*   `ATAN(x)` - Returns the arctangent of a number.
*   `ATANH(x)` - Returns the hyperbolic arctangent of a number.
*   `ATAN2(y, x)` - Returns the arctangent of the quotient of the arguments.
*   `CBRT(x)` - Returns the cube root of a number.
*   `CEIL(x)` - Returns the smallest integer greater than or equal to a number.
*   `COS(x)` - Returns the cosine of a number.
*   `COSH(x)` - Returns the hyperbolic cosine of a number.
*   `DIVIDE(x, y)` - Returns the quotient of two numbers.
*   `EXP(x)` - Returns E to the power of x.
*   `EXPM1(x)` - Returns subtracting 1 from EXP(x).
*   `FACTORIAL(x)` - Returns the factorial of x.
*   `FLOOR(x)` - Returns the largest integer less than or equal to a number.
*   `HYPOT(x[, y[, ...]])` - Returns the square root of the sum of squares of the arguments.
*   `LOG(x)` - Returns the natural logarithm of a number.
*   `LOG1P(x)` - Returns the natural logarithm of 1 + x.
*   `LOG10(x)` - Returns the base 10 logarithm of a number.
*   `LOG2(x)` - Returns the base 2 logarithm of a number.
*   `MAX(x[, y[, ...]])` - Returns the largest of one or more numbers.
*   `MEAN(x[, y[, ...]])` - Returns the mean of one or more numbers.
*   `MIN(x[, y[, ...]])` - Returns the smallest of one or more numbers.
*   `MOD(x, y)` - Returns the modulus of two numbers.
*   `MULTIPLY(x, y)` - Returns the product of two numbers.
*   `POW(x, y)` - Returns base to the exponent power.
*   `SIN(x)` - Returns the sine of a number.
*   `SINH(x)` - Returns the hyperbolic sine of a number.
*   `SQRT(x)` - Returns the positive square root of a number.
*   `SUBTRACT(x, y)` - Returns the difference of two numbers.
*   `SUM(x[, y[, ...]])` - Returns the sum of one or more numbers.
*   `TAN(x)` - Returns the tangent of a number.
*   `TANH(x)` - Returns the hyperbolic tangent of a number.


-Desmond Mullen, 01/24/20