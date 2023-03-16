# COPS (Central Operating Process Supervisor)
[Original Design Review](./docs/Design_Reviews/DR_InitialDesign)

To run COPS, start HybridOS along with any other processes that you have configured COPS to monitor in cops.json (in the config repo). Then, make sure the config path to the cops config folder is correct in cops_start.sh. Finally, run `./cops_start.sh`.