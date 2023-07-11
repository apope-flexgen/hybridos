#!/bin/bash
set -e

# Required environment variables:
# $ARTIFACT_S3_BUCKET
# $ARTIFACT_S3_KEY

# zip up the build artifacts
echo "Zipping up build artifacts..."
local_file_name=$(basename $ARTIFACT_S3_KEY)
zip -r ${local_file_name} . --quiet

# upload zip file to s3 bucket
echo "Uploading build artifacts to S3..."
aws s3 cp ${local_file_name} s3://${ARTIFACT_S3_BUCKET}/${ARTIFACT_S3_KEY} --no-progress
