@logic @less_than_or_equal
Feature: less_than_or_equal

  Scenario: less_than_or_equal function
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
        "bool_output1":false,
        "bool_output2":true,
        "bool_output3":false,
        "bool_output4":true
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "bool_output1":false,
        "bool_output2":true,
        "bool_output3":false,
        "bool_output4":true
      }
      """

  Scenario: less_than_or_equal function, take 2
    When I send a fims pub to /components containing
      """
      {
         "float1" :-22.1,
         "float2" :-22.1,
         "float3" :-12,
         "string1" :"24",
         "string2" :"23"
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "bool_output1":true,
        "bool_output2":false,
        "bool_output3":true,
        "bool_output4":false
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "bool_output1":true,
        "bool_output2":false,
        "bool_output3":true,
        "bool_output4":false
      }
      """