@type_conversion @bool
Feature: bool

  Scenario: bool conversions - true
    When I send a fims pub to /components containing
      """
      {
        "float1": 5.3,
        "int1": 5,
        "uint1": 6,
        "string1": "hi",
        "bool1": true
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_to_bool": true,
        "int_to_bool": true,
        "uint_to_bool": true,
        "string_to_bool": true,
        "bool_to_bool": true
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_to_bool": true,
        "int_to_bool": true,
        "uint_to_bool": true,
        "string_to_bool": true,
        "bool_to_bool": true
      }
      """

  Scenario: bool conversions - false
    When I send a fims pub to /components containing
      """
      {
        "float1": 0,
        "int1": 0,
        "uint1": 0,
        "string1": "false",
        "bool1": false
      }
      """
    Then I expect a fims pub to /some/output containing
      """
      {
        "float_to_bool": false,
        "int_to_bool": false,
        "uint_to_bool": false,
        "string_to_bool": false,
        "bool_to_bool": false
      }
      """
    And a fims get to /some/output should yield
      """
      {
        "float_to_bool": false,
        "int_to_bool": false,
        "uint_to_bool": false,
        "string_to_bool": false,
        "bool_to_bool": false
      }
      """