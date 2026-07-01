#!/bin/bash

if [ ! -d "bin/obj/" ]; then
	echo "bin/obj/ not found, creating directory..."
	mkdir -p bin/obj/
fi

if [ ! -d "lib/replxx/sources/" ]; then
	echo "lib/replxx/sources/ not found, please create the directory and place Replxx sources in it"
	exit 1
fi

if [ ! -d "lib/replxx/headers/" ]; then
	echo "lib/replxx/headers/ not found, please create the directory and place Replxx headers in it"
	exit 1
fi

echo "Please be sure that you're at the root directory of the project"
echo "and lib/replxx/headers/ and lib/replxx/headers are properly populated."
echo "Another thing to note is that you need to have `ar` and `g++` installed."
read -p "Do you want to proceed? (y/N) " response

if [[ ! "$response" =~ ^[yY](es)?$ ]]; then
	echo "Cancelled request, exiting..."
	exit 1
fi

for file in lib/replxx/sources/*.{cpp,cxx,cc}; do
    [ -e "$file" ] || continue
    filename=$(basename "$file")
    basename="${filename%.*}"
    g++ -std=c++17 -c "$file" -Ilib/replxx/headers -o "bin/obj/${basename}.o"
done

ar rcs bin/libreplxx.a bin/obj/*.o

exit 0
