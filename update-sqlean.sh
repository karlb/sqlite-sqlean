#!/bin/bash
set -eo pipefail

rm -rf sqlean/*
git clone --depth=1 git@github.com:nalgeon/sqlean.git sqlean-checkout
cp -r sqlean-checkout/{src,test} sqlean
rm -rf sqlean-checkout
