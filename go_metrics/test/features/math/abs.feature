@math @abs
Feature: abs

  Scenario: absolute value of positive number
    When I send a fims pub to /components containing
      """
      {
        "float1": 34.56,
        "float2": 134.56,
        "float3": 1134.56
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": 34.56,
        "float_output2": 1303.6799999999998
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": 34.56,
        "float_output2": 1303.6799999999998
      }
      """

  Scenario: absolute value of negative number
    When I send a fims pub to /components containing
      """
      {
        "float1" :-1,
        "float2" :-345,
        "float3" :34
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": 1,
        "float_output2": 380
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": 1,
        "float_output2": 380
      }
      """