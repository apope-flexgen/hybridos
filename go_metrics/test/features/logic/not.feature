@logic @not
Feature: not

  Scenario: not
    When I send a fims pub to /components containing
      """
      {
        "int1": 0,
        "int2": 0,
        "bool1": false,
        "bool2": false
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "bool_output1":true,
        "bool_output2":true,
        "bool_output3":true,
        "bool_output4":true
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "bool_output1":true,
        "bool_output2":true,
        "bool_output3":true,
        "bool_output4":true
      }
      """

  Scenario: not, take 2
    When I send a fims pub to /components containing
      """
      {
        "int1": 7,
        "int2": 1,
        "bool1": true,
        "bool2": true
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "bool_output1":false,
        "bool_output2":false,
        "bool_output3":false,
        "bool_output4":false
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "bool_output1":false,
        "bool_output2":false,
        "bool_output3":false,
        "bool_output4":false
      }
      """