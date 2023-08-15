#!/bin/bash

# environment variables set by jenkins
# imageName: centos7
# imageTag: devel
# repoName: hybridos
# dockerTag: v10.1.0

set -x

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
functions_path="${script_path}functions.sh"
source $functions_path
CATCH=$?
if [ $CATCH -eq 1 ]; then
    echo -e "failed to import functions.sh."
    exit 1
fi

shortTag=`echo $dockerTag | sed 's/v//g'`
shortTag=`echo $shortTag | sed 's/-/./g'`

suffix=""
pattern='^([1-9]|[1-2][0-9]\d*)\.([0-9]|[1-2][0-9]\d*)\.([0-9]|[1-2][0-9]\d*)\.([a-zA-Z0-9]*)'
if [[ "$shortTag" =~ $pattern && "$shortTag" != *"release" ]]; then
	suffix="-snapshot"
else
	imageTag=`echo $imageTag | sed 's/devel/release/g'` # pull the correct release container tied to devel (release-10.3.0)
fi

products=("twins_meta" "ess_controller_meta" "site_controller_meta" "fleet_manager_meta")
for productName in "${products[@]}"; do
	dockerName=`echo $productName | sed 's/_meta//g'`
	docker build . --progress=plain --no-cache -t flexgen/"${dockerName}${suffix}":"${shortTag}" --build-arg imageName="${imageName}" --build-arg imageTag="${imageTag}" --build-arg productName="${productName}" --build-arg dockerName="${dockerName}" --build-arg verNum="${shortTag}" || error_trap "failed to build $productName container"
	docker push flexgen/"${dockerName}${suffix}":"${shortTag}" || error_trap "failed to push $productName container"
done

components=("cloud_sync" "dts")
for componentName in "${components[@]}"; do
	if cd "$componentName" ; then
		docker build . --progress=plain --no-cache -t flexgen/"${componentName}${suffix}":"${shortTag}" --build-arg imageName="${imageName}" --build-arg imageTag="${imageTag}" --build-arg verNum="${shortTag}" || error_trap "failed to build $componentName container"
		docker push flexgen/"${componentName}${suffix}":"${shortTag}" || error_trap "failed to build $componentName container"
		cd ../
	else
		echo -e "failed to find component $componentName, exiting."
		exit 1
	fi
done