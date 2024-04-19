@unit_test @reevaluate_metrics
Feature: reevaluate_metrics

  # Ensure only sparse posts are sent
  Scenario: reevaluate_metrics
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

    # Send reevaluate and expect new post
    When I send a fims set to /some/output/output containing
      """
      {"reevaluate": true}
      """
    Then I expect a fims post to /some/output within 1 seconds containing
      """
      {"output": {"value": true}}
      """
    And I expect no fims posts will be sent to /some/output
    And I expect no fims pubs will be sent to /some/output

  # Ensure only sparse posts are sent
  Scenario: reevaluate_metrics, clothed signal
    Given I am listening for a fims post on /some/output
    When I send a fims pub to /example containing
      """
      {"input": false}
      """
    Then I expect a fims post to /some/output within 1 seconds containing
      """
      {"output": {"value": false}}
      """
    # One was received so no more expected until the value changes
    And I expect no fims posts will be sent to /some/output
    And I expect no fims pubs will be sent to /some/output

    # Send reevaluate and expect new post
    When I send a fims set to /some/output/output containing
      """
      {"reevaluate": {"value": true}}
      """
    Then I expect a fims post to /some/output within 1 seconds containing
      """
      {"output": {"value": false}}
      """
    And I expect no fims posts will be sent to /some/output
    And I expect no fims pubs will be sent to /some/output
