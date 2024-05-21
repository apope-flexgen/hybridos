@alerts @dynamic_attributes
Feature: dynamic_attributes

  Scenario: metric with alert = true dynamically adds attributes 
    Given I am listening for a fims post on /some/output
    When I send a fims pub to /example containing
      """
      {"input": true}
      """
    Then I expect a fims post to /some/output/output1 within 3 seconds containing
      """
      {"details":[{"timestamp":"any"}],"name":"output1","severity":2,"source":"Alerts","status":"active","value":true}
      """
    When I send a fims pub to /example containing
      """
      {"input": false}
      """
    Then I expect a fims post to /some/output/output1 within 3 seconds containing
      """
      {"details":[],"name":"output1","severity":2,"source":"Alerts","status":"inactive","value":false}
      """
  Scenario: metric without alert = true only has static attributes 
    Given I am listening for a fims post on /some/output
    When I send a fims pub to /example containing
      """
      {"input": true}
      """
    Then I expect a fims post to /some/output/output2 within 3 seconds containing
      """
      {"output2":{"something":"else","value":true}}
      """
    When I send a fims pub to /example containing
      """
      {"input": false}
      """
    Then I expect a fims post to /some/output/output2 within 3 seconds containing
      """
      {"output2":{"something":"else","value":false}}
      """
    