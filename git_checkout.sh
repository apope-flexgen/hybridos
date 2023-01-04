#!/bin/bash

cwd=$(pwd)

# source build functions (from repository path)
source build_utils.sh || error_trap "failed to import $cwd/build_utils.sh."

function help()
{
    echo -e "usage: ./git_checkout.sh [options] [args]"
    echo -e "[options]"
    echo -e "  -f, --force\tignore dirty repositories"
    echo -e "  -d, --dir\tspecify the relative directory to input file with [args]"
    echo -e "[args]"
    echo -e "  <directory>\tused with '-d', relative directory path to repo.txt"
    echo -e "no input arguments will result in checkout to latest dev"
}

function git_checkout() # $1 - repo; $2 - branch
{
    # check if directory exists before moving, fatal error
    if [ ! -d "$cwd/$1" ]; then echo "$1 directory does not exist."; exit 1; fi;
    cd "$1"

    # abort if repo is dirty, unless force option is specified
    if [ "$force_arg" == false ]; then
        if [[ -n $(git status --porcelain) ]]; then echo "$1 repository is dirty, commit your local changes first."; exit 1; fi
    fi

    echo -e "\n##### checking out $1..."

    # reset local status (just in case)
    git merge --abort
    git rebase --abort
    git cherry-pick --abort

    # clean and reset the repo
    sudo git clean -xdf
    sudo git reset --hard

    # fetch the latest from origin
    git fetch --prune --tags -f
    # delete local tags that were deleted from origin
    git tag -l | xargs git tag -d && git fetch -t

    # delete local version of the branch
    if git checkout master; then echo -e "branch master checked out"; # this should not fail
    elif git checkout main; then echo -e "branch main checked out";
    else
        echo -e "fatal error: master or main branch not found."
        exit 1
    fi
    git branch -D "$2" # no error checking needed here

    # checkout the specified branch, update as needed
    if git checkout "$2"; then echo -e "branch $2 checked out";
    else
        echo -e "branch $2 not found, defaulting to dev"
        git checkout dev # default to dev of the specified branch does not exist
    fi

    # submodule management
    git submodule update --init --recursive

    cd "$cwd" # reset
}

# option variables
force_arg=false
dir_arg=false
help_arg=false

dir_path=./

# getopts
# options may be followed by one colon to indicate they have a required argument
if ! options=$(getopt -o fhd: -l force,help,dir: -- "$@")
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
        -f|--force) force_arg=true;;
        -h|--help) help_arg=true;;
        -d|--dir) dir_arg=true; dir_path="$2";;
        (--) shift; break;;
        (-*) echo -e "$0: error - unrecognized option $1" 1>&2; exit 1;;
        (*) break;;
    esac
    shift
done

# print help text
if [ $help_arg == true ]; then help; exit 0; fi;

# resolve directory path, setup repo file path
dir_path=$(readlink --canonicalize "$dir_path")
repo_file=$dir_path
repo_file+="/repo.txt"

# setup map to hold repo-branch pairs
declare -A repo_branch_map

# setup the file separator
IFS='|'

# recursively update current repo
git submodule update --init --recursive

# iterate over repos and parse out what branch should be checked out
# add the repo-branch pair to repo_branch_map
if [ "$dir_arg" == true ]; then
    if [ ! -d "$dir_path" ]; then echo "$dir_path does not exist."; exit 1;
    elif [ ! -f "$repo_file" ]; then echo "$repo_file does not exist."; exit 1; fi;
    while read repo branch
    do
        repo_branch_map[$repo]=$branch;
    done < $repo_file
else
    for i in "${components[@]}"; do
        repo_branch_map[$i]=dev;
    done
fi

# track repos that are not found in the file system
declare -A repos_not_found
for repo in ${!repo_branch_map[@]}; do
    if [ ! -d "$cwd/$repo" ]; then
        repos_not_found[$repo]=$repo
    fi
done

# checkout designated branch of all repos
for repo in ${!repo_branch_map[@]}; do
    repo_found=true
    for missing_repo in ${!repos_not_found[@]}; do
        if [[ $repo = $missing_repo ]]; then
            repo_found=false
            break
        fi
    done
    if [[ $repo_found = true ]]; then
        git_checkout ${repo} ${repo_branch_map[${repo}]}
    fi
done

# report confirmation
pwd
./git_status.sh

# print a warning for any repos that were not found
for repo in ${!repos_not_found[@]}; do
    echo "WARNING: repo $repo was not found so was skipped"
done
