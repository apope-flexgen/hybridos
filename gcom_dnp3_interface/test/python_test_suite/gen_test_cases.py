'''
Generate modbus or dnp3 test cases based on register types.
Some test cases are fixed, but some are random.
'''
import math
import random
from numeric_limits import MIN_INT_16, MAX_INT_16, MAX_INT_32, \
        MIN_INT_32, MAX_INT_64, MIN_INT_64, MIN_UINT_16, MAX_UINT_16, \
        MAX_UINT_32, MIN_UINT_32, MAX_UINT_64, MAX_FLOAT_32, MIN_FLOAT_32, \
        MAX_FLOAT_64, MIN_FLOAT_64, MIN_UINT_64
from comms_configs import MergedRegister

COMMANDS_BY_TEST_ID = {}    # dictionary of all commands by test_id

def check_limits(register: MergedRegister, value):
    '''
    Make sure that 'value' falls within the limits of the register
    data type. If not, change value so that it does fall within
    the limits of the register data type.
    '''
    register_type = register.data_type
    limits = {
    "int16": (MIN_INT_16, MAX_INT_16),
    "int32": (MIN_INT_32, MAX_INT_32),
    "int64": (MIN_INT_64, MAX_INT_64),
    "uint16": (MIN_UINT_16, MAX_UINT_16),
    "uint32": (MIN_UINT_32, MAX_UINT_32),
    "uint64": (MIN_UINT_64, MAX_UINT_64),
    "float32": (-MAX_FLOAT_32, MAX_FLOAT_32),
    "float64": (-MAX_FLOAT_64, MAX_FLOAT_64),
    }

    min_limit, max_limit = limits.get(register_type, (float('-inf'), float('inf')))
    value = max(min(value, max_limit), min_limit)
    if register_type == "float32" and abs(value) < MIN_FLOAT_32:
        value = 0
    elif register_type == "float64" and abs(value) < MIN_FLOAT_64:
        value = 0
    return value



def expected_value(register: MergedRegister, value):
    '''
    Based on a value of any numeric data type, return the value that
    the client or server should output in sets or pubs if the original
    value is assigned to a specific register.
    '''
    if register.data_type == "bool":
        if not isinstance(value, bool):
            value = bool(value)
        if register.client_scale < 0:
            value = not value
        if register.server_scale < 0:
            value = not value
        return value

    if register.method == "pub":
        first_scale = register.server_scale
        second_scale = register.client_scale
    elif register.method == "set":
        first_scale = register.client_scale
        second_scale = register.server_scale

    if register.data_type in ["uint64", "int64", "int32", "uint32", "int16", "uint16"]:
        value = value * first_scale
        value = round(math.trunc(value))
        value = check_limits(register, value)
        if second_scale == 1:
            value = int(value)
        else:
            value = value / second_scale
    else:
        value = value * first_scale
        value = check_limits(register, value)
        value = value / second_scale
        if register.data_type == "float32":
            value = "{:.{}g}".format(value, 16)
        value = float(value)
    return value

