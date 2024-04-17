@logic @selectorn
Feature: selectorn

  Scenario: selectorn, index = -1
    When I send a fims pub to /components/feeder_52m1 containing
      """
      {
        "v1": false,
        "v2": false,
        "v3": false,
        "v4": false,
        "v5": false
      }
      """
    Then I expect a fims pub to /some/selected/output containing
      """
      {
        "selection": -1
      }
      """
    And a fims get to /some/selected/output should yield
      """
      {
        "selection": -1
      }
      """

  Scenario: selectorn, index = 1
    When I send a fims pub to /components/feeder_52m1 containing
      """
      {
        "v1": true,
        "v2": false,
        "v3": false,
        "v4": false,
        "v5": false
      }
      """
    Then I expect a fims pub to /some/selected/output containing
      """
      {
        "selection": 1
      }
      """
    And a fims get to /some/selected/output should yield
      """
      {
        "selection": 1
      }
      """

  Scenario: selectorn, index = 2
    When I send a fims pub to /components/feeder_52m1 containing
      """
      {
        "v1": false,
        "v2": true,
        "v3": false,
        "v4": false,
        "v5": false
      }
      """
    Then I expect a fims pub to /some/selected/output containing
      """
      {
        "selection": 2
      }
      """
    And a fims get to /some/selected/output should yield
      """
      {
        "selection": 2
      }
      """

  Scenario: selectorn, index = 3
    When I send a fims pub to /components/feeder_52m1 containing
      """
      {
        "v1": false,
        "v2": false,
        "v3": true,
        "v4": false,
        "v5": false
      }
      """
    Then I expect a fims pub to /some/selected/output containing
      """
      {
        "selection": 3
      }
      """
    And a fims get to /some/selected/output should yield
      """
      {
        "selection": 3
      }
      """
  
  Scenario: selectorn, index = 4
    When I send a fims pub to /components/feeder_52m1 containing
      """
      {
        "v1": false,
        "v2": false,
        "v3": false,
        "v4": true,
        "v5": false
      }
      """
    Then I expect a fims pub to /some/selected/output containing
      """
      {
        "selection": 4
      }
      """
    And a fims get to /some/selected/output should yield
      """
      {
        "selection": 4
      }
      """

  Scenario: selectorn, index = 5
    When I send a fims pub to /components/feeder_52m1 containing
      """
      {
        "v1": false,
        "v2": false,
        "v3": false,
        "v4": false,
        "v5": true
      }
      """
    Then I expect a fims pub to /some/selected/output containing
      """
      {
        "selection": 5
      }
      """
    And a fims get to /some/selected/output should yield
      """
      {
        "selection": 5
      }
      """

  Scenario: selectorn, back to index = -1
    When I send a fims pub to /components/feeder_52m1 containing
      """
      {
        "v1": false,
        "v2": false,
        "v3": false,
        "v4": false,
        "v5": false
      }
      """
    Then I expect a fims pub to /some/selected/output containing
      """
      {
        "selection": -1
      }
      """
    And a fims get to /some/selected/output should yield
      """
      {
        "selection": -1
      }
      """