#!/bin/bash
# Example test call

cd ../rbg2cpp

echo -e "\n\tchess-fiftyMove"
make simulate_chess-fiftyMove SIMULATIONS=10000
make perft_chess-fiftyMove DEPTH=5
