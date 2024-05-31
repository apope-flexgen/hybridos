@alerts @subexpression_tracking
Feature: subexpression_tracking

  Scenario: OR'd alert tracks subexpressions
    Given I am listening for a fims post on /some/output
    When I send a fims pub to /example containing
      """
      {"temp": 100}
      """
    Then I expect a fims post to /some/output/output1 within 3 seconds containing
      """
      {"details":[{"message": "Temperature: 100.00 exceeded 90C", "timestamp":"any"}],"name":"output1","severity":2,"source":"Alerts","status":"active","value":true}
      """
    When I clear the fims message list
    And I send a fims pub to /example containing
      """
      {"high_temp_fault": true}
      """
    Then I expect a fims post to /some/output/output1 within 3 seconds containing
      """
      {"details":[{"message": "There was a high temperature fault", "timestamp":"any"}],"name":"output1","severity":2,"source":"Alerts","status":"active","value":true}
      """
    When I clear the fims message list
    And I send a fims set to /some/output/output1 containing
      """
      {"reevaluate": true}
      """
    # Reevaluating causes both conditions to be triggered separately as new incidents
    Then I expect a fims post to /some/output/output1 within 3 seconds containing
      """
      {"details":[{"message": "Temperature: 100.00 exceeded 90C", "timestamp":"any"}, {"message": "There was a high temperature fault", "timestamp":"any"}],"name":"output1","severity":2,"source":"Alerts","status":"active","value":true}
      """
    When I clear the fims message list
    And I send a fims pub to /example containing
      """
      {"temp": 80}
      """
    # The value of the metric is still true due to the high temp fault, therefore no updates are sent and the status remains active
    Then I expect no fims posts will be sent to /some/output
    # Now both expressions are false so the metric posts its changes and the status is inactive
    When I send a fims pub to /example containing
      """
      {"high_temp_fault": false}
      """
    Then I expect a fims post to /some/output/output1 within 3 seconds containing
      """
      {"details":[],"name":"output1","severity":2,"source":"Alerts","status":"inactive","value":false}
      """
    