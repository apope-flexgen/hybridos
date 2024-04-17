@math @ceil
Feature: ceil

  Scenario: ceil with positive numbers
    When I send a fims pub to /components containing
      """
      {
        "float1": 34.56,
        "float2": 134.42
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": 35,
        "float_output2": 135
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": 35,
        "float_output2": 135
      }
      """

  Scenario: ceil with negative numbers
    When I send a fims pub to /components containing
      """
      {
        "float1": -34.56,
        "float2": -134.42
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": -34,
        "float_output2": -134
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": -34,
        "float_output2": -134
      }
      """