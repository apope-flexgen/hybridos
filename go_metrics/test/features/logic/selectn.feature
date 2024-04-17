@logic @selectn
Feature: selectn

  Scenario: selectn, index = 0
    When I send a fims pub to /components/feeder_52m1 containing
      """
      {
        "v1": 42.1,
        "v2": 21,
        "v3": 31,
        "v4": 41,
        "v5": false,
        "selection_index": 0
      }
      """
    Then I expect a fims pub to /some/selected/output containing
      """
      {
        "selection": 0
      }
      """
    And a fims get to /some/selected/output should yield
      """
      {
        "selection": 0
      }
      """

  Scenario: selectn, index = 1
    When I send a fims pub to /components/feeder_52m1 containing
      """
      {
        "v1": 42.1,
        "v2": 21,
        "v3": 31,
        "v4": 41,
        "v5": false,
        "selection_index": 1
      }
      """
    Then I expect a fims pub to /some/selected/output containing
      """
      {
        "selection": 42.1
      }
      """
    And a fims get to /some/selected/output should yield
      """
      {
        "selection": 42.1
      }
      """

  Scenario: selectn, index = 2
    When I send a fims pub to /components/feeder_52m1 containing
      """
      {
        "v1": 42.1,
        "v2": 21,
        "v3": 31,
        "v4": 41,
        "v5": false,
        "selection_index": 2
      }
      """
    Then I expect a fims pub to /some/selected/output containing
      """
      {
        "selection": 21
      }
      """
    And a fims get to /some/selected/output should yield
      """
      {
        "selection": 21
      }
      """

  Scenario: selectn, index = 3
    When I send a fims pub to /components/feeder_52m1 containing
      """
      {
        "v1": 42.1,
        "v2": 21,
        "v3": 31,
        "v4": 41,
        "v5": false,
        "selection_index": 3
      }
      """
    Then I expect a fims pub to /some/selected/output containing
      """
      {
        "selection": 31
      }
      """
    And a fims get to /some/selected/output should yield
      """
      {
        "selection": 31
      }
      """
  
  Scenario: selectn, index = 4
    When I send a fims pub to /components/feeder_52m1 containing
      """
      {
        "v1": 42.1,
        "v2": 21,
        "v3": 31,
        "v4": 41,
        "v5": false,
        "selection_index": 4
      }
      """
    Then I expect a fims pub to /some/selected/output containing
      """
      {
        "selection": 41
      }
      """
    And a fims get to /some/selected/output should yield
      """
      {
        "selection": 41
      }
      """

  Scenario: selectn, index = 5
    When I send a fims pub to /components/feeder_52m1 containing
      """
      {
        "v1": 42.1,
        "v2": 21,
        "v3": 31,
        "v4": 41,
        "v5": true,
        "selection_index": 5
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