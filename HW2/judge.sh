#!/bin/bash
OUTPUT1="$(mktemp -t oj-test-input-1-XXXXXX)"
OUTPUT2="$(mktemp -t oj-test-input-2-XXXXXX)"
tee >(./rsort -m 512 -j 7 > "$OUTPUT1")  >(sort --parallel=7 > "$OUTPUT2") > /dev/null
diff "$OUTPUT1" "$OUTPUT2" -q &> /dev/null
RET=$?
rm -f "$OUTPUT1"
rm -f "$OUTPUT2"
exit $RET

