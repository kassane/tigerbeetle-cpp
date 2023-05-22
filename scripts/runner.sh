#!/usr/bin/env bash
set -eEuo pipefail

COLOR_RED='\033[1;31m'
COLOR_END='\033[0m'

function onerror {
    if [ "$?" == "0" ]; then
        rm running.log
        echo 'Done!!'
    else
        echo -e "${COLOR_RED}"
        echo "Error running with tigerbeetle"
        echo -e "${COLOR_END}"
        cat running.log
    fi

    kill %1
}
trap onerror EXIT


# Be careful to use a running-specific filename so that we don't erase a real data file:
FILE="$PWD/0_0.tigerbeetle"
if [ -f "$FILE" ]; then
    rm "$FILE"
fi

./zig-out/bin/tigerbeetle format --cluster=0 --replica=0 --replica-count=1  "$FILE" > running.log 2>&1
echo "Starting replica 0"
./zig-out/bin/tigerbeetle start --addresses=0.0.0.0:3001  "$FILE" > running.log 2>&1 &

echo ""
echo "running client..."
# shellcheck disable=SC2086
../../tb_cpp
echo ""

if [ -f "$FILE" ]; then
    rm "$FILE"
fi
