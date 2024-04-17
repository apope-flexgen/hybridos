@bit_manipulation @bitfield
Feature: bitfield

  Scenario: basic bitfield parsing
    When I send a fims pub to /assets/ess containing
      """
      {
        "fault_list":[{"value":0, "string":"some_string"}],
        "fault_list2":[{"value":0, "string":"string1"}]
      }
      """
    Then I expect a fims pub to /metrics containing
      """
      {
        "another_string":false,
        "bitfield_length":1,
        "combined_bitfield":[{"value":0,"string":"some_string"},{"value":4,"string":"string1"}],
        "etc.":false,
        "some_string":true,
        "value_in_bitfield":false,
        "yet_another_string":false
      }
      """
    And a fims get to /metrics should yield
      """
      {
        "another_string":false,
        "bitfield_length":1,
        "combined_bitfield":[{"value":0,"string":"some_string"},{"value":4,"string":"string1"}],
        "etc.":false,
        "some_string":true,
        "value_in_bitfield":false,
        "yet_another_string":false
      }
      """

  Scenario: more bitfield parsing
    When I send a fims pub to /assets/ess containing
      """
      {
        "fault_list":[
          {"value":0, "string":"some_string"},
          {"value":1, "string":"another_string"},
          {"value":2, "string":"yet_another_string"},
          {"value":3, "string":"etc."}
          ],
        "fault_list2":[
          {"value":0, "string":"string1"},
          {"value":1, "string":"string2"},
          {"value":2, "string":"string3"},
          {"value":3, "string":"string4"}
          ]
      }
      """
    Then I expect a fims pub to /metrics containing
      """
      {
        "another_string":true,
        "bitfield_length":4,
        "combined_bitfield":[
          {"value":0,"string":"some_string"},
          {"value":1,"string":"another_string"},
          {"value":2,"string":"yet_another_string"},
          {"value":3,"string":"etc."},
          {"value":4,"string":"string1"},
          {"value":5,"string":"string2"},
          {"value":6,"string":"string3"},
          {"value":7,"string":"string4"}
          ],
        "etc.":true,
        "some_string":true,
        "value_in_bitfield":true,
        "yet_another_string":true
      }
      """
    And a fims get to /metrics should yield
      """
      {
        "another_string":true,
        "bitfield_length":4,
        "combined_bitfield":[
          {"value":0,"string":"some_string"},
          {"value":1,"string":"another_string"},
          {"value":2,"string":"yet_another_string"},
          {"value":3,"string":"etc."},
          {"value":4,"string":"string1"},
          {"value":5,"string":"string2"},
          {"value":6,"string":"string3"},
          {"value":7,"string":"string4"}
          ],
        "etc.":true,
        "some_string":true,
        "value_in_bitfield":true,
        "yet_another_string":true
      }
      """