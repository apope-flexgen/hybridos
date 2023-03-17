#!/bin/bash

name=web_ui
type=node
image=
artifacts=(build.tar.gz)
submodules=()
tests=()

function prebuild()
{
    rm -rf build/ # node_modules/
}

function build() # build.sh passes build mode as $1
{
    # npm ci --production
    npm prune --production
    
    npm run build

    if [ ! -d "build" ]; then echo -e "##### ERROR: web_ui build failed."; exit 1; fi # build failed, exit
    files=($(ls build)) # capture build output

    mkdir -p build/$1/build/
    for i in "${files[@]}"; do
        cp -rf build/$i build/$1/build/
    done

    cd build/$1
    tar -czvf build.tar.gz build/
    rm -rf build/
    cd ../../

    cp web_ui.spec build/$1
}

function postbuild()
{
    return 0
}

function install() # build.sh passes build mode as $1
{
    echo "creating /usr/local/bin/web_ui"
    sudo mkdir -p /usr/local/bin/web_ui

    sudo cp build/$1/build.tar.gz /usr/local/bin/web_ui/
    sudo tar -xzvf /usr/local/bin/web_ui/build.tar.gz -C /usr/local/bin/web_ui/
    sudo rm -f /usr/local/bin/web_ui/build.tar.gz
}

function uninstall()
{
    echo "removing /usr/local/bin/web_ui"
    sudo rm -rf /usr/local/bin/web_ui
}

function test() # test.sh passes test binary as $1
{
    return
}