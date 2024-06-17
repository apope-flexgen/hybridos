from behave import given, when, then
import subprocess
import json
import re
import os
from threading import Thread
import time
from process_fims_messages import process_messages

# TODO: Clean up message processing


def compare_json(actual, expected):
    """
    Recursively compare JSON elements.

    :param actual: The actual JSON element.
    :param expected: The expected JSON element.
    :return: True if actual matches expected, otherwise False.
    """
    if isinstance(expected, dict):
        if not isinstance(actual, dict):
            return False, f"\nExpected {expected}\nGot: {actual}\n"
        for key, value in expected.items():
            if key not in actual:
                return False, f"\nMissing key [{key}]"
            ok, message = compare_json(actual[key], value)
            if not ok:
                return False, f'\nExpected: "{key}": {value}\nGot: "{key}": {actual[key]}'
        for key in actual:
            if key not in expected:
                return False, f"\nExtra key [{key}] found"
        return True, ""

    elif isinstance(expected, list):
        if not isinstance(actual, list):
            return False, f"\nExpected {expected}\nGot: {actual}\n"
        if len(actual) != len(expected) and "any" not in expected:
            return False, f"\nExpected list length {len(expected)}, got {len(actual)}"
        for (i, item_expected) in enumerate(expected):
            found_match = False
            for item_actual in actual:  # allows items to be in a different order (needed for alerting)
                ok, message = compare_json(item_actual, item_expected)
                if ok:
                    found_match = True
                    break
            if not found_match:
                return False, f'\nExpected: {item_expected}\nGot: {actual[i]}'
        return True, ""

    else:
        return actual == expected or expected == "any", ""


