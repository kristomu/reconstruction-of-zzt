#!/bin/sh

FAILS=""

while read -d $'\n' NAME
do
	timeout --foreground 5 ./fuzzt $NAME 2>/dev/null
	if [ $? -eq 0 ]
	then
		echo "$NAME: pass"
	else
		echo >&2 "$NAME: fail"
		FAILS="$FAILS"$'\n'"$NAME"
	fi
done <<< "$(find ./testcase -iname "*.zzt")"

FAILS=`echo "$FAILS"|sort`

echo
echo "Test failures: $FAILS"
