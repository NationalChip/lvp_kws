#!/bin/bash

#echo $#

if [ $# != 1 ]; then
	echo "usage : $0 value"
	exit
fi

value=$1
new_value=${value:6:2}${value:4:2}${value:2:2}${value:0:2}
#echo "value : $value new_value : $new_value" > ./lalalalal
echo "$new_value"
