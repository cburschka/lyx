include $(top_srcdir)/config/common.am

LINGUAS = $(srcdir)/../../po/LINGUAS

SUBDIRS = spotlight

bundledir = ${prefix}/Contents
dist_bundle_DATA = Info.plist PkgInfo

dist_bin_SCRIPTS = lyxeditor

dist_pkgdata_DATA = COPYING LyXapp.icns LyX.icns lyxrc.dist


install-data-hook:
	LINGUAS_=`sed -e "/^#/d" -e "s/#.*//" $(LINGUAS)`; \
	for f in $$LINGUAS_ ; do \
	  $(MKDIR_P) $(pkgdatadir)/$$f.lproj ; \
	done
