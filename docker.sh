#!/bin/bash
shortTag=`echo $dockerTag | sed 's/v//g'`

products=("ess_controller" "site_controller" "fleet_manager" "twins")
for productName in "${products[@]}"; do
	docker build . -t flexgen/"${productName}":"${shortTag}" --build-arg productName="${productName}" --build-arg dockerName="${dockerName}" --build-arg verNum="${shortTag}"
	docker tag flexgen/"${productName}":"${shortTag}" flexgen/"${productName}":latest
	docker push flexgen/"${productName}":"${shortTag}"
	docker push flexgen/"${productName}":latest
done


components=("cloud_sync" "dts")
