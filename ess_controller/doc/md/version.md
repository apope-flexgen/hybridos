# Version

Before running `./git_checkout.sh`, make sure you have the `scripts` repo installed. If `scripts` is not already installed, refer to the `Git` section in the [scripts](https://github.com/flexgen-power/scripts) repo.  

----

## Checkout Target Branch
Go to the `scripts` repo
`$ cd scripts`

Then, run `./git_checkout.sh` to switch to the target branch for each repository as listed below.  

Note: `git_checkout.sh` will look for `repo.txt` stored in a config repo. If the shell script does not work, you can manually checkout the branch for each repo. To do that, you can:
* Go to the target repo  
`$ cd [repo]`
* Grab the files and related contents  
`$ git fetch`
* Switch to the target branch  
`$ git checkout [name_of_target_branch]`

----

## List of Versions
The following information below shows the ESS Controller version and the list of git repositories and branch used to run with the ESS Controller.
### release/v1.0.0

```
 REPO                 BRANCH                                   HASH       STATUS
--------------------------------------------------------------------------------
 fims                 bugfix/handle_eintr_for_gdb              2c2ce45
 node_fims            v1.5.0                                   1869e8e
 hybridos_controller  v1.6.0                                   1be8f5c
 modbus_interface     release/v1.5.0                           6c300dc
 mcp                  v1.4.0                                   dd6d773
 storage              v1.5.0                                   e321b6a
 metrics              v1.5.0                                   d472363
 events               v1.5.0                                   ce54d07
 web_server           test/ess_controller                      5f9209f
 web_ui               dev                                      ab17f04
 go_fims              v1.5.0                                   909e65d
 twins                feature/adding-pcs-and-bms-components    e65b7b0
 config               v1.5.2-1-g1cc9a92                        1cc9a92 
 scripts              dev                                      e32bb74
 ```
