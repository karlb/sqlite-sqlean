#!/bin/bash
set -eo pipefail

rm -rf sqlean/*
git clone --depth=1 git@github.com:nalgeon/sqlean.git sqlean-checkout
cp -r sqlean-checkout/{src,test} sqlean
rm -rf sqlean-checkout

SQLITE_BRANCH="3.36"
curl -L "https://github.com/sqlite/sqlite/raw/branch-$SQLITE_BRANCH/ext/misc/json1.c" --output sqlean/src/sqlite3-json1.c
