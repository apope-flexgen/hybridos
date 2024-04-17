@math @rss
Feature: rss

  Scenario: rss of positive numbers
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
        "float_output1": 1141.4342675774194
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": 1141.4342675774194
      }
      """

  Scenario: rss of negative numbers
    When I send a fims pub to /components containing
      """
      {
        "float1": -125,
        "float2": -4.56,
        "float3": -1134.56
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": 1141.4342675774194
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": 1141.4342675774194
      }
      """