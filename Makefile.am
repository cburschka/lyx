include $(top_srcdir)/config/common.am

DISTCLEANFILES += lyx.1 config.status config.cache config.log

MAINTAINERCLEANFILES += $(srcdir)/aclocal.m4 \
	$(srcdir)/configure \
	$(srcdir)/acinclude.m4

DIST_SUBDIRS = m4 config development intl po boost src sourcedoc lib

if USE_INCLUDED_BOOST
SUBDIRS = m4 config development intl po boost src sourcedoc lib
else
SUBDIRS = m4 config development intl po src sourcedoc lib
endif

EXTRA_DIST = ANNOUNCE INSTALL.OS2 INSTALL.autoconf README.OS2 \
	UPGRADING lyx.man autogen.sh images

# Hack so that the targets that use tar will also work with automake 1.4
AMTAR ?= $(TAR)

man_MANS = lyx.1

## Needed by bindist
bindistdir=$(PWD)/lyxbin
bindistfile=$(PACKAGE)-$(VERSION)-bin.tar.gz

lyx.1:
	cp -p $(srcdir)/lyx.man lyx.1

dist-hook:
	cd $(distdir) ; \
	rm -rf `find images -name \*CVS\*`

rpmdist: dist
	ln -s $(srcdir)/lib/images/lyx.xpm . ; \
	if [ -z "`type -path rpmbuild`" ]; \
	then \
		RPMBUILD=rpm; \
	else \
		RPMBUILD=rpmbuild; \
	fi; $$RPMBUILD -ta $(PACKAGE)-$(VERSION).tar.gz ; saved_status=$$?; \
	rm lyx.xpm; exit $$saved_status

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

ACLOCAL_AMFLAGS = -I m4
