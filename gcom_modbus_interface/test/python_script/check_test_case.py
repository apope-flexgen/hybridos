from compare_messages import *
from test_utils import *
from git_info import *
from check_uris import *

def check_test_case(test_case_num_str, expected_test_case, actual_test_case):
    global commands_by_test_id
    passed_subconditions = []
    highest_return_value = -1
    final_return_message = ""
    for expected_message in expected_test_case: 
        one_match = False
        if len(actual_test_case) == 0:
            passed_subconditions.append(False)
            print(f"Test Case {test_case_num_str}:\n{commands_by_test_id[test_id]}Expected: {expected_message['body']}\nGot: No messages for this test case!\n")
        else:
            for message in actual_test_case:
                [return_code, return_message] = compare_messages(test_case_num_str, expected_message, message)
                if return_code == SUCCESS:
                    one_match = True
                else:
                    if return_code >= highest_return_value:
                        highest_return_value = return_code
                        final_return_message = return_message
        if one_match:
            passed_subconditions.append(True)
        else:
            passed_subconditions.append(False)
            if final_return_message!= "":
                print(final_return_message)

    if all(passed_subconditions):
        return True, final_return_message
    else:
        return False, final_return_message
