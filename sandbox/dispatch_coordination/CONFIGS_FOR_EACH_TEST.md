config use-cases used by site_controller pytests 
| Config Use-case Name       | pytests that Use the Config Use-case |
| -------------------------- | ------------------------------------ |
| config_dev                 | pytest -vv -k "not agt and not manual and not test_ranged and not test_traditional" |
| templating_error_testing   | pytest -vv -k "test_ranged or test_traditional" |
| testing_AGT_sequences      | pytest -vv -k agt |
| testing_manual_mode        | pytest -vv -k manual |

scheduler pytests can be run with any config, since the configs are overwritten by the pytests

config_fleetman_dev is not used by any pytests, it currently exists solely for manual testing