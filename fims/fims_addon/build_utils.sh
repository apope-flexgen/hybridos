#!/bin/bash

# build.sh passes build mode as $1, except for uninstall() and test()
# add '|| error_trap "error_message"' to critical sections

name=fims_addon
type=cpp
gcc=11
image=
artifacts=(fims.node)
submodules=()
tests=()

function prebuild()
{
    echo -e "rm -rf build/ node_modules/"
    rm -rf build/ node_modules/ || error_trap "failed to remove build/ and node_modules/."

    # cpp source files must be copied into current directory index
    cp -fv ../src/{libfims.cpp,libaes.cpp,napifims.cpp} .
    cp -fv ../include/{fims.h,libfims.h,libfims_internal.hpp,libaes.h,napifims.h,fps_utils.h} .
    cp -fv /usr/local/include/cjson/{cJSON.c,cJSON.h} .
}

function build()
{
    #if running as root, --unsafe-perm is needed
    if [ $(id -u)  == '0' ]
        then
            npm install --unsafe-perm || error_trap "build failure, please see errors above."
        else
            npm install || error_trap "build failure, please see errors above."
    fi
}

function postbuild()
{
    rm -rf ./*.c ./*.cpp ./*.h ./*.hpp # clean up

    cp -v build/Release/fims.node "$build_output"
}

function install()
{
    echo -e "sudo cp -v $build_output/fims.node $LIB_DIR"
    sudo sh -c "cp -fv $build_output/fims.node $LIB_DIR" || error_trap "install failure, please see errors above."
}

function uninstall()
{
    echo -e "sudo rm -rf $LIB_DIR/fims.node"
    sudo sh -c "rm -rf $LIB_DIR/fims.node"
}

function test() # test.sh passes test binary as $1
{
    return
}

# module-specific functions