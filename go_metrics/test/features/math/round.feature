@math @round
Feature: round

  Scenario: round positive number
    When I send a fims pub to /components containing
      """
      {
        "float1": 125.1,
        "float2": 4.56
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": 125,
        "float_output2": 5
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": 125,
        "float_output2": 5
      }
      """

  Scenario: round negative number
    When I send a fims pub to /components containing
      """
      {
        "float1" :-125.1,
        "float2" :-125.9
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": -125,
        "float_output2": -126
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": -125,
        "float_output2": -126
      }
      """