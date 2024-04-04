# Automated Action tests
from os import times
from pytest_cases import parametrize, fixture
from pytests.assertion_framework import Assertion_Type, Flex_Assertion
from pytests.pytest_steps import Setup, Steps, Teardown
from pytests.fims import fims_set, fims_get
from pytest_utils.fims_listen_parser import listen_reply, listen_reply_validator
from datetime import datetime, timedelta
from subprocess import Popen, PIPE
from typing import List, Union
from pytest_utils.sample_modes import Constants_Variables_Json, Mode, modes_cfg
from pytest_utils.sample_events import balance_event
from json import JSONDecodeError, JSONDecoder, loads
import threading
import time

stdout = []

def grok_reply() -> Union[listen_reply, None]:
    global stdout
    # parse the output into these
    method = ""
    uri = ""
    reply_to = ""
    process_name = ""
    username = ""
    body = {}
    timestamp = ""

    if stdout != None:
        for line in stdout:
            if line.find("Method:") != -1:
                method = line[line.find("Method:") + len("Method:"):].strip()
            if line.find("Uri:") != -1:
                uri = line[line.find("Uri:") + len("Uri:"):].strip()
            if line.find("ReplyTo:") != -1:
                reply_to = line[line.find("ReplyTo:") + len("ReplyTo:"):].strip()
            if line.find("Process Name:") != -1:
                process_name = line[line.find("Process Name:") + len("Process Name:"):].strip()
            if line.find("Username:") != -1:
                username = line[line.find("Username:") + len("Username:"):].strip()
            if line.find("Body:") != -1:
                body_str = line[line.find("Body:") + len("Body:"):].strip()
                try:
                    body = loads(body_str)
                except JSONDecodeError:
                    print("failed to parse body string")
                    print(body_str)
            if line.find("Timestamp:") != -1:
                timestamp = line[line.find("Timestamp:") + len("Timestamp:"):].strip()
                stdout = []
                return listen_reply(method=method, uri=uri, replyto=reply_to, process=process_name, 
                                     username=username, body=body, timestamp=timestamp)
        return None
    else:
        return None

def validate_pubs_silent(timeout: int, uri: Union[str, None] = None) -> Union[listen_reply, None]:
    """ Open a fims_listen on all endpoints and make sure that no one 
    is pubbing actions for some period of time. """
    def thread_timer(timeout: int):
        """ Spawn a thread that will be an indicator to kill the coming subprocess.
        Why a thread??? Answer: So that we can digest the pubs in real time. """
        time.sleep(timeout)

    global stdout
    stdout = []

    # run proc for timeout seconds then kill it and collect the output
    cmd = ["fims_listen"]
    if uri is not None:
        cmd.append("-u")
        cmd.append(uri)
    reply = None
    proc = Popen(cmd, stdout=PIPE, stderr=PIPE, universal_newlines=True)
    # launch the timeout thread
    thread = threading.Thread(target=thread_timer, args=[timeout])
    thread.start()

    # while the proc is alive
    while proc.poll() is None:
        if not thread.is_alive():
            proc.kill()
            break

        # read a single line if you can
        if proc.stdout is not None:
            stdout.append(proc.stdout.readline())
        reply = grok_reply()
        if reply is not None:
            stdout = [] # you have digested a full fims_listen discard.
            print(reply)
            if reply.contains_action_pub():
                assert False

