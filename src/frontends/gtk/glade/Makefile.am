DISTCLEANFILES = *.orig *.rej *~ *.bak *.gladep
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in

GLADE_DIR = glade

GLADE_FILES = *.glade

EXTRA_DIST = ${GLADE_FILES}

gladeinstalldirs:
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/${GLADE_DIR} ;

install-data-local: gladeinstalldirs
	files=`cd $(srcdir) ; echo $(GLADE_FILES)` ; \
	for i in $${files} ; do \
	   $(INSTALL_DATA) $(srcdir)/$$i $(DESTDIR)$(pkgdatadir)/${GLADE_DIR}/$$i ; \
	done

dist-hook:
	cd $(distdir) ; rm -rf `find . -name \*CVS\*` ;
