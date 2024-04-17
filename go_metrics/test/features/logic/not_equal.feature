@logic @not_equal
Feature: not_equal

  Scenario: not_equal function
    When I send a fims pub to /components/float1 containing
      """
      125
      """
    And I send a fims pub to /components/float2 containing
      """
      4.56
      """
    And I send a fims pub to /components/float3 containing
      """
      1134.56
      """
    And I send a fims pub to /components/string1 containing
      """
      "100"
      """
    And I send a fims pub to /components/string2 containing
      """
      "42"
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "bool_output1":true,
        "bool_output2":true,
        "bool_output3":true,
        "bool_output4":true,
        "bool_output5":true
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "bool_output1":true,
        "bool_output2":true,
        "bool_output3":true,
        "bool_output4":true,
        "bool_output5":true
      }
      """

  Scenario: not_equal function, take 2
    When I send a fims pub to /components containing
      """
      {
         "float1" :1,
         "float2" :1,
         "float3" :1,
         "string1" :"1",
         "string2" :"1"
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "bool_output1":false,
        "bool_output2":false,
        "bool_output3":false,
        "bool_output4":false,
        "bool_output5":true
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "bool_output1":false,
        "bool_output2":false,
        "bool_output3":false,
        "bool_output4":false,
        "bool_output5":true
      }
      """