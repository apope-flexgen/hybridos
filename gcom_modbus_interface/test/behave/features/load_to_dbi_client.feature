@load_to_dbi_client
Feature: load_to_dbi_client

  Scenario:
    Given mongo and dbi are running
    And I clear all data in dbi/test_client
    And I clear all data in dbi/test_server
    And modbus_client is running the config test_load_to_from_dbi_client.json
    And modbus_server is running the config test_load_to_from_dbi_server.json
    And I am listening for a fims set on /dbi
    And I am listening for a fims get on /dbi
    And I am listening for a fims set on /components/test_server
    And I am listening for a fims set on /components/test_client
    When I wait 2 seconds
    And I send a fims set to /components/test_client containing
      """
      {
          "hold_1_1": 1,
          "hold_1_2": 2,
          "hold_1_4": 3,
          "hold_2_1": 4,
          "hold_2_2": 5,
          "hold_2_4": 6,
          "shold_1_1": 7,
          "shold_1_2": 8,
          "shold_1_4": 9,
          "shold_2_1": 10,
          "shold_2_2": 11,
          "shold_2_4": 12,
          "fhold_1_2": 13.7,
          "fhold_1_4": 14.8,
          "fshold_1_4": 15.9,
          "scale_1_1": 16,
          "scale_1_2": 17,
          "scale_1_4": 18,
          "shift_1_1": 119.1,
          "shift_1_2": 120,
          "shift_1_4": 121,
          "test_id": 22,
          "inv_1_16": 23,
          "inv_1_4": 24,
          "multi_1": 25,
          "multi_2": 26,
          "multi_3": 27,
          "multi_4": 28,
          "multi_5": 29,
          "multi_6": 30,
          "multi_7": 31,
          "multi_8": 32,
          "multi_9": 33,
          "multi_10": 34,
          "multi_11": 35,
          "multi_12": 36,
          "multi_13": 37,
          "debounce_1": 38,
          "error_1": 139,
          "input_0_1": 40,
          "input_1_1": 41,
          "input_2_1": 42,
          "input_3_1": 43,
          "sinput_4_1": 44.4,
          "input_5_1": 45,
          "input_7_1": 46,
          "input_0_2": 47,
          "input_1_2": 48,
          "input_2_2": 49,
          "input_3_2": 50,
          "sinput_4_2": 51.5,
          "input_5_2": 52,
          "finput_6_2": 53.6,
          "input_7_2": 54,
          "input_0_4": 55,
          "input_1_4": 56,
          "input_2_4": 57,
          "input_3_4": 58,
          "sinput_4_4": 59.7,
          "input_5_4": 60,
          "finput_6_4": 61.8,
          "input_7_4": 62,
          "enum_256_1": 63,
          "enum_257_2": 64,
          "enum_259_4": 65,
          "bsinput_0_4": 66,
          "bsinput_1_4": 67,
          "heartbeat": 68,
          "heartbeat2": 69,
          "ibits_1000_1": 70,
          "some_string": true,
          "another_string": true,
          "yet_another_string": true,
          "etc.": true,
          "some_string_2": true,
          "another_string_2": true,
          "yet_another_string_2": true,
          "etc._2": true,
          "some_string_3": true,
          "another_string_3": true,
          "yet_another_string_3": true,
          "etc._3": true,
          "bf_2007_1": 71,
          "bf_2008_2": 72,
          "bf_2010_4": 73,
          "pack_1_first": 74,
          "pack_1_second": 75,
          "coil_0": true,
          "coil_1": true,
          "coil_2": true,
          "coil_3": true,
          "coil_4": true,
          "coil_5": true,
          "coil_6": true,
          "coil_7": true,
          "break_test_1": true,
          "break_test_2": true,
          "surge_arrester": true,
          "fuse_monitoring": true,
          "door_latch": true,
          "disconnect_switch": true,
          "spare_1": true,
          "e_stop": true,
          "fire_relay": true,
          "trouble_relay": true,
          "component_connected": true
      }
      """
    And I wait 2 seconds
    And I send a fims set to /components/test_client containing
      """
      {
          "hold_1_1": 1,
          "hold_1_2": 2,
          "hold_1_4": 3,
          "hold_2_1": 4,
          "hold_2_2": 5,
          "hold_2_4": 6,
          "shold_1_1": 7,
          "shold_1_2": 8,
          "shold_1_4": 9,
          "shold_2_1": 10,
          "shold_2_2": 11,
          "shold_2_4": 12,
          "fhold_1_2": 13.7,
          "fhold_1_4": 14.8,
          "fshold_1_4": 15.9,
          "scale_1_1": 16,
          "scale_1_2": 17,
          "scale_1_4": 18,
          "shift_1_1": 119.1,
          "shift_1_2": 120,
          "shift_1_4": 121,
          "test_id": 22,
          "inv_1_16": 23,
          "inv_1_4": 24,
          "multi_1": 25,
          "multi_2": 26,
          "multi_3": 27,
          "multi_4": 28,
          "multi_5": 29,
          "multi_6": 30,
          "multi_7": 31,
          "multi_8": 32,
          "multi_9": 33,
          "multi_10": 34,
          "multi_11": 35,
          "multi_12": 36,
          "multi_13": 37,
          "debounce_1": 38,
          "error_1": 139,
          "input_0_1": 40,
          "input_1_1": 41,
          "input_2_1": 42,
          "input_3_1": 43,
          "sinput_4_1": 44.4,
          "input_5_1": 45,
          "input_7_1": 46,
          "input_0_2": 47,
          "input_1_2": 48,
          "input_2_2": 49,
          "input_3_2": 50,
          "sinput_4_2": 51.5,
          "input_5_2": 52,
          "finput_6_2": 53.6,
          "input_7_2": 54,
          "input_0_4": 55,
          "input_1_4": 56,
          "input_2_4": 57,
          "input_3_4": 58,
          "sinput_4_4": 59.7,
          "input_5_4": 60,
          "finput_6_4": 61.8,
          "input_7_4": 62,
          "enum_256_1": 63,
          "enum_257_2": 64,
          "enum_259_4": 65,
          "bsinput_0_4": 66,
          "bsinput_1_4": 67,
          "heartbeat": 68,
          "heartbeat2": 69,
          "ibits_1000_1": 70,
          "some_string": true,
          "another_string": true,
          "yet_another_string": true,
          "etc.": true,
          "some_string_2": true,
          "another_string_2": true,
          "yet_another_string_2": true,
          "etc._2": true,
          "some_string_3": true,
          "another_string_3": true,
          "yet_another_string_3": true,
          "etc._3": true,
          "bf_2007_1": 71,
          "bf_2008_2": 72,
          "bf_2010_4": 73,
          "pack_1_first": 74,
          "pack_1_second": 75,
          "coil_0": true,
          "coil_1": true,
          "coil_2": true,
          "coil_3": true,
          "coil_4": true,
          "coil_5": true,
          "coil_6": true,
          "coil_7": true,
          "break_test_1": true,
          "break_test_2": true,
          "surge_arrester": true,
          "fuse_monitoring": true,
          "door_latch": true,
          "disconnect_switch": true,
          "spare_1": true,
          "e_stop": true,
          "fire_relay": true,
          "trouble_relay": true,
          "component_connected": true
      }
      """
    Then I expect a fims set to /dbi/test_client/saved_registers/components/test_client/hold_1_1 within 1 seconds containing
      """
      1
      """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/hold_1_2 within 1 seconds containing
        """
        2
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/hold_1_4 within 1 seconds containing
        """
        3
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/hold_2_1 within 1 seconds containing
        """
        4
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/hold_2_2 within 1 seconds containing
        """
        5
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/hold_2_4 within 1 seconds containing
        """
        6
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/shold_1_1 within 1 seconds containing
        """
        7
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/shold_1_2 within 1 seconds containing
        """
        8
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/shold_1_4 within 1 seconds containing
        """
        9
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/shold_2_1 within 1 seconds containing
        """
        10
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/shold_2_2 within 1 seconds containing
        """
        11
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/shold_2_4 within 1 seconds containing
        """
        12
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/fhold_1_2 within 1 seconds containing
        """
        13.7
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/fhold_1_4 within 1 seconds containing
        """
        14.8
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/fshold_1_4 within 1 seconds containing
        """
        15.9
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/scale_1_1 within 1 seconds containing
        """
        16
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/scale_1_2 within 1 seconds containing
        """
        17
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/scale_1_4 within 1 seconds containing
        """
        18
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/shift_1_1 within 1 seconds containing
        """
        119.1
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/shift_1_2 within 1 seconds containing
        """
        120
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/shift_1_4 within 1 seconds containing
        """
        121
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/test_id within 1 seconds containing
        """
        22
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/inv_1_16 within 1 seconds containing
        """
        23
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/inv_1_4 within 1 seconds containing
        """
        24
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/multi_1 within 1 seconds containing
        """
        25
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/multi_2 within 1 seconds containing
        """
        26
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/multi_3 within 1 seconds containing
        """
        27
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/multi_4 within 1 seconds containing
        """
        28
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/multi_5 within 1 seconds containing
        """
        29
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/multi_6 within 1 seconds containing
        """
        30
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/multi_7 within 1 seconds containing
        """
        31
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/multi_8 within 1 seconds containing
        """
        32
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/multi_9 within 1 seconds containing
        """
        33
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/multi_10 within 1 seconds containing
        """
        34
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/multi_11 within 1 seconds containing
        """
        35
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/multi_12 within 1 seconds containing
        """
        36
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/multi_13 within 1 seconds containing
        """
        37
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/debounce_1 within 1 seconds containing
        """
        38
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/error_1 within 1 seconds containing
        """
        139
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/coil_0 within 1 seconds containing
        """
        true
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/coil_1 within 1 seconds containing
        """
        true
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/coil_2 within 1 seconds containing
        """
        true
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/coil_3 within 1 seconds containing
        """
        true
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/coil_4 within 1 seconds containing
        """
        true
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/coil_5 within 1 seconds containing
        """
        true
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/coil_6 within 1 seconds containing
        """
        true
        """
      And I expect a fims set to /dbi/test_client/saved_registers/components/test_client/coil_7 within 1 seconds containing
      """
      true
      """
    When I kill modbus_client
    And I kill modbus_server
    And I start modbus_client with the config test_load_to_from_dbi_client.json
    And I start modbus_server with the config test_load_to_from_dbi_server.json
    Then I expect a fims get to /dbi/test_client/saved_registers/components/test_client within 1 seconds containing
      """
      (null)
      """
    And I expect a fims set to /components/test_client within 1 seconds containing
      """
      {
          "hold_1_1": 1,
          "hold_1_2": 2,
          "hold_1_4": 3,
          "hold_2_1": 4,
          "hold_2_2": 5,
          "hold_2_4": 6,
          "shold_1_1": 7,
          "shold_1_2": 8,
          "shold_1_4": 9,
          "shold_2_1": 10,
          "shold_2_2": 11,
          "shold_2_4": 12,
          "fhold_1_2": 13.7,
          "fhold_1_4": 14.8,
          "fshold_1_4": 15.9,
          "scale_1_1": 16,
          "scale_1_2": 17,
          "scale_1_4": 18,
          "shift_1_1": 119.1,
          "shift_1_2": 120,
          "shift_1_4": 121,
          "test_id": 22,
          "inv_1_16": 23,
          "inv_1_4": 24,
          "multi_1": 25,
          "multi_2": 26,
          "multi_3": 27,
          "multi_4": 28,
          "multi_5": 29,
          "multi_6": 30,
          "multi_7": 31,
          "multi_8": 32,
          "multi_9": 33,
          "multi_10": 34,
          "multi_11": 35,
          "multi_12": 36,
          "multi_13": 37,
          "debounce_1": 38,
          "error_1": 139,
          "coil_0": true,
          "coil_1": true,
          "coil_2": true,
          "coil_3": true,
          "coil_4": true,
          "coil_5": true,
          "coil_6": true,
          "coil_7": true
      }
      """
    And I expect a fims set to /components/test_server/hold_1_1 within 1 seconds containing
      """
      {"value": 1}
      """
      And I expect a fims set to /components/test_server/hold_1_2 within 1 seconds containing
        """
        {"value": 2}
        """
      And I expect a fims set to /components/test_server/hold_1_4 within 1 seconds containing
        """
        {"value": 3}
        """
      And I expect a fims set to /components/test_server/hold_2_1 within 1 seconds containing
        """
        {"value": 4}
        """
      And I expect a fims set to /components/test_server/hold_2_2 within 1 seconds containing
        """
        {"value": 5}
        """
      And I expect a fims set to /components/test_server/hold_2_4 within 1 seconds containing
        """
        {"value": 6}
        """
      And I expect a fims set to /components/test_server/shold_1_1 within 1 seconds containing
        """
        {"value": 7}
        """
      And I expect a fims set to /components/test_server/shold_1_2 within 1 seconds containing
        """
        {"value": 8}
        """
      And I expect a fims set to /components/test_server/shold_1_4 within 1 seconds containing
        """
        {"value": 9}
        """
      And I expect a fims set to /components/test_server/shold_2_1 within 1 seconds containing
        """
        {"value": 10}
        """
      And I expect a fims set to /components/test_server/shold_2_2 within 1 seconds containing
        """
        {"value": 11}
        """
      And I expect a fims set to /components/test_server/shold_2_4 within 1 seconds containing
        """
        {"value": 12}
        """
      And I expect a fims set to /components/test_server/fhold_1_2 within 1 seconds containing
        """
        {"value": 13.699999809265137}
        """
      And I expect a fims set to /components/test_server/fhold_1_4 within 1 seconds containing
        """
        {"value": 14.8}
        """
      And I expect a fims set to /components/test_server/fshold_1_4 within 1 seconds containing
        """
        {"value": 15.9}
        """
      And I expect a fims set to /components/test_server/scale_1_1 within 1 seconds containing
        """
        {"value": 16}
        """
      And I expect a fims set to /components/test_server/scale_1_2 within 1 seconds containing
        """
        {"value": 17}
        """
      And I expect a fims set to /components/test_server/scale_1_4 within 1 seconds containing
        """
        {"value": 18}
        """
      And I expect a fims set to /components/test_server/shift_1_1 within 1 seconds containing
        """
        {"value": 19}
        """
      And I expect a fims set to /components/test_server/shift_1_2 within 1 seconds containing
        """
        {"value": 20}
        """
      And I expect a fims set to /components/test_server/shift_1_4 within 1 seconds containing
        """
        {"value": 21}
        """
      And I expect a fims set to /components/test_server/test_id within 1 seconds containing
        """
        {"value": 22}
        """
      And I expect a fims set to /components/test_server/inv_1_16 within 1 seconds containing
        """
        {"value": 24}
        """
      And I expect a fims set to /components/test_server/inv_1_4 within 1 seconds containing
        """
        {"value": 2825}
        """
      And I expect a fims set to /components/test_server/multi_1 within 1 seconds containing
        """
        {"value": 25}
        """
      And I expect a fims set to /components/test_server/multi_2 within 1 seconds containing
        """
        {"value": 26}
        """
      And I expect a fims set to /components/test_server/multi_3 within 1 seconds containing
        """
        {"value": 27}
        """
      And I expect a fims set to /components/test_server/multi_4 within 1 seconds containing
        """
        {"value": 28}
        """
      And I expect a fims set to /components/test_server/multi_5 within 1 seconds containing
        """
        {"value": 29}
        """
      And I expect a fims set to /components/test_server/multi_6 within 1 seconds containing
        """
        {"value": 30}
        """
      And I expect a fims set to /components/test_server/multi_7 within 1 seconds containing
        """
        {"value": 31}
        """
      And I expect a fims set to /components/test_server/multi_8 within 1 seconds containing
        """
        {"value": 32}
        """
      And I expect a fims set to /components/test_server/multi_9 within 1 seconds containing
        """
        {"value": 33}
        """
      And I expect a fims set to /components/test_server/multi_10 within 1 seconds containing
        """
        {"value": 34}
        """
      And I expect a fims set to /components/test_server/multi_11 within 1 seconds containing
        """
        {"value": 35}
        """
      And I expect a fims set to /components/test_server/multi_12 within 1 seconds containing
        """
        {"value": 36}
        """
      And I expect a fims set to /components/test_server/multi_13 within 1 seconds containing
        """
        {"value": 37}
        """
      And I expect a fims set to /components/test_server/debounce_1 within 1 seconds containing
        """
        {"value": 38}
        """
      And I expect a fims set to /components/test_server/error_1 within 1 seconds containing
        """
        {"value": 39}
        """
      And I expect a fims set to /components/test_server/coil_0 within 1 seconds containing
        """
        {"value": true}
        """
      And I expect a fims set to /components/test_server/coil_1 within 1 seconds containing
        """
        {"value": true}
        """
      And I expect a fims set to /components/test_server/coil_2 within 1 seconds containing
        """
        {"value": true}
        """
      And I expect a fims set to /components/test_server/coil_3 within 1 seconds containing
        """
        {"value": true}
        """
      And I expect a fims set to /components/test_server/coil_4 within 1 seconds containing
        """
        {"value": true}
        """
      And I expect a fims set to /components/test_server/coil_5 within 1 seconds containing
        """
        {"value": true}
        """
      And I expect a fims set to /components/test_server/coil_6 within 1 seconds containing
        """
        {"value": true}
        """
      And I expect a fims set to /components/test_server/coil_7 within 1 seconds containing
      """
      {"value": true}
      """
    