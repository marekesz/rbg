#!/bin/bash

cd ../rbg2cpp

echo -e "\n\tChess"
make simulate_chess SIMULATIONS=1000
make perft_chess DEPTH=5

