#!/bin/bash

echo "Hello! This is a script to build this program and its libraries. Please follow the instructions below."
echo "Before you start, you want to make sure you're at the root directory of the project."
echo "This script will literally not care if any of these subscripts fail, please be certain."

read -p "Are you sure that you want to use this script? (y/N): " responseA
if [[ ! "$responseA" =~ ^[yY](es)?$ ]]; then
    echo "Requested cancellation, exiting..."
    exit 1
fi

read -p "Do you want to (re)build static libraries? (y/N): " responseB
if [[ "$responseB" =~ ^[yY](es)?$ ]]; then
   	bash ./mklib.sh
fi

read -p "Do you want to (re)build the program? (y/N): " responseC
if [[ "$responseC" =~ ^[yY](es)?$ ]]; then
   	bash ./mkbin.sh
fi

echo "The task is done, you may now exit..."
exit 0
