#!/bin/bash

echo "Please make sure you're at the root directory of the project."
echo "For this script to run properly, dependencies needed are: g++"
read -p "Are you sure you want to proceed? (y/N): " response

if [[ "$response" =~ ^[yY](es)?$ ]]; then
  	read -p "Do you want to debug? (y/N): " response
  	
  	if [[ "$response" =~ ^[yY](es)?$ ]]; then
  	    echo "Building binary with libreplxx.a, please wait..."
  	  	g++ -g -o -std=c++17 src/main.cpp -Ilib/replxx/headers -Lbin/ -lreplxx -o shush
  	  	echo "Please see g++ messages if there are any."
  	else
        echo "Building binary with libreplxx.a, please wait..."
 	    g++ -std=c++17 src/main.cpp -Ilib/replxx/headers -Lbin/ -lreplxx -o shush
  	fi

  	echo "Please see g++ messages if there are any."
else
    echo "Requested cancellation, exiting"
    exit 1
fi
