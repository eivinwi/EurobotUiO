#!/bin/sh
set -x  # Output executed commands
set -e  # Make script fail as soon as one command fails

# Testing main program by sending a bunch of commands via testclient.
# If working correctly the commands should be queued up and performed chronologically.

./testclient 2 0 980 0
./testclient 5 0 0 380 380
./testclient 5 0 0 100 100
./testclient 2 0 1280 0 
./testclient 5 0 380 380
./testclient 3 0 90
./testclient 5 0 0 50 50
./testclient 2 0 1280 600
./testclient 5 0 380 380
./testclient 2 0 400 0
./testclient 2 0 0 0