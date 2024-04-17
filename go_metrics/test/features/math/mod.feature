@math @mod
Feature: mod

  Scenario: mod of positive numbers
    When I send a fims pub to /components containing
      """
      {
        "int1": 16,
        "int2": 3,
        "int3": 7
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "int_output1": 1,
        "int_output2": 1
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "int_output1": 1,
        "int_output2": 1
      }
      """

  Scenario: mod of negative numbers
    When I send a fims pub to /components containing
      """
      {
        "int1": -22,
        "int2": 3,
        "int3": -2
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "int_output1": -1,
        "int_output2": -1
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "int_output1": -1,
        "int_output2": -1
      }
      """