AUTOMAKE_OPTIONS = foreign
DISTCLEANFILES= *.orig *.rej *~ *.bak lyx.1 core
MAINTAINERCLEANFILES = Makefile.in aclocal.m4 configure
SUBDIRS = intl po src lib
EXTRA_DIST = ANNOUNCE CHANGES INSTALL.OS2 INSTALL.autoconf README.OS2 \
	UPGRADING WHATSNEW acconfig.h lyx.man \
	config development forms images
ETAGS_ARGS = --c++
man_MANS = lyx.1
lyx.1:
	cp -p $(srcdir)/lyx.man lyx.1
