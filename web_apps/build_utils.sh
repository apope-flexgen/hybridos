#!/usr/bin/env bash

name=web_apps
type=cpp # TODO: Modify type to node when Jenkins can detect gcc version without type needing to be cpp.
gcc=11 # NOTE: zeromq has a gcc dependency.
image=
artifacts=(build.tar.gz web_server.service web_server)
submodules=()
tests=(true)

function prebuild()
{
    rm -rf build
    mkdir -p $build_output
}

function build() # build.sh passes build mode as $1
{
    pnpm_install_hybridos_all "$1"
    build_web_server "$1"
    build_web_ui "$1"
}

function postbuild()
{
    echo "------------------- starting hybridos_web_server postbuild ($1) -------------------------"
    pwd
    return 0
}

function install()
{
    install_web_ui "$1"
    install_web_server "$1"
}

function uninstall()
{
    uninstall_web_server "$1"
    uninstall_web_ui "$1"
}

function test()
{
    echo "------------------- starting hybridos_web_ui tests ($1) -------------------------"
    # Test web_ui
    cd packages/hybridos/ui && npm run cy:ci

    echo "------------------- starting hybridos_web_server tests ($1) -------------------------"
    # Test web_server
    # TODO: Use npm run test:all.
    cd ../server && npm run test:e2e
}

function pnpm_install_hybridos_all()
{
    echo "------------------- starting pnpm_install_hybridos_all ($1) -------------------------"

    # FIXME: --prod excludes all dev dependencies, including those required for building.
    # while that seems to work fine with the vite build, that breaks webpack in the server.
    # for now, we'll just install all dependencies, but we should find a better solution.
    pnpm install

    # if [ "$1" == "release" ]; then
    #     pnpm install --frozen-lockfile --prod
    # else
    #     pnpm install --frozen-lockfile
    # fi
}

function build_web_server()
{
    echo "------------------- starting hybridos_web_server build ($1) -------------------------"
    root_dir=$(pwd)
    config_dir="/usr/local/etc/config/web_server"

    cd packages/hybridos/server
    
    # create binary package
    pnpm run build:prod
    cp ./executable/web_server $build_output
    cp web_server.service $build_output

    cd $root_dir
}

function build_web_ui()
{
    echo "------------------- starting hybridos_web_ui build ($1) -------------------------"
    root_dir=$(pwd)

    cd packages/hybridos/ui

    pnpm run build
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

    cp ./build/$1/build.tar.gz $build_output
    cp web_ui.spec $build_output

    cd $root_dir
}

function install_web_server()
{
    echo "------------------- starting hybridos_web_server install ($1) -------------------------"

    sudo cp $build_output/web_server /usr/local/bin/

    echo "creating /var/log/web_server"
    sudo mkdir -p /var/log/web_server
    sudo chown -R $(whoami):$(whoami) /var/log/web_server/
}

function install_web_ui()
{
    echo "------------------- starting hybridos_web_ui install ($1) -------------------------"

    echo "creating /usr/local/bin/web_ui"
    sudo mkdir -p /usr/local/bin/web_ui

    sudo cp $build_output/build.tar.gz /usr/local/bin/web_ui/
    sudo tar -xzvf /usr/local/bin/web_ui/build.tar.gz -C /usr/local/bin/web_ui/
    sudo rm -f /usr/local/bin/web_ui/build.tar.gz
}

function install_web_ui()
{
    echo "------------------- starting hybridos_web_ui install ($1) -------------------------"

    echo "creating /usr/local/bin/web_ui"
    sudo mkdir -p /usr/local/bin/web_ui

    sudo cp $build_output/build.tar.gz /usr/local/bin/web_ui/
    sudo tar -xzvf /usr/local/bin/web_ui/build.tar.gz -C /usr/local/bin/web_ui/
    sudo rm -f /usr/local/bin/web_ui/build.tar.gz
}

function uninstall_web_server()
{
    echo "------------------- starting hybridos_web_server uninstall ($1) -------------------------"

    echo "removing /usr/local/bin/web_server"
    sudo rm -rf /usr/local/bin/web_server

    echo "removing /var/log/web_server"
    sudo rm -rf /var/log/web_server
}

function uninstall_web_ui()
{
    echo "------------------- starting hybridos_web_ui uninstall ($1) -------------------------"

    echo "removing /usr/local/bin/web_ui"
    sudo rm -rf /usr/local/bin/web_ui
}