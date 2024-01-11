#!/bin/sh
# run release tests

for aa in `ls -1 scripts/release` ; do sh scripts/release/$aa; done
