import sys
import json
import time
from threading import Thread
from subprocess import PIPE, CalledProcessError, run
from typing import Any, Dict


def fims(params: str, container: str) -> Dict:
    fims_cmd = "fims_send " + params
    if container!=None:
        if fims_cmd.find("-m get ") != -1:
            fims_cmd = "docker exec -it " + container + " " + fims_cmd
        else:
            fims_cmd = "docker exec -d " + container + " " + fims_cmd
    response = {}
    response_dict = {}
    try:
        response = run(fims_cmd, universal_newlines=True, stdout=PIPE, stderr=sys.stderr, shell=True, check=True)
    except CalledProcessError:
        return None

    if len(response.stdout) > 0:
        response.stdout.strip().splitlines()
        try:
            response_dict = json.loads(response.stdout)
        except json.decoder.JSONDecodeError:
            return None
    return response_dict


def fims_get(uri: str, destination: str = None) -> Dict:
    params = f"-m get -r /pytest -u {uri}"
    response_dict = fims(params, destination)
    return response_dict


def fims_set(uri: str, value: Any, reply_to: str = None, destination: str = None) -> Dict:
    if isinstance(value, bool):
        my_value = str(value).lower()
    elif isinstance(value, dict) or isinstance(value, list):
        my_value = f"'{json.dumps(value)}'"
    else:
        my_value = value
    if reply_to == None:
        params = f"-m set -u {uri} -- {my_value}"
    else:
        params = f"-m set -u {uri} -r {reply_to} -- {my_value}"
    return fims(params, destination)


def fims_del(uri: str):
    params = f"-m del -r /pytest -u {uri}"
    return fims(params)


def fims_threaded_pub(uri: str, value: Any, publish_frequency=.2, duration=30):
    """
        Starts a pub to be fired until kill_command (which is returned by this function) is executed.
    """
    if isinstance(value, dict) or isinstance(value, list):
        value = f"'{json.dumps(value)}'"
    fims_cmd = f"fims_send -m pub -u {uri} -- {value}"

    class PubRoutine:
        def __init__(self):
            self.active = True

        def run(self):
            while (self.active):
                run(fims_cmd, stderr=sys.stderr, shell=True, check=True)
                time.sleep(publish_frequency)
    rt = PubRoutine()
    t = Thread(target=rt.run)
    t.start()
    num_pubs = duration/publish_frequency
    time.sleep(publish_frequency * num_pubs)  # pub for the entire duration

    def kill_command():
        rt.active = False
        t.join()
    return kill_command

# TODO: replace usage of this with Flex_Assertion. Need to add polling option to Flex_Assertion


def poll_until_uri_is_at_value(uri: str, expected: float, tolerance: float = 0, timeout_seconds: int = 10) -> bool:
    """
        Poll the uri periodically with fims_get until the timeout passes or the actual value is
        within tolerance of the expected value.
    """
    # poll once at beginning in case we're already at expected
    actual = fims_get(uri)
    if abs(expected - actual) <= tolerance:
        return True
    # poll periodically until expected is reached or we've hit the timeout, 
    # ensure we poll at least 4 times
    period = min(timeout_seconds / 4, 1)
    time_passed = 0
    while time_passed < timeout_seconds:
        time.sleep(period)
        time_passed += period
        actual = fims_get(uri)
        if abs(expected - actual) <= tolerance:
            return True
    return False


def parse_body_from_fims_listen_output(raw_output: str):
    """
        When given a single message formatted as fims_listen prints it, will parse/return the body as JSON.

    """
    lines = raw_output.splitlines()
    for line in lines:
        if line.startswith('Body:'):
            body = line.lstrip('Body:').strip()
            return json.loads(body)
    return ''


def parse_time_from_fims_trigger_output(raw_output: str) -> int:
    """
        When given the standard output of the fims_trigger command, will parse/return the elapsed time in milliseconds.
    """
    lines = raw_output.splitlines()
    for line in lines:
        if line.startswith('Elapsed Time:'):
            # trims all the text/whitespace surrounding the millisecond value and returns it as a parsed integer
            return int(line.lstrip('Elapsed Time:').strip().rstrip('ms').strip())
    return -1
