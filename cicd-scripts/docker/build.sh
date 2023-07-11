#!/bin/bash
set -e

if git describe --abbrev=0 --tags HEAD &> /dev/null ; then
    git_tag=$(git describe --abbrev=0 --tags HEAD)
    echo "Tag found: $git_tag"
else
    echo "No tag, exiting..."
    exit 0
fi

echo Running Docker script...
dockerTag=${git_tag} ./docker.sh