def validate_action_pubs(uri: Union[str, None] = None, expected_info: Union[List[listen_reply_validator], None] = None, start: Union[str, None] = None, abort_on_step: Union[str, None] = None, set_SoC_on_step: Union[tuple, None] = None, ignore_step:Union[str, None] = None, action_name: Union[str, None] = None, exit_maint_mode_step: Union[str, None] = None) -> Union[listen_reply, None]:
    """ Listen to pubs and ensure they behave as expected. 
    Parameter definitions:
        uri: if provided listen to a specific uri
        expected_info: a list of tests (holds expected data)
        start: should this start the action. (provide uri to send true if so)
        abort_on_step: will issue a stop command
        set_SoC_on_step: will set ess_psm SoC on provided step
        ignore_step: will ignore this step if found (needed in case sequences executes multiple steps before a pub is issued)
        action_name: name of the action to inspect
    """
    global stdout
    stdout = []

    if start is not None:
        fims_set(start, True)
    if action_name is None:
        action_name = "Calibration1"

    cmd = ["fims_listen"]
    if uri is not None:
        cmd.append("-u")
        cmd.append(uri)
    reply = None
    proc = Popen(cmd, stdout=PIPE, stderr=PIPE, universal_newlines=True)

    info_index = 0

    while proc.poll() is None:
        if proc.stdout is not None:
            stdout.append(proc.stdout.readline())

        reply = grok_reply()
        if reply is not None:
            stdout = []
            if reply.contains_action_pub():
                if expected_info is not None and len(expected_info) > 0:
                    if info_index <= len(expected_info) -1:
                        if ignore_step is not None and ignore_step == reply.body['actions'][action_name]['step_name']:
                            continue # this step is hard to catch in pubs because it is so fast
                        print(reply)
                        print(expected_info[info_index])
                        print(info_index)

                        # if you are supposed to find exiting check for it otherwise skip it
                        # why? Because exiting can appear in many pubs and it's hard to make it pub consistently
                        # whats important is that we see the exiting step and then eventually make it out.
                        if reply.body['actions'][action_name]['status'] == "Exiting" and expected_info[info_index].status != "Exiting":
                            continue
                        else: 
                            assert reply.body['actions'][action_name]['path_name'] == expected_info[info_index].path_name
                            assert reply.body['actions'][action_name]['step_name'] == expected_info[info_index].step_name
                            assert reply.body['actions'][action_name]['status'] == expected_info[info_index].status

                            if abort_on_step is not None and reply.body['actions'][action_name]['step_name'] == abort_on_step:
                                fims_set("/assets/ess/ess_1/actions/Calibration1/stop", True)
                            if set_SoC_on_step is not None and reply.body['actions'][action_name]['step_name'] == set_SoC_on_step[0]: 
                                fims_set("/components/ess_psm/bms_soc", set_SoC_on_step[1])
                            if exit_maint_mode_step is not None and reply.body['actions'][action_name]['step_name'] == exit_maint_mode_step: 
                                fims_set("/assets/ess/ess_1/maint_mode", False)
                                proc.kill()
                                proc.wait()

                            info_index = info_index + 1

                if reply.body['actions'][action_name]['status'] == "Completed" or reply.body['actions'][action_name]['status'] == "Aborted" or reply.body['actions'][action_name]['status'] == "Failed":
                    proc.kill()
                    proc.wait()

            reply = None

def schedule_a_battery_balancing_event(expected_info: Union[List[listen_reply_validator], None] = None, set_SoC_on_step: Union[tuple, None] = None, ignore_step:Union[str, None] = None, action_name: Union[str, None] = None) -> Union[listen_reply, None]:
    """ This will create a mode for battery balancing and then schedule an event
    starting in 1 minute from now. Then watch the balancing and make sure it behaves as expected."""

    # first we need to create the mode to make sure it's there
    def check_for_bat_bal():
        reply = fims_get("/scheduler/modes")
        if 'battery_balancing' in reply:
            return True
        return False
    def set_modes():
       assert modes_cfg == fims_set('/scheduler/modes', modes_cfg)
       assert modes_cfg == fims_get('/scheduler/modes')

    while not check_for_bat_bal():
        # create the mode
        set_modes()
    else: 
        print("already there")

    cmd = ["fims_listen", "-u", "/assets/ess/ess_1"]
    reply = None
    proc = Popen(cmd, stdout=PIPE, stderr=PIPE, universal_newlines=True)

    # clear all events
    fims_set('/scheduler/events', {'durham':[]})
    # get current time int utf format
    utc_now = datetime.utcnow()
    # Add one minute to the current time
    one_minute_later = utc_now + timedelta(minutes=1)
    event = balance_event
    event['start_time'] = one_minute_later.strftime("%Y-%m-%dT%H:%M:%SZ")
    fims_set('/scheduler/events', {'durham':[event]})

    global stdout
    stdout = []

    info_index = 0

    while proc.poll() is None:
        if proc.stdout is not None:
            stdout.append(proc.stdout.readline())

        reply = grok_reply()
        if reply is not None:
            stdout = []
            if reply.contains_action_pub():
                if expected_info is not None and len(expected_info) > 0:
                    if info_index <= len(expected_info) -1:
                        if ignore_step is not None and ignore_step == reply.body['actions'][action_name]['step_name']:
                            continue # this step is hard to catch in pubs because it is so fast

                        # if you are supposed to find exiting check for it otherwise skip it
                        # why? Because exiting can appear in many pubs and it's hard to make it pub consistently
                        # whats important is that we see the exiting step and then eventually make it out.
                        if reply.body['actions'][action_name]['status'] == "Exiting" and expected_info[info_index].status != "Exiting":
                            continue
                        else: 
                            assert reply.body['actions'][action_name]['path_name'] == expected_info[info_index].path_name
                            assert reply.body['actions'][action_name]['step_name'] == expected_info[info_index].step_name
                            assert reply.body['actions'][action_name]['status'] == expected_info[info_index].status

                            if set_SoC_on_step is not None and reply.body['actions'][action_name]['step_name'] == set_SoC_on_step[0]: 
                                fims_set("/components/ess_psm/bms_soc", set_SoC_on_step[1])
                            print(fims_get("/assets/ess/ess_1/maint_mode"))
                            if not fims_get("/assets/ess/ess_1/maint_mode"):
                                proc.kill()
                                proc.wait()

                            info_index = info_index + 1

                if reply.body['actions'][action_name]['status'] == "Completed" or reply.body['actions'][action_name]['status'] == "Aborted" or reply.body['actions'][action_name]['status'] == "Failed":
                    proc.kill()
                    proc.wait()

            reply = None


