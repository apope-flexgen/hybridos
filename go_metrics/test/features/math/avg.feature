@math @avg
Feature: avg

  Scenario: basic average
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
        "float_output1": 434.55999999999995
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": 434.55999999999995
      }
      """

  Scenario: average of negatives with 0
    When I send a fims pub to /components containing
      """
      {
        "float1" :0,
        "float2" :-345,
        "float3" :34
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": -103.66666666666667
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": -103.66666666666667
      }
      """