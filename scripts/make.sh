#!/bin/bash
# This script builds the whole project.

cd ../
echo "Building interpreter and parser"
mkdir build -p
cd build
pwd
cmake ../rbgInterpreter/ -DCMAKE_BUILD_TYPE=Release
make -j4

echo "Building compiler"
cd ../rbg2cpp
make -j4

echo "Done."
