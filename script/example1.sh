#! /bin/sh
echo "Echo Test1"
printf "print Test1\n"
printf "Name of Script %s\n" $0


cd ~/catkin_ws && roslaunch motor_final motor_final.launch
