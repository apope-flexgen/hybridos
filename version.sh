#!/bin/bash

# globals
cwd=$(pwd)

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

echo -e "\n##### version"
if [ -n "$name" ]; then
    echo -e "name:\t\t$name"
else
    error_trap "build_utils.sh does not specify 'name' field."
fi

# capture working branch
branch=$(git branch | grep \* | cut -d ' ' -f2)
echo -e "branch:\t\t$branch"
rm -f GIT_BRANCH
echo "$branch" >> GIT_BRANCH
objcopy --input binary \
            --output-target elf64-x86-64 \
            --binary-architecture i386 GIT_BRANCH git_branch.o

# capture current commit
commit=$(git log --pretty=format:'%h' -n 1)
echo -e "commit:\t\t$commit"
rm -f GIT_COMMIT
echo "$commit" >> GIT_COMMIT
objcopy --input binary \
            --output-target elf64-x86-64 \
            --binary-architecture i386 GIT_COMMIT git_commit.o

# capture latest tag
if git describe --match "v*" --abbrev=0 --tags HEAD &> /dev/null ; then
    tag_long=$(git describe --match "v*" --abbrev=0 --tags HEAD)
    if [[ $tag_long == "v"* ]]; then tag=${tag_long:1}; fi # (v1.0.0) -> (1.0.0)
    if [[ $tag_long == *"-"* ]]; then tag=`echo $tag | sed 's/-/./g'`; fi # (1.0.0-rc) - > (1.0.0.rc)
elif git describe --abbrev=0 --tags HEAD &> /dev/null ; then
    tag=$(git describe --abbrev=0 --tags HEAD)
else
    tag=$commit # no tag info, use abbreviated commit hash
fi
echo -e "tag:\t\t$tag"
rm -f GIT_TAG
echo "$tag" >> GIT_TAG
objcopy --input binary \
            --output-target elf64-x86-64 \
            --binary-architecture i386 GIT_TAG git_tag.o

# capture current build number
if [ ! -n "$BUILD" ]; then
    BUILD=$(git rev-list --count "$commit")
fi
echo -e "build:\t\t$BUILD"
rm -f GIT_BUILD
echo "$BUILD" >> GIT_BUILD
objcopy --input binary \
            --output-target elf64-x86-64 \
            --binary-architecture i386 GIT_BUILD git_build.o

# capture environment status
if [ -n "$ENVIRONMENT" ]; then
    release="$BUILD${rc}" # build occuring in Jenkins
else
    release="${BUILD}${rc}.local" # build occuring in a local environment
fi
#echo -e "release:\t$release"

rpmbuild="$name-$tag-$release"
echo -e "rpmbuild:\t$rpmbuild"

check_dirty || warning_trap "repo is dirty. commit your changes or update your .gitignore file."