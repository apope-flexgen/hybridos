#!/bin/bash

# globals
cwd=$(pwd)

# installation directories
BIN_DIR=/usr/local/bin
LIB_DIR=/usr/local/lib
INCLUDE_DIR=/usr/local/include
CONFIG_DIR=/usr/local/etc/config
SERVICE_DIR=/usr/lib/systemd/system
NODE_DIR=/usr/lib/node_modules
GO_ROOT=/usr/lib/golang/src

GO_PATH=$(readlink -f ~/)
GO_PATH+="/go/src"

GLOBAL_PATH=$PATH

ERROR='\033[0;31m'
WARNING='\033[0;33m'
SUCCESS='\033[0;32m'
NONE='\033[0m' # No Color

function error_trap() # error message passed as $1
{
    echo -e "\n${ERROR}##### ERROR: $1${NONE}\n"
    exit 1 # hard stop exceution
}

function warning_trap()
{
    echo -e "\n${WARNING}##### WARNING: $1${NONE}\n"
}

function success_trap()
{
    echo -e "\n${SUCCESS}##### SUCCESS: $1${NONE}\n"
}

function gcc_setup()
{
    # extract the major version of gcc to set the path
    IFS_bak=$IFS  # backup the existing IFS
    IFS='.'
    read -ra gcc_ver <<< "$gcc"
    IFS=$IFS_bak # reset IFS
    # echo -e "$gcc_ver"

    if [[ "$gcc_ver" -gt 4 ]]; then
        echo -e "exporting devtoolset-9"
        export PATH=/opt/rh/devtoolset-9/root/bin:$PATH
    fi

    if [[ "$gcc_ver" -gt 9 ]]; then
        echo -e "exporting devtoolset-11"
        export PATH=/opt/rh/devtoolset-11/root/bin:$PATH
    fi

    # echo $PATH
}

function gcc_reset()
{
    export PATH=$GLOBAL_PATH

    # echo $PATH
}

function go_setup()
{
    if [ ! -d "$GO_PATH" ]; then
        mkdir -pv "$GO_PATH" # do not trap error
    fi

    if [ -f "go.mod" ]; then 
        go env -w GO111MODULE=auto
        go env -w GOPRIVATE=github.com/flexgen-power/* # access to private repos

        go mod tidy # do not trap error
    else
        export GO111MODULE=off # preserve backward compatibility
    fi
    # set go build linker flags
    go_linker_flags="-X github.com/flexgen-power/go_flexgen/buildinfo.gitTag=$tag
        -X github.com/flexgen-power/go_flexgen/buildinfo.gitCommit=$commit
        -X github.com/flexgen-power/go_flexgen/buildinfo.gitBuild=$BUILD"
}

# check to see if repository is dirty
function check_dirty()
{
    if [[ -n $(git status --porcelain) ]]; then
        return 1
    fi
    return 0
}

# verify that builds actually succeeded
# build output directory passed in as $1
function check_artifacts()
{
    echo -e "\ncheck_artifacts..."
    echo -e "current_directory: $(pwd)"
    echo -e "build_output: $build_output"
    echo -e "submodule_build_output: $submodule_build_output"

    for i in "${artifacts[@]}"; do
        # need to use '-e' here to account for executable files and directories
        if [ ! -e "$build_output/$i" ]; then
            if [ ! -e "$submodule_build_output/$i" ]; then
                echo -e "\nbuild artifact $i not found in the search path:"
                echo -e "\tbuild_output: $build_output"
                echo -e "\tsubmodule_build_output: $submodule_build_output"
                return 1
            fi
            echo -e "\nfound build artifact $i in search path:"
            echo -e "\tsubmodule_build_output: $submodule_build_output"
        fi
        echo -e "\nfound build artifact $i in search path:"
        echo -e "\tbuild_output: $build_output"
    done
}

# docker functions
# tag information passed in as $1
function docker_build()
{
    docker build . -t $image:$1 --build-arg verNum=$1
}

function docker_push()
{
    docker push $image:$1
}