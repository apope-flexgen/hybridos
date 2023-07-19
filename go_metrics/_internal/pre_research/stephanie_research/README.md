# Go Metrics
## Research Summary
### Project Scope
1. Phil created a nice [overview document](https://github.com/flexgen-power/echo/blob/research/metrics/pkg/metrics_research/doc/Overview.md) that details some of the current abilities and limitations of echo, metrics, and ess_controller. Basically, ess_controller is fast but incomplete.
2. Phil also compiled all of his design work into [another document](https://github.com/flexgen-power/echo/blob/research/metrics/pkg/metrics_research/MetricsReview.md). This contains a brief listing of echo and metrics functions and comprehensive design thoughts.
3. Phil ALSO made a [cool diagram](./echo2.pdf) detailing the workflow for the new metrics system.
4. The integration team shared this nice [Echo Metrics Requirements](./EchoMetricsRequirements.docx) document that goes over what they need.

### Config Stuff
1. Walker shared this [sample config document](./sample.json) that he was working with the integration team on developing.
2. Graham shared [his ideas for templating](./graham_templating.txt) in Slack. (Still under discussion)

### Memory stuff
1. Good article [here](https://github.com/ricardoerikson/benchmark-golang-maps). TL;DR: Try to avoid using types that can hold pointers when creating maps. (Including interfaces!!) This creates a performance penalty because they have to go through garbage collection scans when they *can* contain pointers. Structs are really good to use instead because they can actually be zero-sized. Note: structs require more allocations, but this is explicitly for performance benefits.
2. Ashton did a bunch of work with various map types in Go. See [here](https://github.com/flexgen-power/echo/tree/research/metrics/pkg/metrics_research/optimize_fims/maps_trial).
3. Walker had an extensive discussion with me about how he designed modbus client to allocate blocks of memory in advance of runtime. See modbus_client files for more details on that. (IT'S A LOT.)

### Other
1. Just for funsies (and also to better learn about what makes the current metrics system tick), I started making a version of metrics in Go. (It's one of the random go files in this folder.)
2. A thought: Would it be more memory efficient if we used dynamic structs instead of typical interfaces for unmarshaling data? See [dynamic struct repo](https://github.com/Ompluscator/dynamic-struct) for more details about dynamic structs. The thought would be: on file load, make a dynamic struct based on the data that's there. When doing processing for the next XXX seconds, you can use the dynamic struct rather than an interface to unmarshal data.