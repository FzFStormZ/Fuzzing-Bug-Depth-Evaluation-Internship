#!/bin/bash

wget https://software.intel.com/sites/landingpage/pintool/downloads/pin-3.18-98332-gaebd7b1e6-gcc-linux.tar.gz

tar -xzf pin-3.18-98332-gaebd7b1e6-gcc-linux.tar.gz && mv pin-3.18-98332-gaebd7b1e6-gcc-linux pin && rm -rf pin-3.18-98332-gaebd7b1e6-gcc-linux.tar.gz

git clone https://github.com/google/fuzzer-test-suite.git

