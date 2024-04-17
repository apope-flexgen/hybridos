@math @min
Feature: min

  Scenario: min of positive numbers
    When I send a fims pub to /components containing
      """
      {
        "float1": 125,
        "float2": 4.56,
        "float3": 1134.56
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": 4.56
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": 4.56
      }
      """

  Scenario: min of negative numbers
    When I send a fims pub to /components containing
      """
      {
        "float1" :-22.1,
        "float2" :-3,
        "float3" :-34.5
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": -34.5
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": -34.5
      }
      """