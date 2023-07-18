import json
import subprocess
import sys
import time
from .fims import fims_set, fims_get

def start_site() -> None:
    """
        Checks if site is already running and returns early if so.
        Otherwise, sends command to start site then sleeps for 10 seconds to give site time to start.
    """
    running_status_flag = fims_get('/site/operation/running_status_flag')['value']
    if running_status_flag:
        return
    fims_set('/site/operation/enable_flag', True)
    # TODO: instead of sleeping then checking, use Flex_Assertion to listen for running_status_flag to switch to true with a timeout.
    # would have to happen after https://flexgen.atlassian.net/browse/DC-43?atlOrigin=eyJpIjoiNzAxYjcxMTg5MWY4NDc0MGI4NjAwOTJhMTIyZjU4OTAiLCJwIjoiaiJ9
    time.sleep(10)
    assert fims_get('/site/operation/running_status_flag')['value'] == True


def toggle_solar_and_gen_maintenance_mode(on_off: bool) -> None:
    fims_set('/assets/generators/gen_1/maint_mode', on_off)
    assert fims_get('/assets/generators/gen_1/maint_mode')['value'] == on_off
    fims_set('/assets/solar/solar_1/maint_mode', on_off)
    assert fims_get('/assets/solar/solar_1/maint_mode')['value'] == on_off
    fims_set('/assets/solar/solar_2/maint_mode', on_off)
    assert fims_get('/assets/solar/solar_2/maint_mode')['value'] == on_off

def run_twins_command(cmd: str):
    """
        Runs given command inside TWINS Docker container.
    """
    result = subprocess.run(f'docker exec twins {cmd}', universal_newlines=True, stdout=subprocess.PIPE, stderr=sys.stderr, shell=True)
    if result.returncode != 0:
        return f'Command failed with return code {result.returncode}.'
    return json.loads(result.stdout.strip())
