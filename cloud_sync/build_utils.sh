#!/bin/bash

name=cloud_sync
type=go
image=flexgen/cloud_sync
artifacts=()
submodules=()
tests=()

function prebuild()
{
    rm -rf build/
    return 0
}

function build()
{
    echo "##### BUILD"
    echo "building $name..."
    go build -ldflags "$go_linker_flags" -o $build_output/$name ./src/
    if [ "$?" -eq 0 ]; then echo "build() - $name built"; else echo "build() - $name not built"; fi
    echo

    echo "vetting $name..."
    go vet ./src/
}

function postbuild()
{
    cp cloud_sync.service $build_output
}

function install()
{
    sudo mkdir -p /usr/local/bin # make sure binary directory exists

    sudo cp $build_output/$name /usr/local/bin/
    if [ "$?" -eq 0 ]; then echo "install() - $name installed"; else echo "install() - $name not installed"; fi
}

function uninstall()
{
    sudo rm -rf /usr/local/bin/$name
}

function test() # test.sh passes test binary as $1
{
    return
}

# module-specific functions
