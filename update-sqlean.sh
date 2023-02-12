#!/bin/bash
set -eo pipefail

rm -rf sqlean/*
git clone --depth=1 git@github.com:nalgeon/sqlean.git sqlean-checkout
cp -r sqlean-checkout/{src,test} sqlean
rm -rf sqlean-checkout

SQLITE_BRANCH="3.36"
curl -L "https://github.com/sqlite/sqlite/raw/branch-$SQLITE_BRANCH/ext/misc/json1.c" --output sqlean/src/sqlite3-json1.c

# Include sqlite3 headers to allow building without installing sqlite3-dev
# package. I would prefer to get those directly from the sqlite repo, but
# sqlite3.h requires a build step which I didn't want to include here.
cp sqlite3/* sqlean/src/
