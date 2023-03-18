/* eslint-disable */
function pulse(inputs, state, param) {
    // pulse operation that will monitor input trigger. after trigger=true, output goes high for time x then low permanently. can repeat anytime trigger set to true
    // param includes inverse field that can flip behavior: on trigger set to true, output will go false for time x then true permanently.
    // second input acts as a reset. in normal operation, true set to reset forces output to high permanently. if inverted, then output goes to low permanently.
    // inputs: trigger, reset
    // params: time, invert
    // state: value, triggerEvent, timeout
    // console.log("IN THE PULSE FUNCTION!!!!---------------------------------");
    const [trigger, reset] = inputs;

    if (!state.triggerEvent) // if we are NOT already in a trigger event...
    {
        if (reset) // if reset and trigger are both set at the same time, reset wins
        {
            state.value = true; // on reset, set output to high
        }
        else if (trigger) // ...check for trigger
        {
            // console.log("PULSE TURNED ON");
            state.triggerEvent = true; // record that we are now in a trigger event
            state.timeout = Date.now() + param.time; // trigger event will end after configured time period
            state.value = true; // output will be high during the trigger event
        }
    }
    else // if we are already in a trigger event...
    {
        if (reset) {
            state.triggerEvent = false;
            state.value = true;
        }
        else if (Date.now() >= state.timeout) // ...check if the trigger event is over
        {
            // console.log("PULSE TURNED OFF");
            state.triggerEvent = false; // mark the trigger event as over
            state.value = false;        // output goes low after a trigger event
        }
    }

    return param.invert ? !state.value : state.value; // invert simply flips the output
}

const tests = [
    {
        inputs: [false, false],
        state: { value: false, triggerEvent: false },
        param: { time: 1000, invert: false },
        result: false
    },
    {
        inputs: [false, false],
        state: { value: false, triggerEvent: false },
        param: { time: 1000, invert: true },
        result: true
    },
    {
        inputs: [false, false],
        state: { value: false, triggerEvent: true },
        param: { time: 1000, invert: false },
        result: false
    },
    {
        inputs: [false, false],
        state: { value: true, triggerEvent: false },
        param: { time: 1000, invert: false },
        result: true
    },
    {
        inputs: [false, false],
        state: { value: true, triggerEvent: false },
        param: { time: 1000, invert: true },
        result: false
    },
    {
        inputs: [false, false],
        state: { value: true, triggerEvent: true },
        param: { time: 1000, invert: false },
        result: true
    },
    {
        inputs: [false, false],
        state: { value: true, triggerEvent: true },
        param: { time: 1000, invert: true },
        result: false
    },
    {
        inputs: [false, true],
        state: { value: false, triggerEvent: false },
        param: { time: 1000, invert: false },
        result: true
    },
    {
        inputs: [false, true],
        state: { value: false, triggerEvent: false },
        param: { time: 1000, invert: true },
        result: false
    },
    {
        inputs: [false, true],
        state: { value: true, triggerEvent: false },
        param: { time: 1000, invert: false },
        result: true
    },
    {
        inputs: [false, true],
        state: { value: true, triggerEvent: false },
        param: { time: 1000, invert: true },
        result: false
    },
    {
        inputs: [false, true],
        state: { value: true, triggerEvent: true },
        param: { time: 1000, invert: false },
        result: true
    },
    {
        inputs: [false, true],
        state: { value: true, triggerEvent: true },
        param: { time: 1000, invert: true },
        result: false
    },
    {
        inputs: [true, false],
        state: { value: false, triggerEvent: false },
        param: { time: 1000, invert: false },
        result: true
    },
    {
        inputs: [true, false],
        state: { value: false, triggerEvent: false },
        param: { time: 1000, invert: true },
        result: false
    },
    {
        inputs: [true, false],
        state: { value: true, triggerEvent: false },
        param: { time: 1000, invert: false },
        result: true
    },
    {
        inputs: [true, false],
        state: { value: true, triggerEvent: false },
        param: { time: 1000, invert: true },
        result: false
    },
    {
        inputs: [true, false],
        state: { value: true, triggerEvent: true },
        param: { time: 1000, invert: false },
        result: true
    },
    {
        inputs: [true, false],
        state: { value: true, triggerEvent: true },
        param: { time: 1000, invert: true },
        result: false
    },
    {
        inputs: [true, true],
        state: { value: false, triggerEvent: false },
        param: { time: 1000, invert: false },
        result: true
    },
    {
        inputs: [true, true],
        state: { value: false, triggerEvent: false },
        param: { time: 1000, invert: true },
        result: false
    },
    {
        inputs: [true, true],
        state: { value: true, triggerEvent: false },
        param: { time: 1000, invert: false },
        result: true
    },
    {
        inputs: [true, true],
        state: { value: true, triggerEvent: false },
        param: { time: 1000, invert: true },
        result: false
    },
    {
        inputs: [true, true],
        state: { value: true, triggerEvent: true },
        param: { time: 1000, invert: false },
        result: true
    },
    {
        inputs: [true, true],
        state: { value: true, triggerEvent: true },
        param: { time: 1000, invert: true },
        result: false
    },
    {
        inputs: [true, false],
        state: { value: true, triggerEvent: false },
        param: { time: 1000, invert: false },
        waitTime: 1000,
        result: false
    },
    {
        inputs: [true, false],
        state: { value: false, triggerEvent: false },
        param: { time: 1000, invert: true },
        waitTime: 1000,
        result: true
    }];

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

function analyzeResult(testCase, index, result) {
    console.log(`New state: value=${testCase.state.value}, triggerEvent=${testCase.state.triggerEvent}`);

    const passed = (result === testCase.result);
    allTestsPassed = allTestsPassed && passed;

    console.log(`Expected result: ${testCase.result}. Actual result: ${result}.`)
    console.log(`Test #${index + 1} ${result == testCase.result ? 'Passed' : 'FAILED!'}`);
    console.log("--------------------------------------------------\n");

    if (tests[index + 1])          // if there are more test cases...
    {
        test(tests, index + 1);   // ...test the next test case
    }
    else    // if there are no more test cases
    {       // print the results
        if (allTestsPassed) {
            console.log("Success! All test cases passed.\n");
        }
        else {
            console.log("Failure. One or more test cases did not pass.\n");
        }
    }
}

function test(tests, index) {
    const testCase = tests[index]; // get individual test values

    // Provide initial information to test engineer
    console.log(`Starting pulse test #${index + 1}`);
    console.log(`Inputs: trigger=${testCase.inputs[0]}, reset=${testCase.inputs[1]}`);
    console.log(`State: value=${testCase.state.value}, triggerEvent=${testCase.state.triggerEvent}`);
    console.log(`Params: time=${testCase.param.time}, invert=${testCase.param.invert}`);
    console.log(`:::::${Date.now()} is beginning time`)

    let result = pulse(testCase.inputs, testCase.state, testCase.param);

    if (testCase.waitTime) // if the test case has a timing component to it, wait then call pulse again before analyzing result
    {
        console.log(`hibernating for ${testCase.waitTime} milliseconds`)
        sleep(testCase.waitTime)
            .then(() => {
                testCase.inputs[0] = false;
                console.log(`:::::${Date.now()} calling pulse computation again`)
                result = pulse(testCase.inputs, testCase.state, testCase.param);
                analyzeResult(testCase, index, result)
            });
    }
    else // if test case only calls pulse once so there is no timing component, skip to analyzing the result
    {
        analyzeResult(testCase, index, result);
    }
}

let allTestsPassed = true;

// Call the test function on the first test case, which will recursively call the test function for the rest of the test cases
test(tests, 0);
