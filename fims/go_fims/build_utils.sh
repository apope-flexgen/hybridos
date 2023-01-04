#!/bin/bash

name=go_fims
type=go
image=
artifacts=(fims.go msg.go verification.go)
submodules=()
tests=()

function prebuild()
{
    return 0
}

function build() # build.sh passes build mode as $1
{
    return 0
}

function postbuild()
{
    cp -v fims.go "$build_output"
    cp -v msg.go "$build_output"
    cp -v verification.go "$build_output"
}

function install() # build.sh passes build mode as $1
{
    # sudo sh -c "ln -sfT $(pwd) $GO_ROOT/fims" || error_trap "failed to install $name files to $GO_ROOT/fims."
    # sudo sh -c "ln -sfT $(pwd) $GO_PATH/fims" || error_trap "failed to install $name files to $GO_PATH/fims."
    
    sudo sh -c "mkdir -p $GO_ROOT/fims" || error_trap "failed to create $GO_ROOT/fims."
    sudo sh -c "cp -fv fims.go msg.go verification.go $GO_ROOT/fims" || error_trap "failed to install $name files to $GO_ROOT/fims."

    if [ "$ENVIRONMENT" == "Jenkins" ]; then # build is occuring in Jenkins
        sh -c "mkdir -p $GO_PATH/fims" || error_trap "failed to create $GO_PATH/fims."
        sh -c "cp -fv fims.go msg.go verification.go $GO_PATH/fims" || error_trap "failed to install $name files to $GO_PATH/fims."
    else
        sudo sh -c "mkdir -p $GO_PATH/fims" || error_trap "failed to create $GO_PATH/fims."
        sudo sh -c "cp -fv fims.go msg.go verification.go $GO_PATH/fims" || error_trap "failed to install $name files to $GO_PATH/fims."
    fi
}

function uninstall()
{
    sudo sh -c "rm -rf $GO_ROOT/fims $GO_PATH/fims"
}

function test() # test.sh passes test binary as $1
{
    return 0
}