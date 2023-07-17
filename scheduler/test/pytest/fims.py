import json
import os
import subprocess
import sys

pid = os.getpid()


def run_and_read(cmd):
    result = subprocess.run(cmd, universal_newlines=True, stdout=subprocess.PIPE, stderr=sys.stderr, shell=True)
    if result.returncode != 0:
        return f'Command failed with return code {result.returncode}.'
    return json.loads(result.stdout.strip())

# Negative numbers require a `--` before them in order to be passed as arguments to the fims_send program.
# Positive numbers are unaffected by the `--`.
# This function serializes the given body and prepends it with a `--` only if the body is a numeric type.


def serialize(body):
    if type(body) == int or type(body) == float:
        return f'-- {json.dumps(body)}'
    return f"'{json.dumps(body)}'"


def send_set(uri, body=''):
    return run_and_read(f'fims_send -m set -u {uri} -r /{pid} {serialize(body)}')


def send_post(uri, body):
    return run_and_read(f'fims_send -m post -u {uri} -r /{pid} {serialize(body)}')


def send_del(uri, body=''):
    return run_and_read(f'fims_send -m del -u {uri} -r /{pid} {serialize(body)}')


def send_get(uri):
    return run_and_read(f'fims_send -m get -u {uri} -r /{pid}')
