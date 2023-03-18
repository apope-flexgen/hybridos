#!/bin/bash

name=scheduler
type=go
image=
artifacts=(scheduler scheduler.service)
submodules=()
tests=(scheduler)

function prebuild()
{
    rm -rf build/
    return
}

function build() # build.sh passes build mode as $1
{
    echo -e "##### BUILD"
    echo -e "building $name..."
    # TODO: update to use dynamic path
    # ./package_utility/version.sh # run version before build
    # Make sure scheduler source flies are in GO_PATH
    ln -sfn $(pwd)/src $GO_PATH/$name

    # Go's embed package does not support relative directories, requiring view of the embedded file location in src/
    mkdir -p src/versioning
    mv GIT_* src/versioning

    if [ "$1" = "test" ]; then # Build test binary if in test mode
        go test -o $build_output/$name ./src/
    else # Otherwise test files aren't built
        go build -o $build_output/$name ./src/
    fi
    
    if [ "$?" -eq 0 ]; then echo "build() - $name built"; else echo "build() - $name not built"; fi
}

function postbuild()
{
    cp scheduler.service $1/
}

function install() # build.sh passes build mode as $1
{
    sudo mkdir -p /usr/local/bin # Make a directory for the scheduler binaries

    sudo cp $build_output/$name /usr/local/bin/
    if [ "$?" -eq 0 ]; then echo "install() - $name installed"; else echo "install() - $name not installed"; fi
}

function uninstall()
{
    sudo rm -rf /usr/local/bin/$name
}

function test() # test.sh passes test binary as $1
{
    ./build/test/$1
}