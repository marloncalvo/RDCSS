center() {
  termwidth="$(tput cols)"
  padding="$(printf '%0.1s' ={1..500})"
  printf '%*.*s %s %*.*s\n' 0 "$(((termwidth-2-${#1})/2))" "$padding" "$1" 0 "$(((termwidth-1-${#1})/2))" "$padding"
}

center "Compiling"

if ! make; then
	center "Error Compiling"
	exit 1
fi

center "Begin Tests"
echo "Good luck :)"

OUT="data.txt"

HEADER=""
MAX_N=""
OPS=""

for i in {1..16}
do
	center "$(printf 'Performing with %2d thread(s)' $i)"
	res=$(./test $i |& tee /dev/tty)
	n=$(echo $res | grep -o "[0-9][0-9][0-9][0-9]*")
	HEADER+="$i"
	MAX_N+="$n"
	OPS+=$(( n / 75 ))

	if [ "$i" -ne "16" ]; then
		HEADER+=","
		MAX_N+=","
		OPS+=","
	fi
done
rm -f $OUT 
touch $OUT
echo "$HEADER" >> $OUT
echo "$MAX_N" >> $OUT
echo "$OPS" >> $OUT