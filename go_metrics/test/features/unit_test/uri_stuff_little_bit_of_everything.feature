@unit_test @uri_stuff_little_bit_of_everything
Feature: uri_stuff_little_bit_of_everything

  Scenario: uri_stuff_little_bit_of_everything
    Then I expect a fims pub to /some/output/uri containing
      """
      {"a":0,"b":0,"c":0,"d":0,"e":0,"f":0,"g":0,"h":0,"i":0,"j":0,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And I expect a fims pub to /another/output/uri containing
      """
      {"a":0,"b":0,"c":0,"d":0,"e":0,"f":0,"g":0,"h":0,"i":0,"j":0,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0,"q":0,"r":0,"s":0,"t":0,"u":0}
      """
    And I expect a fims pub to /some/output/uri/sub containing
      """
      {"a":0,"b":0,"c":0,"d":0,"e":0,"f":0,"g":0,"h":0,"i":0,"j":0,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And I expect a fims pub to /some/one containing
      """
      {"a":0,"b":0,"c":"","d":0,"e":"","f":"","g":0,"h":0,"i":"","j":0,"k":0,"naked_bobcat":0,"naked_cheetah":0}
      """
    And I expect a fims pub to /some/two containing
      """
      {"clothed_bobcat":{"scale":1000,"ui_type":"none","units":"degC","value":0},"clothed_cheetah":{"scale":1000,"ui_type":"none","units":"degC","value":0}}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":0,"b":0,"c":0,"d":0,"e":0,"f":0,"g":0,"h":0,"i":0,"j":0,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And a fims get to /another/output/uri should yield
      """
      {"a":0,"b":0,"c":0,"d":0,"e":0,"f":0,"g":0,"h":0,"i":0,"j":0,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0,"q":0,"r":0,"s":0,"t":0,"u":0}
      """
    And a fims get to /some/output/uri/sub should yield
      """
      {"a":0,"b":0,"c":0,"d":0,"e":0,"f":0,"g":0,"h":0,"i":0,"j":0,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And a fims get to /some/one should yield
      """
      {"a":0,"b":0,"c":"","d":0,"e":"","f":"","g":0,"h":0,"i":"","j":0,"k":0,"naked_bobcat":0,"naked_cheetah":0}
      """
    And a fims get to /some/two should yield
      """
      {"clothed_bobcat":{"scale":1000,"ui_type":"none","units":"degC","value":0},"clothed_cheetah":{"scale":1000,"ui_type":"none","units":"degC","value":0}}
      """

  Scenario: uri_stuff_little_bit_of_everything, take 2
    When I send a fims pub to /some containing
    """
    "6"
    """
    And I send a fims pub to /some/1 containing
    """
    1
    """
    And I send a fims pub to /some/2 containing
    """
    2
    """
    And I send a fims pub to /some/input/uri containing
    """
    {"accontactor":51,"p":47,"plim":93,"pramprise":61,"value":"some_string"}
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
    And I send a fims pub to /some/output/uri containing
    """
    {"a":51,"value":"another_string"}
    """
    Then I expect a fims pub to /some/output/uri containing
      """
      {"a":51,"b":47,"c":93,"d":61,"e":47,"f":30,"g":52,"h":81,"i":31,"j":30,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And I expect a fims pub to /another/output/uri containing
      """
      {"a":51,"b":47,"c":93,"d":61,"e":47,"f":30,"g":52,"h":81,"i":31,"j":30,"k":53,"l":31,"m":69,"n":-70,"o":53,"p":0,"q":0,"r":0,"s":0,"t":0,"u":0}
      """
    And I expect a fims pub to /some/output/uri/sub containing
      """
      {"a":18,"b":-82,"c":-34,"d":-17,"e":-82,"f":-41,"g":0,"h":-81,"i":-29,"j":-41,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And I expect a fims pub to /some/one containing
      """
      {"a":51,"b":47,"c":"some_string","d":51,"e":"another_string","f":"6","g":1,"h":2,"i":"another_string","j":51,"k":51,"naked_bobcat":2,"naked_cheetah":2}
      """
    And I expect a fims pub to /some/two containing
      """
      {"clothed_bobcat":{"scale":1000,"ui_type":"none","units":"degC","value":2},"clothed_cheetah":{"scale":1000,"ui_type":"none","units":"degC","value":2}}
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
    And a fims get to /some/one should yield
      """
      {"a":51,"b":47,"c":"some_string","d":51,"e":"another_string","f":"6","g":1,"h":2,"i":"another_string","j":51,"k":51,"naked_bobcat":2,"naked_cheetah":2}
      """
    And a fims get to /some/two should yield
      """
      {"clothed_bobcat":{"scale":1000,"ui_type":"none","units":"degC","value":2},"clothed_cheetah":{"scale":1000,"ui_type":"none","units":"degC","value":2}}
      """

Scenario: uri_stuff_little_bit_of_everything, take 3
    When I send a fims pub to /some containing
    """
    {
      "1":111,
      "2":222,
      "input": {
                  "uri":{"accontactor":59,"p":16,"plim":4,"pramprise":98,"value":"aaa"}
      },
      "value":"12345",
      "enabled":false,
      "scale":1,
      "output": {
                  "uri": {"a":59,"enabled":false,"scale":1000,"value":"blah"}
      }
    }
    """
    And I send a fims pub to /another/input/uri containing
    """
    {"accontactor":31,"p":98,"plim":85,"pramprise":74,"sub_uri":{"accontactor":47,"p":17,"plim":77,"pramprise":55}}
    """
    And I send a fims pub to /new/some/input/uri containing
    """
    {"accontactor":{"value":28},"p":{"value":33},"plim":{"value":3},"pramprise":{"value":25}}
    """
    And I send a fims pub to /new/another/input/uri containing
    """
    {"accontactor":{"value":64},"p":{"value":95},"plim":{"value":84},"pramprise":{"value":7}}
    """
    Then I expect a fims pub to /some/output/uri containing
      """
      {"a":59,"b":16,"c":4,"d":98,"e":16,"f":31,"g":98,"h":85,"i":74,"j":31,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And I expect a fims pub to /another/output/uri containing
      """
      {"a":59,"b":16,"c":4,"d":98,"e":16,"f":31,"g":98,"h":85,"i":74,"j":31,"k":47,"l":17,"m":77,"n":55,"o":47,"p":0,"q":0,"r":0,"s":0,"t":0,"u":0}
      """
    And I expect a fims pub to /some/output/uri/sub containing
      """
      {"a":28,"b":33,"c":3,"d":25,"e":33,"f":64,"g":95,"h":84,"i":7,"j":64,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And I expect a fims pub to /some/one containing
      """
      {"a":59,"b":16,"c":"aaa","d":59,"e":"blah","f":"12345","g":111,"h":222,"i":"another_string","j":59,"k":59,"naked_bobcat":222,"naked_cheetah":222}
      """
    And I expect a fims pub to /some/two containing
      """
      {"clothed_bobcat":{"scale":1000,"ui_type":"none","units":"degC","value":222},"clothed_cheetah":{"scale":1000,"ui_type":"none","units":"degC","value":222}}
      """
    And a fims get to /some/output/uri should yield
      """
      {"a":59,"b":16,"c":4,"d":98,"e":16,"f":31,"g":98,"h":85,"i":74,"j":31,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And a fims get to /another/output/uri should yield
      """
      {"a":59,"b":16,"c":4,"d":98,"e":16,"f":31,"g":98,"h":85,"i":74,"j":31,"k":47,"l":17,"m":77,"n":55,"o":47,"p":0,"q":0,"r":0,"s":0,"t":0,"u":0}
      """
    And a fims get to /some/output/uri/sub should yield
      """
      {"a":28,"b":33,"c":3,"d":25,"e":33,"f":64,"g":95,"h":84,"i":7,"j":64,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}
      """
    And a fims get to /some/one should yield
      """
      {"a":59,"b":16,"c":"aaa","d":59,"e":"blah","f":"12345","g":111,"h":222,"i":"another_string","j":59,"k":59,"naked_bobcat":222,"naked_cheetah":222}
      """
    And a fims get to /some/two should yield
      """
      {"clothed_bobcat":{"scale":1000,"ui_type":"none","units":"degC","value":222},"clothed_cheetah":{"scale":1000,"ui_type":"none","units":"degC","value":222}}
      """

  Scenario: uri_stuff_little_bit_of_everything, take 4
    When I send a fims pub to /some/output/uri containing
    """
    {"enabled":true,"value":"blah"}
    """
    And I send a fims pub to /some/output/uri containing
    """
    {"enabled":true,"value":"blah"}
    """
    Then I expect a fims pub to /some/one containing
      """
      {"a":59,"b":16,"c":"aaa","d":59,"e":"blah","f":"12345","g":111,"h":222,"i":"blah","j":59,"k":59,"naked_bobcat":222,"naked_cheetah":222}
      """
    And a fims get to /some/one should yield
      """
      {"a":59,"b":16,"c":"aaa","d":59,"e":"blah","f":"12345","g":111,"h":222,"i":"blah","j":59,"k":59,"naked_bobcat":222,"naked_cheetah":222}
      """

  Scenario: uri_stuff_little_bit_of_everything, don't respond to gets on inputs
    Then a fims get to /some/input/uri should timeout
    And a fims get to /some/input/uri/accontactor should timeout

  Scenario: uri_stuff_little_bit_of_everything, take 5
    When I send a fims pub to /some/input/uri/accontactor containing
    """
    {"value":51}
    """
    And I send a fims pub to /some/input/uri/p containing
    """
    {"value":85}
    """
    And I send a fims pub to /some/input/uri/plim containing
    """
    {"value":49}
    """
    And I send a fims pub to /some/input/uri/pramprise containing
    """
    {"value":34}
    """
    And I send a fims pub to /another/input/uri/accontactor containing
    """
    {"value":36}
    """
    And I send a fims pub to /another/input/uri/p containing
    """
    {"value":98}
    """
    And I send a fims pub to /another/input/uri/plim containing
    """
    {"value":83}
    """
    And I send a fims pub to /another/input/uri/pramprise containing
    """
    {"value":28}
    """
    And I send a fims pub to /another/input/uri/sub_uri/accontactor containing
    """
    {"value":56}
    """
    And I send a fims pub to /another/input/uri/sub_uri/p containing
    """
    {"value":40}
    """
    And I send a fims pub to /another/input/uri/sub_uri/plim containing
    """
    {"value":34}
    """
    And I send a fims pub to /another/input/uri/sub_uri/pramprise containing
    """
    {"value":88}
    """
    And I send a fims pub to /new/some/input/uri/accontactor containing
    """
    {"value":52}
    """
    And I send a fims pub to /new/some/input/uri/p containing
    """
    {"value":15}
    """
    And I send a fims pub to /new/some/input/uri/plim containing
    """
    {"value":45}
    """
    And I send a fims pub to /new/some/input/uri/pramprise containing
    """
    {"value":12}
    """
    And I send a fims pub to /new/another/input/uri/accontactor containing
    """
    {"value":81}
    """
    And I send a fims pub to /new/another/input/uri/p containing
    """
    {"value":64}
    """
    And I send a fims pub to /new/another/input/uri/plim containing
    """
    {"value":12}
    """
    And I send a fims pub to /new/another/input/uri/pramprise containing
    """
    {"value":63}
    """
    And I send a fims set to /some/output/uri/k containing
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/l containing
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/m containing
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/n containing
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/o containing
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/p containing
    """
    {"value":5}
    """
    And I send a fims set to /another/output/uri/p containing
    """
    {"value":5}
    """
    And I send a fims set to /another/output/uri/q containing
    """
    {"value":5}
    """
    And I send a fims set to /another/output/uri/r containing
    """
    {"value":5}
    """
    And I send a fims set to /another/output/uri/s containing
    """
    {"value":5}
    """
    And I send a fims set to /another/output/uri/t containing
    """
    {"value":5}
    """
    And I send a fims set to /another/output/uri/u containing
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/sub/k containing
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/sub/l containing
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/sub/m containing
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/sub/n containing
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/sub/o containing
    """
    {"value":5}
    """
    And I send a fims set to /some/output/uri/sub/p containing
    """
    {"value":5}l
    """
    Then I expect a fims pub to /some/output/uri containing
      """
      {"a":51,"b":85,"c":49,"d":34,"e":85,"f":36,"g":98,"h":83,"i":28,"j":36,"k":5,"l":5,"m":5,"n":5,"o":5,"p":5}
      """
    And I expect a fims pub to /another/output/uri containing
      """
      {"a":51,"b":85,"c":49,"d":34,"e":85,"f":36,"g":98,"h":83,"i":28,"j":36,"k":56,"l":40,"m":34,"n":88,"o":56,"p":5,"q":5,"r":5,"s":5,"t":5,"u":5}
      """
    And I expect a fims pub to /some/output/uri/sub containing
      """
      {"a":52,"b":15,"c":45,"d":12,"e":15,"f":81,"g":64,"h":12,"i":63,"j":81,"k":5,"l":5,"m":5,"n":5,"o":5,"p":5}
      """
    And I expect a fims pub to /some/one containing
      """
      {"a":51,"b":85,"c":"aaa","d":59,"e":"blah","f":"12345","g":111,"h":222,"i":"blah","j":51,"k":59,"naked_bobcat":222,"naked_cheetah":222}
      """
    And I expect a fims pub to /some/two containing
      """
      {"clothed_bobcat":{"scale":1000,"ui_type":"none","units":"degC","value":222},"clothed_cheetah":{"scale":1000,"ui_type":"none","units":"degC","value":222}}
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
    And a fims get to /some/one should yield
      """
      {"a":51,"b":85,"c":"aaa","d":59,"e":"blah","f":"12345","g":111,"h":222,"i":"blah","j":51,"k":59,"naked_bobcat":222,"naked_cheetah":222}
      """
    And a fims get to /some/two should yield
      """
      {"clothed_bobcat":{"scale":1000,"ui_type":"none","units":"degC","value":222},"clothed_cheetah":{"scale":1000,"ui_type":"none","units":"degC","value":222}}
      """

  Scenario: list expression parsing
    When I send a fims pub to /this/is/a/uri containing
      """
      25
      """
    Then I expect a fims pub to /random/output/uri containing
      """
      {"l":30}
      """
    Then a fims get to /random/output/uri should yield
      """
      {"l":30}
      """