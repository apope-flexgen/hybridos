@unit_test @internal_outputs
Feature: internal_outputs

  Scenario: internal_outputs
    When I send a fims pub to /some/uri/external_input containing
      """
      5
      """
    Then I expect a fims pub to /test/output containing
      """
      {
        "first_external_output":5,
        "second_external_output":5
      }
      """
    And a fims get to /test/output should yield
      """
      {
        "first_external_output":5,
        "second_external_output":5
      }
      """

  Scenario: internal_outputs, take 2
    When I send a fims pub to /some/uri containing
      """
      {"external_input":23}
      """
    Then I expect a fims pub to /test/output containing
      """
      {
        "first_external_output":23,
        "second_external_output":23
      }
      """
    And a fims get to /test/output should yield
      """
      {
        "first_external_output":23,
        "second_external_output":23
      }
      """