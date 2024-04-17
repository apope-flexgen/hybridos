@logic @or
Feature: or

  Scenario: or function
    When I send a fims pub to /components/int1 containing
      """
      16
      """
    And I send a fims pub to /components/int2 containing
      """
      3
      """
    And I send a fims pub to /components/bool1 containing
      """
      true
      """
    And I send a fims pub to /components/bool2 containing
      """
      true
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "bool_output1":true,
        "bool_output2":true
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "bool_output1":true,
        "bool_output2":true
      }
      """

  Scenario: or function, take 2
    When I send a fims pub to /components containing
      """
      {
         "int1" :0,
         "int2" :0,
         "bool1" :false,
         "bool2" :false
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "bool_output1":false,
        "bool_output2":false
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "bool_output1":false,
        "bool_output2":false
      }
      """