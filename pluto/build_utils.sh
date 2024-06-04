#!/bin/bash

# use $build_output for artifacts

name=pluto
workdir=
type=go
gcc=
image=
artifacts=(pluto pluto.service)
submodules=()
tests=()

function prebuild()
{
    return 0
}

function build()
{
     # we need to explicitly tell package_utility to use go modules since we're using the monomodule go.mod, 
    # so it won't find the go.mod in the expected location (same location as the build_utils.sh)
    export GO111MODULE=on
    go mod tidy
    
    # Make sure cops source flies are in GO_PATH
    ln -sfn $(pwd)/src $GO_PATH/$name

    go build -ldflags "$go_linker_flags" -o $build_output/$name ./cmd/
    if [ "$?" -eq 0 ]; then echo "build() - $name built"; else echo "build() - $name not built"; fi
}

function postbuild()
{
    mkdir -p $build_output
    cp pluto.service $build_output
}

function install()
{
    sudo cp $build_output/$name /usr/local/bin/
    if [ "$?" -eq 0 ]; then echo "install() - $name installed"; else echo "install() - $anme not installed"; fi
}

function uninstall()
{
   sudo rm -rf /usr/local/bin/$name
}

function test()
{
    return 0
}