To determine how far in the future you can use `get_time_double()` and still retain microsecond resolution, we need to look at the properties of the `double` data type.

A `double` in C++ typically uses IEEE 754 double-precision floating point format, which has a 52-bit significand (also known as the fraction or mantissa) plus 1 hidden bit. 

To retain microsecond resolution, you would need enough precision to represent 1e-6 accurately. 

Let's compute the maximum value where a double can still have a microsecond precision:

Given a 53-bit precision:
\[ \frac{1}{2^{53}} \]

To find the value \( x \) at which \( \frac{1}{x} = \frac{1}{2^{53}} \), which is roughly \( 1e-6 \):

\[ x = 2^{53} \]
\[ x \approx 9.007 \times 10^{15} \]

This value represents the number of seconds. To convert it to years for a more intuitive understanding:

\[ \text{seconds in a year} \approx 3.154 \times 10^{7} \]

\[ \text{years} = \frac{9.007 \times 10^{15}}{3.154 \times 10^{7}} \]
\[ \text{years} \approx 2.854 \times 10^{8} \]

So, you can use `get_time_double()` for approximately 285 million years before you lose microsecond resolution. 

In practical terms, you are safe using `double` to represent time with microsecond resolution for any foreseeable application!