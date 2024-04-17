@bit_manipulation @left_shift
Feature: left_shift

  Scenario: basic left_shift
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
        "int_output1": 128,
        "int_output2": 128
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "int_output1": 128,
        "int_output2": 128
      }
      """

  Scenario: failed left shift
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
        "int_output1": 128,
        "int_output2": 128
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "int_output1": 128,
        "int_output2": 128
      }
      """