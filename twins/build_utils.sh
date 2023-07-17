#!/bin/bash

name=twins
type=go
image=
artifacts=(twins.service twins)
submodules=()
tests=()



function prebuild()
{
    return
}

function build() # build.sh passes build mode as $1
{
    go build -o build/release/twins twins || error_trap "failed to build twins"

    cp twins.service build/$1
}

function install() # build.sh passes build mode as $1
{
    sudo cp build/release/twins /usr/local/bin || error_trap "failed to install twins"
}

function uninstall()
{
    sudo rm /usr/local/bin/twins
}

function postbuild()
{
    #build/release
    cp config/twins_dflt.json build/release
    return
}

function test() # test.sh passes test binary as $1
{
    return
}