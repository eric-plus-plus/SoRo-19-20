#! /bin/bash
#Parses the config file below
main=$(cat config.txt | grep MAIN)
main=${main: -3}
left=$(cat config.txt | grep LEFT)
left=${left: -3}
right=$(cat config.txt | grep RIGHT)
right=${right: -3}
main_file=$(v4l2-ctl --list-devices | grep -A1 "$main):" | tail -1)
left_file=$(v4l2-ctl --list-devices | grep -A1 "$left):" | tail -1)
right_file=$(v4l2-ctl --list-devices | grep -A1 "$right):" | tail -1)

echo $main_file $left_file $right_file

#changes to camera settings to be optimized for what we need
v4l2-ctl -d $main_file --set-ctrl=focus_auto=0
v4l2-ctl -d $main_file --set-ctrl=focus_absolute=0
v4l2-ctl -d $main_file --set-ctrl=contrast=255
v4l2-ctl -d $main_file --set-ctrl=sharpness=255

v4l2-ctl -d $left_file --set-ctrl=focus_auto=0
v4l2-ctl -d $left_file --set-ctrl=focus_absolute=0
v4l2-ctl -d $left_file --set-ctrl=contrast=255
v4l2-ctl -d $left_file --set-ctrl=sharpness=255

v4l2-ctl -d $right_file --set-ctrl=focus_auto=0
v4l2-ctl -d $right_file --set-ctrl=focus_absolute=0
v4l2-ctl -d $right_file --set-ctrl=contrast=255
v4l2-ctl -d $right_file --set-ctrl=sharpness=255

cd build
./autonomous $main_file $left_file $right_file
#gdb autonomous $main_file $left_file $right_file #switch to this to debug
