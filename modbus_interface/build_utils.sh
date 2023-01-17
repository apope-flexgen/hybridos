#!/bin/bash

name=modbus_interface
type=cpp
gcc=9.3.1
image=
artifacts=(modbus_client modbus_server modbus_client@.service modbus_server@.service)
submodules=()
tests=()

function prebuild()
{
    make clean
}

function build() # build.sh passes build mode as $1
{
    make BUILD_MODE=$1
}

function postbuild()
{
    # copy service files
    cp modbus_client@.service $build_output
    cp modbus_server@.service $build_output
}

function install() # build.sh passes build mode as $1
{
    sudo make BUILD_MODE=$1 install
}

function uninstall()
{
    sudo make uninstall
}

function test() # test.sh passes test binary as $1
{
    return
}
