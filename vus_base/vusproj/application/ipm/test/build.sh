#!/bin/bash

cd $(dirname $0)


if [ ! -d build ]; then
   mkdir build
else
   rm build/* -R
fi

cd build
cmake ..
make

cp ../data ./ -R
cd -
