AUTOMAKE_OPTIONS = foreign
DISTCLEANFILES= *.orig *.rej *~ *.bak lyx.1 core
MAINTAINERCLEANFILES = Makefile.in aclocal.m4 configure \
	development/lyx.spec
SUBDIRS = intl po src lib

EXTRA_DIST = ANNOUNCE OLD-CHANGES INSTALL.OS2 INSTALL.autoconf README.OS2 \
	UPGRADING lyx.man \
	config development forms images
ETAGS_ARGS = --c++
man_MANS = lyx.1
lyx.1:
	cp -p $(srcdir)/lyx.man lyx.1

dist-hook:
	cd $(distdir) ; rm -rf `find config -name \*CVS\*` ; \
	rm -rf `find development -name \*CVS\*` ; \
	rm -rf `find forms -name \*CVS\*` ; \
	rm -rf `find images -name \*CVS\*`

rpmdist: dist
	ln -s images/lyx.xpm . ; \
	rpm -ta ${PACKAGE}-${VERSION}.tar.gz ; \
	rm lyx.xpm

