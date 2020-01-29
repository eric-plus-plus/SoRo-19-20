#! /bin/bash
#Parses the tempConfig file below
middle=$(cat tempConfig.txt | grep MIDDLE)
middle=${middle: -3}
left=$(cat tempConfig.txt | grep LEFT)
left=${left: -3}
right=$(cat tempConfig.txt | grep RIGHT)
right=${right: -3}
middle_file=$(v4l2-ctl --list-devices | grep -A1 "$middle):" | tail -1)
left_file=$(v4l2-ctl --list-devices | grep -A1 "$left):" | tail -1)
right_file=$(v4l2-ctl --list-devices | grep -A1 "$right):" | tail -1)

v4l2-ctl -d $middle_file --set-ctrl=focus_auto=0
v4l2-ctl -d $middle_file --set-ctrl=focus_absolute=0
v4l2-ctl -d $middle_file --set-ctrl=contrast=255
v4l2-ctl -d $middle_file --set-ctrl=sharpness=255

v4l2-ctl -d $left_file --set-ctrl=focus_auto=0
v4l2-ctl -d $left_file --set-ctrl=focus_absolute=0
v4l2-ctl -d $left_file --set-ctrl=contrast=255
v4l2-ctl -d $left_file --set-ctrl=sharpness=255

v4l2-ctl -d $right_file --set-ctrl=focus_auto=0
v4l2-ctl -d $right_file --set-ctrl=focus_absolute=0
v4l2-ctl -d $right_file --set-ctrl=contrast=255
v4l2-ctl -d $right_file --set-ctrl=sharpness=255

build/autonomous $left_file $middle_file $right_file

