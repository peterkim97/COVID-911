#! /bin/sh
echo "Echo Test2"
printf "print Test2\n"
printf "Name of Script %s\n" $0


sleep 3
cd ../mask_detection && python3 real_time_mask_detection.py





