include $(top_srcdir)/config/common.am

LINGUAS = $(srcdir)/../../po/LINGUAS

SUBDIRS = spotlight

bundledir = ${prefix}/Contents
dist_bundle_DATA = PkgInfo
nodist_bundle_DATA = Info.plist

dist_bin_SCRIPTS = lyxeditor maxima inkscape lilypond lilypond-book

dist_pkgdata_DATA = COPYING LyXapp.icns LyX.icns LyX.sdef dmg-background.png
nodist_pkgdata_DATA = lyxrc.dist


install-data-hook:
	LINGUAS_=`sed -e "/^#/d" -e "s/#.*//" $(LINGUAS)`; \
	for f in en $$LINGUAS_ ; do \
	  $(MKDIR_P) $(pkgdatadir)/$$f.lproj ; \
	done
