#!/bin/bash
set -e

echo Logging in to Docker Hub...
echo $DOCKERHUB_PASSWORD | docker login --username $DOCKERHUB_USERNAME --password-stdin