@logic @srff
Feature: srff

  Scenario: srff, both false
    When I send a fims pub to /components/feeder_52m1 containing
      """
      {
        "v1": false,
        "v2": false
      }
      """
    Then I expect a fims pub to /some/selected/output containing
      """
      {
        "selection": false
      }
      """
    And a fims get to /some/selected/output should yield
      """
      {
        "selection": false
      }
      """

  Scenario: srff, set input1 to true
    When I send a fims pub to /components/feeder_52m1 containing
      """
      {
        "v1": true,
        "v2": false
      }
      """
    Then I expect a fims pub to /some/selected/output containing
      """
      {
        "selection": true
      }
      """
    And a fims get to /some/selected/output should yield
      """
      {
        "selection": true
      }
      """

  Scenario: srff, set input1 to false after being true
    When I send a fims pub to /components/feeder_52m1 containing
      """
      {
        "v1": false,
        "v2": false
      }
      """
    Then I expect a fims pub to /some/selected/output containing
      """
      {
        "selection": true
      }
      """
    And a fims get to /some/selected/output should yield
      """
      {
        "selection": true
      }
      """

  Scenario: srff, set input2 to true after srff being true
    When I send a fims pub to /components/feeder_52m1 containing
      """
      {
        "v1": false,
        "v2": true
      }
      """
    Then I expect a fims pub to /some/selected/output containing
      """
      {
        "selection": false
      }
      """
    And a fims get to /some/selected/output should yield
      """
      {
        "selection": false
      }
      """