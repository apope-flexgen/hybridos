# As a SWD, I want to be able to `set` individual commands to a database to retrieve later
https://app.zenhub.com/workspaces/full-stack-project-management-5fda4dfadc580f000e7f4f53/issues/flexgen-power/full_stack_pm/52

User feedback from site_controller which already has a reference implementation of the consumer.

> To store setpoints
> The user module receives a set on 
> /assets/ess/ess_1/maintenance_mode '{"value":true}'
> 
> Then sends that value back out to be stored
> m set
> u /storage/site_controller/setpoints/assets/ess/ess_1/maintenance_mode
> r None
> {"value":true}
> 
> When it wishes to retrieve that later
> m get
> u /storage/site_controller/setpoints
> r /site_controller/setpoints
> 
> and it will parse out the JSON response

In this scheme, we can replace `storage` with `dbi`, `hybridos` would be the `:database` fragment and probably changes to `site_controller` (but that's moot because this is dynamic), `setpoints` is the `:collection` fragment, and the rest sort of has to be built ad-hoc. 

If this example were the first set, I would expect a document being created in the hybridos database, setpoints collection, and that document would end up looking like this

```json
{
  "_uuid": <generated by MongoDB>,
  "assets": {
    "ess": {
      "ess_1": {
        "maintenance_mode": true
      }
    }
  }
}
```

Thanks to @jnshade for the input

Criteria for completion for this story are to be MVP, meaning:
* No schemas should be involved, even as placeholders
* Since we would only have one document, `post` support is not needed yet, `set` can create the document since there is none, and then updates from there
* It would be useful to developers to include a `del` that can operate just at the :database/:collection level to completely erase the one document created, no need to be able to `del` individual objects within the document at this point
* No other metadata should be included by `dbi` such as a received timestamp, number of updates, etc.
* Other HybridOS platform integration like COPS or dealing with off box replication do not need to be considered
* Does not include get/set of configs, that will be a separate story
* No limitations on what can be stored in `dbi`, meaning no config is necessary to tell it which URIs to listen for, it just needs to listen on `/dbi`

Since the feature set is limited, I would want to see certain parts of the "Approach" section of the design review laid out in a bit of detail before implementation
* The expected flow of Mongo interactions listed out for the operations, i.e. does a `set` do a `findOne()` on the collection before creating the document, then branch to a `update()` or `create()` depending on the answer
* Overall process control structure, i.e. is it a while loop that just calls a `processFims()`, `setInterval`, or otherwise
* Start up and tear down, i.e. making the mongo connections, closing later
* Error handling, such as if `mongod` dies or the connection is lost