def test_basics(starting_test_id: int, test_register: MergedRegister, register: MergedRegister):
    '''
    Construct several tests to probe basic considerations for registers (extremes, 0, strings,
    bools, etc.)
    '''
    fims_commands = []
    expected_messages = {}
    test_id = starting_test_id

    if register.data_type is None:
        return test_id, fims_commands, expected_messages

    # negative float
    value = -random.random()*512
    while (register.client_scale * value > MAX_INT_16 or
           register.client_scale * value < MIN_INT_16 or
           register.server_scale * value > MAX_INT_16 or
           register.server_scale * value < MIN_INT_16):
        value = -random.random()*512
    expected_val = expected_value(register, value)

    fims_commands.append([test_register.gen_fims_message(test_id),register.gen_fims_message(value)])
    COMMANDS_BY_TEST_ID[test_id] = fims_commands[-1]

    expected_messages[test_id] = [test_register.gen_expected_result(test_id),
                                  register.gen_expected_result(expected_val)]

    test_id += 1

    # positive float
    value = random.random()*512
    while (register.client_scale * value > MAX_INT_16 or
           register.client_scale * value < MIN_INT_16 or
           register.server_scale * value > MAX_INT_16 or
           register.server_scale * value < MIN_INT_16):
        value = random.random()*512
    expected_val = expected_value(register, value)

    fims_commands.append([test_register.gen_fims_message(test_id),register.gen_fims_message(value)])
    COMMANDS_BY_TEST_ID[test_id] = fims_commands[-1]

    expected_messages[test_id] = [test_register.gen_expected_result(test_id),
                                  register.gen_expected_result(expected_val)]

    test_id += 1

    # negative int
    value = random.randint(-512,-1)
    while (register.client_scale * value > MAX_INT_16 or
           register.client_scale * value < MIN_INT_16 or
           register.server_scale * value > MAX_INT_16 or
           register.server_scale * value < MIN_INT_16):
        value = random.randint(-512,-1)
    expected_val = expected_value(register, value)

    fims_commands.append([test_register.gen_fims_message(test_id),register.gen_fims_message(value)])
    COMMANDS_BY_TEST_ID[test_id] = fims_commands[-1]

    expected_messages[test_id] = [test_register.gen_expected_result(test_id),
                                  register.gen_expected_result(expected_val)]

    test_id += 1

    # positive int
    value = random.randint(1,512)
    while (register.client_scale * value > MAX_INT_16 or
           register.client_scale * value < MIN_INT_16 or
           register.server_scale * value > MAX_INT_16 or
           register.server_scale * value < MIN_INT_16):
        value = random.randint(1,512)
    expected_val = expected_value(register, value)

    fims_commands.append([test_register.gen_fims_message(test_id),register.gen_fims_message(value)])
    COMMANDS_BY_TEST_ID[test_id] = fims_commands[-1]

    expected_messages[test_id] = [test_register.gen_expected_result(test_id),
                                  register.gen_expected_result(expected_val)]

    test_id += 1

    # 0
    value = 0
    expected_val = expected_value(register, value)

    fims_commands.append([test_register.gen_fims_message(test_id),register.gen_fims_message(value)])
    COMMANDS_BY_TEST_ID[test_id] = fims_commands[-1]

    expected_messages[test_id] = [test_register.gen_expected_result(test_id),
                                  register.gen_expected_result(expected_val)]

    test_id += 1

    # true
    value = True
    expected_val = expected_value(register, value)

    fims_commands.append([test_register.gen_fims_message(test_id),register.gen_fims_message(value)])
    COMMANDS_BY_TEST_ID[test_id] = fims_commands[-1]

    expected_messages[test_id] = [test_register.gen_expected_result(test_id),
                                  register.gen_expected_result(expected_val)]

    test_id += 1

    # false
    value = False
    expected_val = expected_value(register, value)

    fims_commands.append([test_register.gen_fims_message(test_id),register.gen_fims_message(value)])
    COMMANDS_BY_TEST_ID[test_id] = fims_commands[-1]

    expected_messages[test_id] = [test_register.gen_expected_result(test_id),
                                  register.gen_expected_result(expected_val)]

    test_id += 1

    # string
    value = "\"some_random_string\""


    fims_commands.append([test_register.gen_fims_message(test_id),register.gen_fims_message(value)])
    COMMANDS_BY_TEST_ID[test_id] = fims_commands[-1]

    expected_messages[test_id] = [test_register.gen_expected_result(test_id)]

    test_id += 1

    # int16 overflow
    value = 1 << 17
    expected_val = expected_value(register, value)

    fims_commands.append([test_register.gen_fims_message(test_id),register.gen_fims_message(value)])
    COMMANDS_BY_TEST_ID[test_id] = fims_commands[-1]

    expected_messages[test_id] = [test_register.gen_expected_result(test_id),
                                  register.gen_expected_result(expected_val)]

    test_id += 1

    # int16 underflow
    value = -(1 << 17)
    expected_val = expected_value(register, value)

    fims_commands.append([test_register.gen_fims_message(test_id),register.gen_fims_message(value)])
    COMMANDS_BY_TEST_ID[test_id] = fims_commands[-1]

    expected_messages[test_id] = [test_register.gen_expected_result(test_id),
                                  register.gen_expected_result(expected_val)]

    test_id += 1

    # int32 overflow
    value = 1 << 33
    expected_val = expected_value(register, value)

    fims_commands.append([test_register.gen_fims_message(test_id),register.gen_fims_message(value)])
    COMMANDS_BY_TEST_ID[test_id] = fims_commands[-1]

    expected_messages[test_id] = [test_register.gen_expected_result(test_id),
                                  register.gen_expected_result(expected_val)]

    test_id += 1

    # int32 underflow
    value = -(1 << 33)
    expected_val = expected_value(register, value)

    fims_commands.append([test_register.gen_fims_message(test_id),register.gen_fims_message(value)])
    COMMANDS_BY_TEST_ID[test_id] = fims_commands[-1]

    expected_messages[test_id] = [test_register.gen_expected_result(test_id),
                                  register.gen_expected_result(expected_val)]

    test_id += 1

    # float32 overflow
    value = MAX_FLOAT_32*2
    expected_val = expected_value(register, value)

    fims_commands.append([test_register.gen_fims_message(test_id),register.gen_fims_message(value)])
    COMMANDS_BY_TEST_ID[test_id] = fims_commands[-1]

    expected_messages[test_id] = [test_register.gen_expected_result(test_id),
                                  register.gen_expected_result(expected_val)]

    test_id += 1

    # float32 underflow
    value = MIN_FLOAT_32 * 0.0001
    expected_val = expected_value(register, value)

    fims_commands.append([test_register.gen_fims_message(test_id),register.gen_fims_message(value)])
    COMMANDS_BY_TEST_ID[test_id] = fims_commands[-1]

    expected_messages[test_id] = [test_register.gen_expected_result(test_id),
                                  register.gen_expected_result(expected_val)]

    test_id += 1

    # float64 overflow
    value = '9'*400
    # expected_val = expected_val # message doesn't get parsed

    fims_commands.append([test_register.gen_fims_message(test_id),register.gen_fims_message(value)])
    COMMANDS_BY_TEST_ID[test_id] = fims_commands[-1]

    if register.method == "pub":
        expected_messages[test_id] = [test_register.gen_expected_result(test_id),
                                      register.gen_expected_result(expected_val)]
    else:
        expected_messages[test_id] = [test_register.gen_expected_result(test_id)]

    test_id += 1

    # float64 underflow
    value = '0'*400
    value = '0.' + value
    expected_val = 0

    fims_commands.append([test_register.gen_fims_message(test_id),register.gen_fims_message(value)])
    COMMANDS_BY_TEST_ID[test_id] = fims_commands[-1]

    if register.method == "pub":
        expected_messages[test_id] = [test_register.gen_expected_result(test_id),
                                      register.gen_expected_result(expected_val)]
    else:
        expected_messages[test_id] = [test_register.gen_expected_result(test_id)]

    test_id += 1

    return test_id, fims_commands, expected_messages
