#!/bin/bash

echo -e "\n\tChess"
../build/benchmark ../games/chess.rbg -n 1000 -s 0
../build/benchmark ../games/chess.rbg -d 5
