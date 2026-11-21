#!/bin/sh
# Fetch the pinned upstream data without downloading its full history.
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
git -C "$root" submodule update --init --depth 1 -- game
git -C "$root/game" submodule update --init --depth 1 --jobs 4 -- 'data/*'
