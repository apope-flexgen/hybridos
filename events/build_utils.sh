#!/bin/bash

name=events
type=node
image=
artifacts=(events events.service)
submodules=()
tests=()


function prebuild()
{
    rm -rf build node_modules/ events *-linux *-macos *-win.exe
}

function build() # build.sh passes build mode as $1
{
    npm ci --production
    npm prune --production
    npm link fims

    # create binary package
    node_modules/pkg/lib-es5/bin.js --targets node16-linux-x64 src/events.js -o $build_output/events
    cp events.service $build_output
}

function postbuild()
{
    return 0
}

function install() # build.sh passes build mode as $1
{
    sudo rm -rf /usr/local/bin/events
    sudo cp $build_output/events /usr/local/bin/
}

function uninstall()
{
    echo "removing /usr/local/bin/events"
    sudo rm -rf /usr/local/bin/events
}

function test() # test.sh passes test binary as $1
{
    return
}