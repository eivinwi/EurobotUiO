#!/bin/sh
set -x  # Output executed commands
set -e  # Make script fail as soon as one command fails

# Testing main program by sending a bunch of commands via testclient.
# If working correctly the commands should be queued up and performed chronologically.

./testclient 3 0 270
./testclient 3 0 180
./testclient 3 0 90
./testclient 3 0 0
