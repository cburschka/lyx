Summary:   A WYSIWYG frontend to LaTeX
Name:      lyx
Version:   1.0.2
Release:   1
Source:    ftp://ftp.lyx.org/pub/lyx/stable/lyx-%PACKAGE_VERSION.tar.gz
Copyright: GPL
Group:     X11/Applications/Publishing
Requires:  xforms >= 0.88, tetex-xdvi, tetex, tetex-latex
Packager:  Mate Wierdl <mw@moni.msci.memphis.edu>
URL:       http://www.lyx.org
Buildroot: /tmp/lyx-root
%description
LyX is a wordprocessor frontend to LaTeX, which gives both the
ease-of-use of a wordprocessor, and the flexibility and power of LaTeX.

%prep

%setup

%build

CXXFLAGS="$RPM_OPT_FLAGS" ./configure --with-included-gettext 

make

%install
#rename the installation root more appropriately
INST_ROOT=$RPM_BUILD_ROOT
rm -rf $INST_ROOT

make	install prefix=$INST_ROOT/usr/local 
 
cp -a images/lyx.xpm $INST_ROOT/usr/local/share/lyx/images

strip $INST_ROOT/usr/local/bin/lyx

# Have a copy of the reLyX README in the doc directory
cp lib/reLyX/README reLyX.README
%post
# Before configuring lyx for the local system
# PATH needs to be imported 
if [ -f /etc/profile ]; then
    . /etc/profile
fi

# Now configure LyX
echo "Configuring LyX for your system..."
cd /usr/local/share/lyx
./configure --srcdir

# Take care of leftover lyxrc in /etc
if [ -f /etc/lyxrc ]; then
    echo "Moving the old config file /etc/lyxrc to" 
    echo "/usr/local/share/lyx/lyxrc-from-etc"
    echo "If you do not need it delete it."     
    mv /etc/lyxrc /usr/local/share/lyx/lyxrc-from-etc
fi

%clean
#rename the installation root more appropriately
INST_ROOT=$RPM_BUILD_ROOT
rm -rf $INST_ROOT

%files
%doc ANNOUNCE BUGS CHANGES COPYING README
%doc ABOUT-NLS UPGRADING WHATSNEW  reLyX.README
/usr/local/bin/*
/usr/local/man/*/*
/usr/local/share/lyx
/usr/local/share/locale/*/LC_MESSAGES/lyx.mo

