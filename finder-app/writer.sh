#!/bin/bash

if [ $# -ne 2 ] #The $# variable will tell you the number of input arguments the script was passed.
then
	echo "No arguments passed"
	exit 1
else
	mkdir -p "$(dirname "$1")" && touch "$1"    #create a file and path if it doesn't exist

	if [ -f "$1" ]
	then
		echo "$2" > "$1"   #write in the file
	else
		echo "faild to create a file"        
		exit 1
	fi
fi

