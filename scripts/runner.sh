#!/usr/bin/env bash
set -eEuo pipefail

ZIG=$1
ZIG_BUILD_TYPE=$2
# TB_ADDRESS=$3
CLIENT=$3

echo ""
echo "running client..."
$ZIG build run $ZIG_BUILD_TYPE -- $CLIENT
echo ""
$ZIG uninstall