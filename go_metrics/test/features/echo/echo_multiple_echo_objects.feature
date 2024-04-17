@echo @echo_multiple_echo_objects
Feature: echo_multiple_echo_objects

  Scenario: echo with multiple echo objects
    When I send a fims pub to /some/input/uri containing
      """
      {"accontactor":-21,"p":-15,"plim":55,"pramprise":-57}
      """
    And I send a fims pub to /another/input/uri containing
      """
      {"accontactor":-87,"p":9,"plim":16,"pramprise":-70}
      """
    And I send a fims pub to /new/some/input/uri containing
      """
      {"accontactor":53,"p":31,"plim":-83,"pramprise":-22}
      """
    And I send a fims pub to /new/another/input/uri containing
      """
      {"accontactor":-32,"p":-24,"plim":-61,"pramprise":-84}
      """
    Then I expect a fims pub to /some/output/uri containing
      """
      {"a":-21,"b":-15,"c":55,"d":-57,"e":-15,"f":-87,"g":9,"h":16,"i":-70,"j":-87,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And I expect a fims pub to /another/output/uri containing
      """
      {"a":-21,"b":-15,"c":55,"d":-57,"e":-15,"f":-87,"g":9,"h":16,"i":-70,"j":-87,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And I expect a fims pub to /yet_another/output/uri containing
      """
      {"a":53,"b":31,"c":-83,"d":-22,"e":31,"f":-32,"g":-24,"h":-61,"i":-84,"j":-32,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":-21,"b":-15,"c":55,"d":-57,"e":-15,"f":-87,"g":9,"h":16,"i":-70,"j":-87,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And a fims get to /another/output/uri should yield
      """
      {"a":-21,"b":-15,"c":55,"d":-57,"e":-15,"f":-87,"g":9,"h":16,"i":-70,"j":-87,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And a fims get to /yet_another/output/uri should yield
      """
      {"a":53,"b":31,"c":-83,"d":-22,"e":31,"f":-32,"g":-24,"h":-61,"i":-84,"j":-32,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """

  Scenario: echo with multiple echo objects, take 2
    When I send a fims pub to /some/input/uri containing
      """
      {"accontactor":-45,"p":44,"plim":63,"pramprise":-35}
      """
    And I send a fims pub to /another/input/uri containing
      """
      {"accontactor":20,"p":-51,"plim":-63,"pramprise":-83}
      """
    And I send a fims pub to /new/some/input/uri containing
      """
      {"accontactor":17,"p":-21,"plim":-99,"pramprise":90}
      """
    And I send a fims pub to /new/another/input/uri containing
      """
      {"accontactor":40,"p":-16,"plim":84,"pramprise":-23}
      """
    Then I expect a fims pub to /some/output/uri containing
      """
      {"a":-45,"b":44,"c":63,"d":-35,"e":44,"f":20,"g":-51,"h":-63,"i":-83,"j":20,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And I expect a fims pub to /another/output/uri containing
      """
      {"a":-45,"b":44,"c":63,"d":-35,"e":44,"f":20,"g":-51,"h":-63,"i":-83,"j":20,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And I expect a fims pub to /yet_another/output/uri containing
      """
      {"a":17,"b":-21,"c":-99,"d":90,"e":-21,"f":40,"g":-16,"h":84,"i":-23,"j":40,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":-45,"b":44,"c":63,"d":-35,"e":44,"f":20,"g":-51,"h":-63,"i":-83,"j":20,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And a fims get to /another/output/uri should yield
      """
      {"a":-45,"b":44,"c":63,"d":-35,"e":44,"f":20,"g":-51,"h":-63,"i":-83,"j":20,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And a fims get to /yet_another/output/uri should yield
      """
      {"a":17,"b":-21,"c":-99,"d":90,"e":-21,"f":40,"g":-16,"h":84,"i":-23,"j":40,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """

  Scenario: echo with multiple echo objects, clothed messages
    When I send a fims pub to /some/input/uri containing
      """
      {"accontactor":{"value":15},"p":{"value":-92},"plim":{"value":-93},"pramprise":{"value":-71}}
      """
    And I send a fims pub to /another/input/uri containing
      """
      {"accontactor":{"value":-73},"p":{"value":69},"plim":{"value":50},"pramprise":{"value":-59}}
      """
    And I send a fims pub to /new/some/input/uri containing
      """
      {"accontactor":{"value":11},"p":{"value":-8},"plim":{"value":87},"pramprise":{"value":-45}}
      """
    And I send a fims pub to /new/another/input/uri containing
      """
      {"accontactor":{"value":-34},"p":{"value":-95},"plim":{"value":56},"pramprise":{"value":-37}}
      """
    Then I expect a fims pub to /some/output/uri containing
      """
      {"a":15,"b":-92,"c":-93,"d":-71,"e":-92,"f":-73,"g":69,"h":50,"i":-59,"j":-73,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And I expect a fims pub to /another/output/uri containing
      """
      {"a":15,"b":-92,"c":-93,"d":-71,"e":-92,"f":-73,"g":69,"h":50,"i":-59,"j":-73,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And I expect a fims pub to /yet_another/output/uri containing
      """
      {"a":11,"b":-8,"c":87,"d":-45,"e":-8,"f":-34,"g":-95,"h":56,"i":-37,"j":-34,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":15,"b":-92,"c":-93,"d":-71,"e":-92,"f":-73,"g":69,"h":50,"i":-59,"j":-73,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And a fims get to /another/output/uri should yield
      """
      {"a":15,"b":-92,"c":-93,"d":-71,"e":-92,"f":-73,"g":69,"h":50,"i":-59,"j":-73,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And a fims get to /yet_another/output/uri should yield
      """
      {"a":11,"b":-8,"c":87,"d":-45,"e":-8,"f":-34,"g":-95,"h":56,"i":-37,"j":-34,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """

   Scenario: set forwarding with multiple echo objects
    Given I am listening for a fims set on /some/input/uri
    When I send a fims set to /some/output/uri containing
      """
      {"a":1,"b":1,"c":1,"d":1,"e":1,"k":{"value":84},"l":{"value":-35},"m":{"value":58},"n":{"value":-12},"o":{"value":-12},"p":{"value":-12}}
      """
    And I send a fims set to /another/output/uri containing
      """
      {"k":1,"l":1,"m":1,"n":1,"o":1,"p":1}
      """
    And I send a fims set to /yet_another/output/uri/k containing
      """
      22
      """
    And I send a fims set to /yet_another/output/uri/l containing
      """
      {"value":11}
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
      {"a":15,"b":-92,"c":-93,"d":-71,"e":-92,"f":-73,"g":69,"h":50,"i":-59,"j":-73,"k":84,"l":-35,"m":58,"n":-12,"o":-12,"p":-12}
      """
    And I expect a fims pub to /another/output/uri containing
      """
      {"a":15,"b":-92,"c":-93,"d":-71,"e":-92,"f":-73,"g":69,"h":50,"i":-59,"j":-73,"k":1,"l":1,"m":1,"n":1,"o":1,"p":1}
      """
    And I expect a fims pub to /yet_another/output/uri containing
      """
      {"a":11,"b":-8,"c":87,"d":-45,"e":-8,"f":-34,"g":-95,"h":56,"i":-37,"j":-34,"k":22,"l":11,"m":0,"n":0,"o":0,"p":0}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":15,"b":-92,"c":-93,"d":-71,"e":-92,"f":-73,"g":69,"h":50,"i":-59,"j":-73,"k":84,"l":-35,"m":58,"n":-12,"o":-12,"p":-12}
      """
    And a fims get to /another/output/uri should yield
      """
      {"a":15,"b":-92,"c":-93,"d":-71,"e":-92,"f":-73,"g":69,"h":50,"i":-59,"j":-73,"k":1,"l":1,"m":1,"n":1,"o":1,"p":1}
      """
    And a fims get to /yet_another/output/uri should yield
      """
      {"a":11,"b":-8,"c":87,"d":-45,"e":-8,"f":-34,"g":-95,"h":56,"i":-37,"j":-34,"k":22,"l":11,"m":0,"n":0,"o":0,"p":0}
      """