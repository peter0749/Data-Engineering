#!/bin/bash

./parser
sync
sort --parallel=3 sentences.txt > dataset.txt

