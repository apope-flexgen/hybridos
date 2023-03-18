// All computation modules expect an array as first input (even if single value)
// second argument a state object
// third argument an param object

/**
 * Checks if value is in bounds, otherwise returns bounds
 * @param {*} x value to compare
 * @param {*} upper upper limit
 * @param {*} lower lower limit
 * @returns {number} returns value or bounds
 */
function inRangeAndCoerce(x, upper, lower) {
    if ((upper || upper === 0) && x > upper) return upper;
    if ((lower || lower === 0) && x < lower) return lower;
    return x;
}

/**
 * Computations
 * @module metrics/computations
 */
module.exports = {
    // eslint-disable-next-line no-unused-vars
    /**
     * Returns max of arguments
     * @param {Array} x array of values
     * @param {object} y unused
     * @param {object} z unused
     * @returns {number} max value
     */
    max: (x, y, z) => Math.max(...x),
    // eslint-disable-next-line no-unused-vars
    /**
     * Returns min of arguments
     * @param {Array} x array of values
     * @param {object} y unused
     * @param {object} z unused
     * @returns {number} min value
     */
    min: (x, y, z) => Math.min(...x),
    /**
     * Evalutes addition and subtraction of inputs with optional offset
     * @param {Array} inputs array of values
     * @param {object} state unused
     * @param {object} param holds addition and subtraction operators as well as offset
     * @returns {number} value with or without offset
     */
    sum: (inputs, state, param) => {
        const ops = param ? param.operations : false;
        let result;
        if (ops) {
            if (ops.length !== inputs.length) throw new Error('Sum: number of inputs and length of operations string must match');
            result = inputs.reduce((u, v, i) => {
                if (ops[i] === '+') return u + v;
                if (ops[i] === '-') return u - v;
                throw new Error('Sum: non + or - character found in operations');
            }, 0);
        } else result = inputs.reduce((u, v) => u + v, 0);
        return param && param.offset ? param.offset + result : result;
    },
    // eslint-disable-next-line no-unused-vars
    /**
     * Returns sum of all values
     * @param {Array} x array of values
     * @param {object} y unused
     * @param {object} z unused
     * @returns {number} min value
     */
    add: (x, y, z) => x.reduce((u, v) => u + v, 0),
    /**
     * Evaluates multiplication and division of inputs with optional gain
     * @param {Array} inputs array of values
     * @param {object} state unused
     * @param {object} param holds multiplication and division operators as well as offset
     * @returns {number} value with or without gain
     */
    product: (inputs, state, param) => {
        const ops = param ? param.operations : false;
        const upper = param ? param.upperLimit : false;
        const lower = param ? param.lowerLimit : false;
        const gain = param ? param.gain : false;
        let result;
        if (ops) {
            if (ops.length !== inputs.length) throw new Error('Product: number of inputs and length of operations string must match');
            result = inputs.reduce((u, v, i) => {
                if (ops[i] === '*') return u * v;
                if (ops[i] === '/') {
                    if (u === 0) return 0;
                    if (v === 0) {
                        if (u >= 0) return upper || 0;
                        return lower || 0;
                    }
                    return inRangeAndCoerce(u / v, upper, lower);
                }
                throw new Error('Product: non * or / character found in operations');
            }, 1);
        } else result = inputs.reduce((u, v) => u * v, 1);
        result = gain ? gain * result : result;
        return inRangeAndCoerce(result, upper, lower);
    },
    // eslint-disable-next-line no-unused-vars
    /**
     * Returns mean of all values
     * @param {Array} x array of values
     * @param {object} y unused
     * @param {object} z unused
     * @returns {number} average value
     */
    average: (x, y, z) => {
        const sum = x.reduce((u, v) => (u + v), 0);
        return sum / x.length;
    },
    // eslint-disable-next-line no-unused-vars
    /**
     * Echoes a value
     * @param {Array} x singular input
     * @param {object} y unused
     * @param {object} z unused
     * @returns {number} echoed value
     */
    echo: (x, y, z) => {
        // Turn sets into pubs
        if (x.length !== 1) throw new Error('Echo takes exactly 1 input');
        return x[0];
    },
    /**
     * Rectangle rule numerical integration of current input and time since last update
     * @param {Array} inputs singular input
     * @param {object} state holds time data and new value
     * @param {object} param holds multiplication and division operators as well as offset
     * @returns {number} integrated value
     */
    integrate: (inputs, state, param) => {
        // May want to be more explicit about method, ZOH,
        // trapezoidal, maybe Newton, probably not Euler?
        if (inputs.length !== 1) throw new Error('Integrate takes exactly 1 input');
        const input = inputs[0];
        const timescale = param.timescale ? param.timescale * 3600 * 1000
            : 3600 * 1000; // 1 hour default
        const timestamp = (new Date()).getTime();
        const minutes = (new Date()).getMinutes();
        /* eslint-disable no-param-reassign */
        if (state.timestamp) {
            const delta = (input * (timestamp - state.timestamp)) / timescale;
            if (param.minuteReset && minutes !== state.minute
                && !((minutes - (param.minuteOffset ? param.minuteOffset : 0))
                    % param.minuteReset)) {
                state.value = param.abs ? Math.abs(delta) : delta;
            } else {
                state.value += param.abs ? Math.abs(delta) : delta;
            }
        }
        state.minute = minutes;
        state.timestamp = timestamp;
        /* eslint-enable no-param-reassign */
        return state.value;
    },
    /**
     * Bitwise or bitfield 'and' operator of any type of value
     * @param {Array} inputs array of values
     * @param {object} state unused
     * @param {object} param sets if evaluating bitfield or bitwise
     * @returns {number} resulting value
     */
    and: (inputs, state, param) => {
        // ex. "status":[{"value":5,"string":"Fault"},{"value":7,"string":"Stop"}],
        // Defaulting to bitwise and and write out as an integer
        const bitfield = [];
        inputs.forEach((bf) => {
            bf.forEach((bit) => {
                bitfield[bit.value] = bit;
            });
        });
        if (param && param.bitfield) return bitfield.filter((x) => x);
        return bitfield.reduce((u, v) => u + 2 ** v.value, 0);
    },
    // eslint-disable-next-line no-unused-vars
    /**
     * Evaluates any truthy values
     * @param {Array} inputs array of values
     * @returns {boolean} true if any are true
     */
    or: (inputs, state, param) => inputs.some((x) => x),
    // Defaulting to OR of all input booleans into a single output boolean
    // eslint-disable-next-line no-unused-vars
    /**
     * No implementation
     */
    enum: (inputs, state, param) => {
        // Send out strings in response to integers
    },
    /**
     * Calculates time charge or discharge (depending on sign of power output) time remaining in hours if inputs are given in *kWh* and *kW*
     * @param {Array} inputs special order of inputs [charge energy, discharge energy, power output].
     * @param {object} state unused
     * @param {object} param optional parameters
     * @returns {number} time charge or discharge
     */
    runtime: (inputs, state, param) => {
        // Special function with defined order of inputs
        const Echg = inputs[0];
        const Edschg = inputs[1];
        const P = inputs[2];
        const gain = param ? param.gain : false;
        const upper = param ? param.upperLimit : false;
        const minP = param ? param.minP : 0;
        const defaultP = param ? (param.defaultP || minP) : minP;
        if (P === 0) return upper || 0;
        // If P below threshold, return discharge time at default power specified
        if (Math.abs(P) <= minP) {
            return inRangeAndCoerce(gain ? (gain * Edschg) / defaultP
                : Edschg / defaultP, upper, null);
        }
        if (P >= 0) return inRangeAndCoerce(gain ? (gain * Edschg) / P : Edschg / P, upper, null);
        return inRangeAndCoerce(gain ? (gain * Echg) / -P : Echg / -P, upper, null);
    },
    /**
     * Packs boolean or truthy inputs into a bitfield
     * @param {Array} inputs array of inputs
     * @param {object} state unused
     * @param {object} param optional parameters
     * @returns {Array} resultant bitfield
     */
    bitfield: (inputs, state, param) => {
        const pos = param ? param.position : null;
        const strings = param ? param.string : null;
        const invert = param ? param.invertMask : null;
        const output = [];
        inputs.forEach((x, i) => {
            const entry = {};
            const check = !(invert && invert[i]);
            if (x === check) {
                entry.value = pos ? pos[i] : i;
                entry.string = strings ? strings[i] : `Position ${i} reads ${x}`;
                output.push(entry);
            }
        });
        return output;
    },
    /**
     * By default, counts all instances of truthy values in inputs
     * @param {Array} inputs array of inputs
     * @param {object} state unused
     * @param {object} param optional parameters
     * @returns {number} count of truthy values
     */
    bitfieldpositioncount: (inputs, state, param) => {
        // By default, counts all instances of truthy values in inputs
        // Truthy values include any non-empty string and any non-zero number
        // ex. "status":[{"value":5,"string":"Fault"},{"value":7,"string":"Stop"}]
        const invert = param ? param.invert : null;
        const pos = param ? param.position : 0;
        const output = inputs.reduce((prev, current) => {
            const found = current.some((x) => x.value === pos);
            let theInvertValue;
            if (found) {
                theInvertValue = invert ? 0 : 1;
            } else {
                theInvertValue = invert ? 1 : 0;
            }
            return prev + theInvertValue;
        }, 0);
        return output;
    },
    // eslint-disable-next-line no-unused-vars
    /**
     * Calculates square root of inputs
     * @param {Array} inputs array of inputs
     * @param {object} state unused
     * @param {object} param unused
     * @returns {number} square root of inputs
     */
    rss: (inputs, state, param) => Math.sqrt(inputs.reduce((x, y) => x + (y ** 2), 0)),
    /**
     * Calculates how much of `base` is supplied by `compare`
     * @param {Array} inputs array of inputs
     * @param {object} state unused
     * @param {object} param optional parameters
     * @returns {number} comparison value
     */
    unicompare: (inputs, state, param) => {
        const invert = param ? param.invert : false; // May need to invert the
        // first input if comparing charging battery to discharging solar
        const balance = param ? param.balance : false; // To find what amount
        // of the first input is not supplied by the second (so how much charging from the grid)
        const compare = inputs[1] ? inputs[1] : 0; // If second input is not supplied, compare
        // to zero (good for discharge with balance:true)
        let base = invert ? -inputs[0] : inputs[0];
        base = base > 0 ? base : 0; // unidirectional compare, don't want to consider
        // discharging as negative charging
        if (compare > base) return balance ? 0 : base;
        return balance ? base - compare : compare;
    },
    // eslint-disable-next-line no-unused-vars
    /**
     * Implementation of an SR flip-flop (S dominated), with the truth table:
     * | S   | R   | Output (Q) |
     * | --- | --- | ---------- |
     * | 0   | 0   | No change  |
     * | 0   | 1   | 0          |
     * | 1   | 0   | 1          |
     * | 1   | 1   | 1          |
     * @param {Array} inputs array of inputs
     * @param {object} state last ouput value
     * @param {object} param unused
     * @returns {number} comparison value
     */
    srff: (inputs, state, param) => {
        if (inputs.length !== 2) throw new Error(`srff takes exactly 2 inputs, got ${inputs.length}`);
        /* eslint-disable no-param-reassign */
        if (!state) state = { q: false };
        // eslint-disable-next-line no-prototype-builtins
        else if (!state.hasOwnProperty('q')) state.q = false;
        const [s, r] = inputs;
        if (s) {
            state.q = true;
        } else if (r) {
            state.q = false;
        }
        /* eslint-enable no-param-reassign */
        return state.q;
    },
    /**
     * Greater than, less than, equal to, in various combinations in order across inputs
     * @param {Array} inputs array of inputs
     * @param {object} state unused
     * @param {object} param optional parameters
     * @returns {boolean} comparison value
     */
    compare: (inputs, state, param) => {
        const op = param && param.operation ? param.operation : 'eq';
        const inputsRef = inputs;
        if (param && param.hasOwnProperty('reference') && param.reference !== undefined && param.reference !== null) {
            inputsRef.push(param.reference);
        }
        if (inputsRef.length < 2) return false;
        return inputsRef.every((a, i, arr) => {
            if (i < inputsRef.length - 1) {
                switch (op) {
                    case 'gt':
                        return a > arr[i + 1];
                    case 'gte':
                        return a >= arr[i + 1];
                    case 'lt':
                        return a < arr[i + 1];
                    case 'lte':
                        return a <= arr[i + 1];
                    case 'eq':
                        return a === arr[i + 1];
                    case 'ne':
                        return a !== arr[i + 1];
                    default:
                        return false;
                }
            } else {
                return true;
            }
        });
    },
    /**
     * Compare each input to the reference, true if all are true
     * @param {Array} inputs array of inputs
     * @param {object} state unused
     * @param {object} param optional parameters
     * @returns {boolean} comparison value
     */
    compareand: (inputs, state, param) => {
        if (!param) {
            throw new Error(`param not defined`)
        }
        if (!param.hasOwnProperty('reference') || param.reference === undefined || param.reference === null) {
            throw new Error(`compareand missing reference parameter`);
        }
        const op = param.operation ? param.operation : 'eq';
        return inputs.every((a) => {
            switch (op) {
                case 'gt':
                    return a > param.reference;
                case 'gte':
                    return a >= param.reference;
                case 'lt':
                    return a < param.reference;
                case 'lte':
                    return a <= param.reference;
                case 'eq':
                    return a === param.reference;
                case 'ne':
                    return a !== param.reference;
                default:
                    return false;
            }
        });
    },
    /**
     * Compare each input to the reference, true if any are true
     * @param {Array} inputs array of inputs
     * @param {object} state unused
     * @param {object} param optional parameters
     * @returns {boolean} comparison value
     */
    compareor: (inputs, state, param) => {
        if (!param) {
            throw new Error(`param not defined`)
        }
        if (!param.hasOwnProperty('reference') || param.reference === undefined || param.reference === null) {
            throw new Error(`compareor missing reference parameter`);
        }
        const op = param && param.operation ? param.operation : 'eq';
        return inputs.some((a) => {
            switch (op) {
                case 'gt':
                    return a > param.reference;
                case 'gte':
                    return a >= param.reference;
                case 'lt':
                    return a < param.reference;
                case 'lte':
                    return a <= param.reference;
                case 'eq':
                    return a === param.reference;
                case 'ne':
                    return a !== param.reference;
                default:
                    return false;
            }
        });
    },
    /**
     * Takes at least a single input, and using Javascript's truthiness rules,
     * determines whether to return the *true* case or the *false* case. The
     * true case and false case can either be static by using the param field,
     * or they can be dynamic using the inputs field. The param field takes
     * precedent if it exists.
     * @param {Array} inputs array of inputs
     * @param {object} state unused
     * @param {object} param optional parameters
     * @returns {boolean} comparison value
     */
    select: (inputs, state, param) => {
        // eslint-disable-next-line prefer-const
        let [selector, trueCase, falseCase] = inputs;
        /* eslint-disable no-prototype-builtins */
        if (param && param.hasOwnProperty('trueCase')) {
            if (param.hasOwnProperty('falseCase')) {
                // on this path, both trueCase and falseCase are static
                falseCase = param.falseCase;
            } else {
                // on this path, falseCase falls back to input[1]
                falseCase = trueCase;
            }
            trueCase = param.trueCase;
        } else if (param && param.hasOwnProperty('falseCase')) {
            /* eslint-enable no-prototype-builtins */
            // on this path, trueCase is provided by input[1], falseCase is static
            falseCase = param.falseCase;
        }
        if (typeof trueCase === 'undefined' || trueCase === null
            || typeof falseCase === 'undefined' || falseCase === null) {
            throw new Error('Select: true/falseCase param missing or null, or inputs not present.');
        }
        if (selector) return trueCase;
        return falseCase;
    },
    // eslint-disable-next-line no-unused-vars
    /**
     * Compare each input to the reference, true if any are true
     * @param {Array} inputs array of inputs
     * @param {object} state unused
     * @param {object} param unused
     * @returns {boolean} comparison value
     */
    length: (inputs, state, param) => inputs.reduce((prev, cur) => prev + (cur.length || 0), 0),
    /* eslint-disable consistent-return, no-unused-vars */
    /**
     * A number 0-3.999 that represents quadrant integer + cos(phi) of power factor within that quadrant
     * @param {Array} inputs array of inputs
     * @param {object} state unused
     * @param {object} param unused
     * @returns {boolean} signed power factor
     */
    quadtosigned: (inputs, state, param) => {
        // Take a 0-3.999 range, which represents a unit circle cos()
        // plus an integer to represent the quadrant.
        // This is a common way of representing power factor and
        // encoding the polarity of P vs Q
        const pf = inputs[0];
        if (pf < 0 || pf > 3.999) return 0;
        if (pf < 1) return pf;
        if (pf < 2) return -(pf - 1); // quadrant 4 1.99 becomes -0.99
        if (pf < 3) return pf - 2; // quadrant 3 2.99 becomes 0.99
        if (pf < 4) return -(pf - 3); // quadrant 2 3.99 becomes -0.99
    },
    /**
     * A number, signed power factor and returns 0-1.999. Incomplete inversion of `quadtosigned`
     * @param {Array} inputs array of inputs
     * @param {object} state unused
     * @param {object} param unused
     * @returns {boolean} quadrant integer + cos(phi) of power factor
     */
    signedtoquad: (inputs, state, param) => {
        // Inverse of quadtosigned, but since a single signed pf
        // input doesn't encode all of the P and Q sign data, it only
        // represents positive (quadrants 1 and 3) or
        // negative (quadrants 2 and 4)
        const pf = inputs[0];
        if (pf <= -1) return -1;
        if (pf < 0) return 1 - pf; // -0.99 becomes 1.99
        if (pf < 1) return pf;
        if (pf >= 1) return 1;
    },
    /**
     * Triggers pulse event based on inputs
     * @param {Array} inputs array of inputs
     * @param {object} state holds values to pulse
     * @param {object} param parameters
     * @returns {boolean} output based on if pulse active or not
     */
    pulse: (inputs, state, param) => {
        // pulse operation that will monitor input trigger. after trigger=true, output goes
        // high for time x then low permanently. can repeat anytime trigger set to true.
        // param includes inverse field that can flip behavior: on trigger=true, output will
        // go false for time x then true permanently.
        // second input acts as a reset. in normal operation, reset=true forces output to
        // high permanently. if inverted, then output goes to low permanently.
        // inputs: trigger, reset
        // params: time, invert
        // state: value, triggerEvent, timeout
        const [trigger, reset] = inputs;

        if (!state.triggerEvent) {
            // if we are NOT already in a trigger event...
            if (reset) {
                // if reset and trigger are both set at the same time, reset wins
                /* eslint-disable no-param-reassign */
                state.value = true; // on reset, set output to high
            } else if (trigger) {
                // ...check for trigger
                state.triggerEvent = true; // record that we are now in a trigger event
                state.timeout = Date.now() + param.time; // trigger event will end after
                // configured time period
                state.value = true; // output will be high during the trigger event
            }
        } else if (reset) {
            // if we are already in a trigger event...
            state.triggerEvent = false;
            state.value = true;
        } else if (Date.now() >= state.timeout) {
            // ...check if the trigger event is over
            state.triggerEvent = false; // mark the trigger event as over
            state.value = false; // output goes low after a trigger event
        }

        return param.invert ? !state.value : state.value; // invert simply flips the output
    },
    /* eslint-enable consistent-return, no-unused-vars, no-param-reassign */
    /**
     * Selects from array of inputs
     * @param {Array} inputs array of inputs
     * @returns {boolean} selected index value
     */
    selectn: (inputs) => {
        // first entry of inputs should be the enumerated index.
        // NOTE to users of this function: Please give us an integer for the index. We will
        // round any floats received for the index.
        let ind = Math.round(inputs[0]);
        if (Number.isNaN(ind)) throw new Error('selectn: unable to parse index. It should be a number');
        const maxInd = inputs.length - 1; // -1 to account for the index at the start of inputs
        ind = (ind > maxInd || ind < 1) ? maxInd : ind;
        // if ind is more than the number of uris or ind < 1 then we default to the last uri
        // a little background:  if ind === 0 then that indicates that none of selectorn's
        // inputs were truthy (or someone simply gave us a zero!). if ind < 0 then that
        // indicates that someone gave us a negative index
        return inputs[ind];
    },
    /**
     * Not implemented
     */
    selectorn: (inputs) => inputs.findIndex((input) => input) + 1,
    // returns enumerator based on inputs

    // The following functions relate to sending heartbeats and comparing returned
    // heartbeat values to the current time.
    // For pinging DNP3 devices, we use the milliseconds value of heartbeatMsg (a
    // simple integer, e.g., 1605197080690) returned from a DNP3 device to check
    // connected status by comparing the value returned from the DNP3 device and
    // the current time. If that difference is [param.operation] (usually "greater
    // than") [param.reference] (time in milliseconds) then we mark that DNP3
    // device as not connected.
    /**
     * Returns current time in milliseconds
     * @returns {number} current time in milliseconds
     */
    currentTimeMilliseconds: () => Date.now() || 0,
    /**
     * Splits time in milliseconds to two 32 bit integers
     * @param {Array} inputs array of inputs
     * @param {object} state unused
     * @param {object} param determines if return should be high or low
     * @returns {number} high or low 32 bit integer
     */
    splitMillisecondsTo32BitInts: (inputs, state, param) => {
        // time in milliseconds is a 41 bit integer. The maximum this code can
        // handle is a 53 bit number before JavaScript starts to round the
        // number. For an unsigned integer, the limit is 0 to 9007199254740991
        // the maximum value of the highInt is 2097151, and the maximum value
        // of the lowInt is 4294967295
        const milliseconds = inputs[0];
        const twoTo32nd = 4294967296; // (2 ** 32) = 4294967296
        if (param && param.highOrLowInt === 'high') {
            const theHighInt = Math.floor(milliseconds / twoTo32nd);
            if (theHighInt < 2097152 && theHighInt > -1) return theHighInt;
            return 0;
            // we return zero in cases where theHighInt is invalid - maybe above
            // the 53 bit range
        }
        return Math.floor(milliseconds - (Math.floor(milliseconds / twoTo32nd)) * twoTo32nd);
        // this line will return zero if the milliseconds input is out of range
    },
    /**
     * Converts two 32 bit integers back to time in milliseconds
     * @param {Array} inputs array of inputs
     * @param {object} state unused
     * @param {object} param unused
     * @returns {number} time in milliseconds
     */
    reassembleMillisecondsFrom32BitInts: (inputs) => {
        // (2 ** 32) = 4294967296
        const highInt = inputs[0];
        const lowInt = inputs[1];
        return lowInt + highInt * 4294967296;
    },
    /**
     * Compares input milliseconds to current time based on parameters
     * @param {Array} inputs array of inputs
     * @param {object} state unused
     * @param {object} param determines type of comparison
     * @returns {boolean} output based on comparison
     */
    compareMillisecondsToCurrentTime: (milliseconds, state, param) => {
        // in comparing an input in milliseconds to the current time in milliseconds,
        // a positive number means that the input is older than the current time.
        // This will usually be the case.
        const difference = Date.now() - milliseconds;
        if (!param) {
            return difference;
        }
        const { reference } = param;
        const operation = param.operation ? param.operation : 'lt';
        switch (operation) {
            case 'gt':
                return difference > reference;
            case 'gte':
                return difference >= reference;
            case 'lt':
                return difference < reference;
            case 'lte':
                return difference <= reference;
            case 'eq':
                return difference === reference;
            case 'ne':
                return difference !== reference;
            default:
                return false;
        }
    },
    /**
     * Converts time in milliseconds to human-readable RFC3339 format
     * @param {Array} inputs array of inputs
     * @param {object} state unused
     * @param {object} param parameters
     * @returns {string} human-readable RFC3339 format string
     */
    millisecondsToRFC3339: (inputs, state, param) => {
        // converts milliseconds to human-readable RFC3339 format
        const milliseconds = inputs[0];
        if (milliseconds > -1 && milliseconds < 9999999999999) {
            if (!param || param.operation === 'zulu') {
                return new Date(milliseconds).toISOString();
            }
            if (param.operation === 'localTime' || param.operation === 'timezone') {
                const date = new Date(milliseconds);
                let suppliedOffset;
                if (param.operation === 'timezone') suppliedOffset = param.reference || 0;
                // suppliedOffset is a number of hours before Zulu time (negative numbers) or
                // after Zulu time (positive numbers). For reference, during Standard Time, the
                // east coast of the US is -5 hours from Zulu time the "* -1" bits you see in
                // this code reverse the sign so that any suppliedOffset matches the format of
                // the output. Specifically, a "-4" offset will result in a time like
                // "2020-11-21T23:36:46.727-04:00" matching the minus signs so it makes the
                // most sense.
                const timeZoneOffset = suppliedOffset !== undefined ? (suppliedOffset * 60) * -1
                    : date.getTimezoneOffset();
                const dateWithTimezoneOffset = new Date(date.getTime() + timeZoneOffset * 60
                    * 1000);
                const offsetInHours = timeZoneOffset / 60;
                const dateGetHours = date.getHours();
                dateWithTimezoneOffset.setHours(dateGetHours - offsetInHours);
                const theSign = (Math.sign((suppliedOffset * -1) || offsetInHours) === 1) ? '-' : '+';
                const theOffset = String(Math.floor(Math.abs(offsetInHours))).padStart(2, '0');
                const theMinutes = String((offsetInHours % 1).toFixed(2) * 60).padStart(2, '0');
                // timezones generally are separated by full hours, but it is possible for a
                // timezone to be hours and minutes different from UTC. With this in mind, we
                // take any decimal part of the timeZoneOffset (like ".5" from a timeZoneOffset
                // of "4.5") and convert it to minutes
                const convertedDate = dateWithTimezoneOffset.toISOString().replace('Z', `${theSign}${theOffset}:${theMinutes}`);
                // the "Z" in an ISO date string indicates Zulu time, no timezone offset. When a
                // timezone offset is shown (which is what this function is all about), we strip
                // off the Z and show the offset in hours and minutes
                return convertedDate;
            }
            return null;
        }
        return '1970-01-01T00:00:00.000Z'; // which is what gets returned if the milliseconds is zero.
    },
    /**
     * Returns inputs, in order, in an array. Usually used for controlling enabled status in
     * the UI by sending the control object's value and enabled status in an array.
     * @param {Array} inputs inputs
     * @returns {string} output array string
     */
    combineInputsToArray: (inputs) => {
        // Returns inputs, in order, in an array. Usually used for controlling enabled status in
        // the UI by sending the control object's value and enabled status in an array.
        const outputArray = [];
        inputs.forEach((input) => {
            outputArray.push(input);
        });
        return `[${outputArray}]`;
    },
};
