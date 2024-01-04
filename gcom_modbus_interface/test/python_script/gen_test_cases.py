import random
from test_utils import *
import math
from configs import *
  
def check_limits(register: MergedRegister, value):
    register_type = register.data_type
    if register_type == "int16":
        if value > MAX_INT_16:
            value = MAX_INT_16
        elif value < MIN_INT_16:
            value = MIN_INT_16
    elif register_type == "int32":
        if value > MAX_INT_32:
            value = MAX_INT_32
        elif value < MIN_INT_32:
            value = MIN_INT_32
    elif register_type == "int64":
        if value > MAX_INT_64:
            value = MAX_INT_64
        elif value < MIN_INT_64:
            value = MIN_INT_64
    elif register_type == "uint16":
        if value > MAX_UINT_16:
            value = MAX_UINT_16
        elif value < 0:
            value = 0
    elif register_type == "uint32":
        if value > MAX_UINT_32:
            value = MAX_UINT_32
        elif value < 0:
            value = 0
    elif register_type == "uint64":
        if value > MAX_UINT_64:
            value = MAX_UINT_64
        elif value < 0:
            value = 0
    elif register_type == "float32":
        if value > MAX_FLOAT_32:
            value = MAX_FLOAT_32
        elif value < -MAX_FLOAT_32:
            value = -MAX_FLOAT_32
        elif abs(value) < MIN_FLOAT_32:
            value = 0
    elif register_type == "float64":
        if value > MAX_FLOAT_64:
            value = MAX_FLOAT_64
        elif value < -MAX_FLOAT_64:
            value = -MAX_FLOAT_64
        elif abs(value) < MIN_FLOAT_64:
            value = 0
    return value



def expected_value(register: MergedRegister, value):
    if register.data_type == "bool":
        if not isinstance(value, bool):
            if value <= 0:
                value = False
            else:
                value = True
        if register.client_scale < 0:
            value = not value
        if register.server_scale < 0:
            value = not value
        return value

    if register.method == "pub":
        first_scale = register.server_scale
        first_sign = register.server_signed
        second_scale = register.client_scale
        second_sign = register.client_signed
    elif register.method == "set":
        first_scale = register.client_scale
        first_sign = register.client_signed
        second_scale = register.server_scale
        second_sign = register.server_signed

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
        value = float(value)
    return value

