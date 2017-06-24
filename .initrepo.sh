#!/bin/bash
if [ $# -eq 0 ]; then
	echo "ERROR: Missing directory"
	exit 0
fi
if [ $# -ne 1 ]; then
	echo "ERROR: Wrong number of parameters"
	exit 0
fi
if [ ! -d $1 ]; then
	echo "ERROR: $1 is not a directory"
	exit 0
fi
DIR="$1"
if [ `expr index "$DIR" "/"` == 0 ]; then
	DIR="$DIR/"
fi
FILES=(`ls -a $1`)
#First two arguments are "." and ".."
#Should ignore them
for ((i=2;i<${#FILES[@]};i++)); do
	if [ -e ${FILES[i]} ]; then
		echo "Removed: ${FILES[i]}"
		rm -rf ${FILES[i]}
	fi
	mv $DIR${FILES[i]} .
	echo "Copied: $DIR${FILES[i]}"
done
rm -rf $DIR
echo "Removed: $DIR"
exit 0
