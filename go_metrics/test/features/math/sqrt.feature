@math @sqrt
Feature: sqrt

  Scenario: sqrt of positive number
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
        "float_output1": 11.180339887498949,
        "float_output2":15.451171188311474,
        "value2_echo":4.56,
        "value3_echo":4.56
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": 11.180339887498949,
        "float_output2":15.451171188311474,
        "value2_echo":4.56,
        "value3_echo":4.56
      }
      """

# Should not work and should just yield the same as the previous test case
  Scenario: sqrt of negative number
    When I send a fims pub to /components containing
      """
      {
        "float1" :-125,
        "float2" :-3
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": 11.180339887498949,
        "float_output2":15.451171188311474,
        "value2_echo":-3,
        "value3_echo":-3
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": 11.180339887498949,
        "float_output2":15.451171188311474,
        "value2_echo":-3,
        "value3_echo":-3
      }
      """