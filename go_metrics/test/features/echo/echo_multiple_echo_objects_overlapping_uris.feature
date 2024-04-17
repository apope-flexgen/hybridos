@echo @echo_multiple_echo_objects_overlapping_uris
Feature: echo_multiple_echo_objects_overlapping_uris

  Scenario: multiple echo objects with overlapping uris
    When I send a fims pub to /some/input/uri containing
      """
      {"accontactor":51,"p":47,"plim":93,"pramprise":61}
      """
    And I send a fims pub to /another/input/uri containing
      """
      {"accontactor":30,"p":52,"plim":81,"pramprise":31,"sub_uri":{"accontactor":53,"p":31,"plim":69,"pramprise":-70}}
      """
    And I send a fims pub to /new/some/input/uri containing
      """
      {"accontactor":18,"p":-82,"plim":-34,"pramprise":-17}
      """
    And I send a fims pub to /new/another/input/uri containing
      """
      {"accontactor":-41,"p":0,"plim":-81,"pramprise":-29}
      """
    Then I expect a fims pub to /another/output/uri containing
      """
      {"a":51,"b":47,"c":93,"d":61,"e":47,"f":30,"g":52,"h":81,"i":31,"j":30,"k":53,"l":31,"m":69,"n":-70,"o":53,"p":0,"q":0,"r":0,"s":0,"t":0,"u":0}
      """
    And I expect a fims pub to /some/output/uri/sub containing
      """
      {"a":18,"b":-82,"c":-34,"d":-17,"e":-82,"f":-41,"g":0,"h":-81,"i":-29,"j":-41,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":51,"b":47,"c":93,"d":61,"e":47,"f":30,"g":52,"h":81,"i":31,"j":30,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And a fims get to /another/output/uri should yield
      """
      {"a":51,"b":47,"c":93,"d":61,"e":47,"f":30,"g":52,"h":81,"i":31,"j":30,"k":53,"l":31,"m":69,"n":-70,"o":53,"p":0,"q":0,"r":0,"s":0,"t":0,"u":0}
      """
    And a fims get to /some/output/uri/sub should yield
      """
      {"a":18,"b":-82,"c":-34,"d":-17,"e":-82,"f":-41,"g":0,"h":-81,"i":-29,"j":-41,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """

  Scenario: multiple echo objects with overlapping uris with single naked messages
    When I send a fims pub to /some/input/uri/accontactor containing:
    """
    63
    """
    And I send a fims pub to /some/input/uri/p containing:
    """
    35
    """
    And I send a fims pub to /some/input/uri/plim containing:
    """
    29
    """
    And I send a fims pub to /some/input/uri/pramprise containing:
    """
    5
    """
    And I send a fims pub to /another/input/uri/accontactor containing:
    """
    87
    """
    And I send a fims pub to /another/input/uri/p containing:
    """
    71
    """
    And I send a fims pub to /another/input/uri/plim containing:
    """
    73
    """
    And I send a fims pub to /another/input/uri/pramprise containing:
    """
    97
    """
    And I send a fims pub to /another/input/uri/sub_uri/accontactor containing:
    """
    5
    """
    And I send a fims pub to /another/input/uri/sub_uri/p containing:
    """
    43
    """
    And I send a fims pub to /another/input/uri/sub_uri/plim containing:
    """
    69
    """
    And I send a fims pub to /another/input/uri/sub_uri/pramprise containing:
    """
    53
    """
    And I send a fims pub to /new/some/input/uri/accontactor containing:
    """
    54
    """
    And I send a fims pub to /new/some/input/uri/p containing:
    """
    52
    """
    And I send a fims pub to /new/some/input/uri/plim containing:
    """
    18
    """
    And I send a fims pub to /new/some/input/uri/pramprise containing:
    """
    10
    """
    And I send a fims pub to /new/another/input/uri/accontactor containing:
    """
    40
    """
    And I send a fims pub to /new/another/input/uri/p containing:
    """
    3
    """
    And I send a fims pub to /new/another/input/uri/plim containing:
    """
    87
    """
    And I send a fims pub to /new/another/input/uri/pramprise containing:
    """
    11
    """
    And I send a fims set to /some/output/uri/k containing:
    """
    1
    """
    And I send a fims set to /some/output/uri/l containing:
    """
    1
    """
    And I send a fims set to /some/output/uri/m containing:
    """
    1
    """
    And I send a fims set to /some/output/uri/n containing:
    """
    1
    """
    And I send a fims set to /some/output/uri/o containing:
    """
    1
    """
    And I send a fims set to /some/output/uri/p containing:
    """
    1
    """
    And I send a fims set to /another/output/uri/p containing:
    """
    1
    """
    And I send a fims set to /another/output/uri/q containing:
    """
    1
    """
    And I send a fims set to /another/output/uri/r containing:
    """
    1
    """
    And I send a fims set to /another/output/uri/s containing:
    """
    1
    """
    And I send a fims set to /another/output/uri/t containing:
    """
    1
    """
    And I send a fims set to /another/output/uri/u containing:
    """
    1
    """
    And I send a fims set to /some/output/uri/sub/k containing:
    """
    1
    """
    And I send a fims set to /some/output/uri/sub/l containing:
    """
    1
    """
    And I send a fims set to /some/output/uri/sub/m containing:
    """
    1
    """
    And I send a fims set to /some/output/uri/sub/n containing:
    """
    1
    """
    And I send a fims set to /some/output/uri/sub/o containing:
    """
    1
    """
    And I send a fims set to /some/output/uri/sub/p containing:
    """
    1
    """
    Then I expect a fims pub to /another/output/uri containing
      """
      {"a":63,"b":35,"c":29,"d":5,"e":35,"f":87,"g":71,"h":73,"i":97,"j":87,"k":5,"l":43,"m":69,"n":53,"o":5,"p":1,"q":1,"r":1,"s":1,"t":1,"u":1}
      """
    And I expect a fims pub to /some/output/uri/sub containing
      """
      {"a":54,"b":52,"c":18,"d":10,"e":52,"f":40,"g":3,"h":87,"i":11,"j":40,"k":1,"l":1,"m":1,"n":1,"o":1,"p":1}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":63,"b":35,"c":29,"d":5,"e":35,"f":87,"g":71,"h":73,"i":97,"j":87,"k":1,"l":1,"m":1,"n":1,"o":1,"p":1}
      """
    And a fims get to /another/output/uri should yield
      """
      {"a":63,"b":35,"c":29,"d":5,"e":35,"f":87,"g":71,"h":73,"i":97,"j":87,"k":5,"l":43,"m":69,"n":53,"o":5,"p":1,"q":1,"r":1,"s":1,"t":1,"u":1}
      """
    And a fims get to /some/output/uri/sub should yield
      """
      {"a":54,"b":52,"c":18,"d":10,"e":52,"f":40,"g":3,"h":87,"i":11,"j":40,"k":1,"l":1,"m":1,"n":1,"o":1,"p":1}
      """

  Scenario: multiple echo objects with overlapping uris with single clothed messages
    When I send a fims pub to /some/input/uri/accontactor containing:
    """
    {"value":51}
    """
    And I send a fims pub to /some/input/uri/p containing:
    """
    {"value":85}
    """
    And I send a fims pub to /some/input/uri/plim containing:
    """
    {"value":49}
    """
    And I send a fims pub to /some/input/uri/pramprise containing:
    """
    {"value":34}
    """
    And I send a fims pub to /another/input/uri/accontactor containing:
    """
    {"value":36}
    """
    And I send a fims pub to /another/input/uri/p containing:
    """
    {"value":98}
    """
    And I send a fims pub to /another/input/uri/plim containing:
    """
    {"value":83}
    """
    And I send a fims pub to /another/input/uri/pramprise containing:
    """
    {"value":28}
    """
    And I send a fims pub to /another/input/uri/sub_uri/accontactor containing:
    """
    {"value":56}
    """
    And I send a fims pub to /another/input/uri/sub_uri/p containing:
    """
    {"value":40}
    """
    And I send a fims pub to /another/input/uri/sub_uri/plim containing:
    """
    {"value":34}
    """
    And I send a fims pub to /another/input/uri/sub_uri/pramprise containing:
    """
    {"value":88}
    """
    And I send a fims pub to /new/some/input/uri/accontactor containing:
    """
    {"value":52}
    """
    And I send a fims pub to /new/some/input/uri/p containing:
    """
    {"value":15}
    """
    And I send a fims pub to /new/some/input/uri/plim containing:
    """
    {"value":45}
    """
    And I send a fims pub to /new/some/input/uri/pramprise containing:
    """
    {"value":12}
    """
    And I send a fims pub to /new/another/input/uri/accontactor containing:
    """
    {"value":81}
    """
    And I send a fims pub to /new/another/input/uri/p containing:
    """
    {"value":64}
    """
    And I send a fims pub to /new/another/input/uri/plim containing:
    """
    {"value":12}
    """
    And I send a fims pub to /new/another/input/uri/pramprise containing:
    """
    {"value":63}
    """
    And I send a fims set to /some/output/uri/k containing:
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/l containing:
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/m containing:
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/n containing:
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/o containing:
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/p containing:
    """
    {"value":5}
    """
    And I send a fims set to /another/output/uri/p containing:
    """
    {"value":5}
    """
    And I send a fims set to /another/output/uri/q containing:
    """
    {"value":5}
    """
    And I send a fims set to /another/output/uri/r containing:
    """
    {"value":5}
    """
    And I send a fims set to /another/output/uri/s containing:
    """
    {"value":5}
    """
    And I send a fims set to /another/output/uri/t containing:
    """
    {"value":5}
    """
    And I send a fims set to /another/output/uri/u containing:
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/sub/k containing:
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/sub/l containing:
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/sub/m containing:
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/sub/n containing:
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/sub/o containing:
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/sub/p containing:
    """
    {"value":5}
    """
    Then I expect a fims pub to /another/output/uri containing
      """
      {"a":51,"b":85,"c":49,"d":34,"e":85,"f":36,"g":98,"h":83,"i":28,"j":36,"k":56,"l":40,"m":34,"n":88,"o":56,"p":5,"q":5,"r":5,"s":5,"t":5,"u":5}
      """
    And I expect a fims pub to /some/output/uri/sub containing
      """
      {"a":52,"b":15,"c":45,"d":12,"e":15,"f":81,"g":64,"h":12,"i":63,"j":81,"k":5,"l":5,"m":5,"n":5,"o":5,"p":5}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":51,"b":85,"c":49,"d":34,"e":85,"f":36,"g":98,"h":83,"i":28,"j":36,"k":5,"l":5,"m":5,"n":5,"o":5,"p":5}
      """
    And a fims get to /another/output/uri should yield
      """
      {"a":51,"b":85,"c":49,"d":34,"e":85,"f":36,"g":98,"h":83,"i":28,"j":36,"k":56,"l":40,"m":34,"n":88,"o":56,"p":5,"q":5,"r":5,"s":5,"t":5,"u":5}
      """
    And a fims get to /some/output/uri/sub should yield
      """
      {"a":52,"b":15,"c":45,"d":12,"e":15,"f":81,"g":64,"h":12,"i":63,"j":81,"k":5,"l":5,"m":5,"n":5,"o":5,"p":5}
      """