@echo @echo_inputs
Feature: echo_inputs

  Scenario: echo_inputs
    When I send a fims pub to /some/input/uri containing
      """
      {"accontactor":-16,"p":-73,"plim":-13,"pramprise":-21}
      """
    And I send a fims pub to /another/input/uri containing
      """
      {"accontactor":-45,"p":-97,"plim":-57,"pramprise":94}
      """
    Then I expect a fims pub to /some/output/uri containing
      """
      {"a":-16,"b":-73,"c":-13,"d":-21,"e":-73,"f":-45,"g":-97,"h":-57,"i":94,"j":-45}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":-16,"b":-73,"c":-13,"d":-21,"e":-73,"f":-45,"g":-97,"h":-57,"i":94,"j":-45}
      """

  Scenario: echo_inputs, take 2
    When I send a fims pub to /some/input/uri containing
      """
      {
        "accontactor": 123,
        "p": 456,
        "plim": 789,
        "pramprise": 121
      }
      """
    And I send a fims pub to /another/input/uri containing
      """
      {
        "accontactor": 111,
        "p": 222,
        "plim": 333,
        "pramprise": 444
      }
      """
    Then I expect a fims pub to /some/output/uri containing
      """
      {"a":123,"b":456,"c":789,"d":121,"e":456,"f":111,"g":222,"h":333,"i":444,"j":111}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":123,"b":456,"c":789,"d":121,"e":456,"f":111,"g":222,"h":333,"i":444,"j":111}
      """

  Scenario: echo_inputs with clothed messages
    When I send a fims pub to /some/input/uri containing
      """
      {"accontactor":{"value":84},"p":{"value":-35},"plim":{"value":58},"pramprise":{"value":-12}}
      """
    And I send a fims pub to /another/input/uri containing
      """
      {"accontactor":{"value":35},"p":{"value":-7},"plim":{"value":78},"pramprise":{"value":-26}}
      """
    Then I expect a fims pub to /some/output/uri containing
      """
      {"a":84,"b":-35,"c":58,"d":-12,"e":-35,"f":35,"g":-7,"h":78,"i":-26,"j":35}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":84,"b":-35,"c":58,"d":-12,"e":-35,"f":35,"g":-7,"h":78,"i":-26,"j":35}
      """

  Scenario: echo_inputs set forwarding
    Given I am listening for a fims set on /some/input/uri
    And I am listening for a fims set on /another/input/uri
    When I send a fims set to /some/output/uri containing
      """
      {"a":84,"b":-35,"c":58,"d":-12,"e":-35,"f":35,"g":-7,"h":78,"i":-26,"j":35}
      """
    Then I expect a fims set to /some/input/uri/accontactor within 1 seconds containing
      """
      84
      """
    And I expect a fims set to /some/input/uri/p within 1 seconds containing
      """
      -35
      """
    And I expect a fims set to /some/input/uri/plim within 1 seconds containing
      """
      58
      """
    And I expect a fims set to /some/input/uri/pramprise within 1 seconds containing
      """
      -12
      """
    And I expect a fims set to /another/input/uri/accontactor within 1 seconds containing
      """
      35
      """
    And I expect a fims set to /another/input/uri/p within 1 seconds containing
      """
      -7
      """
    And I expect a fims set to /another/input/uri/plim within 1 seconds containing
      """
      78
      """
    And I expect a fims set to /another/input/uri/pramprise within 1 seconds containing
      """
      -26
      """
    And I expect a fims pub to /some/output/uri containing
      """
      {"a":84,"b":-35,"c":58,"d":-12,"e":-35,"f":35,"g":-7,"h":78,"i":-26,"j":35}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":84,"b":-35,"c":58,"d":-12,"e":-35,"f":35,"g":-7,"h":78,"i":-26,"j":35}
      """