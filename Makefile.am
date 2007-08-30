include $(top_srcdir)/config/common.am

ACLOCAL_AMFLAGS = -I m4 -I config

DISTCLEANFILES += lyx.1 config.status config.cache config.log

MAINTAINERCLEANFILES += $(srcdir)/aclocal.m4 \
	$(srcdir)/configure

DIST_SUBDIRS = m4 config development intl po boost src sourcedoc lib

if BUILD_CLIENT_SUBDIR
CLIENT = src/client
endif

if USE_INCLUDED_BOOST
BOOST = boost
endif

SUBDIRS = config development intl po $(BOOST) src sourcedoc lib \
	$(CLIENT) src/tex2lyx


EXTRA_DIST = ANNOUNCE INSTALL.autoconf RELEASE-NOTES UPGRADING \
	INSTALL.Win32 INSTALL.MacOSX INSTALL.scons INSTALL.cmake \
	README.Win32 README.Cygwin lyx.man autogen.sh

man_MANS = lyx.1

## Needed by bindist
bindistdir=$(PWD)/lyxbin
bindistfile=$(PACKAGE)-$(VERSION)-bin.tar.gz

lyx.1:
	cp -p $(srcdir)/lyx.man lyx.1

rpmdist: dist
	$(LN_S) $(srcdir)/lib/images/lyx.png . ; \
	if [ -z "`type -path rpmbuild`" ]; \
	then \
		RPMBUILD=rpm; \
	else \
		RPMBUILD=rpmbuild; \
	fi; $$RPMBUILD -ta $(PACKAGE)-$(VERSION).tar.gz ; saved_status=$$?; \
	rm lyx.png; exit $$saved_status

bindist:
	rm -f $(bindistfile)
	$(MAKE) $(AM_MAKEFLAGS) DESTDIR=$(bindistdir) install-strip
	if test -f $(top_srcdir)/README.bin ; then \
	  $(INSTALL) $(top_srcdir)/README.bin \
		$(bindistdir)$(prefix)/README.bin ; \
	fi
	(cd $(bindistdir)$(prefix) ; $(AMTAR) cf - . ) | \
	GZIP=$(GZIP_ENV) gzip > $(bindistfile)
	-chmod -R a+w $(bindistdir) > /dev/null 2>&1 ; rm -rf $(bindistdir)
	@if test -f $(top_srcdir)/README.bin ; then \
	  echo "*** Did you remember to check the contents of README.bin?" ; \
	else \
	  echo "*** WARNING: You did not provide a README.bin file." ; \
	  echo "*** Please make one now from the example file" ; \
	  echo "***   development/tools/README.bin.example" ; \
	fi
	@echo "*** It should mention any problem concerning your binary"
	@echo "*** distribution and refer to *you* in case of problem."
	@echo "*** $(bindistfile) has been created."

doxydoc:
	cd sourcedoc; make doxydoc

lgbtags:
	etags --totals=yes --recurse=yes -o TAGS $(top_srcdir)/*

.PHONY: doxydoc

