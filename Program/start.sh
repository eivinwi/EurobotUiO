#!/bin/sh
set -x
set -e

(cd /home/bendik/Eurobot-andre/Eurobot/Kalmanfilter && exec ./exec) &
(cd /home/bendik/eurobot/Program && exec ./control) && fg
 