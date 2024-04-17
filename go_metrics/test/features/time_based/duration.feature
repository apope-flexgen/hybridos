@time_based @duration
Feature: duration

    Scenario: duration function
        Given I am listening for a fims pub on /some/output
        When I send a fims pub to /example containing
            """
            {"v1": 1}
            """
        Then I expect a fims pub to /some/output containing
            """
            {"duration": {"value": false}}
            """
        Then I expect a fims pub to /some/output after 7 seconds containing
            """
            {"duration": {"value": true}}
            """
