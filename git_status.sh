#!/bin/bash

cwd=$(pwd)
fail_on_exit=false

# source build functions (from repository path)
if ! source functions.sh; then
    echo -e "failed to import functions.sh"
    exit 1
fi
source build_utils.sh || error_trap "failed to import $cwd/build_utils.sh."

function help()
{
    echo -e "usage: ./git_status.sh [options] [args]"
    echo -e "[options]"
    echo -e "  -e, --export\texport repo status to file"
    echo -e "\t\twill automatically select in order of availability [branches -> tags -> hashes]"
    echo -e "  -d, --dir\tspecify the relative directory to output file with [args]"
    echo -e "\t\toptional and will default to current directory"
    echo -e "[args]"
    echo -e "  <directory>\tused with '-d', relative directory path to repo.txt"
}

function git_dirty()
{
    [[ $(git status --porcelain 2> /dev/null | tail -n1) != "" ]] && echo "dirty"
}

# option variables
export_arg=false
dir_path=./
help_arg=false

# getopts
if [ $# = 0 ]; then
    : # no default behavior
else
    # options may be followed by one colon to indicate they have a required argument
    if ! options=$(getopt -o ed:h -l export,dir:,help -- "$@")
    then
        # something went wrong
        echo -e "invalid options provided."
        help # print help text
        exit 1
    fi

    eval set -- "$options"

    while [ $# -gt 0 ]
    do
        case $1 in
            -e|--export) export_arg=true;;
            -d|--dir) dir_path="$2"; shift;;
            -h|--help) help_arg=true;;
            (--) shift; break;;
            (-*) echo -e "$0: error - unrecognized option $1" 1>&2; exit 1;;
            (*) break;;
        esac
        shift
    done

fi

# print help text
if [ $help_arg == true ]; then help; exit 0; fi;

# resolve directory path, setup repo file path
dir_path=$(readlink --canonicalize "$dir_path")
repo_file=$dir_path
repo_file+="/repo.txt"

# get current monorepo tag
monorepo_tag=$(git describe --tags --always)
if [[ "$monorepo_tag" =~ ^(v{1})([1-9]|[1-2][0-9]\d*)\.([0-9]|[1-2][0-9]\d*)\.([0-9]|[1-2][0-9]\d*)$ ]]; then
    # if there is no trailing "type" section, it is a release
    build_type="release"
else
    build_type=`echo $monorepo_tag | sed 's/.*[.-]//'`
fi
case $build_type in # generate valid component tag types based off build_type
    "release")
        # release, dev
        valid_types=("release")
    ;;
    "rc")
        # release, dev, rc, hotfix
        valid_types=("release" "rc" "dev" "hotfix")
    ;;
    "beta")
        # release, dev, rc, hotfix, bugfix, beta
        valid_types=("release" "rc" "beta" "dev" "hotfix" "bugfix")
    ;;
    *) # catch-all: alpha and below
        valid_types=("") # <- branch/tags will be compared to *""*, so everything will pass :)
    ;;
esac

# if exporting, check that directory exists and clean repo.txt file if it exists
if [ $export_arg == true ]; then
    if [ ! -d "$dir_path" ]; then echo "$dir_path does not exist."; exit 1; fi;
    if [ -f "$repo_file" ]; then rm $repo_file; fi;
fi

# setup formatting
divider=----------
divider=$divider$divider$divider$divider$divider$divider$divider$divider$divider
header="\n %-20s %-40s %-10s %-10s %-10s\n"
format=" %-20s %-40s %-10s %-10s %-10s\n"
width=91

# print table
printf "$header" "REPO" "BRANCH" "HASH" "STATUS" "VALID"
printf "%$width.${width}s\n" "$divider"

for i in "${components[@]}"; do
    # check if directory exists before moving, fatal error
    if [ ! -d "../$1" ]; then echo "$1 directory does not exist."; exit 1; fi;
    cd "$i"
    repo=$i
    repo="${repo:0:23}"
    
    # get the branch name
    branch=$(git branch | grep "\*" | cut -d ' ' -f2)
    branch="${branch:0:43}"

    # check if repo is detached, if so extract tag instead
    if [[ "$branch" = *"detached"* ]] || [[ "$branch" = *"HEAD"* ]]; then
        branch=$(git describe --tags --always) # extract tag information, or fallback on hash instead
        branch=`echo $branch | sed 's/-g/-/g'`
    fi;

    # get the current SHA
    hash=$(git log --pretty=format:'%h' -n 1)

    # check if there are uncommitted files
    status=$(git_dirty)

    # validate component tag against current monorepo build type
    valid=false
    if [[ "$branch" =~ ^(v{1})([1-9]|[1-2][0-9]\d*)\.([0-9]|[1-2][0-9]\d*)\.([0-9]|[1-2][0-9]\d*)$ ]]; then
        # full release tags are valid for all versions
        valid=true
    else
        # check against valid tag types
        for type in "${valid_types[@]}"; do
            if [[ "$branch" == *"$type"* ]]; then
                valid=true
            fi
        done
    fi
    if [ $valid = false ]; then
        fail_on_exit=true
    fi

    printf  "$format" \
            "$repo" \
            "$branch" \
            "$hash" \
            "$status" \
            "$valid"

    cd "$cwd" # reset

    # if export option is selected, append to repo.txt file
    if [ $export_arg == true ]; then echo "$repo|$branch" >> $repo_file; fi;

done

if [ $export_arg == true ]; then echo -e "\nexported repo.txt to $repo_file"; fi;

if [ "$fail_on_exit" = "true" ]; then
    echo -e "ERROR: invalid component tags or branches selected for monorepo tag: $monorepo_tag\n"
    exit 1
fi