#!/bin/bash

mkdir -p .exe_pile

# compiler flags:
CPPSTD="-std=c++17"
CPPOPT="-O2"
CPPWARNINGS="-Wall -Wextra -Wpedantic -Wconversion -Wshadow"
CPPMACROS="-DSPDLOG_COMPILED_LIB"
CPPINCLUDE="-I include/"

CPPFLAGS="${CPPSTD} ${CPPOPT} ${CPPWARNINGS} ${CPPMACROS} ${CPPINCLUDE}"

CPPLINKS="-pthread -lfims -lspdlog -lsimdjson"

TESTFOLDER="test/unit_tests"

echo "building unit tests"
for file in ${TESTFOLDER}/*
do
    if [[ $file == *.cpp ]]; then
        # build units tests:
        base_name=$(basename ${file})
        g++ ${CPPFLAGS} $file ${CPPLINKS} -o .exe_pile/${base_name%%.*}
    fi
done

echo "running unit tests"
for file in ${TESTFOLDER}/*
do
    if [[ $file == *.cpp ]]; then
        # run unit tests:
        base_name=$(basename ${file})
        .exe_pile/${base_name%%.*}
    fi
done