def test_basics(starting_test_id: int, test_register: MergedRegister, register: MergedRegister):
    global commands_by_test_id
    fims_commands = []
    expected_messages = {}
    test_id = starting_test_id

    if register.data_type == None:
        return test_id, fims_commands, expected_messages
    
    # negative float
    value = -random.random()*512
    while register.client_scale * value > MAX_INT_16 or register.client_scale * value < MIN_INT_16 or register.server_scale * value > MAX_INT_16 or register.server_scale * value < MIN_INT_16:
        value = -random.random()*512
    expected_val = expected_value(register, value)

    fims_commands.append(test_register.gen_fims_message(test_id))
    fims_commands.append(register.gen_fims_message(value))
    commands_by_test_id[test_id] = fims_commands[-1][0:fims_commands[-1].find("\n")+1]
    
    expected_messages[test_id] = [register.gen_expected_result(expected_val)]
    
    test_id += 1

    
    # positive float
    value = random.random()*512
    while register.client_scale * value > MAX_INT_16 or register.client_scale * value < MIN_INT_16 or register.server_scale * value > MAX_INT_16 or register.server_scale * value < MIN_INT_16:
        value = random.random()*512
    expected_val = expected_value(register, value)

    fims_commands.append(test_register.gen_fims_message(test_id))
    fims_commands.append(register.gen_fims_message(value))
    commands_by_test_id[test_id] = fims_commands[-1][0:fims_commands[-1].find("\n")+1]
    
    expected_messages[test_id] = [register.gen_expected_result(expected_val)]
    
    test_id += 1
    
    # negative int
    value = random.randint(-512,-1)
    while register.client_scale * value > MAX_INT_16 or register.client_scale * value < MIN_INT_16 or register.server_scale * value > MAX_INT_16 or register.server_scale * value < MIN_INT_16:
        value = random.randint(-512,-1)
    expected_val = expected_value(register, value)

    fims_commands.append(test_register.gen_fims_message(test_id))
    fims_commands.append(register.gen_fims_message(value))
    commands_by_test_id[test_id] = fims_commands[-1][0:fims_commands[-1].find("\n")+1]
    
    expected_messages[test_id] = [register.gen_expected_result(expected_val)]
    
    test_id += 1
    
    # positive int
    value = random.randint(1,512)
    while register.client_scale * value > MAX_INT_16 or register.client_scale * value < MIN_INT_16 or register.server_scale * value > MAX_INT_16 or register.server_scale * value < MIN_INT_16:
        value = random.randint(1,512)
    expected_val = expected_value(register, value)

    fims_commands.append(test_register.gen_fims_message(test_id))
    fims_commands.append(register.gen_fims_message(value))
    commands_by_test_id[test_id] = fims_commands[-1][0:fims_commands[-1].find("\n")+1]
    
    expected_messages[test_id] = [register.gen_expected_result(expected_val)]
    
    test_id += 1
    
    # 0
    value = 0
    expected_val = expected_value(register, value)

    fims_commands.append(test_register.gen_fims_message(test_id))
    fims_commands.append(register.gen_fims_message(value))
    commands_by_test_id[test_id] = fims_commands[-1][0:fims_commands[-1].find("\n")+1]
    
    expected_messages[test_id] = [register.gen_expected_result(expected_val)]
    
    test_id += 1
    
    # true
    value = True
    expected_val = expected_value(register, value)

    fims_commands.append(test_register.gen_fims_message(test_id))
    fims_commands.append(register.gen_fims_message(value))
    commands_by_test_id[test_id] = fims_commands[-1][0:fims_commands[-1].find("\n")+1]
    
    expected_messages[test_id] = [register.gen_expected_result(expected_val)]
    
    test_id += 1
    
    # false
    value = False
    expected_val = expected_value(register, value)

    fims_commands.append(test_register.gen_fims_message(test_id))
    fims_commands.append(register.gen_fims_message(value))
    commands_by_test_id[test_id] = fims_commands[-1][0:fims_commands[-1].find("\n")+1]
    
    expected_messages[test_id] = [register.gen_expected_result(expected_val)]
    
    test_id += 1
    
    # string
    value = "\"some_random_string\""


    fims_commands.append(test_register.gen_fims_message(test_id))
    fims_commands.append(register.gen_fims_message(value))
    commands_by_test_id[test_id] = fims_commands[-1][0:fims_commands[-1].find("\n")+1]
    
    expected_messages[test_id] = []
    
    test_id += 1
    
    # int16 overflow
    value = 1 << 17
    expected_val = expected_value(register, value)

    fims_commands.append(test_register.gen_fims_message(test_id))
    fims_commands.append(register.gen_fims_message(value))
    commands_by_test_id[test_id] = fims_commands[-1][0:fims_commands[-1].find("\n")+1]
    
    expected_messages[test_id] = [register.gen_expected_result(expected_val)]
    
    test_id += 1

    # int16 underflow
    value = -(1 << 17)
    expected_val = expected_value(register, value)

    fims_commands.append(test_register.gen_fims_message(test_id))
    fims_commands.append(register.gen_fims_message(value))
    commands_by_test_id[test_id] = fims_commands[-1][0:fims_commands[-1].find("\n")+1]
    
    expected_messages[test_id] = [register.gen_expected_result(expected_val)]
    
    test_id += 1

    # int32 overflow
    value = 1 << 33
    expected_val = expected_value(register, value)

    fims_commands.append(test_register.gen_fims_message(test_id))
    fims_commands.append(register.gen_fims_message(value))
    commands_by_test_id[test_id] = fims_commands[-1][0:fims_commands[-1].find("\n")+1]
    
    expected_messages[test_id] = [register.gen_expected_result(expected_val)]
    
    test_id += 1

    # int32 underflow
    value = -(1 << 33)
    expected_val = expected_value(register, value)

    fims_commands.append(test_register.gen_fims_message(test_id))
    fims_commands.append(register.gen_fims_message(value))
    commands_by_test_id[test_id] = fims_commands[-1][0:fims_commands[-1].find("\n")+1]
    
    expected_messages[test_id] = [register.gen_expected_result(expected_val)]
    
    test_id += 1

    # float32 overflow
    value = MAX_FLOAT_32*2
    expected_val = expected_value(register, value)

    fims_commands.append(test_register.gen_fims_message(test_id))
    fims_commands.append(register.gen_fims_message(value))
    commands_by_test_id[test_id] = fims_commands[-1][0:fims_commands[-1].find("\n")+1]
    
    expected_messages[test_id] = [register.gen_expected_result(expected_val)]
    
    test_id += 1

    # float32 underflow
    value = MIN_FLOAT_32 * 0.0001
    expected_val = expected_value(register, value)

    fims_commands.append(test_register.gen_fims_message(test_id))
    fims_commands.append(register.gen_fims_message(value))
    commands_by_test_id[test_id] = fims_commands[-1][0:fims_commands[-1].find("\n")+1]
    
    expected_messages[test_id] = [register.gen_expected_result(expected_val)]
    
    test_id += 1

    # float64 overflow
    value = '9'*400
    expected_val = expected_val # message doesn't get parsed

    fims_commands.append(test_register.gen_fims_message(test_id))
    fims_commands.append(register.gen_fims_message(value))
    commands_by_test_id[test_id] = fims_commands[-1][0:fims_commands[-1].find("\n")+1]
    
    if register.method == "pub":
        expected_messages[test_id] = [register.gen_expected_result(expected_val)]
    else:
        expected_messages[test_id] = []
    
    test_id += 1

    # float64 underflow
    value = '0'*400
    value = '0.' + value
    expected_val = 0

    fims_commands.append(test_register.gen_fims_message(test_id))
    fims_commands.append(register.gen_fims_message(value))
    commands_by_test_id[test_id] = fims_commands[-1][0:fims_commands[-1].find("\n")+1]
    
    if register.method == "pub":
        expected_messages[test_id] = [register.gen_expected_result(expected_val)]
    else:
        expected_messages[test_id] = []
    
    test_id += 1

    return test_id, fims_commands, expected_messages