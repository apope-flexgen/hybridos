@unit_test @filters_with_strings
Feature: filters_with_strings

  Scenario: filters_with_strings
    When I send a fims pub to /components/ess_01 containing
      """
      {
        "pcs_1_online_flag": [{"value": 1, "string":"Online"}],
        "pcs_2_online_flag": [{"value": 1, "string":"Online"}],
        "pcs_3_online_flag": [{"value": 1, "string":"Online"}],
        "pcs_4_online_flag": [{"value": 1, "string":"Online"}],
        "pcs_5_online_flag": [{"value": 1, "string":"Online"}],
        "pcs_6_online_flag": [{"value": 1, "string":"Online"}],
        "pcs_7_online_flag": [{"value": 1, "string":"Online"}],
        "pcs_8_online_flag": [{"value": 1, "string":"Online"}],
        "pcs_9_online_flag": [{"value": 1, "string":"Online"}],
        "pcs_10_online_flag": [{"value": 1, "string":"Online"}],
        "pcs_11_online_flag": [{"value": 1, "string":"Online"}],
        "pcs_12_online_flag": [{"value": 1, "string":"Online"}]
      }
      """
    Then I expect a fims pub to /components/ess_01 containing
      """
      {"online_count":12}
      """
    And a fims get to /components/ess_01 should yield
      """
      {"online_count":12}
      """

  Scenario: filters_with_strings, take 2
    When I send a fims pub to /components/ess_01 containing
      """
      {
        "pcs_1_online_flag": [{"value": 1, "string":"Offline"}],
        "pcs_2_online_flag": [{"value": 1, "string":"Offline"}],
        "pcs_3_online_flag": [{"value": 1, "string":"Offline"}],
        "pcs_4_online_flag": [{"value": 1, "string":"Offline"}],
        "pcs_5_online_flag": [{"value": 1, "string":"Offline"}],
        "pcs_6_online_flag": [{"value": 1, "string":"Offline"}],
        "pcs_7_online_flag": [{"value": 1, "string":"Offline"}],
        "pcs_8_online_flag": [{"value": 1, "string":"Offline"}],
        "pcs_9_online_flag": [{"value": 1, "string":"Offline"}],
        "pcs_10_online_flag": [{"value": 1, "string":"Offline"}],
        "pcs_11_online_flag": [{"value": 1, "string":"Offline"}],
        "pcs_12_online_flag": [{"value": 1, "string":"Offline"}]
      }
      """
    Then I expect a fims pub to /components/ess_01 containing
      """
      {"online_count":0}
      """
    And a fims get to /components/ess_01 should yield
      """
      {"online_count":0}
      """

  Scenario: filters_with_strings, take 3
    When I send a fims pub to /components/ess_01 containing
      """
      {
        "pcs_1_online_flag": [{"value": 1, "string":"Online"}],
        "pcs_2_online_flag": [{"value": 1, "string":"Offline"}],
        "pcs_3_online_flag": [{"value": 1, "string":"Online"}],
        "pcs_4_online_flag": [{"value": 1, "string":"Offline"}],
        "pcs_5_online_flag": [{"value": 1, "string":"Online"}],
        "pcs_6_online_flag": [{"value": 1, "string":"Online"}],
        "pcs_7_online_flag": [{"value": 1, "string":"Offline"}],
        "pcs_8_online_flag": [{"value": 1, "string":"Offline"}],
        "pcs_9_online_flag": [{"value": 1, "string":"Online"}],
        "pcs_10_online_flag": [{"value": 1, "string":"Offline"}],
        "pcs_11_online_flag": [{"value": 1, "string":"Offline"}],
        "pcs_12_online_flag": [{"value": 1, "string":"Offline"}]
      }
      """
    Then I expect a fims pub to /components/ess_01 containing
      """
      {"online_count":5}
      """
    And a fims get to /components/ess_01 should yield
      """
      {"online_count":5}
      """