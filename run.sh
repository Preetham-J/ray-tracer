#! /bin/sh

rm a.out
rm output.ppm

g++ main.cpp
./a.out

display output.ppm
