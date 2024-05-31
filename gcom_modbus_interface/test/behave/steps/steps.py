from behave import given, when, then
import subprocess
import json
import re
import os
from threading import Thread
import time
from process_fims_messages import process_messages

# TODO: Clean up message processing


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
        context.expected_message = context.text.strip()
    except:
        assert False, "Something went wrong when trying to decode the expected message"

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

@given(u'mongo and dbi are running')
def step_impl(context):
    subprocess.run(["pkill dbi"], shell=True)
    subprocess.run(["pkill mongo"], shell=True)
    time.sleep(0.5)
    subprocess.run(["mongod --config /etc/mongod.conf &"], shell=True)
    time.sleep(1)
    subprocess.run(["dbi &"], shell=True)
    time.sleep(1)
    context.start_time = time.time()

@given(u'I clear all data in dbi/{endpoint}')
def step_impl(context, endpoint):
    subprocess.run(["fims_send", "-m", "del", "-u", f"/dbi/{endpoint}"])
    context.start_time = time.time()

@given(u'modbus_{server_or_client} is running the config {config_file}')
def step_impl(context, server_or_client, config_file):
    subprocess.run([f"gcom_modbus_{server_or_client} /home/docker/hybridos/gcom_modbus_interface/test/behave/configs/{config_file} >/dev/null 2>&1 &"], shell=True)
    context.start_time = time.time()
    
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

@when(u'I start modbus_{server_or_client} with the config {config_file}')
def step_impl(context, server_or_client, config_file):
    subprocess.run([f"gcom_modbus_{server_or_client} /home/docker/hybridos/gcom_modbus_interface/test/behave/configs/{config_file} >/dev/null 2>&1 &"], shell=True)
    context.start_time = time.time()

@when(u'I kill modbus_{server_or_client}')
def step_impl(context, server_or_client):
    pkill_msg = "cli"
    if server_or_client == "server":
        pkill_msg = "ser"
    subprocess.run(["pkill gcom_modbus_{pkill_msg}"], shell=True)
    context.start_time = time.time()

@when(u'I wait {seconds} seconds')
def step_impl(context, seconds):
    time.sleep(float(seconds))
    context.start_time = time.time()


@when(u'I send a fims {method} to {uri} containing')
def step_impl(context, method, uri):
    context.start_time = time.time()
    subprocess.run(["fims_send", "-m", method, "-u", uri, context.text],
                   stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False)


@then(u'I expect a fims {method} to {uri} within {delay} seconds containing')
def step_impl(context, method, uri, delay):
    if not hasattr(context, "fims_listen_result_array"):
        assert False, f"Must include step containing 'Given I am listening for a fims {method} on {uri}'"

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
                    if 'body' in match:
                        body = match['body']
                    else:
                        body = None
                    # try:
                    #     body = json.loads(body_str)
                    # except json.JSONDecodeError:
                    #     assertion_text = f"Invalid json message sent by go_metrics "
                    # except (TypeError, ValueError):
                    #     message_type = type(body_str)
                    #     try:
                    #         expected_val = message_type(context.text)
                    #         if expected_val == body_str:
                    #             assert True
                    #             return
                    #         else:
                    #             assertion_text = f'\nExpected: "{uri}": {context.text}\nGot:"{uri}": {body_str}'
                    #     except (TypeError, ValueError):
                    #         assertion_text = f"Unexpected type {message_type} received from go_metrics message."
                    # except Exception as e:
                    #     print(e)
                    if isinstance(body, dict) and isinstance(context.expected_message, dict):
                        for key, value in body.items():
                            if key in context.expected_message:
                                # Reserve "any" as a accepting any value and just checking that the key is present
                                if value == context.expected_message[key] or context.expected_message[key] == "any":
                                    continue
                                else:
                                    assertion_text = f'\nExpected: "{key}": {context.expected_message[key]}\nGot:"{key}": {value}'
                                    break
                            else:
                                assertion_text = f"Extra key [{key}] found"
                                break
                        for key, value in context.expected_message.items():
                            if key not in body:
                                assertion_text = f"Missing key [{key}]"
                    else:
                        message_type = type(body)
                        try:
                            if message_type == bool:
                                if context.text.lower() != "true" or context.text.lower() != "false":
                                    assertion_text = f'\nExpected: "{uri}": {context.text}\nGot:"{uri}": {body}'
                            expected_val = message_type(context.text.strip())
                            if message_type == str:
                                body = body.strip()
                            if expected_val == body or expected_val == "any":
                                return
                            else:
                                assertion_text = f'\nExpected: "{uri}": {context.text}\nGot:"{uri}": {body}'
                        except (TypeError, ValueError):
                            assertion_text = f'Unexpected type {message_type} received from go_metrics message.\nGot:"{uri}": {body}'
                        except:
                            assertion_text = f'Something else went wrong...\nGot:"{uri}": {body}'
                    # Return early if a match was found
                    if found_match and assertion_text == "":
                        return
        if assertion_text != "":
            assert False, assertion_text
        if not found_match:
            assert False, f"Did not receive fims message to {uri}"
    else:
        assert False, f"Did not receive complete fims message {context.matches}"


@then(u'I expect a fims {method} to {uri} after {delay} seconds containing')
def step_impl(context, method, uri, delay):
    if not hasattr(context, "fims_listen_result_array"):
        assert False, f"Must include step containing 'Given I am listening for a fims {method} on {uri}'"

    # Wait for fims_listen results before proceeding
    while (time.time() - context.start_time) < float(delay)+1:
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
                    # try:
                    #     body = json.loads(body_str)
                    # except json.JSONDecodeError:
                    #     assert False, f"Invalid json message sent by go_metrics "
                    # except (TypeError, ValueError):
                    #     message_type = type(body_str)
                    #     try:
                    #         expected_val = message_type(context.text)
                    #         if expected_val == body_str:
                    #             assert True
                    #             return
                    #         else:
                    #             assert False, f'\nExpected: "{uri}": {context.text}\nGot:"{uri}": {body_str}'
                    #     except (TypeError, ValueError):
                    #         assert False, f"Unexpected type {message_type} received from go_metrics message."
                    # except Exception as e:
                    #     print(e)

                    if isinstance(body, dict) and isinstance(context.expected_message, dict):
                        for key, value in body.items():
                            if key in context.expected_message:
                                assertionResult = (
                                    # Reserve "any" as a accepting any value and just checking that the key is present
                                    value == context.expected_message[key] or context.expected_message[key] == "any"
                                )
                                assert assertionResult, f'\nExpected: "{key}": {context.expected_message[key]}\nGot:"{key}": {value}'
                            else:
                                assert False, f"Extra key [{key}] found"
                        for key, value in context.expected_message.items():
                            if key not in body:
                                assert False, f"Missing key [{key}]"
                    else:
                        message_type = type(body)
                        try:
                            expected_val = message_type(context.text)
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
        assert False, f"Did not receive complete fims message {context.matches}"


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
    except:
        assert False, "Something went wrong when trying to decode the expected message"
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
        expected_message = context.text
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
            output = result.stdout.decode('utf-8')
            if output == expected_message:
                return True
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
            for key, value in expected_message.items():
                if key not in body:
                    assert False, f"Missing key [{key}]"
        else:
            if body == expected_message:
                return
            assert False, f"Error converting json message to python dictionary"        
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
