#!/bin/bash

# build.sh passes build mode as $1, except for uninstall() and test()
# add '|| error_trap "error_message"' to critical sections

name=pyfims
type=python
image=
artifacts=()
submodules=()
tests=()

function prebuild()
{
    echo -e "rm -rf build/ *.cpp"
    rm -rf build/ ./*.cpp || error_trap "failed to remove build/."
}

function build()
{
    CPPFLAGS="-std=c++11" python3 setup.py build_ext
}

function postbuild()
{
    return
}

function install()
{
    sudo sh -c 'pip3 install . --upgrade'
}

function uninstall()
{
    sudo sh -c 'pip3 uninstall -y pyfims'
}

function test() # test.sh passes test binary as $1
{
    return
}

# module-specific functions
