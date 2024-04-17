@echo @echo_inputs_and_echo
Feature: echo_inputs_and_echo

  Scenario: echo_inputs and echo
    When I send a fims pub to /some/input/uri containing
      """
      {"accontactor":-49,"p":23,"plim":85,"pramprise":-63}
      """
    And I send a fims pub to /another/input/uri containing
      """
      {"accontactor":-48,"p":12,"plim":-93,"pramprise":2}
      """
    Then I expect a fims pub to /some/output/uri containing
      """
      {"a":-49,"b":23,"c":85,"d":-63,"e":23,"f":-48,"g":12,"h":-93,"i":2,"j":-48,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":-49,"b":23,"c":85,"d":-63,"e":23,"f":-48,"g":12,"h":-93,"i":2,"j":-48,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """

  Scenario: echo_inputs and echo with clothed messages
    When I send a fims pub to /some/input/uri containing
      """
      {"accontactor":{"value":-31},"p":{"value":-1},"plim":{"value":78},"pramprise":{"value":48}}
      """
    And I send a fims pub to /another/input/uri containing
      """
      {"accontactor":{"value":21},"p":{"value":10},"plim":{"value":-74},"pramprise":{"value":18}}
      """
    Then I expect a fims pub to /some/output/uri containing
      """
      {"a":-31,"b":-1,"c":78,"d":48,"e":-1,"f":21,"g":10,"h":-74,"i":18,"j":21,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":-31,"b":-1,"c":78,"d":48,"e":-1,"f":21,"g":10,"h":-74,"i":18,"j":21,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """

  Scenario: echo_inputs and echo with clothed messages, take 2
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
      {"a":84,"b":-35,"c":58,"d":-12,"e":-35,"f":35,"g":-7,"h":78,"i":-26,"j":35,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":84,"b":-35,"c":58,"d":-12,"e":-35,"f":35,"g":-7,"h":78,"i":-26,"j":35,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """

  Scenario: echo_inputs and echo set forwarding
    Given I am listening for a fims set on /some/input/uri
    When I send a fims set to /some/output/uri containing
      """
      {"a":1,"b":1,"c":1,"d":1,"e":1,
      "k":{"value":84},"l":{"value":-35},"m":{"value":58},"n":{"value":-12},"o":{"value":-12},"p":{"value":-12}}
      """
    Then I expect a fims set to /some/input/uri/accontactor within 1 seconds containing
      """
      1
      """
    And I expect a fims set to /some/input/uri/p within 1 seconds containing
      """
      1
      """
    And I expect a fims set to /some/input/uri/plim within 1 seconds containing
      """
      1
      """
    And I expect a fims set to /some/input/uri/pramprise within 1 seconds containing
      """
      1
      """
    And I expect a fims pub to /some/output/uri containing
      """
      {"a":84,"b":-35,"c":58,"d":-12,"e":-35,"f":35,"g":-7,"h":78,"i":-26,"j":35,"k":84,"l":-35,"m":58,"n":-12,"o":-12,"p":-12}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":84,"b":-35,"c":58,"d":-12,"e":-35,"f":35,"g":-7,"h":78,"i":-26,"j":35,"k":84,"l":-35,"m":58,"n":-12,"o":-12,"p":-12}
      """