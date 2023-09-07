#!/bin/bash

name=fims_relay
type=cpp # change type to node when new CI/CD is ready.
gcc=11
image=flexgen/fims_relay
artifacts=(fims_relay fims_relay.service)
submodules=()
tests=()

function prebuild()
{
    rm -rf build node_modules dist
}

function build()
{
    echo "------------------- starting build ($1) -------------------------"

    npm install --python=python2.7

    # create binary package
    npx -y @nestjs/cli build
    node_modules/pkg/lib-es5/bin.js --targets node16-linux-x64 dist/main.js -o ./build/release/fims_relay --config pkg.json
    cp fims_relay.service build/$1
}

function postbuild()
{
    cp rpm.txt $build_output
}

function install()
{
    sudo cp build/$1/fims_relay /usr/local/bin/

    echo "creating /var/log/fims_relay"
    sudo mkdir -p /var/log/fims_relay
    sudo chown -R $(whoami):$(whoami) /var/log/fims_relay/
}

function uninstall()
{
   echo "removing /usr/local/bin/fims_relay"
   sudo rm -rf /usr/local/bin/fims_relay

   echo "removing /var/log/fims_relay"
   sudo rm -rf /var/log/fims_relay
}

function test()
{
    return 0
}

# optionally add module-specific functions below
