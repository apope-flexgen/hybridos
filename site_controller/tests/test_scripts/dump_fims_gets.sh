#!/bin/bash

# Dumps the responses from fims gets to a variety of uris into the specified file
# It is not comprehensive, it does not check every valid uri
# Once you have the dump from two different builds of site controller, you can diff them
# Assumes that the given file either doesn't exist yet or is empty

uris=(
    #/assets
    /assets
    #/assets/type
    /assets/ess
    /assets/feeders
    /assets/generators
    /assets/solar
    #/assets/type/summary
    /assets/ess/summary
    /assets/feeders/summary
    /assets/generators/summary
    /assets/solar/summary
    #/assets/type/type_instance
    /assets/ess/ess_1
    /assets/feeders/feed_1
    /assets/generators/gen_1
    /assets/solar/solar_1
    #/assets/type/summary/variable
    /assets/ess/summary/name
    /assets/ess/summary/ess_average_soc
    /assets/ess/summary/grid_forming_voltage_slew
    #/assets/type/type_instance/variable
    /assets/feeders/feed_1/name
    /assets/feeders/feed_1/breaker_status
    /assets/feeders/feed_1/breaker_open
    
    #/site
    /site
    /site/summary
    /site/operation
    /site/configuration
    /site/cops
    /site/input_sources
    #/site with 3 frags
    /site/summary/site_state
    /site/operation/name
    /site/configuration/reserved_bool_11
    /site/input_sources/local

    #/features
    /features
    /features/summary
    /features/active_power
    /features/reactive_power
    /features/standalone_power
    /features/site_operation
    #/features with 3 frags
    /features/summary/name
    /features/active_power/ess_kW_cmd
    /features/reactive_power/reactive_setpoint_mode_enable_flag
    /features/standalone_power/active_power_closed_loop_enable
    /features/site_operation/watchdog_enable

)


for uri in "${uris[@]}"; do
    echo "get at " $uri
    fims_send -m get -u $uri -r /me
    echo
done