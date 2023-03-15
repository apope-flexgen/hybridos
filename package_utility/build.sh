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
install_flag=true
uninstall_flag=false
mode_arg=release
help_flag=false

function help()
{
    echo -e "usage: ./build.sh [options] [args]"
    echo -e "[options]"
    echo -e "  -b, --build"
    echo -e "  -i, --install"
    echo -e "  -u, --uninstall"
    echo -e "  -m, --mode\tspecify build mode with [args]"
    echo -e "  -h, --help"
    echo -e "[args]"
    echo -e "  release"
    echo -e "  debug" # if combine debug + devel then debug hogs the console and you can't see devel prints. 
    echo -e "  devel" # print to console as well as log
    echo -e "  test"
    echo -e "default behavior: ./build.sh -bi -m release"
}

# options may be followed by one colon to indicate they have a required argument
options_sub="$@" # backup input arguments for submodule processing
if ! options=$(getopt -o bium:h -l build,install,uninstall,mode:,help -- "$@")
then
    # something went wrong, getopt will put out an error message for us
    "invalid arguments, please run with '--help' to review available options."
    exit 1
fi

eval set -- "$options"

while [ $# -gt 0 ]
do
    case $1 in
        -b|--build) install_flag=false;;
        -i|--install) build_flag=false;;
        -u|--uninstall) uninstall_flag=true;;
        -m|--mode) mode_arg="$2"; shift;;
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
pwd
source build_utils.sh || error_trap "failed to import $cwd/build_utils.sh."

# complete checks before proceeding...

if [ ! -n "$name" ]; then
    error_trap "build_utils.sh does not specify 'name' field."
fi

# conditionally create build_output directory
if [ ! -d "$build_output" ]; then
    mkdir -p "$build_output" || error_trap "failed to create "$build_output" directory."
fi

# successfully passed all checks, begin...

# source version from version.sh
version_path="${script_path}version.sh"
source $version_path

# overwrite options logic
if [ $uninstall_flag == true ]; then
    # no need to build/install if uninstall is specified
    build_flag=false
    install_flag=false
elif [ $build_flag == false ] && [ $install_flag == false ]; then
    # this case is for default options/backward compatibility
    build_flag=true
    install_flag=true
fi

if [ "$build_flag" == true ]; then
    echo -e "\n##### building $name ($mode_arg)..."

    # update working directory
    if [ -n "$workdir" ]; then
        if [ ! -d "$workdir" ]; then
            error_trap "working directory $workdir not found"
        fi
        currdir=$(pwd)
        cd $workdir || error_trap "failed to change working directory"
        echo "changing working directory: $(pwd)"
    fi

    # setup gcc (leaving type check commented as cpp is default)
    if [ "$type" == "cpp" ]; then
        gcc_setup
    fi

    # conditionally create GO_PATH directory if go module
    if [ "$type" == "go" ]; then
        go_setup
    fi

    prebuild "$mode_arg" || error_trap "prebuild() failure."
    build "$mode_arg" || error_trap "build() failure."

    # update working directory
    if [ -n "$workdir" ]; then
        cd $currdir
        echo "resetting working directory"
    fi

    if [ "$type" == "cpp" ]; then
        gcc_reset
    fi

    postbuild "$build_output" || error_trap "postbuild() failure."
    check_artifacts "$build_output" || error_trap "check_artifacts() failure."
    success_trap "$name build complete."
fi
if [ "$install_flag" == true ]; then
    echo -e "\n##### installing $name..."
    install "$mode_arg" || error_trap "install() failure."
    echo -e "install complete."
fi
if [ "$uninstall_flag" == true ]; then
    echo -e "\n##### uninstalling $name..."
    uninstall # do not check for errors
    echo -e "uninstall complete."
fi

# execute submodules
for i in "${submodules[@]}"; do
    if cd "$i" ; then
        ../package_utility/build.sh "$options_sub" || warning_trap "failed to build submodule $i."
        cd ../
    else
        warning_trap "submodule $i not found."
    fi
done

check_dirty || warning_trap "repo is dirty. commit your changes or update your .gitignore file.\n${WARNING}$(git status)"