#####################################################################TESTS########################################################################

# This test will put an ESS in maint_mode and then 
# call an automated action and wait for it it complete
# This test will simply charge to 80%

test1 = []
test1.append(listen_reply_validator(path_name="ESS Calibration", step_name="Start ESS", status="In Progress"))
test1.append(listen_reply_validator(path_name="ESS Calibration", step_name="Setup limits", status="In Progress"))
test1.append(listen_reply_validator(path_name="ESS Calibration", step_name="Enable limits", status="In Progress"))
test1.append(listen_reply_validator(path_name="ESS Calibration", step_name="Set Maint Active Power", status="In Progress"))
test1.append(listen_reply_validator(path_name="ESS Calibration", step_name="Set Maint Active Power 0", status="In Progress"))
test1.append(listen_reply_validator(path_name="ESS Calibration", step_name="Completed", status="Exiting"))
test1.append(listen_reply_validator(path_name="ESS Calibration", step_name="Completed", status="Completed"))

@ fixture
@ parametrize("test", [
    # place all assets in maint_mode
    # start an automated action and monitor it's progress until completed
    Setup(
        "Test clean run",
        {
            **Steps.config_dev_place_assets_in_maint(),
            # Stop the ESS so that the action can start it up
            "/assets/ess/ess_1/stop": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "Stopped", wait_secs=5),
        ]
    ),
    Steps(
        {
        },
        [
        ],
        pre_lambda=[
            # starts the action in function so we can start listening before the first pub
            lambda: validate_action_pubs(uri="/assets/ess/ess_1", expected_info=test1, start="/assets/ess/ess_1/actions/Calibration1/start", 
                                         ignore_step="Charge to 80%", set_SoC_on_step=("Setup limits", 75)),
        ]    
    ),
    # Pause for 10 seconds to let the shutdown_sequence execute
    Steps(
        {
        },
        [
            # shutdown_sequence should stop the action
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "Stopped", wait_secs=10),
        ]
    ),
    # Turn off maint_mode
    Teardown(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": 0,
            **Steps.config_dev_remove_assets_from_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", False),
        ]
    )
])
def test_completed_automated_action(test):
    return test

# This test will put an ESS in maint_mode and then 
# call an automated action and then terminate it when
# it begins step Setup Limits
# (this will not stop the charge)

test2 = []
test2.append(listen_reply_validator(path_name="ESS Calibration", step_name="Start ESS", status="In Progress"))
test2.append(listen_reply_validator(path_name="ESS Calibration", step_name="Setup limits", status="In Progress"))
test2.append(listen_reply_validator(path_name="ESS Calibration", step_name="Setup limits", status="Exiting"))
test2.append(listen_reply_validator(path_name="ESS Calibration", step_name="Setup limits", status="Aborted"))

