#!/bin/bash

# clear sourced variables
name=
type=
image=
artifacts=()
submodules=()
tests=()

# option variables, setup default behavior
build_flag=true
push_flag=true

function help()
{
    echo -e "usage: ./docker.sh [options] [args]"
    echo -e "[options]"
    echo -e "  -b, --build"
    echo -e "  -p, --push"
    echo -e "  -t, --tag \toverride tag info with [args]"
    echo -e "  -h, --help"
    echo -e "[args]"
    echo -e "  (i.e. 1.0.0)"
    echo -e "default behavior: ./build.sh -bp -t 1.0.0"
}

# options may be followed by one colon to indicate they have a required argument
options_sub="$@" # backup input arguments for submodule processing
if ! options=$(getopt -o bpt:h -l build,push,tag:,help -- "$@")
then
    # something went wrong, getopt will put out an error message for us
    "invalid arguments, please run with '--help' to review available options."
    exit 1
fi

eval set -- "$options"

while [ $# -gt 0 ]
do
    case $1 in
        -b|--build) push_flag=false;;
        -p|--push) build_flag=false;;
        -t|--tag) tag_arg="$2"; shift;;
        -h|--help) help_flag=true;;
        (--) shift; break;;
        (-*) echo -e "$0: error - unrecognized option $1" 1>&2; exit 1;;
        (*) break;;
    esac
    shift
done

# print help text
if [ "$help_flag" == true ]; then
    help
    exit 0
fi

# capture script and build relative paths
substr=""
IFS_bak=$IFS  # backup the existing IFS
IFS='/'
dir=$(dirname "$0")
read -ra substr <<< "$dir"
IFS=$IFS_bak # reset IFS

script_path="" # relative path to package_utility directory
for i in "${substr[@]}"; do
    if [ index = ${#substr[@]} ]; then break; fi
    script_path+=$i
    script_path+="/"  # optionally add trailing slash
done
# echo "$script_path"

build_path=$(readlink -f ${substr[0]}) # resolve relative path to build directory
build_output="$build_path/build/$mode_arg"
# echo "$build_path"
# echo "$build_output"

# source helper functions (from relative path)
functions_path="${script_path}functions.sh"
source $functions_path
CATCH=$?
if [ $CATCH -eq 1 ]; then
    echo -e "failed to import functions.sh."
    exit 1
fi

# source build functions (from repository path)
source build_utils.sh || error_trap "failed to import $cwd/build_utils.sh."

# complete checks before proceeding...

if [ ! -n "$image" ]; then
    error_trap "build_utils.sh does not specify 'image' field."
fi

# check for spec file
if [ ! -f "Dockerfile" ]; then
    error_trap "expected Dockerfile file does not exist."
fi

# successfully passed all checks, begin...

# if user did not specify tag info, then pull from version.sh
if [ ! -n "$tag_arg" ]; then
    # source version from version.sh
    version_path="${script_path}version.sh"
    source $version_path
    tag_arg=`echo $branch | sed 's/feature\///g'`
fi

if [ "$build_flag" == true ]; then
    echo -e "\n##### building $image:$tag_arg..."
    docker_build "$tag_arg" || error_trap "docker_build() failure."
    echo -e "build complete."
fi
if [ "$push_flag" == true ]; then
    echo -e "\n##### pushing $image:$tag_arg..."
    docker_push "$tag_arg" || error_trap "docker_push() failure."
    echo -e "push complete."
fi