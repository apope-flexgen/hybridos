#!/bin/bash

name=gcom_dnp3_interface
type=cpp
gcc=9.3.1
image=
artifacts=(gcom_dnp3_server gcom_dnp3_client)
submodules=()
tests=()

function prebuild()
{
    make clean
}

function build() # build.sh passes build mode as $1
{
    cd TMW
    make
    cd ..
    make BUILD_MODE="$mode_arg"
}

function postbuild()
{
    cp gcom_dnp3_server@.service "$build_output"
    cp gcom_dnp3_client@.service "$build_output"
}

function install() # build.sh passes build mode as $1
{
    sudo make BUILD_MODE="$mode_arg" install
}

function uninstall()
{
    sudo make uninstall
}

function test() # test.sh passes test binary as $1
{
   ls 
}

# module-specific functions
