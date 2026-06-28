# Segmentated Hirachically Untitled Shell
Welcome to shush! This is a terminal shell primarily oriented to portability and simple customization.<br>
Currently doesn't support many customizations. Self-brought config will be implemented in the future.<br>
When this is completed, this will not support scripting, jobs control nor redirectors like `if`, `else`, `while`,... (When a script is detected, it will use `system()` instead.)<br>
Potentially cancerous code.<br>
(Use `gcc -c linenoise.c -o linenoise.o && g++ main.cpp linenoise.o -o shush` to build and you may want to specifically use `-std=c++17` too)<br>
### This branch is for prerelease snapshots of snapshot 5. Please refer to the main branch for stable codebases.
