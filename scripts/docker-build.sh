#!/bin/sh
set -e

npm install
node build.mjs
node --test tests/io.test.js
node --test tests/io.async.test.js
