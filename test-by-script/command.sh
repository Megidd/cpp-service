#!/bin/sh

# System call by command-line:

cd ~/repos/cpp-service/build/

./cpp-service \
 --get-points \
 -mesh ~/repos/cpp-service/test-input/teapot_10elev.stl \
 -config ~/repos/cpp-service/test-input/config.json \
 -slices ~/repos/cpp-service/test-input/slices.json \
 -args ~/repos/cpp-service/test-input/autoargs.json \
 -outputpoints ~/auto_points.json

./cpp-service \
 --generate \
 -mesh ~/repos/cpp-service/test-input/teapot_10elev.stl \
 -config ~/repos/cpp-service/test-input/config.json \
 -points ~/repos/cpp-service/test-input/points.json \
 -outputmesh ~/output_mesh.stl
