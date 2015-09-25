include $(top_srcdir)/config/common.am

.c.o:
	$(CC) -I$(top_builddir) $< -O2 -c -o $@

.rc.o:
	cp $(top_srcdir)/development/Win32/packaging/icons/lyx_*32x32.ico .
	windres -I$(top_builddir) --preprocessor "$(CPP) -xc-header -DRC_INVOKED" $< -o $@

bin_PROGRAMS = lyxeditor lyxwin
bin_SCRIPTS = lyxeditor.sh

lyxdistdir = $(pkgdatadir)
dist_lyxdist_DATA = lyxprofile lyxrc.dist

lyxeditor_SOURCES = lyxeditor.c
lyxwin_SOURCES = lyxwin.c lyxwinres.rc

lyxeditor_LDFLAGS=-mwindows -e _mainCRTStartup
lyxwin_LDFLAGS=-mwindows -e _mainCRTStartup

CLEANFILES = lyxeditor.sh lyx_32x32.ico lyx_doc_32x32.ico

lyxeditor.sh:
	cp $(top_srcdir)/development/tools/lyxeditor $@
