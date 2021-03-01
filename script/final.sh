#! /bin/sh
printf "Run All\n"
parallel -u ::: './fanusb.sh' './example0.sh' './example1.sh' './example2.sh' './example3.sh'

