@math @floor
Feature: floor

  Scenario: floor of positive number
    When I send a fims pub to /components containing
      """
      {
        "float1": 125.6,
        "float2": 4.3
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": 125,
        "float_output2": 4
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": 125,
        "float_output2": 4
      }
      """

  Scenario: floor of negative number
    When I send a fims pub to /components containing
      """
      {
        "float1" :-22.1,
        "float2" :-3.55
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": -23,
        "float_output2": -4
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": -23,
        "float_output2": -4
      }
      """