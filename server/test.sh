#!/bin/sh
#--------------------------------------
# Author: Yusuf Abdulsttar
#--------------------------------------

echo "Clean, Clean ,unload ,make ,load"

make clean

make
../aesd-char-driver/aesdchar_unload
../aesd-char-driver/aesdchar_load 

./aesdsocket

echo "Compleat Clean ,unload ,make ,load"
