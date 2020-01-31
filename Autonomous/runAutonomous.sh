#! /bin/bash
#Parses the tempConfig file below
main=$(cat tempConfig.txt | grep MAIN)
main=${main: -3}
secondary=$(cat tempConfig.txt | grep SECONDARY)
secondary=${secondary: -3}
main_file=$(v4l2-ctl --list-devices | grep -A1 "$main):" | tail -1)
secondary_file=$(v4l2-ctl --list-devices | grep -A1 "$secondary):" | tail -1)

echo $main_file $secondary_file

v4l2-ctl -d $main_file --set-ctrl=focus_auto=0
v4l2-ctl -d $main_file --set-ctrl=focus_absolute=0
v4l2-ctl -d $main_file --set-ctrl=contrast=255
v4l2-ctl -d $main_file --set-ctrl=sharpness=255

v4l2-ctl -d $secondary_file --set-ctrl=focus_auto=0
v4l2-ctl -d $secondary_file --set-ctrl=focus_absolute=0
v4l2-ctl -d $secondary_file --set-ctrl=contrast=255
v4l2-ctl -d $secondary_file --set-ctrl=sharpness=255

cd build
./autonomous $secondary_file $main_file

