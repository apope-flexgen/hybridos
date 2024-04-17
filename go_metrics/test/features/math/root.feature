@math @root
Feature: root

  Scenario: root of positive number
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
        "float_output1": 2.883026850104486,
        "float_output2": 2.883026850104486
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": 2.883026850104486,
        "float_output2": 2.883026850104486
      }
      """

# Should not work and should just yield the same as the previous test case
  Scenario: root of negative number
    When I send a fims pub to /components containing
      """
      {
        "float1" :-1,
        "float2" :3
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": 2.883026850104486,
        "float_output2": 2.883026850104486
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": 2.883026850104486,
        "float_output2": 2.883026850104486
      }
      """