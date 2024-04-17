@bit_manipulation @right_shift
Feature: right_shift

  Scenario: basic right_shift
    When I send a fims pub to /components containing
      """
      {
        "int1": 16,
        "int2": 3
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "int_output1": 2,
        "int_output2": 2
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "int_output1": 2,
        "int_output2": 2
      }
      """

  Scenario: big right shift
    When I send a fims pub to /components containing
      """
      {
        "int1": 345,
        "int2": 100
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "int_output1": 0,
        "int_output2": 0
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "int_output1": 0,
        "int_output2": 0
      }
      """