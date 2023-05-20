#!/bin/bash

# Run tigerbeetle start in the background
./zig-out/bin/tigerbeetle start $@ &
echo "Tigerbeetle start process ID: $!"

# Save the process ID to a file for later use
echo $! > $PWD/tigerbeetle.pid

# Read the process ID from the saved file
pid=$(cat $PWD/tigerbeetle.pid)

sleep 5
# Kill the tigerbeetle start process
kill -9 "$pid"
kill -9 $(echo $!)