@math @floor_div
Feature: floor_div

  Scenario: floor_div with positive numbers
    When I send a fims pub to /components containing
      """
      {
        "float1": 125,
        "float2": 4.6
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": 27
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": 27
      }
      """

  Scenario: floor_div with negative numbers
    When I send a fims pub to /components containing
      """
      {
        "float1" :-22.1,
        "float2" :3,
        "float3" :34
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": -8
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": -8
      }
      """