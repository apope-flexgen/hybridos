#!/bin/bash
set -e

# Required environment variables:
# $ARTIFACT_S3_BUCKET
# $ARTIFACT_S3_KEY

if git describe --abbrev=0 --tags HEAD &> /dev/null ; then
    tag=$(git describe --abbrev=0 --tags HEAD)
    echo "Tag found: $tag"
else
    echo "No tag, exiting..."
    exit 0
fi

if ! [[ "$tag" =~ ^(v{1})([1-9]|[1-2][0-9]\d*)\.([0-9]|[1-2][0-9]\d*)\.([0-9]|[1-2][0-9]\d*)((-|\.)(alpha|beta|rc|release))?(\.([a-zA-Z0-9]*))?$ ]]; then
    echo "Invalid tag, exiting..."
    exit 0
fi

# zip up the build artifacts
echo "Zipping up build artifacts..."
local_file_name=$(basename $ARTIFACT_S3_KEY)
zip -r ${local_file_name} . --quiet

# upload zip file to s3 bucket
echo "Uploading build artifacts to S3..."
aws s3 cp ${local_file_name} s3://${ARTIFACT_S3_BUCKET}/${ARTIFACT_S3_KEY} --no-progress
