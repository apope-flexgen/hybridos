@math @add
Feature: add

  Scenario: basic addition
    When I send a fims pub to /components containing
      """
      {
        "float1": 34.56,
        "float2": 134.56,
        "float3": 1134.56,
        "string1": "100",
        "string2": "42"
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": 1303.6799999999998,
        "string_output1": "10042",
        "float_output2": 1303.6799999999998,
        "string_output2": "10042"
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": 1303.6799999999998,
        "string_output1": "10042",
        "float_output2": 1303.6799999999998,
        "string_output2": "10042"
      }
      """

  Scenario: adding negatives
    When I send a fims pub to /components containing
      """
      {
        "float1" :0,
        "float2" :-345,
        "float3" :34,
        "string1":"200",
        "string2":"23"
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_output1": -311,
        "string_output1": "20023",
        "float_output2": -311,
        "string_output2": "20023"
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_output1": -311,
        "string_output1": "20023",
        "float_output2": -311,
        "string_output2": "20023"
      }
      """