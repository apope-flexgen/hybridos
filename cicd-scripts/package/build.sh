#!/bin/bash
set -e

build_number=$CODEBUILD_BUILD_NUMBER

BUILD=$build_number ./package.sh