@bit_manipulation @bitwise_and
Feature: bitwise_and

  Scenario: bitwise_and of two true bool values
    When I send a fims pub to /components containing
      """
      {
        "int1": 16,
        "int2": 3,
        "int3": 7,
        "bool1": true,
        "bool2": true
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "int_output1": 0,
        "int_output2": 0,
        "bool_output1": true,
        "bool_output2": true
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "int_output1": 0,
        "int_output2": 0,
        "bool_output1": true,
        "bool_output2": true
      }
      """

  Scenario: bitwise_and of true and false bool values
    When I send a fims pub to /components containing
      """
      {
        "int1" :345,
        "int2" :23,
        "int3" :5,
        "bool1": false,
        "bool2": false
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "int_output1": 1,
        "int_output2": 1,
        "bool_output1": false,
        "bool_output2": false
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "int_output1": 1,
        "int_output2": 1,
        "bool_output1": false,
        "bool_output2": false
      }
      """