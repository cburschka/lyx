#PWD=`pwd`
. ./shared_variables.sh
if [ "$USER" != keytest ]
then
	echo USER is "$USER", not keytest! Exiting.
	exit
fi

for file in $ROOT_OUTDIR/to*/*pure
do
	if ! cat $file > /dev/null
	then
		echo cannot read $file, perhaps permissions are wrong?
		exit 1
	fi
done
#(DISPLAY=:1 sudo -u keytest -H nice -18 ; cd $PWD ; ./autolyx)
./killtest.sh # kill any previous test, so does not interfere
./autolyx
