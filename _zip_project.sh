#!/bin/bash 
cd "$(dirname "$0")"
echo 'Compiling zip.'

cp -v ./Debug/Project.hex ./Project.hex

date="$(date +"%Y-%m-%dT%H%M%S")"
zip "_45294583_CSSE2010_Project.zip" _features.pdf Project.hex *.c *.h 
rm -fv ./Project.hex
