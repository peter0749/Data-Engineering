#!/bin/bash

grep -P "^$1[^\t]*" sentances_sorted.txt > "$2"

