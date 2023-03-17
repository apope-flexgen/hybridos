# UI Events Display

## Goals

* Allow user to select, sort, and review events for troubleshooting and information purposes.


## Modules/Repos Affected

* web_ui
* web_server
* events


## Assumptions

* Debug, Info, Status, Alarm, and Fault events can occur during operation of HybridOS system.
* Users may want or need to review those events for troubleshooting or information purposes.
* Display of events should correspond to user experience, e.g., an alert yesterday afternoon should be easily found and displayed through selection and sorting.
* Display of events should be in list form, with x lines per page and pages as needed to display the events of the query.
* Number of lines per page to display should be small enough that scrolling is not required. As of 10/07/19, the lines/events per page is 12.


## Interface Variables

* calendar selectors to select date range to view: "Only view events on or after: [date] and before: [date]". If no dates are set, all events are included in the search.
* checkboxes to select event severities to view: "Only view these event types: x Info  x Status  x Alarm  x Fault". All event types are initially checked/selected.
* user-selected sorting by Source, Message, Severity, or Timestamp. The sorted column is indicated with a red arrow pointing up or down corresponding to A-Z/Z-A, or severity level, or newest/oldest. When sorting by anything other than timestamp, timestamp is used as a secondary sort to make the most recent event show at the top. This secondary sorting is indicated by a light gray arrow. At present, the user cannot make the secondary sort invert (oldest at the top).


## Algorithm

* make MongoDB filter and sort events corresponding to the user's query
* retrieve data from MongoDB in chunks of 12 events by supplying MongoDB with the filter/sort query and the page number (the offset)


## Issues

* none


## Future Development

* No additional items considered as of 10/07/19.


-Desmond Mullen, 10/07/19