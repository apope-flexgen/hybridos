#!/bin/bash

name=node_fims
type=node
image=
artifacts=(fimsListener.js node_modules.tar.gz)
submodules=()
tests=()

function prebuild()
{
    echo 
    if [ $EUID -eq 0 ] || [ "$ENVIRONMENT" == "Jenkins" ]; then
        rm -rf node_modules/;
    else
        sudo rm -rf node_modules/;
    fi
}

function build() # build.sh passes build mode as $1
{
    if [ $EUID -eq 0 ] || [ "$ENVIRONMENT" == "Jenkins" ]; then
        npm install --unsafe-perm || error_trap "build failure, please see errors above."
        npm link;
    else
        npm install || error_trap "build failure, please see errors above."
        sudo npm link;
    fi
}

function postbuild()
{
    cp -vf fimsListener.js package.json $build_output

    tar -czf "$build_output/node_modules.tar.gz" node_modules/ || error_trap "failed to archive node_modules to $build_output."
}

function install() # build.sh passes build mode as $1
{
    return
}

function uninstall()
{
    echo "removing symlink /usr/lib/node_modules/fims"
    sudo sh -c "rm /usr/lib/node_modules/fims"
}

function test() # test.sh passes test binary as $1
{
    return
}
