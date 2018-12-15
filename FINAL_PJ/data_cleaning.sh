#!/bin/bash
find "$1" -type f -exec sh -c '(perl -ne "s/\<[a-zA-Z]+.*\>//g; s/[ \t]//g; print;" "$1" | opencc) > "$1.t" && mv "$1.t" "$1" ' sh "{}" \;

