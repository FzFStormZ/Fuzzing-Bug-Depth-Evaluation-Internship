#!/bin/bash

shopt -s nullglob
set -e

# arguments
binary=false
specific=false
if [ -z "${1}" ]
then
    echo "[+] Error: Please enter an application name"
    exit 1
fi

if [ -z "${2}" ]
then
    specific=true
    echo "[+] Info: Your application is in 'reach specific line' mode!"
fi

if [ -n "${3}" ]
then
    binary=true
    echo "[+] Info: Your application is in 'specific binary' mode!"
fi


# compile, build and run
DIR=$(find fuzzer-test-suite -type d -iname "${1}")
if [ -z "${DIR}" ]
then
    echo "[+] Error: Unknown application"
    exit 1
fi

NAME=$(echo "${DIR}" | cut -d "/" -f2)


# build script
BUILD=$(find scripts -type d -iname build)
if [ -z "${BUILD}" ]
then
    cd scripts && make && cd ..
fi

cd run_eval


# already run the binary?
BINARY=$(find . -type d -iname "${NAME}")
if [ -z "${BINARY}" ]
then
    mkdir "${NAME}" && cd "${NAME}" && ../../fuzzer-test-suite/"${NAME}"/build.sh
else
    cd "${NAME}"
fi


# run evaluation
if [ "${specific}" = true ] && [ "${binary}" = true ]
then
    ../../pin/pin -t ../../scripts/build/bugdepthevaluation.so -o ../../results/"${NAME}"-reach-line-bugdepthevaluation.out -- ./"${3}"

elif [ "${specific}" = true ] && [ "${binary}" = false ]
then
    ../../pin/pin -t ../../scripts/build/bugdepthevaluation.so -o ../../results/"${NAME}"-reach-line-bugdepthevaluation.out -- ./"${NAME}"-fsanitize_fuzzer

elif [ "${specific}" = false ] && [ "${binary}" = true ]
then
    ../../pin/pin -t ../../scripts/build/bugdepthevaluation.so -o ../../results/"${NAME}"-"${2}"-bugdepthevaluation.out -- ./"${3}" ../../fuzzer-test-suite/"${NAME}"/"${2}"

else
    ../../pin/pin -t ../../scripts/build/bugdepthevaluation.so -o ../../results/"${NAME}"-"${2}"-bugdepthevaluation.out -- ./"${NAME}"-fsanitize_fuzzer ../../fuzzer-test-suite/"${NAME}"/"${2}"
fi

cd ..
