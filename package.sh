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

# source version from version.sh
version_path="${script_path}version.sh"
source $version_path
if [ $CATCH -eq 1 ]; then
    echo -e "failed to import version.sh."
    exit 1
fi

functions_path="${script_path}functions.sh"
source $functions_path
CATCH=$?
if [ $CATCH -eq 1 ]; then
    echo -e "failed to import functions.sh."
    exit 1
fi

# complete checks before proceeding...

if [ ! -n "$name" ]; then
    error_trap "build_utils.sh does not specify 'name' field."
fi
name_meta="$name"
echo -e "name_meta: $name_meta"

if [ ! -n "$components" ]; then
    error_trap "build_utils.sh does not specify 'components' field."
fi

# create build directory for meta-RPM
build_output="${script_path}build/release"
package_output="${script_path}output"
# mkdir -p "$build_output"
mkdir -p "$package_output"

build_output_meta=$(readlink -f $build_output)
package_output_meta=$(readlink -f $package_output)
echo -e "build_output_meta: $build_output_meta"
echo -e "package_output_meta: $package_output_meta"

# clear out the repo.txt file
rm -rf "$build_output_meta/repo.txt"

# iterate through modules
for i in "${components[@]}"; do
    echo -e "\n##### packaging: $i..."
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
        if [ "$force_flag" == false ]; then
            check_dirty || error_trap "repo is dirty. commit your changes or update your .gitignore file.\n${ERROR}$(git status)"
        else
            check_dirty || warning_trap "repo is dirty. commit your changes or update your .gitignore file.\n${WARNING}$(git status)"
        fi

        # successfully passed all checks, begin...

        # export output tag, branch, and commit to an output file included in RPM
        # i.e. v1.7.0|feature/build-pipeline|7db842b|43
        touch "$build_output/$i.repo"
        echo "$tag|$branch|$commit|$BUILD" > "$build_output/$i.repo"

        rpmname="$i-$tag-$release"

        cp -av "$build_output" "$rpmname" # ./build/release -> ./fims-1.7.0-43.local
        tar -czvf "$rpmname".tar.gz "$rpmname"
        rm -rf "$rpmname"

        # create the directory structure
        mkdir -p "$buildroot"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}

        # copy spec and source
        cp "$i".spec "$buildroot"/SPECS
        mv "$i"*.tar.gz "$buildroot"/SOURCES

        # using the version.sh from the monorepo, not submodule
        echo -e "name:\t\t$i"
        echo -e "version:\t$tag"
        echo -e "release:\t$release"

        # capture fims dependency for components
        # fims installed, and we're not building fims
        if [ "$name" != "fims" ]; then
            lineno=$(grep -n "Requires" ${i}.spec | grep -Eo '^[^:]+')
            if [[ "$lineno" != "" ]]; then # component has no 'Requires' dependencies
                sed -i "${lineno}s/fims/fims\ =\ ${tag}/" "${buildroot}/SPECS/${i}.spec"
            fi
        fi

        # construct rpmbuild command
        rpmbuild --ba --clean \
            --define "_topdir $buildroot" \
            --define "_name $i" \
            --define "_version $tag" \
            --define "_release $release" \
            "${buildroot}/SPECS/$i".spec || error_trap "failed to build rpm."

        success_trap "$i packaging complete."

        output=($(ls $buildroot/RPMS/x86_64/))
        for j in "${output[@]}"; do
            cp -r "$buildroot/RPMS/x86_64/$j" "$package_output_meta"
        done

        # capture the "true" component version
        commit_submodule=$(git log --pretty=format:'%h' -n 1)

        if git describe --match v* --abbrev=0 --tags HEAD &> /dev/null ; then
            tag_long=$(git describe --match "v*" --abbrev=0 --tags HEAD)
            if [[ $tag_long == "v"* ]]; then tag_submodule=${tag_long:1}; fi # (v1.0.0) -> (1.0.0)
            if [[ $tag_long == *"-rc" ]]; then tag_submodule=$(echo $tag | cut -d'-' -f 1); rc=".rc"; fi # (v1.0.0-rc) - > (v.1.0.0.rc)

        else
            tag_submodule=$commit_submodule # no tag info, use abbreviated commit hash
        fi

        # put individual commit/tag information into repo.txt
        echo "$i|$tag_submodule|$commit_submodule" >> "$build_output_meta/repo.txt"
        cd ../
    fi
done

if [ ! -f "$build_output_meta/repo.txt" ]; then
    echo -e "$build_output_meta/repo.txt was not created, aborting."
    exit 1
fi

echo -e "\n##### packaging hybridos"

echo -e "$commit"
echo -e "$tag"

for i in "${meta[@]}"; do
    echo -e "\n##### packaging: $i"
    package=$(echo $rpmbuild | sed "s/hybridos/$i/g")
    echo -e "$package"
    echo -e "build_output_meta: $build_output_meta"

    cp -av "$build_output_meta" "$package" # ./build/release/repo.txt -> ./ess_controller_meta-10.1.0-1.local
    tar -czvf "$package".tar.gz "$package"
    rm -rf "$package"

    # create the directory structure
    buildroot="$(pwd)/$i/rpmbuild"
    mkdir -p "$buildroot"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}

    cp "$name_meta".spec "$buildroot"/SPECS/"$i".spec
    mv "$package"*.tar.gz "$buildroot"/SOURCES

    if [ "$i" == "ess_controller_meta" ]; then
        deps=("${ess_controller_meta[@]}")
    elif [ "$i" == "site_controller_meta" ]; then
        deps=("${site_controller_meta[@]}")
    elif [ "$i" == "fleet_manager_meta" ]; then
        deps=("${fleet_manager_meta[@]}")
    elif [ "$i" == "twins_meta" ]; then
        deps=("${twins_meta[@]}")
    fi

    DEPS=""
    for j in "${deps[@]}"; do
        DEPS+="$j-$tag "
        echo -e "$j-$tag"
    done
    DEPS=$(echo $DEPS | sed -r 's/[-]+/ = /g')
    echo -e "$DEPS"

    # build meta-RPM
    rpmbuild --ba --clean \
    --define "_topdir $buildroot" \
    --define "_name $i" \
    --define "_version $tag" \
    --define "_release $release" \
    --define "_reqs $DEPS" \
    "${buildroot}/SPECS/${i}".spec || error_trap "failed to build rpm."

    output=($(ls $buildroot/RPMS/x86_64/))
    for j in "${output[@]}"; do
        echo -e "$j"
        cp -r "$buildroot/RPMS/x86_64/$j" "$package_output_meta"
    done
done
