#!/bin/bash

# capture script and build relative paths
cwd="$(pwd)"
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
#echo "script_path: $script_path"

# source build functions (from repository path)
source build_utils.sh || error_trap "failed to import $cwd/build_utils.sh."
name_meta="$name"
submodules_meta=("${submodules[@]}")

# source version from version.sh
version_path="${script_path}version.sh"
source $version_path

functions_path="${script_path}functions.sh"
source $functions_path
CATCH=$?
if [ $CATCH -eq 1 ]; then
    echo -e "failed to import functions.sh."
    exit 1
fi

# create build directory for meta-RPM
build_output="${script_path}build/release"
mkdir -p "$build_output"

build_output_meta=$(readlink -f $build_output)
mkdir -p "$build_output_meta"
mkdir -p "${script_path}output"

# iterate through modules
for i in "${submodules[@]}"; do
    echo -e "##### packaging: $i..."
    if cd "$i" ; then
        buildroot="$(pwd)/rpmbuild"
        build_output="$(pwd)/build/release" # package.sh uses 'release' only

        # source build functions (from repository path)
        source build_utils.sh || error_trap "failed to import $cwd/build_utils.sh."
        
        if [ ! -n "$name" ]; then
            error_trap "build_utils.sh does not specify 'name' field."
        fi

        # check for build directory
        if [ ! -d "$build_output" ]; then
            error_trap "$build_output directory does not exist."
        fi

        # check for spec file
        if [ ! -f "$i".spec ]; then
            error_trap "expected $i.spec file does not exist."
        fi

        # dirty error/warning check
        #
        #if [ "$force_flag" == false ]; then
        #    check_dirty || error_trap "repo is dirty. commit your changes or update your .gitignore file.\n${ERROR}$(git status)"
        #else
        #    check_dirty || warning_trap "repo is dirty. commit your changes or update your .gitignore file.\n${WARNING}$(git status)"
        #fi

        # successfully passed all checks, begin...

        # export output tag, branch, and commit to an output file included in RPM
        # i.e. v1.7.0|feature/build-pipeline|7db842b|43
        touch "$build_output/$i.repo"
        echo "$tag|$branch|$commit|$BUILD" > "$build_output/$i.repo"

        rpmname="$i-$tag-$release"

        cp -av "$build_output" "$rpmname" # ./build/release -> ./build/fims-1.7.0-43.local
        tar -czvf "$rpmname".tar.gz "$rpmname"
        rm -rf "$rpmname"

        # create the directory structure
        mkdir -p "$buildroot"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}

        # copy spec and source
        cp "$i".spec "$buildroot"/SPECS
        mv "$i"*.tar.gz "$buildroot"/SOURCES

        echo -e "\n##### packaging $i..."
        echo -e "name:\t\t$i"
        echo -e "version:\t$tag"
        echo -e "release:\t$release"

        # construct rpmbuild command
        rpmbuild --ba --clean \
            --define "_topdir $buildroot" \
            --define "_name $i" \
            --define "_version $tag" \
            --define "_release $release" \
            "${buildroot}/SPECS/$i".spec || error_trap "failed to build rpm."

        success_trap "$i packaging successful."

        output=($(ls $buildroot/RPMS/x86_64/))
        for j in "${output[@]}"; do
            cp -r "$buildroot/RPMS/x86_64/$j" "${cwd}/output/"
        done

        commit_submodule=$(git log --pretty=format:'%h' -n 1)
        echo -e "commit:\t\t$commit_submodule"

        if git describe --match v* --abbrev=0 --tags HEAD &> /dev/null ; then
            tag_long=$(git describe --match "v*" --abbrev=0 --tags HEAD)
            if [[ $tag_long == "v"* ]]; then tag_submodule=${tag_long:1}; fi # (v1.0.0) -> (1.0.0)
            if [[ $tag_long == *"-rc" ]]; then tag_submodule=$(echo $tag | cut -d'-' -f 1); rc=".rc"; fi # (v1.0.0-rc) - > (v.1.0.0.rc)

        else
            tag_submodule=$commit_submodule # no tag info, use abbreviated commit hash
        fi
        echo -e "tag:\t\t$tag_submodule"

        # put individual commit/tag information into version.txt
        echo "$i|$tag_submodule|$commit_submodule" >> "${cwd}/build/release/version.txt"
        cd ../
    fi
done

echo -e "\n\n##### bulding meta-RPM"

cp -av "$build_output_meta" "$rpmbuild" # ./build/release -> ./fims-1.7.0-43.local
tar -czvf "$rpmbuild".tar.gz "$rpmbuild"
rm -rf "$rpmbuild"

# create the directory structure
buildroot="$(pwd)/rpmbuild"
mkdir -p "$buildroot"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}

# copy spec and source
cp "$name_meta".spec "$buildroot"/SPECS
mv "$name_meta"*.tar.gz "$buildroot"/SOURCES

# gather list of dependencies
DEPS=""
for i in "${submodules_meta[@]}"; do
    DEPS+="$i-$tag "
    echo -e "$i"
done
DEPS=$(echo $DEPS | sed -r 's/[-]+/ = /g')

# build meta-RPM
rpmbuild --ba --clean \
    --define "_topdir $buildroot" \
    --define "_name $name_meta" \
    --define "_version $tag" \
    --define "_release $release" \
    --define "_reqs $DEPS" \
    "${buildroot}/SPECS/${name_meta}".spec || error_trap "failed to build rpm."

output=($(ls $buildroot/RPMS/x86_64/))
for j in "${output[@]}"; do
    echo -e "$j"
    cp -r "$buildroot/RPMS/x86_64/$j" "${cwd}/output/"
done
