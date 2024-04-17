@type_conversion @int
Feature: int

  Scenario: int conversions
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
        "float_to_int": 5,
        "int_to_int": 5,
        "uint_to_int": 6,
        "string_to_int": 5,
        "bool_to_int": 1
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_to_int": 5,
        "int_to_int": 5,
        "uint_to_int": 6,
        "string_to_int": 5,
        "bool_to_int": 1
      }
      """

  Scenario: int conversions, take 2
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
        "float_to_int": -1,
        "int_to_int": -1,
        "uint_to_int": 1,
        "string_to_int": 5,
        "bool_to_int": 0
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_to_int": -1,
        "int_to_int": -1,
        "uint_to_int": 1,
        "string_to_int": 5,
        "bool_to_int": 0
      }
      """