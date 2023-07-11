#!/bin/bash
set -e

echo Getting container credentials...
STS_RESPONSE=$(curl 169.254.170.2$AWS_CONTAINER_CREDENTIALS_RELATIVE_URI)
export AWS_ACCESS_KEY_ID=$(echo $STS_RESPONSE | jq .AccessKeyId | tr -d \")
export AWS_SECRET_ACCESS_KEY=$(echo $STS_RESPONSE | jq .SecretAccessKey | tr -d \")
export AWS_SESSION_TOKEN=$(echo $STS_RESPONSE | jq .Token | tr -d \")
aws sts get-caller-identity

echo Creating and activating Docker ECS context...
# docker context create ecs hybridos-test --from-env
# docker context use hybridos-test
echo no-op

echo Standing up hybridos in ECS via Docker Compose...
# docker compose up
echo no-op
