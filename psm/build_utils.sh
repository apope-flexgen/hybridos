#!/bin/bash

name=psm
type=go
image=
artifacts=(psm.service psm)
submodules=()
tests=()



function prebuild()
{
    return
}

function build() # build.sh passes build mode as $1
{
    go build -o build/release/psm psm || error_trap "failed to build psm"

    cp psm.service build/$1
}

function install() # build.sh passes build mode as $1
{
    sudo cp build/release/psm /usr/local/bin || error_trap "failed to install psm"
}

function uninstall()
{
    sudo rm /usr/local/bin/psm
}

function postbuild()
{
    #build/release
    cp config/psm_dflt.json build/release
    return
}

function test() # test.sh passes test binary as $1
{
    return
}