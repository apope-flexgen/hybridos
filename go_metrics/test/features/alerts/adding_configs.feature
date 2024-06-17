@alerts @adding_configs
Feature: adding_configs
  Scenario: Handling valid and invalid configs receive on fims. Should always receive a response with success/error and only backup to dbi on success
    # Invalid json tests
    Given I am listening for a fims set on /my/replyto
    Given I am listening for a fims set on /dbi/go_metrics
    When I send a fims set with replyto /my/replyto to /go_metrics/configuration/id1 with containing
      """
      "hello"
      """
    Then I expect a fims set to /my/replyto within 1 seconds containing
      """
      {"message":"could not handle configuration received from fims","success":false}
      """
    And I expect no fims sets will be sent to /dbi/go_metrics
    When I send a fims set with replyto /my/replyto to /go_metrics/configuration/id1 containing
      """
      [4]
      """
    Then I expect a fims set to /my/replyto within 1 seconds containing
      """
      {"message":"could not handle configuration received from fims","success":false}
      """
    And I expect no fims sets will be sent to /dbi/go_metrics
    # Mismatched input/metric/output tests
    When I send a fims set with replyto /my/replyto to /go_metrics/configuration/id2 containing
      """
      {"inputs":{"mismatch":{"uri":"/example/soc","type":"float"}},"outputs":{"id2":{"uri":"/events/alerts","flags":["clothed","post","sparse","flat","lonely","no_heartbeat"],"attributes":{"source":"Alerts"}}},"metrics":[{"id":"id2","type":"bool","outputs":"id2","expression":"soc > 90","alert":true,"messages":[{"soc > 90":"soc {soc} exceeded 90%"}],"enabled":true}]}
      """
    Then I expect a fims set to /my/replyto within 1 seconds containing
      """
      {"message":"could not handle configuration received from fims","success":false}
      """
    And I expect no fims sets will be sent to /dbi/go_metrics
    When I send a fims set with replyto /my/replyto to /go_metrics/configuration/id3 containing
      """
      {"inputs":{"soc":{"uri":"/example/soc","type":"float"}},"outputs":{"mismatch":{"uri":"/events/alerts","flags":["clothed","post","sparse","flat","lonely","no_heartbeat"],"attributes":{"source":"Alerts"}}},"metrics":[{"id":"id3","type":"bool","outputs":"id3","expression":"soc > 90","alert":true,"messages":[{"soc > 90":"soc {soc} exceeded 90%"}],"enabled":true}]}
      """
    Then I expect a fims set to /my/replyto within 1 seconds containing
      """
      {"message":"could not handle configuration received from fims","success":false}
      """
    And I expect no fims sets will be sent to /dbi/go_metrics
    When I send a fims set with replyto /my/replyto to /go_metrics/configuration/id4 containing
      """
      {"inputs":{"soc":{"uri":"/example/soc","type":"float"}},"outputs":{"id4":{"uri":"/events/alerts","flags":["clothed","post","sparse","flat","lonely","no_heartbeat"],"attributes":{"source":"Alerts"}}},"metrics":[{"id":"id4","type":"bool","outputs":"mismatch","expression":"soc > 90","alert":true,"messages":[{"soc > 90":"soc {soc} exceeded 90%"}],"enabled":true}]}
      """
    Then I expect a fims set to /my/replyto within 1 seconds containing
      """
      {"message":"could not handle configuration received from fims","success":false}
      """
    And I expect no fims sets will be sent to /dbi/go_metrics
    # Valid config, success
    When I send a fims set with replyto /my/replyto to /go_metrics/configuration/placeholder containing
      """
      {"inputs":{"soc":{"uri":"/example/soc","type":"float"}},"outputs":{"placeholder":{"uri":"/events/alerts","flags":["clothed","post","sparse","flat","lonely","no_heartbeat","generate_uuid"],"attributes":{"source":"Alerts"}}},"metrics":[{"id":"placeholder","type":"bool","outputs":"placeholder","expression":"soc > 90","alert":true,"messages":[{"soc > 90":"soc {soc} exceeded 90%"}],"enabled":true}]}
      """
    Then I expect a fims set to /my/replyto within 1 seconds containing
      """
      {"message":"could not handle configuration received from fims","success":false}
      """
    And I expect a fims set to /dbi/go_metrics/placeholder within 3 seconds containing
      """
      {"inputs":{"soc":{"uri":"/example/soc","type":"float"}},"outputs":{"placeholder":{"uri":"/events/alerts","flags":["clothed","post","sparse","flat","lonely","no_heartbeat","generate_uuid"],"attributes":{"source":"Alerts"}}},"metrics":[{"id":"placeholder","type":"bool","outputs":"placeholder","expression":"soc > 90","alert":true,"messages":[{"soc > 90":"soc {soc} exceeded 90%"}],"enabled":true}]}
      """