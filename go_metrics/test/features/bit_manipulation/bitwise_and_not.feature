@bit_manipulation @bitwise_and_not
Feature: bitwise_and_not

  Scenario: bitwise_and_not of two true bools
    When I send a fims pub to /components containing
      """
      {
        "int1": 16,
        "int2": 3,
        "bool1": true,
        "bool2": true
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "int_output1": 16,
        "int_output2": 16,
        "bool_output1": false,
        "bool_output2": false
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "int_output1": 16,
        "int_output2": 16,
        "bool_output1": false,
        "bool_output2": false
      }
      """

  Scenario: bitwise_and_not of true and false bool
    When I send a fims pub to /components containing
      """
      {
        "int1": 345,
        "int2": 23,
        "bool1": true,
        "bool2": false
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "int_output1": 328,
        "int_output2": 328,
        "bool_output1": true,
        "bool_output2": true
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "int_output1": 328,
        "int_output2": 328,
        "bool_output1": true,
        "bool_output2": true
      }
      """