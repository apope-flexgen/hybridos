@unit_test @range_templating
Feature: range_templating

  # Ensure only sparse posts are sent
  Scenario: supply multiple templated ranges in configuration
    Given I am listening for a fims pub on /some/output
    Then I expect a fims pub to /some/output containing
      """
      {"output1":{"value":false},"output10":{"value":false},"output2":{"value":false},"output3":{"value":false},"output5":{"value":false},"output9":{"value":false}}
      """
    When I send a fims pub to /example containing
      """
      {"input1": true}
      """
    Then I expect a fims pub to /some/output containing
      """
      {"output1":{"value":true},"output10":{"value":false},"output2":{"value":false},"output3":{"value":false},"output5":{"value":false},"output9":{"value":false}}
      """
    When I send a fims pub to /example containing
      """
      {"input2": true}
      """
    Then I expect a fims pub to /some/output containing
      """
      {"output1":{"value":true},"output10":{"value":false},"output2":{"value":true},"output3":{"value":false},"output5":{"value":false},"output9":{"value":false}}
      """
    When I send a fims pub to /example containing
      """
      {"input3": true}
      """
    Then I expect a fims pub to /some/output containing
      """
      {"output1":{"value":true},"output10":{"value":false},"output2":{"value":true},"output3":{"value":true},"output5":{"value":false},"output9":{"value":false}}
      """
    # Input 4 is not part of the range, should do nothing
    When I send a fims pub to /example containing
      """
      {"input4": true}
      """
    Then I expect a fims pub to /some/output containing
      """
      {"output1":{"value":true},"output10":{"value":false},"output2":{"value":true},"output3":{"value":true},"output5":{"value":false},"output9":{"value":false}}
      """
    When I send a fims pub to /example containing
      """
      {"input5": true}
      """
    Then I expect a fims pub to /some/output containing
      """
      {"output1":{"value":true},"output10":{"value":false},"output2":{"value":true},"output3":{"value":true},"output5":{"value":true},"output9":{"value":false}}
      """
