#!/bin/bash
shortTag=`echo $dockerTag | sed 's/v//g'`

products=("ess_controller_meta" "site_controller_meta" "fleet_manager_meta" "twins_meta")
for productName in "${products[@]}"; do
	dockerName=`echo $productName | sed 's/_meta//g'`
	docker build . -t flexgen/"${dockerName}":"${shortTag}" --build-arg productName="${productName}" --build-arg dockerName="${dockerName}" --build-arg verNum="${shortTag}"
	docker tag flexgen/"${dockerName}":"${shortTag}" flexgen/"${productName}":latest
	docker push flexgen/"${dockerName}":"${shortTag}"
	docker push flexgen/"${dockerName}":latest
done

components=("cloud_sync" "dts")
