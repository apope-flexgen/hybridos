@unit_test @shadow_uris
Feature: shadow_uris

  Scenario: shadow_uris
    When I send a fims pub to /features/reactive_power containing
      """
      {
        "value":5,
        "runmode1_kVAR_mode_cmd":{"value":3}
      }
      """
    And I send a fims pub to /features/active_power containing
      """
      {
        "value":125,
        "runmode1_kW_mode_cmd":{
                                  "value":23,
                                  "nested_value":{"value":33,"other_value":2}
                                }
      }
      """
    Then I expect a fims pub to /metrics/site_status containing
      """
      {
        "active_power_command_feedback":125,
        "active_power_command_mode_feedback":23,
        "active_power_operating_mode":23,
        "reactive_power_command_feedback":5,
        "reactive_power_operating_mode":3,
        "znested_value":33,
        "zznested_value":2
      }
      """
    And a fims get to /metrics/site_status should yield
      """
      {
        "active_power_command_feedback":125,
        "active_power_command_mode_feedback":23,
        "active_power_operating_mode":23,
        "reactive_power_command_feedback":5,
        "reactive_power_operating_mode":3,
        "znested_value":33,
        "zznested_value":2
      }
      """

  Scenario: shadow_uris, take 2
    When I send a fims pub to /features/reactive_power containing
      """
      {
        "value":1,
        "runmode1_kVAR_mode_cmd":{"value":2}
      }
      """
    And I send a fims pub to /features/active_power containing
      """
      {
        "value":3,
        "runmode1_kW_mode_cmd":{
                                "value":4,
                                "nested_value":{"value":5,"other_value":6}
                              }
      }
      """
    Then I expect a fims pub to /metrics/site_status containing
      """
      {
        "active_power_command_feedback":3,
        "active_power_command_mode_feedback":4,
        "active_power_operating_mode":4,
        "reactive_power_command_feedback":1,
        "reactive_power_operating_mode":2,
        "znested_value":5,
        "zznested_value":6
      }
      """
    And a fims get to /metrics/site_status should yield
      """
      {
        "active_power_command_feedback":3,
        "active_power_command_mode_feedback":4,
        "active_power_operating_mode":4,
        "reactive_power_command_feedback":1,
        "reactive_power_operating_mode":2,
        "znested_value":5,
        "zznested_value":6
      }
      """