#!/bin/bash

mkfileP() { mkdir -p "$(dirname "$1")" || return; touch "$1"; }

DIR1=$1
TGZ_PREFIX=$2
TARGET_DIR=$3

TEMP=$(mktemp)

tar -zxvf "$TGZ_PREFIX.tar.gz"

$(cd ${TGZ_PREFIX} && find . -type f > ${TEMP})

while read line; do
    filepath="$TARGET_DIR/$line"
    mkfileP ${filepath}
    ./delta_resolver "$DIR1/$line" "$TGZ_PREFIX/$line" 2000 > "$filepath"
done <${TEMP}

rm -f "$TEMP"

