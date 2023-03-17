#!/bin/bash

# globals
#   build mode (release, debug, test) can be accessed as "$mode_arg"
#   build output directory (i.e. ./build/release) can be accessed as "$build_output"
# building (./package_utility/build.sh)
#   'type' (cpp, go, node, etc.) can be optionally specified
#   all artifacts must be output to ./build/[mode]
#   populate 'submodules' to issue builds from within folders in the repository
#   submodule build_utils.sh files need to use the global $build_output for artifacts
# testing (./package_utility/test.sh)
#   elements in 'tests' array are passed to test() as $1 in a looped fashion
# packaging (./package_utility/package.sh)
#   'name' is required and will be used as the RPM package name
#   package.sh will only generate RPMs from ./build/release
#   create [name].spec file to specifiy software package details
#       package.sh will pull only one spec file from the module's top-level directory
#   [name].service OR [name]@.service file to specify execution in deployed environment
#       multiple service files can be added to $build_output directory as needed
# docker (./package_utility/docker.sh)
#   'image' is required and is used as the Docker repository name, which must already exist
#       i.e. flexgen/centos7
#   create a Dockerfile at the top-level directory to generate a Docker image
#   git short tag version is passed to both docker_build() and docker_push() as $1 to use for image tag
#       i.e. v1.0.0 -> flexgen/centos7:1.0.0
# error handling
#   add '|| error_trap "error_message"' to critical sections
#   add '|| warning_trap "warning_message"' to issue warnings

name=echo
type=go
image=
artifacts=(echo echo@.service)
submodules=()
tests=()

function prebuild()
{
    return 0
}

function build()
{
    cd src/ && go build echo.go
}

function postbuild()
{
    mkdir -p $build_output
    cp echo ../echo@.service $build_output
}

function install()
{
    sudo cp echo /usr/local/bin/
}

function uninstall()
{
   sudo rm /usr/local/bin/echo
}

function test()
{
    return 0
}

# optionally add module-specific functions below