@ fixture
@ parametrize("test", [
    # place all assets in maint_mode
    # start an automated action and monitor it's progress until completed
    Setup(
        "Test aborted run",
        {
            **Steps.config_dev_place_assets_in_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "Running"),
        ]
    ),
    # Stop the ESS so that the action can start it up
    Steps(
        {
            "/assets/ess/ess_1/stop": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "Stopped", wait_secs=5),
        ]
    ),
    Steps(
        {
        },
        [
        ],
        pre_lambda=[
            # starts the action in function so we can start listening before the first pub
            lambda: validate_action_pubs(uri="/assets/ess/ess_1", expected_info=test2, start="/assets/ess/ess_1/actions/Calibration1/start", abort_on_step="Setup limits", ignore_step="Charge to 80%"),
        ]    
    ),
    # Pause for 10 seconds to let the shutdown_sequence execute
    Steps(
        {
        },
        [
            # shutdown_sequence should stop the action
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "Stopped", wait_secs=10),
        ]
    ),
    # Turn off maint_mode
    Teardown(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": 0,
            **Steps.config_dev_remove_assets_from_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", False),
        ]
    )
])
def test_aborted_automated_action(test):
    return test

# This test will put an ESS in maint_mode and then 
# call an automated action that has a path switch
# both paths will be tested. The switch is based on SoC

test3 = []
# for some reason the pathswitch is only pubbed if there is not a switch performed. 
# probably just changes the vars before pubbing
test3.append(listen_reply_validator(path_name="ESS Calibration (Charge to top)", step_name="Determine Charge Direction", status="In Progress"))
test3.append(listen_reply_validator(path_name="ESS Calibration (Charge to top)", step_name="Start ESS", status="In Progress"))
test3.append(listen_reply_validator(path_name="ESS Calibration (Charge to top)", step_name="Setup limits", status="In Progress"))
test3.append(listen_reply_validator(path_name="ESS Calibration (Charge to top)", step_name="Enable limits", status="In Progress"))
test3.append(listen_reply_validator(path_name="ESS Calibration (Charge to top)", step_name="Set Maint Active Power", status="In Progress"))
test3.append(listen_reply_validator(path_name="ESS Calibration (Charge to top)", step_name="Set Maint Active Power 0", status="In Progress"))
test3.append(listen_reply_validator(path_name="ESS Calibration (Charge to top)", step_name="Completed", status="Exiting"))
test3.append(listen_reply_validator(path_name="ESS Calibration (Charge to top)", step_name="Completed", status="Completed"))

test4 = []
# for some reason the pathswitch is only pubbed if there is not a switch performed. 
# probably just changes the vars before pubbing
# test4.append(listen_reply_validator(path_name="ESS Calibration (Charge to top)", step_name="Determine Charge Direction", status="In Progress")) 
test4.append(listen_reply_validator(path_name="ESS Calibration (Discharge to bottom)", step_name="Start ESS", status="In Progress"))
test4.append(listen_reply_validator(path_name="ESS Calibration (Discharge to bottom)", step_name="Setup limits", status="In Progress"))
test4.append(listen_reply_validator(path_name="ESS Calibration (Discharge to bottom)", step_name="Enable limits", status="In Progress"))
test4.append(listen_reply_validator(path_name="ESS Calibration (Discharge to bottom)", step_name="Set Maint Active Power", status="In Progress"))
test4.append(listen_reply_validator(path_name="ESS Calibration (Discharge to bottom)", step_name="Set Maint Active Power 0", status="In Progress"))
test4.append(listen_reply_validator(path_name="ESS Calibration (Discharge to bottom)", step_name="Completed", status="Exiting"))
test4.append(listen_reply_validator(path_name="ESS Calibration (Discharge to bottom)", step_name="Completed", status="Completed"))

