@type_conversion @float
Feature: float

  Scenario: float conversions
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
        "float_to_float": 5.3,
        "int_to_float": 5,
        "uint_to_float": 6,
        "string_to_float": 5,
        "bool_to_float": 1
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_to_float": 5.3,
        "int_to_float": 5,
        "uint_to_float": 6,
        "string_to_float": 5,
        "bool_to_float": 1
      }
      """

  Scenario: float conversions, take 2
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
        "float_to_float": -1.3,
        "int_to_float": -1,
        "uint_to_float": 1,
        "string_to_float": 5,
        "bool_to_float": 0
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_to_float": -1.3,
        "int_to_float": -1,
        "uint_to_float": 1,
        "string_to_float": 5,
        "bool_to_float": 0
      }
      """