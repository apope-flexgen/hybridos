#!/bin/bash

# use $build_output for artifacts

name=overwatch
workdir=
type=go
gcc=
image=
artifacts=(overwatch overwatch.service)
submodules=()
tests=()

function prebuild()
{
    return 0
}

function build()
{
    go mod tidy
    go build -ldflags "$go_linker_flags" -o $cwd/$name ./src/
}

function postbuild()
{
    mkdir -p $build_output
    cp overwatch overwatch.service $build_output
}

function install()
{
    sudo cp overwatch /usr/local/bin
}

function uninstall()
{
    sudo rm /usr/local/bin/overwatch
}

function test()
{
    return 0
}