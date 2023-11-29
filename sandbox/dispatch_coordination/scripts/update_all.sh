#!/bin/bash

: '
Runs the updater tool against all the configurations inside of config/ as well as select individual files
inside of unit_test_configs. This was used in pursuit of another ticket and is only intended to prevent
having to ctrl-f and modify a ton of configs everytime there is a minor rename to anything more
complicated such as a bit value being shifted.

At present it is hardcoded to go from 11.3.0 to 11.4.0

Impacted files:
.
├── config/**/*.json
└── unit_test_configs
    └── site_controller
        ├── assets/**/*.json
        └── variables/**/*.json
'

# TODO add a dry run
# TODO it might be useful to allow params for other versions.
# TODO support more than just sitecontroller files.

# vars
update_path="/home/hybridos/git/update"
sandbox_path="/home/hybridos/git/hybridos/sandbox/dispatch_coordination"

# functions
# @param $1 config_path
function runUpdate() {
    config_path=$1
    origin=$(pwd)
    cd $update_path
        cmd=" \
            go run ${update_path}/cmd/server_update/main.go \
            -inputs=${config_path} \
            -outputs=${origin}/tmp/output \
            -from=11.3.0 \
            -to=11.4.0 \
            -product=sitecontroller \
        "
        output=$(eval $cmd 2>&1)
    cd $origin
}

function mainConfigs() {
    echo "Updating ./configs"
    configs=$(dirname $(dirname $(find "$(pwd)/config" -type f | grep site_controller/variables.json)))

    for config in $configs; do
        echo "-- ${config}"

        runUpdate $config
        cp -r ./tmp/output/* ${config_path}/

        rm -r ./tmp/output
    done
}

function unitTestConfigs() {
    echo "Updating ./unit_test_configs"

    variables_files=$(find "$(pwd)/unit_test_configs/site_controller/variables" -type f)
    assets_files=$(find "$(pwd)/unit_test_configs/site_controller/assets" -type f)

    # prepare input configs, extended from config_dev
    mkdir -p tmp/input
    cp -r \
        /home/hybridos/git/hybridos/sandbox/dispatch_coordination/config/config_dev/config/site/site-controller/config \
        tmp/input

    # variables.json
    input_location=$(pwd)/tmp/input/config/site_controller/variables.json
    output_location=$(pwd)/tmp/output/site_controller/variables.json
    for config_source in $variables_files; do
        echo "-- ${config_source}"

        cp $config_source $input_location
        runUpdate $(pwd)/tmp/input/config
        cp $output_location $config_source

        rm -r tmp/output
    done

    # assets.json
    input_location=$(pwd)/tmp/input/config/site_controller/assets.json
    output_location=$(pwd)/tmp/output/site_controller/assets.json
    for config_source in $assets_files; do
        echo "-- ${config_source}"

        cp $config_source $input_location
        runUpdate $(pwd)/tmp/input/config
        cp $output_location $config_source

        rm -r tmp/output
    done
}

function main() {
    return_to=$(pwd)
    cd $sandbox_path
        mainConfigs
        unitTestConfigs
        rm -r tmp
    cd $return_to
}

main
