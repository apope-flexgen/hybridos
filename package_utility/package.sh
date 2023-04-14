#!/bin/bash

# clear sourced variables
name=
type=
image=
artifacts=()
submodules=()
tests=()

# option variables
force_flag=false
help_flag=false

function help()
{
    echo -e "usage: ./package.sh [options]"
    echo -e "[options]"
    echo -e "  -f, --force\tignore dirty repo"
    echo -e "  -h, --help"
}

# options may be followed by one colon to indicate they have a required argument
options_sub="$@" # backup input arguments for submodule processing
if ! options=$(getopt -o fh -l force,help -- "$@")
then
    # something went wrong, getopt will put out an error message for us
    "invalid arguments, please run with '--help' to review available options."
    exit 1
fi

eval set -- "$options"

while [ $# -gt 0 ]
do
    case $1 in
        -f|--force) force_flag=true;;
        -h|--help) help_flag=true;;
        (--) shift; break;;
        (-*) echo "$0: error - unrecognized option $1" 1>&2; exit 1;;
        (*) break;;
    esac
    shift
done

# print help text
if [ "$help_flag" = true ]; then
    help
    exit 0
fi

# capture script and build relative paths
# $0 is the actual running script, ./package_utility/build.sh
script_path=$(readlink -f "$(dirname "$0")")/ # this is absolute path to the script, ./package_utility/
echo -e "script_path: $script_path"

# resolve relative path to build directory
build_output="$(pwd)/build/$mode_arg/"
echo -e "build_output: $build_output"

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

if [ ! -n "$name" ]; then
    error_trap "build_utils.sh does not specify 'name' field."
fi

# check for build directory
if [ ! -d "$build_output" ]; then
    error_trap "$build_output directory does not exist."
fi

# check for spec file
if [ ! -f "$name".spec ]; then
    error_trap "expected $name.spec file does not exist."
fi

# dirty error/warning check
if [ "$force_flag" == false ]; then
    check_dirty || error_trap "repo is dirty. commit your changes or update your .gitignore file.\n${ERROR}$(git status)"
else
    check_dirty || warning_trap "repo is dirty. commit your changes or update your .gitignore file.\n${WARNING}$(git status)"
fi

# successfully passed all checks, begin...

# source version from version.sh
version_path="${script_path}version.sh"
source $version_path

# export output tag, branch, and commit to an output file included in RPM
# i.e. v1.7.0|feature/build-pipeline|7db842b|43
touch "$build_output/$name.repo"
echo "$tag|$branch|$commit|$BUILD" > "$build_output/$name.repo"

cp -av "$build_output" "$rpmbuild" # ./build/release -> ./build/fims-1.7.0-43.local
tar -czvf "$rpmbuild".tar.gz "$rpmbuild"
rm -rf "$rpmbuild"

# capture buildroot
buildroot="${build_path}/rpmbuild"

# create the directory structure
mkdir -p "$buildroot"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}

# copy spec and source
cp "$name".spec "$buildroot"/SPECS
mv "$name"*.tar.gz "$buildroot"/SOURCES

echo -e "\n##### packaging $name..."
echo -e "name:\t\t$name"
echo -e "version:\t$tag"
echo -e "release:\t$release"

# capture fims dependency for components
fimsVer=$(rpm -q fims | cut -d'-' -f 2)
# fims installed, and we're not building fims
if [ "$fimsVer" != "package fims is not installed" ] && [ "$name" != "fims" ]; then
    lineno=$(grep -n "Requires" ${name}.spec | grep -Eo '^[^:]+')
    if [[ "$lineno" != "" ]]; then # component has no 'Requires' dependencies
        sed -i "${lineno}s/fims/fims\ =\ ${fimsVer}/" "${buildroot}/SPECS/${name}.spec"
    fi
fi

# construct rpmbuild command
rpmbuild --ba --clean \
    --define "_topdir $buildroot" \
    --define "_name $name" \
    --define "_version $tag" \
    --define "_release $release" \
    "${buildroot}/SPECS/${name}".spec || error_trap "failed to build rpm."

success_trap "$name packaging successful."