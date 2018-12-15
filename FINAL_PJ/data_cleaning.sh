#!/bin/bash
ls data | while read line; do opencc -i "./data/$line" > "./data/$line.zhTW"; mv "./data/$line.zhTW" "./data/$line" ; done

