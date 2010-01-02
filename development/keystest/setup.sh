
#This won't work on an RPM based distro. Easy to fix though.
sudo apt-get install xclip xvkbd wmctrl xvfb libqt4-dbg icewm #svn pylint
sudo adduser keytest < /dev/null

if ! grep keytest /etc/sudoers
then
	echo allowing admin users to switch to keytest user
	#echo '%adm ALL =(keytest) NOPASSWD: ALL' >> /etc/sudoers
fi
# cat /mnt/jaunty/etc/cups/printers.conf |grep -o '[^ ]*>$' |grep -v '^<'| sed 's/>$//'

#we should really handle each printer seperately, but this will work if they are similar 
if grep AllowUser /etc/cups/printers.conf
then
	echo printer: using whitelisting, nothings needs be done.
	exit
fi
	
if grep DenyUser /etc/cups/printers.conf
then
	echo There are already denied users. We do not support this yet, exiting
	exit
fi

#(cd /etc/cups/ppd/ && ls *.ppd) | sed s/.ppd$// | while read L
cat /etc/cups/printers.conf |grep -o '[^ ]*>$' |grep -v '^<'| sed 's/>$//' | while read L
do
	echo $L
	echo lpadmin -p $L -u deny:keytest
	lpadmin -p $L -u deny:keytest
done
