#!/bin/bash

mkfileP() { mkdir -p "$(dirname "$1")" || return; touch "$1"; }

DIR1=$1
DIR2=$2
TARGET_DIR=$3

TEMP=$(mktemp)

$(cd ${DIR2} && find . -type f > ${TEMP})

while read line; do
    filepath="$TARGET_DIR/$line"
    mkfileP ${filepath}
    ./copy_append_model "$DIR1/$line" "$DIR2/$line" 12 > "$filepath"
done <${TEMP}

tar -zcvf "$TARGET_DIR.tar.gz" ${TARGET_DIR}

rm -f "$TEMP"
rm -rf ${TARGET_DIR}

