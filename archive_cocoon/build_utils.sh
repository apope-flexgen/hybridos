#!/bin/bash

name=archive_cocoon
type=go
image=flexgen/archive_cocoon
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

    go mod tidy

    go build -ldflags "$go_linker_flags" -o $build_output/$name ./src/
    if [ "$?" -eq 0 ]; then echo "build() - $name built"; else echo "build() - $name not built"; fi
    echo # empty line between previous output and next output

    echo "vetting $name..."
    go vet ./src/
}

function postbuild()
{
    cp archive_cocoon.service $1/
}

function install()
{
    sudo cp $build_output/$name /usr/local/bin/
    if [ "$?" -eq 0 ]; then echo "install() - $name installed"; else echo "install() - $name not installed"; fi
}

function uninstall()
{
    sudo rm /usr/local/bin/$name
}

function test() # test.sh passes test binary as $1
{
    return 0
}

# module-specific functions
