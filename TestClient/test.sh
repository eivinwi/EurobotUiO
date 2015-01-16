#!/bin/sh
set -x  # Output executed commands
set -e  # Make script fail as soon as one command fails

#./testclient 2 0 100 100
#./testclient 2 1 250 150
#./testclient 3 2 150
#./testclient 2 3 500 -500
#./testclient 2 4 0 0


./testclient 2 0 200 0
./testclient 2 1 0 200
./testclient 2 2 200 200
./testclient 2 3 0 0 
./testclient 3 4 100
./testclient 3 5 0