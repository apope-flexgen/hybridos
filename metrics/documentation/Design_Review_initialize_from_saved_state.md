# Metrics - Initialize from Saved State

## Goals

"As a system administrator, I want metrics to initialize its inputs on start to avoid delays waiting for other modules to publish." Whenever possible and appropriate, we want metrics to start up where it left off in the event of a restart. We want it to retain all the values and states it can.


## Affected Repos

`metrics` is the only repo directly affected.


## Approach

Every time metrics starts, it configures a "Master Data Object" that we call `mdo`. The mdo is then the "living, breathing" metrics module, in effect. When metrics receives FIMS sets and pubs, it references and/or modifies the mdo and responds accordingly. Because, over time, the mdo retains various changing values and states that are supplied by, and returned to, other modules, we want to keep track of those values and states so we can get back to where we were if metrics restarts in a planned or unplanned manner. By saving the mdo to a .json file on the server, we can read that file any time metrics starts and, if appropriate, use it to restore the mdo to the state it was last in.


## Considerations

When HybridOS is running, the mdo changes many hundreds of times per second. In comparison, a restart of metrics takes many seconds. Within the many seconds it takes for a restart, the mdo would have changed thousands of times. With that in mind, we can see that mdo changes from one fraction of a second to another fraction of a second cannot realistically be captured and restored. However, because the mdo is constantly revised when metrics is running, changes not captured will quickly be updated. The value of capturing and restoring the mdo may primarily be in the relatively static `initalValues` and states that it keeps.

Clearly, a saved mdo object will eventually become out-of-date. Using an mdo that was saved many minutes, hours, or days ago likely is no better - and may be worse - than starting with an mdo freshly configured from the `metrics.json` config file. With that in mind, an `mdo.json` file older than 5 minutes will not be used.


## Interface

Similar to most other modules in HybridOS, metrics reads a .json config file at startup to "orient" itself to its local environment. When metrics is running, it automatically creates an `mdo.json` file containing the latest state of the mdo. A saved `mdo.json` file constitutes an *optional* config file for metrics; accordingly, if `mdo.json` exists, it will be found in the same place as metrics' config file (`metrics.json`) nested in the `/config` directory. metrics does not need to have an `mdo.json` file to start, it uses information from `metrics.json` to create a new mdo when needed at startup.

In development testing, the mdo is modified in some manner by metrics' various operations 200-400 times per second. The difference of one change to the next is mostly moot when considering that we are saving state for a restart condition which will take a number of seconds to complete. With that in mind - and with input from John C and Tony O, while metrics is running, we save the mdo to `mdo.json` every 10 seconds. This pace *does not* noticeably increase metrics' CPU usage whereas saving every modification *does* substantially increase CPU usage.

We have chosen to use the `fs.writeFileSync` method to write `mdo.json`. Because each write of `mdo.json` is simply a "snapshot" of the mdo, we want to overwrite `mdo.json` every time. `writeFileSync` does this in the simplest, most robust way.

An `mdo.json` file older than 5 minutes will be considered "stale" and will not be used. An `mdo.json` file based on an old `metrics.json` file will not be used. In other words, if the `metrics.json` file has been updated, then an `mdo.json` file based on an older `metrics.json` file will no longer be valid. Therefore the `mdo.json` will not be used.

When restarting metrics, any existing `mdo.json` file can be "invalidated" by simply removing it from the metrics config directory. Note that if metrics is running, a new `mdo.json` file will be created every 10 seconds, so deleting an `mdo.json` while metrics is running may be an exercise in futility!


## Testing

To test this functionality, we have outputted and compared the resulting mdo objects produced both by the pre-existing functionality and by the new read-from-`mdo.json` functionality. We have found them to match in every respect other than, of course, the updated values and states of the `mdo.json` method.

For additional testing, we have induced errors into the `mdo.json` file to ensure that the new functionality (using the mdo from `mdo.json`) correctly falls-back to the pre-existing functionality (configuring a new mdo from data in the `metrics.json` config file). metrics correctly initializes whether there is an `mdo.json` file or not, whether the file has JSON object format errors or not, and whether the file is "stale" or "fresh".


## Backward Compatibility

This new functionality retains the original functionality in respect to reading a `metrics.json` file at startup to configure the mdo. Using this new functionality with an old `metrics.json` config file will not alter the expected function of metrics.


## Configuration

This functionality automatically configures itself. It automatically generates and saves an `mdo.json` file to the same directory in which metrics finds its `metrics.json` config file. If a valid, fresh `mdo.json` file - not older than 5 minutes - exists in metrics' config directory, it will be used for the mdo. If an `mdo.json` file does not exist, is stale, or is corrupted, then it will not be used and the mdo will be configured using data in `metrics.json`.