#!/bin/bash

# environment variables set by jenkins
# imageName: centos7
# imageTag: devel
# repoName: hybridos
# dockerTag: v10.1.0

set -x

shortTag=`echo $dockerTag | sed 's/v//g'`
shortTag=`echo $shortTag | sed 's/-/./g'`

suffix=""
pattern='^([1-9]|[1-2][0-9]\d*)\.([0-9]|[1-2][0-9]\d*)\.([0-9]|[1-2][0-9]\d*)\.([a-zA-Z0-9]*)'
if [[ "$shortTag" =~ $pattern && "$shortTag" != *"release" ]]; then
    imageTag="devel" # since we are building a snapshot artifact, we need to pull from the devel image
	suffix="-snapshot"
fi

products=("ess_controller_meta" "site_controller_meta" "fleet_manager_meta" "twins_meta")
for productName in "${products[@]}"; do
	dockerName=`echo $productName | sed 's/_meta//g'`
	docker build . -t flexgen/"${dockerName}${suffix}":"${shortTag}" --build-arg imageName="${imageName}" --build-arg imageTag="${imageTag}" --build-arg productName="${productName}" --build-arg dockerName="${dockerName}" --build-arg verNum="${shortTag}"
	docker push flexgen/"${dockerName}${suffix}":"${shortTag}"
done

components=("cloud_sync" "dts")
for componentName in "${components[@]}"; do
	if cd "$componentName" ; then
		docker build . -t flexgen/"${componentName}${suffix}":"${shortTag}" --build-arg imageName="${imageName}" --build-arg imageTag="${imageTag}" --build-arg verNum="${shortTag}"
		docker push flexgen/"${componentName}${suffix}":"${shortTag}"
		cd ../
	else
		echo -e "failed to find component $componentName, exiting."
		exit 1
	fi
done