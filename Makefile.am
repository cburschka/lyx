AUTOMAKE_OPTIONS = foreign
DISTCLEANFILES = -r *.orig *.rej *~ *.bak lyx.1 core $(srcdir)/sourcedoc/*~ \
	$(srcdir)/sourcedoc/html $(srcdir)/sourcedoc/latex \
	$(srcdir)/sourcedoc/man $(srcdir)/sourcedoc/Doxyfile
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in $(srcdir)/aclocal.m4 \
	$(srcdir)/configure $(srcdir)/development/lyx.spec \
	$(srcdir)/acinclude.m4 $(srcdir)/sourcedoc/Doxyfile
SUBDIRS = intl po sigc++ boost src lib

EXTRA_DIST = ANNOUNCE INSTALL.OS2 INSTALL.autoconf README.OS2 \
	UPGRADING lyx.man acconfig.h autogen.sh \
	config development forms images sourcedoc

#ETAGS_ARGS = --language-force=c++
man_MANS = lyx.1

## Needed by bindist
bindistdir=lyxbin
bindistfile=$(PACKAGE)-$(VERSION)-bin.tar.gz

lyx.1:
	cp -p $(srcdir)/lyx.man lyx.1

dist-hook:
	cd $(distdir) ; rm -rf `find config -name \*CVS\*` ; \
	rm -rf `find development -name \*CVS\*` ; \
	rm -rf `find forms -name \*CVS\*` ; \
	rm -rf `find images -name \*CVS\*`

rpmdist: dist
	ln -s lib/images/lyx.xpm . ; \
	rpm -ta ${PACKAGE}-${VERSION}.tar.gz ; saved_status=$$?; \
	rm lyx.xpm; exit $$saved_status


bindist:
	mkdir $(bindistdir)
	if test -f README.bin ; then \
	  $(INSTALL) README.bin  $(bindistdir)/README.bin ; \
	fi 
	$(MAKE) $(AM_MAKEFLAGS) install-strip prefix=`pwd`/$(bindistdir)
	(cd $(bindistdir) ; tar cf - .) | gzip >$(bindistfile)
	rm -rf $(bindistdir)
	@if test -f README.bin ; then \
	  echo "*** Did you remember to check the contents of README.bin?" ; \
	else \
	  echo "*** WARNING: You did not provide a README.bin file." ; \
	  echo "*** Please make one now from the example file" ; \
	  echo "***   development/tools/README.bin.example" ; \
	fi  
	@echo "*** It should mention any problem concerning your binary"
	@echo "*** distribution and refer to *you* in case of problem."
	@echo "*** $(bindistfile) has been created."


doxydoc: sourcedoc/Doxyfile
	cd sourcedoc; \
	doxygen ./Doxyfile

sourcedoc/Doxyfile: sourcedoc/Doxyfile.in config.status
	cd $(top_builddir) \
	&& CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status

lgbtags:
	etags --totals=yes --recurse=yes -o TAGS $(top_srcdir)/*

.PHONY: sourcedoc
