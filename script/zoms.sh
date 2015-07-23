#!/bin/sh

USAGE="zombies.sh app-name [-k(ill]"

if [ -z "$1" ]; then echo "usage: $USAGE"; exit 1; fi

APP="$1"

echo "$@" | grep -q '\-k'
kill=$?

ps -e | grep "$APP" | grep -v grep | grep -v zoms

if [ $kill -eq 0 ]; then
  ps -e | grep "$APP" | grep -v grep | grep -v zoms | awk '{ print $1 }' | xargs kill
fi
