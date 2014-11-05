#!/usr/bin/sh
set -x  # Output executed commands
set -e  # Make script fail as soon as one command fails

read MESSAGE

git add -A
git commit -m "$MESSAGE"
git push origin master