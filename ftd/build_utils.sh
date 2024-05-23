#!/bin/bash

name=ftd
type=go
image=
artifacts=()
submodules=()
tests=()

function prebuild()
{
    rm -rf build/
}

function build()
{
    echo "##### BUILD"
    echo "building $name..."

    # we need to explicitly tell package_utility to use go modules since we're using the monomodule go.mod, 
    # so it won't find the go.mod in the expected location (same location as the build_utils.sh)
    export GO111MODULE=on
    go mod tidy

    go build -ldflags "$go_linker_flags" -o $build_output/$name ./src/
    if [ "$?" -eq 0 ]; then echo "build() - $name built"; else echo "build() - $name not built"; fi
    echo

    echo "vetting $name..."
    go vet ./src/

    # build fims_format_scan
    ./cmd/fims_format_scan/build.sh $build_output/fims_format_scan

    # build ftd_config_gen
    ./cmd/ftd_config_gen/build.sh $build_output/ftd_config_gen
}

function postbuild()
{  
    cp ftd.service "$build_output"
}

function install()
{
    sudo cp $build_output/$name $BIN_DIR/
    sudo cp $build_output/fims_format_scan $BIN_DIR/
    sudo cp $build_output/ftd_config_gen $BIN_DIR/
}

function uninstall()
{
    sudo rm -f $BIN_DIR/$name
    sudo rm -f $BIN_DIR/fims_format_scan
    sudo rm -f $BIN_DIR/ftd_config_gen
}

function test() # test.sh passes test binary as $1
{
    return 0
}

# module-specific functions