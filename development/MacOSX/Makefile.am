include $(top_srcdir)/config/common.am

LINGUAS = $(srcdir)/../../po/LINGUAS

SUBDIRS = spotlight

bundledir = ${prefix}/Contents
dist_bundle_DATA = PkgInfo
nodist_bundle_DATA = Info.plist

dist_bin_SCRIPTS = lyxeditor

dist_pkgdata_DATA = COPYING LyXapp.icns LyX.icns
nodist_pkgdata_DATA = lyxrc.dist


install-data-hook:
	LINGUAS_=`sed -e "/^#/d" -e "s/#.*//" $(LINGUAS)`; \
	for f in en $$LINGUAS_ ; do \
	  mkdir -p  $(pkgdatadir)/$$f.lproj ; \
	done
