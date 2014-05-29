include $(top_srcdir)/config/common.am

ACLOCAL_AMFLAGS = -I m4 -I config

MAINTAINERCLEANFILES += $(srcdir)/aclocal.m4 $(srcdir)/configure \
	$(srcdir)/config.h.in

DIST_SUBDIRS = m4 config development po boost src sourcedoc lib

if BUILD_CLIENT_SUBDIR
CLIENT = src/client
endif

if USE_INCLUDED_BOOST
BOOST = boost
endif

SUBDIRS = config development po $(BOOST) src sourcedoc lib \
	$(CLIENT) src/tex2lyx


EXTRA_DIST = ANNOUNCE INSTALL.autoconf RELEASE-NOTES UPGRADING \
	INSTALL.Win32 INSTALL.MacOSX INSTALL.cmake \
	README.Win32 README.Cygwin README.localization lyx.1in \
	autogen.sh

man_MANS = lyx.1

#Wait some time for bumping automake 1.11, which can use dist-xz
#directly without this code, which is to be removed.
#xz has low compression by default, but can be affected via
#export XZ_OPT=-9ev put somewhere in the makefile.
lyxdist: dist
	bunzip2 $(PACKAGE)-$(VERSION).tar.bz2
	xz -9 $(PACKAGE)-$(VERSION).tar
	ls -hl $(PACKAGE)-$(VERSION).tar.*

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
	cd sourcedoc; $(MAKE) doxydoc

lfundoc: $(srcdir)/lib/doc/LFUNs.lyx

$(srcdir)/lib/doc/LFUNs.lyx: $(srcdir)/development/tools/gen_lfuns.py $(srcdir)/src/LyXAction.cpp
	$(AM_V_GEN)$(PYTHON) $^ >$@ 2>/dev/null

keystest:
	development/keystest/lyx_make.sh

alltests: check alltests-recursive

alltests-recursive:
	cd lib; $(MAKE) alltests-recursive
	cd src; $(MAKE) alltests-recursive

updatetests:
	cd src; $(MAKE) updatetests

lgbtags:
	etags --totals=yes --recurse=yes -o TAGS $(top_srcdir)/*

.PHONY: doxydoc alltests alltests-recursive updatetests

