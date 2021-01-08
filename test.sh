#!/bin/sh

FAILS=""

while read -d $'\n' NAME
do
	BEFORE_TIME=`date +%s.%N`
	timeout --foreground 5 ./fuzzt $NAME 2>/dev/null
	if [ $? -eq 0 ]
	then
		AFTER_TIME=`date +%s.%N`
		ELAPSED=`echo "$AFTER_TIME-$BEFORE_TIME"|bc`
		echo "$NAME: pass ( $ELAPSED s )"
	else
		echo >&2 "$NAME: fail"
		FAILS="$FAILS"$'\n'"$NAME"
	fi
done <<< "$(find ./testcase -iname "*.zzt")"

FAILS=`echo "$FAILS"|sort`

echo
echo "Test failures: $FAILS"
