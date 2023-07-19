#!/bin/bash

name=go_metrics
type=go
image=
artifacts=()
submodules=()
tests=()

function prebuild()
{
    rm -rf build/
}

function build()
{
    echo "##### BUILD"
    echo "building $name..."
    #sudo cp -a fims /usr/lib/golang/src
    mkdir -p $build_output
    go build -ldflags "$go_linker_flags" -o $build_output/$name ./src/
    if [ "$?" -eq 0 ]; then echo "build() - $name built"; else echo "build() - $name not built"; fi
    echo

    #echo "vetting $name..."
    #go vet ./src/

}

function postbuild()
{  
    cp go_metrics@.service "$build_output"
}

function install()
{
    sudo cp $build_output/$name $BIN_DIR/
}

function uninstall()
{
    sudo rm -rf $BIN_DIR/$name
}

function test() # test.sh passes test binary as $1
{
    return 0
}

# module-specific functions
