#!/bin/bash

name=metrics
type=node
image=
artifacts=(metrics metrics@.service)
submodules=()
tests=()

function prebuild()
{
    rm -rf build node_modules/ metrics *-linux *-macos *-win.exe
}

function build() # build.sh passes build mode as $1
{
    npm ci --production
    npm prune --production
    npm link fims

    # create binary package
    node_modules/pkg/lib-es5/bin.js --targets node16-linux-x64 src/metrics.js -o $build_output/metrics || error_trap "failed to generate metrics binary"
}

function postbuild()
{
    cp metrics@.service $build_output
}

function install() # build.sh passes build mode as $1
{
    sudo cp -rf $build_output/metrics /usr/local/bin/
}

function uninstall()
{
    echo "removing /usr/local/bin/metrics"
    sudo rm -rf /usr/local/bin/metrics
}

function test() # test.sh passes test binary as $1
{
    return
}