#!/bin/bash

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
apt -y install cmake
apt -y install golang
apt -y install ragel
apt -y install libbz2-dev libxml2-dev libxslt-dev python-dev libssl-dev liblzma-dev
apt -y install nasm
apt -y install subversion
apt -y install libgcrypt-dev
apt -y install libarchive-dev

# config to have all we have want
PIN=$(find . -type d -iname pin)
FUZZER=$(find . -type d -iname fuzzer-test-suite)

if [ -z "${PIN}" ] && [ -z "${FUZZER}" ]
then
    wget https://software.intel.com/sites/landingpage/pintool/downloads/pin-3.18-98332-gaebd7b1e6-gcc-linux.tar.gz
    tar -xzf pin-3.18-98332-gaebd7b1e6-gcc-linux.tar.gz && mv pin-3.18-98332-gaebd7b1e6-gcc-linux pin && rm -rf pin-3.18-98332-gaebd7b1e6-gcc-linux.tar.gz
    git clone https://github.com/google/fuzzer-test-suite.git
fi