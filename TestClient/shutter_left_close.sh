#!/bin/sh
set -x  # Output executed commands
set -e  # Make script fail as soon as one command fails

# Testing main program by sending a bunch of commands via testclient.
# If working correctly the commands should be queued up and performed chronologically.

./testclient 4 0 6
