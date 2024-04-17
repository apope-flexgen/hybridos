@unit_test @specify_fims_method
Feature: specify_fims_method

  Scenario: specify_fims_method, set
    When I send a fims set to /some/input/a containing
      """
      5
      """
    Then I expect a fims pub to /some/one containing
      """
      {"a":5,"b":0,"c":5,"d":5,"e":15}
      """
    And a fims get to /some/one should yield
      """
      {"a":5,"b":0,"c":5,"d":5,"e":15}
      """

  Scenario: specify_fims_method, pub
    When I send a fims pub to /some/input/a containing
      """
      3
      """
    Then I expect a fims pub to /some/one containing
      """
      {"a":5,"b":3,"c":3,"d":3,"e":14}
      """
    And a fims get to /some/one should yield
      """
      {"a":5,"b":3,"c":3,"d":3,"e":14}
      """