#! /bin/sh

mkdir -p build/
cd build/
cmake ..
make

./ray-tracer
display output.jpg
