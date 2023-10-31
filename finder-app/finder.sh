#!/bin/bash

if [ $# -ne 2 ]  #The $# variable will tell you the number of input arguments the script was passed.
then
	echo "Incorrect Number of Arguments"
	exit 1
elif [ -d $1 ]
then
	NumOfFile=$(find $1 -type f | wc -l)
	NumOfLine=$(grep -r  $2 $1| wc -l)
	echo The number of files are $NumOfFile and the number of matching lines are $NumOfLine
else
	echo "not a directory"
	exit 1
fi	
