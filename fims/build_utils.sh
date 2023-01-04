#!/bin/bash

name=fims
type=cpp
gcc=9.3.1
image=
artifacts=(fims_echo fims_listen fims_send fims_server fims_trigger libfims.so)
submodules=(go_fims pyfims fims_addon node_fims)
tests=(fims_server)

function prebuild()
{
    make clean
}

function build() # build.sh passes build mode as $1
{
    make BUILD_MODE="$mode_arg"
}

function postbuild()
{
    cp include/fims.h "$build_output"
    cp include/fps_utils.h "$build_output"
    cp include/libfims.h "$build_output"
    cp include/defer.hpp "$build_output"
    cp fims.service "$build_output"
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
    ./build/test/$1 --gtest_output="xml:./tests/$1.xml"
}

# module-specific functions