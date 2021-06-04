#!/bin/bash

shopt -s nullglob
set -e

if [ -z "${1}" ]
then
    echo "[+] Error: Please enter an application name"
    exit 1
fi

# apt update
# apt full-upgrade
# apt -y install git

# install all requirements
apt -y install pkg-config
apt -y install zlib1g-dev
apt -y install wget
apt -y install make
apt -y install g++
apt -y install clang
apt -y install autoconf
apt -y install libtool


# config to have all we have want
wget https://software.intel.com/sites/landingpage/pintool/downloads/pin-3.18-98332-gaebd7b1e6-gcc-linux.tar.gz
tar -xzf pin-3.18-98332-gaebd7b1e6-gcc-linux.tar.gz && mv pin-3.18-98332-gaebd7b1e6-gcc-linux pin && rm -rf pin-3.18-98332-gaebd7b1e6-gcc-linux.tar.gz
git clone https://github.com/google/fuzzer-test-suite.git


# compile, build and run
DIR=$(find . -type d -iname \*"${1}"\*)

if [ -z "${DIR}" ]
then
    echo "[+] Error: Unknown application"
    exit 1
fi

NAME=$(echo "${DIR}" | cut -d "/" -f3)

cd scripts && make
cd .. && mkdir run_eval && cd run_eval && mkdir "${NAME}" && cd "${NAME}"
../../fuzzer-test-suite/"${NAME}"/build.sh


for input in $(find ../../fuzzer-test-suite/libxml2-v2.9.2 -type f ! -name "*.*" | cut -d "/" -f5)
do
    ../../pin/pin -t ../../scripts/build/bugdepthevaluation.so -o ../../results/libxml2-v2.9.2-"${input}"-bugdepthevaluation.out -- ./libxml2-v2.9.2-fsanitize_fuzzer ../../fuzzer-test-suite/libxml2-v2.9.2/"${input}"
done