def fims_listen_thread(method, uri, return_value_array):
    # Listen and use stdbuf -o0 to immediately flush the result rather than buffering it
    result = subprocess.Popen(["stdbuf", "-o0", "fims_listen", "-m", method, "-u", uri],
                              stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    for line in iter(result.stdout.readline, ""):
        if not line:
            break
        return_value_array.append(line)


# Process and store the results of fims_listen in the context
def load_fims_results(context):
    try:
        context.expected_message = json.loads(context.text)
    except json.JSONDecodeError:
        context.expected_message = context.text

    resultList = []
    try:
        results = context.fims_listen_result_array
        if isinstance(results, list) and len(results) > 0:
            for result in results:
                for line in result:
                    stdout = line.decode('utf-8')
                    resultList.extend(stdout)
    except subprocess.TimeoutExpired:
        assert False, "Did not receive message while running fims_listen."

    # TODO: make this less redundant. This reads all the lines, concatenates them,
    # then process_messages splits them into separate lines again for parsing
    concatenatedResults = ""
    for line in resultList:
        concatenatedResults += line
    context.matches = process_messages(concatenatedResults)


@given(u'I am listening for a fims {method} on {uri}')
def step_impl(context, method, uri):
    if not hasattr(context, "fims_listen_result_array"):
        context.fims_listen_result_array = []
        context.fims_listen_threads = []
    context.fims_listen_result_array.append([])
    context.fims_listen_threads.append(Thread(target=fims_listen_thread, args=[
                                       method, uri, context.fims_listen_result_array[-1]]))
    context.fims_listen_threads[-1].start()
    context.start_time = time.time()


@when(u'I send a fims {method} with replyto {replyto} to {uri} containing')
def step_impl(context, method, replyto, uri):
    context.start_time = time.time()
    subprocess.run(["fims_send", "-m", method, "-r", replyto, "-u", uri, context.text.strip()],
                   stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False)


@when(u'I send a fims {method} to {uri} containing')
def step_impl(context, method, uri):
    context.start_time = time.time()
    subprocess.run(["fims_send", "-m", method, "-u", uri, context.text.strip()],
                   stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False)


@when(u'I clear the fims message list')
def step_impl(context):
    if hasattr(context, "fims_listen_result_array"):
        num_results = len(context.fims_listen_result_array)
        for i in range(0, num_results):
            context.fims_listen_result_array[i].clear()


@then(u'I expect a fims {method} to {uri} within {delay} seconds containing')
def step_impl(context, method, uri, delay):
    if not hasattr(context, "fims_listen_result_array"):
        assert False, "Must include step containing 'Given I am listening for a fims {method} on {uri}'"

    # Wait for fims_listen results before proceeding
    while (time.time() - context.start_time) < float(delay):
        time.sleep(float(delay) - (time.time() - context.start_time))

    # Process and store the results received from fims_listen
    load_fims_results(context)

    if len(context.matches) > 0:
        found_match = False
        assertion_text = ""
        for match in reversed(context.matches):
            if "uri" in match and match['uri'] == uri:
                if "timestamp" in match and match['timestamp'] <= context.start_time + float(delay):
                    found_match = True
                    body = match['body']

                    if isinstance(body, dict) and isinstance(context.expected_message, dict):
                        ok, assertion_text = compare_json(body, context.expected_message)
                    else:
                        message_type = type(body)
                        try:
                            expected_val = message_type(context.text.strip())
                            if expected_val == body or expected_val == "any":
                                assert True
                                return
                            else:
                                assertion_text = f'\nExpected: "{uri}": {context.text}\nGot:"{uri}": {body}'
                        except (TypeError, ValueError):
                            assertion_text = f"Unexpected type {message_type} received from go_metrics message."
                    # Return early if a match was found
                    if found_match and assertion_text == "":
                        return
        if assertion_text != "":
            assert False, assertion_text
        if not found_match:
            assert False, f"Did not receive fims message to {uri}"
    else:
        assert False, f"Did not receive complete fims message. Message list is: {context.matches}"


@then(u'I expect a fims {method} to {uri} after {delay} seconds containing')
def step_impl(context, method, uri, delay):
    if not hasattr(context, "fims_listen_result_array"):
        assert False, "Must include step containing 'Given I am listening for a fims {method} on {uri}'"

    # Wait for fims_listen results before proceeding
    while (time.time() - context.start_time) < float(delay) + 1:
        time.sleep(float(delay) + 1 - (time.time() - context.start_time))

    # Process and store the results received from fims_listen
    load_fims_results(context)

    if len(context.matches) > 0:
        found_match = False
        for match in reversed(context.matches):
            if "uri" in match and match['uri'] == uri:
                if "timestamp" in match and match['timestamp'] >= (float(delay) + context.start_time):
                    found_match = True
                    body = match['body']

                    if isinstance(body, dict) and isinstance(context.expected_message, dict):
                        ok, assertion_text = compare_json(body, context.expected_message)
                        assert ok, assertion_text
                    else:
                        message_type = type(body)
                        try:
                            expected_val = message_type(context.text.strip())
                            if expected_val == body:
                                assert True
                                return
                            else:
                                assert False, f'\nExpected: "{uri}": {context.text}\nGot:"{uri}": {body}'
                        except (TypeError, ValueError):
                            assert False, f"Unexpected type {message_type} received from go_metrics message."
                    # Return early if a match was found
                    if found_match:
                        return
        if not found_match:
            assert False, f"Did not receive fims message to {uri}"
    else:
        assert False, f"Did not receive complete fims message. Message list is: {context.matches}"


@then(u'I expect a fims {method} to {uri} containing')
def step_impl(context, method, uri):
    match = ""
    expected_message = {}
    result = ""
    found_match = False
    n = 1
    try:
        expected_message = json.loads(context.text)
    except json.JSONDecodeError:
        expected_message = context.text
    while not found_match and n < 5:
        try:
            result = subprocess.run(["fims_listen", "-n", f"{n}", "-m", method, "-u", uri],
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False, timeout=5)
            if result and result.stdout:
                stdout = result.stdout.decode('utf-8')
                matches = process_messages(stdout)
                if len(matches) > 0:
                    for item in matches:
                        if 'uri' in item and item['uri'] == uri and 'body' in item:
                            found_match = True
                            body = item['body']
                            break
                    n += 1
                else:
                    assert False, "Something went terribly, terribly wrong"
        except subprocess.TimeoutExpired:
            assert False, "Did not receive message after 5 seconds."

    if isinstance(body, dict) and isinstance(expected_message, dict):
        for key, value in body.items():
            if key in expected_message:
                assert value == expected_message[
                    key], f'\nExpected: "{key}": {expected_message[key]}\nGot:"{key}": {value}'
            else:
                assert False, f"Extra key [{key}] found"
        for key, value in expected_message.items():
            if key not in body:
                assert False, f"Missing key [{key}]"
    elif isinstance(body, type(expected_message)):
        if expected_message == body:
            assert True
            return
        else:
            assert False, f'\nExpected: "{uri}": {context.text}\nGot:"{uri}": {body}'
    else:
        assert False, f"Did not receive complete fims message {match}"


@then(u'I expect no fims {method}s will be sent to {uri}')
def step_impl(context, method, uri):
    # TODO: warn the user if the given statement is not present without checking that the fims_listen_result_array exists
    try:
        result = subprocess.run(["fims_listen", "-n", "1", "-m", method, "-u", uri],
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False, timeout=5)
        if result and result.stdout:
            assert False, f"Expected no messages but received fims {method}."
    except subprocess.TimeoutExpired:
        assert True


@then(u'a fims get to {uri} should yield')
def step_impl(context, uri):
    match = ""
    expected_message = {}
    result = ""
    try:
        expected_message = json.loads(context.text)
    except json.JSONDecodeError:
        assert False, "Invalid expected json message"
    try:
        current_pid = str(os.getpid())
        result = subprocess.run(["fims_send", "-m", "get", "-u", uri, "-r",
                                f"/{current_pid}"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False, timeout=5)
    except subprocess.TimeoutExpired:
        assert False, "Did not receive message after 5 seconds."

    if result and result.stdout:
        try:
            output = result.stdout.decode('utf-8')
            body = json.loads(output)
        except json.JSONDecodeError:
            assert False, f"Invalid json message sent by go_metrics {output}"
        except UnicodeDecodeError:
            assert False, f"Failed to properly decode message {result.stdout}"

        if isinstance(body, dict):
            for key, value in body.items():
                if key in expected_message:
                    assert value == expected_message[
                        key], f'\nExpected: "{key}": {expected_message[key]}\nGot:"{key}": {value}'
                else:
                    assert False, f"Extra key [{key}] found"
        else:
            if body == expected_message:
                return
            assert False, f"Error converting json message to python dictionary"

        for key, value in expected_message.items():
            if key not in body:
                assert False, f"Missing key [{key}]"
    else:
        print(result.stderr)
        assert False, "Received Timeout"


@then(u'a fims get to {uri} should timeout')
def step_impl(context, uri):
    try:
        current_pid = str(os.getpid())
        result = subprocess.run(["fims_send", "-m", "get", "-u", uri, "-r",
                                f"/{current_pid}"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False, timeout=5)
        if result and result.stdout:
            output = result.stdout.decode('utf-8')
            if "Receive Timeout." in output:
                assert True
            else:
                assert False, "Received a message when we should have!"
    except subprocess.TimeoutExpired:
        assert True
