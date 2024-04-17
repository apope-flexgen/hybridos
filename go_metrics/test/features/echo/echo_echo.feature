@echo @echo_echo
Feature: echo_echo

  Scenario: echo set forwarding
    When I send a fims set to /some/output/uri containing
      """
      {
        "a": 3,
        "b": 5,
        "c": 7,
        "d": 12,
        "e": 5,
        "f":22
      }
      """
    Then I expect a fims pub to /some/output/uri containing
      """
      {"a":3,"b":5,"c":7,"d":12,"e":5,"f":22}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":3,"b":5,"c":7,"d":12,"e":5,"f":22}
      """

  Scenario: no set forwarding for pubs
    When I send a fims pub to /some/output/uri containing
      """
      {
        "a": 1,
        "b": 2,
        "c": 3,
        "d": 4,
        "e": 5,
        "f":6
      }
      """
    Then I expect a fims pub to /some/output/uri containing
      """
      {"a":3,"b":5,"c":7,"d":12,"e":5,"f":22}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":3,"b":5,"c":7,"d":12,"e":5,"f":22}
      """

  Scenario: echo set forwarding, take 2
    When I send a fims set to /some/output/uri containing
      """
      {
        "a": 1,
        "b": 2,
        "c": 3,
        "d": 4,
        "e": 5,
        "f":6
      }
      """
    Then I expect a fims pub to /some/output/uri containing
      """
      {"a":1,"b":2,"c":3,"d":4,"e":5,"f":6}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":1,"b":2,"c":3,"d":4,"e":5,"f":6}
      """

  Scenario: echo set forwarding with clothed message
    When I send a fims set to /some/output/uri containing
      """
      {
        "a": {"value":9},
        "b": {"value":9},
        "c": {"value":9},
        "d": {"value":9},
        "e": {"value":9},
        "f": {"value":9}
      }
      """
    Then I expect a fims pub to /some/output/uri containing
      """
      {"a":9,"b":9,"c":9,"d":9,"e":9,"f":9}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":9,"b":9,"c":9,"d":9,"e":9,"f":9}
      """

  Scenario: echo set forwarding with single value clothed message
    When I send a fims set to /some/output/uri/a containing
      """
      {"value":123}
      """
    Then I expect a fims pub to /some/output/uri containing
      """
      {"a":123,"b":9,"c":9,"d":9,"e":9,"f":9}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":123,"b":9,"c":9,"d":9,"e":9,"f":9}
      """