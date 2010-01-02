#PWD=`pwd`
if [ "$USER" != keytest ]
then
	echo USER is "$USER", not keytest! Exiting.
	exit
fi
#(DISPLAY=:1 sudo -u keytest -H nice -18 ; cd $PWD ; ./autolyx)
./killtest.sh # kill any previous test, so does not interfere
./autolyx
