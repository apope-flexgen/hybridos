#!/bin/bash
cwd=$(pwd)

shortTag=`echo $dockerTag | sed 's/v//g'`

products=("ess_controller_meta" "site_controller_meta" "fleet_manager_meta" "twins_meta")
for productName in "${products[@]}"; do
	dockerName=`echo $productName | sed 's/_meta//g'`
	docker build . -t flexgen/"${dockerName}":"${shortTag}" --build-arg productName="${productName}" --build-arg dockerName="${dockerName}" --build-arg verNum="${shortTag}"
	docker push flexgen/"${dockerName}":"${shortTag}"
done

echo -e "$cwd"

components=("cloud_sync" "dts")
for componentName in "${components[@]}"; do
	if cd "$i" ; then
		docker build . -t flexgen/"${componentName}":"${shortTag}" --build-arg verNum="${shortTag}"
		docker push flexgen/"${componentName}":"${shortTag}"
		cd $cwd
	else
		echo -e "failed to find component $componentName, exiting."
		exit 1
	fi
done