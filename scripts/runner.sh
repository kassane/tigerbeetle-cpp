#!/usr/bin/env bash
set -eEuo pipefail

function onerror {
    if [ "$?" == "0" ]; then
        rm running.log
        echo 'Done!!'
    else
        echo "Error running with tigerbeetle"
        cat running.log
    fi

    kill %1
}
trap onerror EXIT

TB_ADDRESS=$2

# Be careful to use a running-specific filename so that we don't erase a real data file:
FILE="$PWD/0_0.tigerbeetle"
if [ -f "$FILE" ]; then
    rm "$FILE"
fi

./tigerbeetle format --cluster=0 --replica=0 --replica-count=1  "$FILE" > running.log 2>&1
echo "Starting replica 0"
./tigerbeetle start --addresses=$TB_ADDRESS  "$FILE" > running.log 2>&1 &

echo ""
echo "running client..."
$1
echo ""

if [ -f "$FILE" ]; then
    rm "$FILE"
fi
