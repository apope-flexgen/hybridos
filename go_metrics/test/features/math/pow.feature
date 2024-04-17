@math @pow
Feature: pow

  Scenario: pow of positive numbers
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
        "float_output1": 3646780797.5460644,
        "float_output2": 789.0481
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": 3646780797.5460644,
        "float_output2": 789.0481
      }
      """

  Scenario: pow of negative numbers
    When I send a fims pub to /components containing
      """
      {
        "float1" :-22.1,
        "float2" :-3,
        "float3" :34
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": -0.00009264525455719689,
        "float_output2": 5.1207939995951607e+45
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": -0.00009264525455719689,
        "float_output2": 5.1207939995951607e+45
      }
      """