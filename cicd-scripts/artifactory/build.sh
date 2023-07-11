#!/bin/bash
set -e

# the following environment variables must be set:
# ARTIFACTORY_USERNAME
# ARTIFACTORY_PASSWORD
# imageName

ARTIFACTORY_BASE_URL=https://flexgenpower.jfrog.io
ARTIFACTORY_REPO=flexgen-rpm

# loop through RPM files in the output folder
for file_rel_path in $(find output -name "*.rpm"); do
    # file_rel_path=output/<product_name>-<gitref>-<release>.<platform>.rpm
    file_name=$(basename $file_rel_path) # remove the output/ prefix
    
    if [[ $file_name == *-devel* ]]; then
        UPLOAD_BUCKET=devel
    else
        UPLOAD_BUCKET=snapshot
    fi
    
    # upload RPM to Artifactory
    echo "Uploading $file_name RPM to Artifactory..."
    curl -X PUT -u ${ARTIFACTORY_USERNAME}:${ARTIFACTORY_PASSWORD} --data-binary @${file_rel_path} $ARTIFACTORY_BASE_URL/artifactory/${ARTIFACTORY_REPO}/${imageName}/${UPLOAD_BUCKET}/${file_name}
done