@ fixture
@ parametrize("test", [
    # place all assets in maint_mode
    # start an automated action and monitor it's progress until completed
    Setup(
        "Test path switch run",
        {
            **Steps.config_dev_place_assets_in_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "Running"),
        ]
    ),
    # Stop the ESS so that the action can start it up
    Steps(
        {
            "/assets/ess/ess_1/stop": True,
            "/components/ess_psm/bms_soc": 50
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "Stopped", wait_secs=5),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/soc", 50),
        ]
    ),
    Steps(
        {
        },
        [
        ],
        pre_lambda=[
            # starts the action in function so we can start listening before the first pub
            lambda: validate_action_pubs(uri="/assets/ess/ess_1", expected_info=test3, start="/assets/ess/ess_1/actions/Calibration2/start", ignore_step="Charge to 80%", action_name="Calibration2", set_SoC_on_step=("Start ESS", 85)),
            lambda: time.sleep(2)
        ]    
    ),
    # Pause for 10 seconds to let the shutdown_sequence execute
    Steps(
        {
        },
        [
            # shutdown_sequence should stop the action
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "Stopped", wait_secs=10),
        ]
    ),
    Steps(
        {
            "/components/ess_psm/bms_soc": 85
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/soc", 85),
        ]    
    ),
    Steps(
        {
        },
        [
        ],
        pre_lambda=[
            # starts the action in function so we can start listening before the first pub
            lambda: validate_action_pubs(uri="/assets/ess/ess_1", expected_info=test4, start="/assets/ess/ess_1/actions/Calibration2/start", ignore_step="Discharge to 20%", action_name="Calibration2", set_SoC_on_step=("Start ESS", 15)),
        ]    
    ),
    # Pause for 10 seconds to let the shutdown_sequence execute
    Steps(
        {
        },
        [
            # shutdown_sequence should stop the action
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "Stopped", wait_secs=10),
        ]
    ),
    # Turn off maint_mode
    Teardown(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": 0,
            **Steps.config_dev_remove_assets_from_maint(),
            **Steps.ess_1_disable_all_maint_feats(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", False),
        ]
    )
])
def test_path_switch_automated_action(test):
    return test

# This test will listen for action pubs when no action is undergoing
# making sure we never pub when we shouldn't be

@ fixture
@ parametrize("test", [
    # place all assets in maint_mode
    Setup(
        "Test silent run",
        {
            **Steps.config_dev_place_assets_in_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "Running"),
        ]
    ),
    Steps(
        {
        },
        [
        ],
        pre_lambda=[
            lambda: validate_pubs_silent(timeout=20),
        ]    
    ),
    # Turn off maint_mode
    Teardown(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": 0,
            **Steps.config_dev_remove_assets_from_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", False),
        ]
    )
])
def test_silent_automated_action(test):
    return test

# This test will put an ESS in maint_mode and then 
# call an automated action that contains a fault
# ensure the action exits as failed

test5 = []
test5.append(listen_reply_validator(path_name="ESS Calibration 3", step_name="Pub Fault", status="Exiting"))
test5.append(listen_reply_validator(path_name="ESS Calibration 3", step_name="Pub Fault", status="Failed"))

@ fixture
@ parametrize("test", [
    # place all assets in maint_mode
    Setup(
        "Test failed run",
        {
            **Steps.config_dev_place_assets_in_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "Running"),
        ]
    ),
    Steps(
        {
        },
        [
        ],
        pre_lambda=[
            # starts the action in function so we can start listening before the first pub
            lambda: validate_action_pubs(uri="/assets/ess/ess_1", expected_info=test5, start="/assets/ess/ess_1/actions/Calibration3/start", action_name="Calibration3"),
        ]    
    ),
    Steps(
        {
        },
        [
            # ensure the asset is stopped
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "Stopped", wait_secs=5),
        ]
    ),
    # Turn off maint_mode
    Teardown(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": 0,
            "/assets/ess/ess_1/clear_faults": True,
            **Steps.config_dev_remove_assets_from_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", False),
        ]
    )
])
def test_failed_automated_action(test):
    return test

# This test will put an ESS in maint_mode and then 
# call an automated action that contains a test alarm
# ensure the alarm shows up
test6 = []
test6.append(listen_reply_validator(path_name="ESS Calibration 4", step_name="Pub Alarm", status="In Progress"))

@ fixture
@ parametrize("test", [
    # place all assets in maint_mode
    Setup(
        "Test alarm run",
        {
            **Steps.config_dev_place_assets_in_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "Running"),
        ]
    ),
    Steps(
        {
        },
        [
        ],
        pre_lambda=[
            # starts the action in function so we can start listening before the first pub
            lambda: validate_action_pubs(uri="/assets/ess/ess_1", expected_info=test6, start="/assets/ess/ess_1/actions/Calibration4/start", action_name="Calibration4"),
        ]    
    ),
    # Pause for 10 seconds to let the shutdown_sequence execute
    Steps(
        {
        },
        [
            # shutdown_sequence should stop the action
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "Stopped", wait_secs=10),
        ]
    ),
    # Turn off maint_mode
    Teardown(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": 0,
            "/assets/ess/ess_1/clear_faults": True,
            **Steps.config_dev_remove_assets_from_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", False),
        ]
    )
])
def test_alarm_automated_action(test):
    return test

