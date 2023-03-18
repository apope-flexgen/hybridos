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
    go build -ldflags "$go_linker_flags" -o $build_output/$name ./src/
    if [ "$?" -eq 0 ]; then echo "build() - $name built"; else echo "build() - $name not built"; fi
    echo

    echo "vetting $name..."
    go vet ./src/
}

function postbuild()
{  
    cp ftd.service "$build_output"
}

function install()
{
    sudo cp $build_output/$name $BIN_DIR/
}

function uninstall()
{
    sudo rm -rf $BIN_DIR/ftd
}

function test() # test.sh passes test binary as $1
{
    return 0
}

# module-specific functions