#! /bin/bash
#Parses the config file below
middle=$(cat config.txt | grep MIDDLE)
middle=${middle: -3}
left=$(cat config.txt | grep LEFT)
left=${left: -3}
right=$(cat config.txt | grep RIGHT)
right=${right: -3}
middle_file=$(v4l2-ctl --list-devices | grep -A1 "$middle):" | tail -1)
left_file=$(v4l2-ctl --list-devices | grep -A1 "$left):" | tail -1)
right_file=$(v4l2-ctl --list-devices | grep -A1 "$right):" | tail -1)

v4l2-ctl -d $middle_file --set-ctrl=focus_auto=0
v4l2-ctl -d $middle_file --set-ctrl=focus_absolute=0
v4l2-ctl -d $middle_file --set-ctrl=contrast=255
v4l2-ctl -d $middle_file --set-ctrl=sharpness=255
v4l2-ctl -d $middle_file --set-ctrl=saturation=0

v4l2-ctl -d $left_file --set-ctrl=focus_auto=0
v4l2-ctl -d $left_file --set-ctrl=focus_absolute=0
v4l2-ctl -d $left_file --set-ctrl=contrast=255
v4l2-ctl -d $left_file --set-ctrl=sharpness=255
v4l2-ctl -d $left_file --set-ctrl=saturation=0

v4l2-ctl -d $right_file --set-ctrl=focus_auto=0
v4l2-ctl -d $right_file --set-ctrl=focus_absolute=0
v4l2-ctl -d $right_file --set-ctrl=contrast=255
v4l2-ctl -d $right_file --set-ctrl=sharpness=255
v4l2-ctl -d $right_file --set-ctrl=saturation=0

build/autonomous $middle_file $left_file $right_file

