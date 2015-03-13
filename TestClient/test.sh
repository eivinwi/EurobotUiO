#!/bin/sh
set -x  # Output executed commands
set -e  # Make script fail as soon as one command fails

# Testing main program by sending a bunch of commands via testclient.
# If working correctly the commands should be queued up and performed chronologically.

./testclient 2 0 100 100
./testclient 2 1 250 150
./testclient 2 2 0 0

./testclient 4 3 2
./testclient 4 4 1
./testclient 4 5 0

./testclient 4 6 3
./testclient 4 6 4
./testclient 4 6 3
./testclient 4 6 4