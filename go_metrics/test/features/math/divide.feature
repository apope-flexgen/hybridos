@math @divide
Feature: divide

  Scenario: divide positive numbers
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
        "float_output1": 0.2568370986920333,
        "float_output2": 0.00022637595075803246
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": 0.2568370986920333,
        "float_output2": 0.00022637595075803246
      }
      """

  Scenario: divide negative numbers
    When I send a fims pub to /components containing
      """
      {
        "float1" :-22.1,
        "float2" :345,
        "float3" :34
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": -0.06405797101449276,
        "float_output2": -0.001884057971014493
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": -0.06405797101449276,
        "float_output2": -0.001884057971014493
      }
      """

  Scenario: divide by zero
    When I send a fims pub to /components containing
      """
      {
        "float1" :-22.1,
        "float2" :0,
        "float3" :34
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": -0.06405797101449276,
        "float_output2": -0.001884057971014493
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": -0.06405797101449276,
        "float_output2": -0.001884057971014493
      }
      """