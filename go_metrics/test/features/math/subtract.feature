@math @subtract
Feature: subtract

  Scenario: subtract positive numbers
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
        "float_output1": 120.44,
        "float_output2": -1014.1199999999999
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": 120.44,
        "float_output2": -1014.1199999999999
      }
      """

  Scenario: subtract negative numbers
    When I send a fims pub to /components containing
      """
      {
        "float1": -22.1,
        "float2": -3,
        "float3": 34
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": -19.1,
        "float_output2": -53.1
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": -19.1,
        "float_output2": -53.1
      }
      """