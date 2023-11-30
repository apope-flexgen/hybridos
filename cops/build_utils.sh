#!/bin/bash

name=cops
type=go
image=
artifacts=(cops cops.service)
submodules=()
tests=()

function clean()
{
    return
}

function prebuild()
{
    return
}

function build() # build.sh passes build mode as $1
{
    # we need to explicitly tell package_utility to use go modules since we're using the monomodule go.mod, 
    # so it won't find the go.mod in the expected location (same location as the build_utils.sh)
    export GO111MODULE=on
    go mod tidy
    
    # Make sure cops source flies are in GO_PATH
    ln -sfn $(pwd)/src $GO_PATH/$name

    go build -ldflags "$go_linker_flags" -o $build_output/$name ./src/
    if [ "$?" -eq 0 ]; then echo "build() - $name built"; else echo "build() - $name not built"; fi
}

function postbuild()
{
    # Copy service into build folder
    cp cops.service $build_output
}

function install() # build.sh passes build mode as $1
{
    sudo cp $build_output/$name /usr/local/bin/
    if [ "$?" -eq 0 ]; then echo "install() - $name installed"; else echo "install() - $anme not installed"; fi
}

function uninstall()
{
    sudo rm -rf /usr/local/bin/$name
}

function test() # test.sh passes test binary as $1
{
    return
}