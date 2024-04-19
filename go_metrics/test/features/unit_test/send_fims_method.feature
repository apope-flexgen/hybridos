@unit_test @send_fims_method
Feature: send_fims_method

  # Ensure only sparse posts are sent
  Scenario: send_fims_method, post
    Given I am listening for a fims post on /some/output
    When I send a fims pub to /example containing
      """
      {"input": true}
      """
    Then I expect a fims post to /some/output within 1 seconds containing
      """
      {"output": {"value": true}}
      """
    # One was received so no more expected until the value changes
    And I expect no fims posts will be sent to /some/output
    And I expect no fims pubs will be sent to /some/output

    # New value, ensure same behavior
    When I send a fims pub to /example containing
      """
      {"input": false}
      """
    Then I expect a fims post to /some/output within 1 seconds containing
      """
      {"output": {"value": false}}
      """
    And I expect no fims posts will be sent to /some/output
    And I expect no fims pubs will be sent to /some/output
