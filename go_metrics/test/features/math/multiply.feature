@math @multiply
Feature: multiply

  Scenario: multiply positive numbers
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
        "float_output1": 646699.2,
        "float_output2": 646699.2
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": 646699.2,
        "float_output2": 646699.2
      }
      """

  Scenario: multiply negative numbers
    When I send a fims pub to /components containing
      """
      {
        "float1" :-22.1,
        "float2" :-3,
        "float3" :-34
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": -2254.2000000000003,
        "float_output2": -2254.2000000000003
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": -2254.2000000000003,
        "float_output2": -2254.2000000000003
      }
      """