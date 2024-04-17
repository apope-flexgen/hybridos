@echo @echo_inputs_and_nested_inputs_and_echo
Feature: echo_inputs_and_nested_inputs_and_echo

  Scenario: echo inputs, nested inputs, and echo
    When I send a fims pub to /some/input/uri containing
      """
      {"accontactor":-27,"p":89,"plim":53,"pramprise":56}
      """
    And I send a fims pub to /another/input/uri containing
      """
      {"accontactor":71,"p":-61,"plim":-82,"pramprise":-17,"sub_uri":{"value":5,"alpha":-55,"beta":-56,"gamma":85,"delta":-68,"sub_sub":{"alpha":-69,"beta":53,"gamma":30,"delta":-25}}}
      """
    Then I expect a fims pub to /some/output/uri containing
      """
      {"a":-27,"b":89,"c":53,"d":56,"e":89,"f":71,"g":-61,"h":-82,"i":-17,"j":71,"k":-55,"l":-56,"m":85,"n":-68,"o":-56,"p":-69,"q":53,"r":30,"s":-25,"t":53,"u":0,"v":0,"w":0,"x":0,"y":0,"z":0,"zz":5}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":-27,"b":89,"c":53,"d":56,"e":89,"f":71,"g":-61,"h":-82,"i":-17,"j":71,"k":-55,"l":-56,"m":85,"n":-68,"o":-56,"p":-69,"q":53,"r":30,"s":-25,"t":53,"u":0,"v":0,"w":0,"x":0,"y":0,"z":0,"zz":5}
      """

  Scenario: echo inputs, nested inputs, and echo with clothed messages
    When I send a fims pub to /some/input/uri containing
      """
      {"accontactor":{"value":28},"p":{"value":-36},"plim":{"value":80},"pramprise":{"value":-63}}
      """
    And I send a fims pub to /another/input/uri containing
      """
      {"accontactor":{"value":68},"p":{"value":85},"plim":{"value":-24},"pramprise":{"value":38},"sub_uri":{"value":3,"alpha":{"value":-15},"beta":{"value":-47},"gamma":{"value":-37},"delta":{"value":47},"sub_sub":{"alpha":{"value":34},"beta":{"value":-30},"gamma":{"value":32},"delta":{"value":-66}}}}
      """
    Then I expect a fims pub to /some/output/uri containing
      """
      {"a":28,"b":-36,"c":80,"d":-63,"e":-36,"f":68,"g":85,"h":-24,"i":38,"j":68,"k":-15,"l":-47,"m":-37,"n":47,"o":-47,"p":34,"q":-30,"r":32,"s":-66,"t":-30,"u":0,"v":0,"w":0,"x":0,"y":0,"z":0,"zz":3}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":28,"b":-36,"c":80,"d":-63,"e":-36,"f":68,"g":85,"h":-24,"i":38,"j":68,"k":-15,"l":-47,"m":-37,"n":47,"o":-47,"p":34,"q":-30,"r":32,"s":-66,"t":-30,"u":0,"v":0,"w":0,"x":0,"y":0,"z":0,"zz":3}
      """

  Scenario: echo inputs, nested inputs, and echo set forwarding
    Given I am listening for a fims set on /some/input/uri
    When I send a fims set to /some/output/uri containing
      """
      {"a":1,"b":1,"c":1,"d":1,"e":1,"u":5,"v":5,"w":5,"x":5,"y":5,"z":5}
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
      {"a":28,"b":-36,"c":80,"d":-63,"e":-36,"f":68,"g":85,"h":-24,"i":38,"j":68,"k":-15,"l":-47,"m":-37,"n":47,"o":-47,"p":34,"q":-30,"r":32,"s":-66,"t":-30,"u":5,"v":5,"w":5,"x":5,"y":5,"z":5,"zz":3}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":28,"b":-36,"c":80,"d":-63,"e":-36,"f":68,"g":85,"h":-24,"i":38,"j":68,"k":-15,"l":-47,"m":-37,"n":47,"o":-47,"p":34,"q":-30,"r":32,"s":-66,"t":-30,"u":5,"v":5,"w":5,"x":5,"y":5,"z":5,"zz":3}
      """