include $(top_srcdir)/config/common.am

ACLOCAL_AMFLAGS = -I m4 -I config

DISTCLEANFILES += lyx.1

MAINTAINERCLEANFILES += $(srcdir)/aclocal.m4 $(srcdir)/configure \
	$(srcdir)/config.h.in

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
	README.Win32 README.Cygwin README.localization lyx.man autogen.sh

man_MANS = lyx.1

lyx.1:
	sed -e "s/LYX_DIR_VER/$(LYX_DIR_VER)/g" \
	    -e "s/LYX_USERDIR_VER/$(LYX_USERDIR_VER)/g" $(srcdir)/lyx.man > lyx.1

rpmdist: dist
	$(LN_S) $(srcdir)/lib/images/lyx.png . ; \
	if [ -z "`type -path rpmbuild`" ]; \
	then \
		RPMBUILD=rpm; \
	else \
		RPMBUILD=rpmbuild; \
	fi; $$RPMBUILD -ta $(PACKAGE)-$(VERSION).tar.gz ; saved_status=$$?; \
	rm lyx.png; exit $$saved_status

doxydoc:
	cd sourcedoc; make doxydoc

lgbtags:
	etags --totals=yes --recurse=yes -o TAGS $(top_srcdir)/*

.PHONY: doxydoc

