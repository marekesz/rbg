#!/bin/bash
# Example test call

echo -e "\n\tchess-fiftyMove"
../build/benchmark ../games/chess-fiftyMove.rbg -n 1 -s 0
../build/benchmark ../games/chess-fiftyMove.rbg -d 5
