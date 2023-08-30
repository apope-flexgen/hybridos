#!/bin/bash

name=site_controller
type=cpp
image=
artifacts=(site_controller site_controller.service)
submodules=()
tests=(site_controller)
# GNU flag to build in parallel based on the number of cores available
export MAKEFLAGS="-j $(grep -c ^processor /proc/cpuinfo)"

function prebuild()
{
    make clean
}

function build() # build.sh passes build mode as $1
{
    mkdir -p build/$1_obj/
    mv git*.o build/$1_obj/

    make BUILD_MODE=$1
    
    cp site_controller.service build/$1
}

function postbuild()
{
    return 0
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
    ./build/test/$1 --gtest_output="xml:./tests/$1.xml"
}
