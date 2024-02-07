'''
The check_test_case module contains tools for checking if an actual test case is identical
to the expected test case.
'''
import json
from compare_messages import compare_messages, SUCCESS

def check_test_case(test_case_num_str, expected_test_case, actual_test_case):
    '''
    check_test_case compares the messages in expected_test_case to the messages
    in actual_test_case. If every message in expected_test_case has a message
    in actual_test_case that contains the same contents, the function returns
    True and an empty message. If the test case does not match what is expected,
    the function returns False and a message describing the expected vs. actual.
    '''
    if expected_test_case is not None and actual_test_case is None:
        return False, print(f"Test Case {test_case_num_str}:\n" +
                            f"Expected: {json.dumps(expected_test_case)}\n" +
                            "Got: No messages for this test case!\n")
    if actual_test_case is not None and expected_test_case is None:
        return False, print(f"Test Case {test_case_num_str}:\n" +
                            "Expected: No messages for this test case!\n" +
                            f"Got: {json.dumps(actual_test_case)}\n")
    passed_subconditions = []
    highest_return_value = -1
    final_return_message = ""
    for expected_message in expected_test_case:
        one_match = False
        if len(actual_test_case) == 0:
            passed_subconditions.append(False)
            print(f"Test Case {test_case_num_str}:\n" +
                  f"Expected: {expected_message['body']}\n" +
                  "Got: No messages for this test case!\n")
        else:
            for message in actual_test_case:
                [return_code, return_message] = compare_messages(test_case_num_str,
                                                                 expected_message,
                                                                 message)
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

    return False, final_return_message
