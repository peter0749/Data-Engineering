#!/bin/bash

for ((i=0; i<$1; ++i)); do
    echo -n "Testing on check point $((i+1))/$1 ... "
    if ( ./test $2 | ./judge.sh ); then
        echo "passed."
    else
        echo "wrong!"
    fi
done

