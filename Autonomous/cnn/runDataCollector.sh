#! /bin/bash

# Arguments: video file, output file

#adjusts camera settings to be able to see the tag better
v4l2-ctl -d $1 --set-ctrl=focus_auto=0
v4l2-ctl -d $1 --set-ctrl=focus_absolute=0
v4l2-ctl -d $1 --set-ctrl=contrast=255
v4l2-ctl -d $1 --set-ctrl=sharpness=255

cd build
./collectData $1 ../$2 ../$3