# This test will put an ESS in maint_mode and then 
# call an automated action 
# However we will then exit maint_mode while the action is running. 
# This should reset the action and clear it's state.
# We will then re-enter maint and run the action and make sure it
# starts at the first step. (aka was actually reset)
test7 = []
test7.append(listen_reply_validator(path_name="ESS Calibration", step_name="Start ESS", status="In Progress"))
test7.append(listen_reply_validator(path_name="ESS Calibration", step_name="Setup limits", status="In Progress"))
test7.append(listen_reply_validator(path_name="ESS Calibration", step_name="Enable limits", status="In Progress"))
test7.append(listen_reply_validator(path_name="ESS Calibration", step_name="Set Maint Active Power", status="In Progress"))
# issue exit maint_mode

@ fixture
@ parametrize("test", [
    # place all assets in maint_mode
    Setup(
        "Test maint mode interactions",
        {
            **Steps.config_dev_place_assets_in_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", True),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "Running"),
        ]
    ),
    Steps(
        {
        },
        [
        ],
        pre_lambda=[
            # starts the action in function so we can start listening before the first pub
            lambda: validate_action_pubs(uri="/assets/ess/ess_1", expected_info=test7, ignore_step="Charge to 80%", start="/assets/ess/ess_1/actions/Calibration1/start",
                                         action_name="Calibration1", exit_maint_mode_step="Set Maint Active Power"),
        ]    
    ),
    # re-enter maint after exiting in above lambda
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": True,
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", True),
        ]    
    ),
    Steps(
        {
        },
        [
        ],
        pre_lambda=[
            # starts the action in function so we can start listening before the first pub
            lambda: validate_action_pubs(uri="/assets/ess/ess_1", expected_info=test1, ignore_step="Charge to 80%", start="/assets/ess/ess_1/actions/Calibration1/start", 
                                         set_SoC_on_step=("Setup limits", 75), action_name="Calibration1"),
        ]    
    ),
    # Turn off maint_mode
    Teardown(
        {
            "/assets/ess/ess_1/maint_active_power_setpoint": 0,
            "/assets/ess/ess_1/clear_faults": True,
            **Steps.config_dev_remove_assets_from_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", False),
        ]
    )
])
def test_maint_mode_early_exit(test):
    return test

# This test will set up a scheduler event starting in 1 minute and then 
# make sure that it works and successfully automatically exits maintenance mode.
# issue exit maint_mode
test8 = []
test8.append(listen_reply_validator(path_name="ESS Calibration", step_name="Start ESS", status="In Progress"))
test8.append(listen_reply_validator(path_name="ESS Calibration", step_name="Setup limits", status="In Progress"))
test8.append(listen_reply_validator(path_name="ESS Calibration", step_name="Enable limits", status="In Progress"))
test8.append(listen_reply_validator(path_name="ESS Calibration", step_name="Set Maint Active Power", status="In Progress"))
test8.append(listen_reply_validator(path_name="ESS Calibration", step_name="Set Maint Active Power 0", status="In Progress"))
test8.append(listen_reply_validator(path_name="ESS Calibration", step_name="Completed", status="Exiting"))
test8.append(listen_reply_validator(path_name="ESS Calibration", step_name="Completed", status="Completed"))

@ fixture
@ parametrize("test", [
    # place all assets in maint_mode
    Setup(
        "Test maint mode interactions",
        {},
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/status", "Running"),
        ]
    ),
    Steps(
        {},
        [],
        pre_lambda=[
            lambda: schedule_a_battery_balancing_event(expected_info=test8, ignore_step="Charge to 80%", set_SoC_on_step=("Setup limits", 75), action_name="Calibration1"),
        ]    
    ),
    # re-enter maint after exiting in above lambda
    Steps(
        {
            "/assets/ess/ess_1/maint_mode": True,
        },
        []    
    ),
    # Turn off maint_mode
    Teardown(
        {
            "/assets/ess/ess_1/clear_faults": True,
            **Steps.config_dev_remove_assets_from_maint(),
        },
        [
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/generators/gen_1/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/ess/ess_2/maint_mode", False),
            Flex_Assertion(Assertion_Type.approx_eq, "/assets/solar/solar_2/maint_mode", False),
        ]
    )
])
def test_scheduler_balancing(test):
    return test
