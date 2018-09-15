#!/bin/bash

grep -P "^$1[^\t]*" dataset.txt > "$2"

