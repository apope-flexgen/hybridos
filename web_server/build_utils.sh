#!/bin/bash

name=web_server
type=node
image=
artifacts=(web_server web_server.service)
submodules=()
tests=()

function prebuild()
{
    rm -rf build node_modules/ web_server *-linux *-macos *-win.exe
}

function build() # build.sh passes build mode as $1
{
    # TODO: randomize this string
    echo "JWT_SECRET='somethingSuperSecret'" >> .env

    npm ci --production
    npm prune --production
    npm link fims

    # create binary package
    node_modules/pkg/lib-es5/bin.js --targets node16-linux-x64 src/web_server.js -o build/$1/web_server --config pkg.json
    cp web_server.service build/$1
}

function postbuild()
{
    return 0
}

function install() # build.sh passes build mode as $1
{
    sudo cp build/$1/web_server /usr/local/bin/

    echo "creating /var/log/web_server"
    sudo mkdir -p /var/log/web_server
    sudo chown -R $(whoami):$(whoami) /var/log/web_server/
}

function uninstall()
{
    echo "removing /usr/local/bin/web_server"
    sudo rm -rf /usr/local/bin/web_server

    echo "removing /var/log/web_server"
    sudo rm -rf /var/log/web_server
}

function test() # test.sh passes test binary as $1
{
    return
}