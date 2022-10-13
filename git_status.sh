#!/bin/bash

# source repo list
mapfile -t hybridos < hybridos.txt

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

# if exporting, check that directory exists and clean repo.txt file if it exists
if [ $export_arg == true ]; then
    if [ ! -d "$dir_path" ]; then echo "$dir_path does not exist."; exit 1; fi;
    if [ -f "$repo_file" ]; then rm $repo_file; fi;
fi

# setup formatting
divider=----------
divider=$divider$divider$divider$divider$divider$divider$divider$divider
header="\n %-20s %-40s %-10s %-10s\n"
format=" %-20s %-40s %-10s %-10s\n"
width=80

# print table
printf "$header" "REPO" "BRANCH" "HASH" "STATUS"
printf "%$width.${width}s\n" "$divider"

for i in "${hybridos[@]}"; do
    cd ../"$i"
    repo=$i
    repo="${repo:0:23}"
    
    # get the branch name
    branch=$(git branch | grep "\*" | cut -d ' ' -f2)
    branch="${branch:0:43}"

    # check if repo is detached, if so extract tag instead
    if [[ "$branch" = *"detached"* ]] || [[ "$branch" = *"HEAD"* ]]; then
        branch=$(git describe --tags --always) # extract tag information, or fallback on hash instead
    fi;

    # get the current SHA
    hash=$(git log --pretty=format:'%h' -n 1)

    # check if there are uncommitted files
    status=$(git_dirty)

    printf  "$format" \
            "$repo" \
            "$branch" \
            "$hash" \
            "$status"

    cd ../scripts # reset

    # if export option is selected, append to repo.txt file
    if [ $export_arg == true ]; then echo "$repo|$branch" >> $repo_file; fi;

done

if [ $export_arg == true ]; then echo -e "\nexported repo.txt to $repo_file"; fi;
