#!/bin/bash

echo "This script copy the default configurations from this repository to ~, intended for snapshot 6.14 and up."
echo "Please make sure you're at the root directory of the project and no valuable configs are in ~/.shushcfg/"
read -p "Are you sure you want to proceed? (y/N): " response

if [[ "$response" =~ ^[yY](es)?$ ]]; then
	rm -rf ~/.shushcfg/
   	mkdir -p ~/.shushcfg/
   	cp .shushcfg/config.shush ~/.shushcfg/config.shush
   	cp .shushcfg/alias.shush ~/.shushcfg/alias.shush
   	echo "Copied default configurations successfully, exiting..."
else
    echo "Requested cancellation, exiting"
    exit 1
fi
