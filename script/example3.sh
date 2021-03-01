#! /bin/sh
echo "Echo Test3"
printf "print Test3\n"
printf "Name of Script %s\n" $0

sleep 15
cd ../foot_detection && python3 real_time_foot_detection.py
