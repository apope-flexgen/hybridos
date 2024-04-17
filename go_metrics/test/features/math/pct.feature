@math @pct
Feature: pct

  Scenario: pct with positive numbers
    When I send a fims pub to /components containing
      """
      {
        "float1": 125,
        "float2": 4.56
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": 2741.228070175439
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": 2741.228070175439
      }
      """

  Scenario: pct with negative numbers
    When I send a fims pub to /components containing
      """
      {
        "float1" :3,
        "float2" :-22.1
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": -13.574660633484163
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": -13.574660633484163
      }
      """