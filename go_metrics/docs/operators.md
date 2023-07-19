# Metrics Operators
All operators are the equivalent of applying the corresponding [metrics function](functions.md) with two operands in the order that the operands appear.
## Binary Operators
The following binary operators are supported:
* `+` : Addition or concatenation
* `-` : Subtraction
* `*` : Multiplication
* `/` : Division
* `%` : Modulo
* `&` : Bitwise and
* `|` : Bitwise or
* `^` : Bitwise xor
* `<<`: Left shift
* `>>`: Right shift
* `&^`: Bitwise and not
* `&&`: Logical and
* `||`: Logical or
* `==`: Equal
* `!=`: Not equal
* `<`: Less than
* `>`: Greater than
* `<=`: Less than or equal
* `>=`: Greater than or equal

## Unary Operators
The following unary operators are supported:
* `!` : Logical negation
* `-` : Negation

Note: an attempt was made to integrate other unary operators such as `++` and `--`, but the Golang parser didn't parse these operators correctly so they were removed.