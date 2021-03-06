#!/bin/sh

# System call by command-line:

cd ../build/

### Following command generates output mesh for manual placeholders.

./cpp-service \
 --generate \
 -mesh ~/repos/cpp-service/test-input/teapot_10elev.stl \
 -config ~/repos/cpp-service/test-input/config.json \
 -points ~/repos/cpp-service/test-input/points.json \
 -output ~/output_mesh_manual.stl

### Following commands get output auto points.
### Then according to auto points, output mesh is generated.

PATH_output_auto_points="$HOME/auto_points.json" # Output of 1st command and input of 2nd command

./cpp-service \
 --get-points \
 -mesh ~/repos/cpp-service/test-input/teapot_10elev.stl \
 -config ~/repos/cpp-service/test-input/config.json \
 -slices ~/repos/cpp-service/test-input/slices.json \
 -args ~/repos/cpp-service/test-input/autoargs.json \
 -output ${PATH_output_auto_points}

./cpp-service \
 --generate \
 -mesh ~/repos/cpp-service/test-input/teapot_10elev.stl \
 -config ~/repos/cpp-service/test-input/config.json \
 -points ${PATH_output_auto_points} \
 -output ~/output_mesh_auto.stl

### Following command hollows a mesh.

./cpp-service \
 --hollow \
 -mesh ~/repos/cpp-service/test-input/teapot_10elev.stl \
 -config ~/repos/cpp-service/test-input/config_hollow.json \
 -output ~/output_hollowed.stl
