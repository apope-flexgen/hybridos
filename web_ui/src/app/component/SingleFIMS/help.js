/* eslint-disable react/no-unescaped-entities */
/* eslint-disable react/prop-types */
/* eslint-disable max-len */
import React from 'react';

/**
 * Renders help messages for FIMS
 * @param {*} props
 */
const FIMSListenHelp = (props) => (
    <>
        <div id='full-page-help'>
            <div style={{ textAlign: 'right' }}><btn id='close-button' onClick={props.onClick}>X</btn></div>
            <h2 id="ui-fims-listen-on-the-fims-page-in-inspector">FIMS Listen in the UI</h2>
            <h3 id="interface">Interface</h3>
            <ul>
                <li>User enters standard URIs and key/ID names to drill down into FIMS message objects to see value(s) they are interested in. If using multiple index levels, they are written using JavaScript standard &quot;dot&quot; notation, e.g., <code>/assets/ess/ess_1 maint_mode.options.0.name</code>. Query results are displayed with the URI and keys of your query: &quot;<strong>/assets/ess/ess_1</strong> active_power_setpoint.value: 0&quot;.</li>
                <li>Multiple queries can be performed at the same time, simply separate them with a comma.</li>
                <li>Multiple queries can be wrapped in straight brackets to display the values in neat columns in the order they appear in your query.</li>
                <li>You can apply simple or complex calculations to queries wrapped in straight brackets (even a single query). If you have, for example, three queries returning three values, you can enter a math equation to perform on those queries. Your first query will be represented by &quot;a&quot;, your second by &quot;b&quot; and so on up to ten queries (&quot;j&quot;). Simply enter a space and then your calculation after the closing bracket of your queries, for example, <code>[/assets/ess/ess_1 active_power_setpoint.value, /assets/ess/ess_2 active_power_setpoint.value] MAX(a, b)</code> or <code>... (a+b)/2</code> or <code>... (a+(b*2))/4</code> or <code>... MEAN(SQRT(b), a, a+b)</code> or whatever you like. (Note that I am not a mathematician, these examples may be uninteresting.)</li>
                <li>When doing calculations on an array of queries, you can add reference queries <em>before</em> your bracketed queries. These &quot;references&quot; can then be compared visually to calculated values.</li>
                <li>The URI and key/value object for anything displayed in the UI can be determined by simply clicking in the display area for that URI/key/value <em>when the Inspector pages are visible</em>. For example, clicking on &quot;Features/Active Power Management/Site Load Active Power&quot; (click somewhere between the text and the numerical display) shows <code>/features/active_power &#39;&#123;&quot;site_kW_load&quot;: 1398.7576904296875}&#39;</code> in the browser&#39;s console. You can use this info to formulate a FIMS Send or a FIMS Listen.</li>
            </ul>
            <h3 id="interface-details">Interface Details</h3>
            <ul>
                <li>ARRAYS: Put brackets around any query to make it display values only, in neat columns. With an array, you can do calculations like this: &quot;[/assets/ess/ess_1 active_power_setpoint.value, /components/sungrow_ess_1 active_power_setpoint, /site/summary ess_kW_cmd.value] (a+b)/c&quot;. Your first query item is represented by &quot;a&quot;, your second by &quot;b&quot;, etc. You can do any sort of calculation and can use up to ten variables (a through j).</li>
                <li>REFERENCES: When using brackets to do calculations on numbers in an array, you can put additional queries before (outside) the bracketed queries. These &quot;references&quot; can then be compared visually to calculated values.</li>
                <li>FLAGS: The available flags are &quot;-keys&quot;, &quot;-uris&quot;, &quot;-exact&quot;, and &quot;-csv&quot;. They can be used in any order and must be separated from other parts of your query with a space.</li>
                <li>KEYS FLAG: Type &quot;-keys&quot; after a URI to see the keys or IDs available at that URI. e.g., &quot;/assets/ess/ess_1 -keys&quot;. The query <code>/assets/ess/ess_1</code> will return &quot;name,active_power,active_power_setpoint,alarms,apparent_power,...&quot;. Once you know the key of the data you want to see, enter a space and the key/ID you want to display, after the URI e.g., &quot;/assets/ess/ess_1 maint_mode&quot;. You can enter multiple keys using JavaScript-style dot notation like this: &quot;/assets/ess/ess_1 maint_mode.options.0.name&quot;.</li>
                <li>URIS FLAG: Type &quot;-uris&quot; after a URI to see the full URIs available at that URI root. The query <code>/assets -uris</code> will return &quot;/assets/ess/ess_1,/assets/ess/ess_2,/assets/ess/summary,/assets/feeders/feed_1,...&quot;. A partial match like <code>/assets/ess -uris</code> will return only those URIs starting with &quot;/assets/ess&quot;, in this example, &quot;/assets/ess/ess_1,/assets/ess/ess_2,/assets/ess/summary&quot;.</li>
                <li>EXACT FLAG: Type &quot;-exact&quot; to view only your exact query, e.g. use &quot;/components/bess_aux -exact&quot; if you don&#39;t want to see data from &quot;/components/bess_aux_load&quot;.</li>
                <li>CSV FLAG: Type &quot;-csv&quot; to output your results in the display area as comma-separated values. These can easily be copy-and-pasted into Excel. In Excel, after pasting, select &quot;Data&gt;Text to Columns...&quot; to separate the CSV into Excel columns. NOTE: Any references, the result of any calculation, and the calculation itself are included in CSV output. Additionally, date, time, and milliseconds columns are added to CSV output for sorting and time reference.</li>
                <li>THROTTLE FLAG: Type &quot;-throttle[any number of milliseconds]&quot; to throttle the results. The default is no throttling; the results are displayed as soon as they arrive.</li>
            </ul>
            <h3 id="reference">Reference</h3>
            <h4 id="operators">Operators</h4>
            <ul>
                <li><code>+</code> - Add / Unary Plus</li>
                <li><code>-</code> - Subtract / Unary Minus</li>
                <li><code>*</code> - Multiply</li>
                <li><code>/</code> - Divide</li>
                <li><code>^</code> - Power</li>
                <li><code>%</code> - Modulo</li>
                <li><code>(</code> - Begin Group</li>
                <li><code>)</code> - End Group</li>
                <li><code>,</code> - Separate Argument</li>
            </ul>
            <h4 id="constants">Constants</h4>
            <ul>
                <li><code>E</code> - Euler&#39;s constant and the base of natural logarithms.</li>
                <li><code>LN2</code> - Natural logarithm of 2.</li>
                <li><code>LN10</code> - Natural logarithm of 10.</li>
                <li><code>LOG2E</code> - Base 2 logarithm of E.</li>
                <li><code>LOG10E</code> - Base 10 logarithm of E.</li>
                <li><code>PHI</code> - Golden ratio.</li>
                <li><code>PI</code> - Ratio of the circumference of a circle to its diameter.</li>
                <li><code>SQRT1_2</code> - Square root of 1/2.</li>
                <li><code>SQRT2</code> - Square root of 2.</li>
                <li><code>TAU</code> - Ratio of the circumference of a circle to its radius.</li>
            </ul>
            <h4 id="methods">Methods</h4>
            <ul>
                <li><code>ABS(x)</code> - Returns the absolute value of a number.</li>
                <li><code>ACOS(x)</code> - Returns the arccosine of a number.</li>
                <li><code>ACOSH(x)</code> - Returns the hyperbolic arccosine of a number.</li>
                <li><code>ADD(x, y)</code> - Returns the total of two numbers.</li>
                <li><code>ASIN(x)</code> - Returns the arcsine of a number.</li>
                <li><code>ASINH(x)</code> - Returns the hyperbolic arcsine of a number.</li>
                <li><code>ATAN(x)</code> - Returns the arctangent of a number.</li>
                <li><code>ATANH(x)</code> - Returns the hyperbolic arctangent of a number.</li>
                <li><code>ATAN2(y, x)</code> - Returns the arctangent of the quotient of the arguments.</li>
                <li><code>CBRT(x)</code> - Returns the cube root of a number.</li>
                <li><code>CEIL(x)</code> - Returns the smallest integer greater than or equal to a number.</li>
                <li><code>COS(x)</code> - Returns the cosine of a number.</li>
                <li><code>COSH(x)</code> - Returns the hyperbolic cosine of a number.</li>
                <li><code>DIVIDE(x, y)</code> - Returns the quotient of two numbers.</li>
                <li><code>EXP(x)</code> - Returns E to the power of x.</li>
                <li><code>EXPM1(x)</code> - Returns subtracting 1 from EXP(x).</li>
                <li><code>FACTORIAL(x)</code> - Returns the factorial of x.</li>
                <li><code>FLOOR(x)</code> - Returns the largest integer less than or equal to a number.</li>
                <li><code>HYPOT(x[, y[, ...]])</code> - Returns the square root of the sum of squares of the arguments.</li>
                <li><code>LOG(x)</code> - Returns the natural logarithm of a number.</li>
                <li><code>LOG1P(x)</code> - Returns the natural logarithm of 1 + x.</li>
                <li><code>LOG10(x)</code> - Returns the base 10 logarithm of a number.</li>
                <li><code>LOG2(x)</code> - Returns the base 2 logarithm of a number.</li>
                <li><code>MAX(x[, y[, ...]])</code> - Returns the largest of one or more numbers.</li>
                <li><code>MEAN(x[, y[, ...]])</code> - Returns the mean of one or more numbers.</li>
                <li><code>MIN(x[, y[, ...]])</code> - Returns the smallest of one or more numbers.</li>
                <li><code>MOD(x, y)</code> - Returns the modulus of two numbers.</li>
                <li><code>MULTIPLY(x, y)</code> - Returns the product of two numbers.</li>
                <li><code>POW(x, y)</code> - Returns base to the exponent power.</li>
                <li><code>SIN(x)</code> - Returns the sine of a number.</li>
                <li><code>SINH(x)</code> - Returns the hyperbolic sine of a number.</li>
                <li><code>SQRT(x)</code> - Returns the positive square root of a number.</li>
                <li><code>SUBTRACT(x, y)</code> - Returns the difference of two numbers.</li>
                <li><code>SUM(x[, y[, ...]])</code> - Returns the sum of one or more numbers.</li>
                <li><code>TAN(x)</code> - Returns the tangent of a number.</li>
                <li><code>TANH(x)</code> - Returns the hyperbolic tangent of a number.</li>
            </ul>
            <p>-Desmond Mullen, 01/24/20</p>
        </div>
    </>
);
export default FIMSListenHelp;
