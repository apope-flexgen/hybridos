@type_conversion @string
Feature: string

  Scenario: string conversions
    When I send a fims pub to /components containing
      """
      {
        "float1": 5.3,
        "int1": 5,
        "uint1": 6,
        "string1": "5",
        "bool1": true
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_to_string": "5.300000",
        "int_to_string": "5",
        "uint_to_string": "6",
        "string_to_string": "5",
        "bool_to_string": "true"
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_to_string": "5.300000",
        "int_to_string": "5",
        "uint_to_string": "6",
        "string_to_string": "5",
        "bool_to_string": "true"
      }
      """

  Scenario: string conversions, take 2
    When I send a fims pub to /components containing
      """
      {
        "float1": -1.3,
        "int1": -1,
        "uint1": 1,
        "string1": "false",
        "bool1": false
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_to_string": "-1.300000",
        "int_to_string": "-1",
        "uint_to_string": "1",
        "string_to_string": "false",
        "bool_to_string": "false"
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_to_string": "-1.300000",
        "int_to_string": "-1",
        "uint_to_string": "1",
        "string_to_string": "false",
        "bool_to_string": "false"
      }
      """