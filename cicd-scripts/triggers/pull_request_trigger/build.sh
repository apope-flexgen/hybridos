#!/bin/bash
set -e

# Required environment variables:
# $ARTIFACT_S3_BUCKET
# $ARTIFACT_S3_KEY

# extract git metadata for future reference
commit_sha=$(git rev-parse --short HEAD)
echo "Commit SHA: ${commit_sha}"
branch=$(git for-each-ref --format='%(objectname) %(refname:short)' refs/heads | awk "/^$(git rev-parse HEAD)/ {print \$2}") # in detached HEAD state, so need to do some magic to determine the branch
echo "Branch: ${branch}"
# tag=$(git describe --tags --exact-match 2>/dev/null || true)
# echo "Tag: ${tag}"
author_email=$(git log -1 --pretty=format:'%ae')
echo "Author Email: ${author_email}"

# zip up the build artifacts
echo -e "\nZipping up build artifacts..."
local_file_name=$(basename $ARTIFACT_S3_KEY)
zip -r ${local_file_name} . --quiet

# upload zip file to s3 bucket
echo "Uploading build artifacts to S3..."
aws s3 cp ${local_file_name} s3://${ARTIFACT_S3_BUCKET}/${ARTIFACT_S3_KEY} --metadata "branch=${branch},tag=${tag},author_email=${author_email},commit_sha=${commit_sha}" --no-progress
