#!/bin/bash

name=dbi
type=go
image=
artifacts=(dbi dbi.service)
submodules=()
tests=()

function prebuild()
{
    rm -rf build node_modules/ dbi *-linux *-macos *-win.exe
}

function build() # build.sh passes build mode as $1
{
    npm ci --production
    npm prune --production
    npm link fims

    # create binary package
    node_modules/pkg/lib-es5/bin.js --targets node10-linux-x64 src/dbi.js -o build/$1/dbi
    cp dbi.service $build_output
}

function postbuild()
{
    return 0
}

function install() # build.sh passes build mode as $1
{
    echo "creating /usr/local/bin/dbi"
    sudo mkdir -p /usr/local/bin/dbi
    sudo cp build/$1/dbi /usr/local/bin/dbi/

    echo "creating /var/log/dbi"
    sudo mkdir -p /var/log/dbi
    sudo chown -R $(whoami):$(whoami) /var/log/dbi/
}

function uninstall()
{
    echo "removing /usr/local/bin/dbi"
    sudo rm -rf /usr/local/bin/dbi

    echo "removing /var/log/dbi"
    sudo rm -rf /var/log/dbi
}

function test() # test.sh passes test binary as $1
{
    return
}