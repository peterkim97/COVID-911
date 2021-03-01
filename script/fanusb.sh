#!/bin/sh


echo '2525'|sudo -S sh -c 'echo 255> /sys/devices/pwm-fan/target_pwm'
echo '2525'|sudo -S chmod 777 /dev/ttyUSB0
