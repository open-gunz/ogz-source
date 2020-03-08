#!/bin/bash

mkdir -p build/linux

cd build/linux

cmake ../../src/.
cmake --build . --config Release --target install