@logic @and_nested
Feature: and_nested

  Scenario: and function with nested uri
    When I send a fims pub to /components/int1 containing
      """
      16
      """
    And I send a fims pub to /components/int2 containing
      """
      3
      """
    And I send a fims pub to /components/int3 containing
      """
      7
      """
    And I send a fims pub to /components/bool1 containing
      """
      true
      """
    And I send a fims pub to /components/bool2 containing
      """
      true
      """
    Then I expect a fims pub to /some/output/sub containing
      """
      {
        "bool_sub_output3":false
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "bool_output1":true,
        "bool_output2":true,
        "bool_output3":false
      }
      """
    And a fims get to /some/output/sub should yield
      """
      {
        "bool_sub_output3":false
      }
      """

  Scenario: and function with nested uri, take 2
    When I send a fims pub to /components containing
      """
      {
         "int1" :0,
         "int2" :0,
         "int3" :5,
         "bool1" :true,
         "bool2" :false
      }
      """
    And I send a fims pub to /components/int1 containing
      """
      {
         "sub1" :345
      }
      """
    Then I expect a fims pub to /some/output/sub containing
      """
      {"bool_sub_output3":true}
      """
    And a fims get to /some/output should yield
      """
      {
        "bool_output1":false,
        "bool_output2":false,
        "bool_output3":false
      }
      """
    And a fims get to /some/output/sub should yield
      """
      {"bool_sub_output3":true}
      """