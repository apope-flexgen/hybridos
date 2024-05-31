@alerts @adding_configs
Feature: adding_configs

  Scenario:
    Given I am listening for a fims set on /dbi/go_metrics
    When I send a fims set to /go_metrics/configuration/placeholder containing
      """
      {"inputs":{"soc":{"uri":"/example/soc","type":"float"}},"outputs":{"placeholder":{"uri":"/events/alerts","flags":["clothed","post","sparse","flat","lonely","no_heartbeat"],"attributes":{"source":"Alerts"}}},"metrics":[{"id":"placeholder","type":"bool","outputs":"placeholder","expression":"soc > 90","alert":true,"messages":[{"soc > 90":"soc {soc} exceeded 90%"}],"enabled":true}]}
      """
    Then I expect a fims set to /dbi/go_metrics/placeholder within 3 seconds containing
      """
      {"inputs":{"soc":{"uri":"/example/soc","type":"float"}},"outputs":{"placeholder":{"uri":"/events/alerts","flags":["clothed","post","sparse","flat","lonely","no_heartbeat"],"attributes":{"source":"Alerts"}}},"metrics":[{"id":"placeholder","type":"bool","outputs":"placeholder","expression":"soc > 90","alert":true,"messages":[{"soc > 90":"soc {soc} exceeded 90%"}],"enabled":true}]}
      """