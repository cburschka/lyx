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
	UPGRADING lyx.man autogen.sh

# Hack so that the targets that use tar will also work with automake 1.4
AMTAR ?= $(TAR)

man_MANS = lyx.1

## Needed by bindist
bindistdir=$(PWD)/lyxbin
bindistfile=$(PACKAGE)-$(VERSION)-bin.tar.gz

dist_noinst_DATA = \
	images/ChangeLog \
	images/README \
	images/ams_arrows.xbm \
	images/ams_misc.xbm \
	images/ams_nrel.xbm \
	images/ams_ops.xbm \
	images/ams_rel.xbm \
	images/arrows.xbm \
	images/bop.xbm \
	images/brel.xbm \
	images/deco.xbm \
	images/deco.xpm \
	images/delim.xbm \
	images/delim.xpm \
	images/delim0.xpm \
	images/delim1.xpm \
	images/dots.xbm \
	images/equation.xpm \
	images/font.xbm \
	images/frac.xpm \
	images/greek.xbm \
	images/matrix.xpm \
	images/misc.xbm \
	images/smallcaps.xpm \
	images/space.xpm \
	images/sqrt.xpm \
	images/style.xbm \
	images/style.xpm \
	images/sub.xpm \
	images/super.xpm \
	images/varsz.xbm

lyx.1:
	cp -p $(srcdir)/lyx.man lyx.1

rpmdist: dist
	$(LN_S) $(srcdir)/lib/images/lyx.xpm . ; \
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
