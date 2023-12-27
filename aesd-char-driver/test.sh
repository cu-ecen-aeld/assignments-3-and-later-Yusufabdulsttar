#!/bin/sh
#--------------------------------------
# Author: Yusuf Abdulsttar
#--------------------------------------

echo "Clean, Clean ,unload ,make ,load"

make clean


./aesdchar_unload

make

./aesdchar_load 

../assignment-autotest/test/assignment9/drivertest.sh

make clean

echo "Compleat Clean ,unload ,make ,load"
