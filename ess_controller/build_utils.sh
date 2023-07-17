#!/bin/bash

name=ess_controller
type=cpp
image=
artifacts=(ess_controller ess_controller@.service gpio_controller gpio_controller.service modprobe_i2c.service libess.so libessfunc.so)
submodules=()
tests=(ess_controller)

function prebuild()
{
 if [ ! -d /usr/local/include/spdlog ]; then
    echo "installing spdlog"
    sudo cp -a spdlog_inc/spdlog /usr/local/include/
 fi
 echo default prebuild
}

function postbuild()
{
 echo default postbuild
}

function clean()
{
    make clean
}

function build() # build.sh passes build mode as $1
{
    make BUILD_MODE=$1

    cp ess_controller@.service build/$1
    cp gpio_controller.service build/$1
    cp modprobe_i2c.service build/$1
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