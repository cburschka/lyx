include $(top_srcdir)/config/common.am

.c.o:
	$(CC) -I$(top_builddir) $< -O2 -c -o $@ -mwindows -e _mainCRTStartup

.rc.o:
	cp $(top_srcdir)/development/Win32/packaging/icons/lyx_*32x32.ico .
	windres -I$(top_builddir) --preprocessor "$(CPP) -xc-header -DRC_INVOKED" $< -o $@

bin_PROGRAMS = lyxeditor lyxwin
bin_SCRIPTS = lyxeditor.sh
dist_noinst_DATA = lyxprofile

lyxeditor_SOURCES = lyxeditor.c
lyxwin_SOURCES = lyxwin.c lyxwinres.rc

CLEANFILES += lyxeditor.sh lyx_32x32.ico lyx_doc_32x32.ico

lyxeditor.sh:
	cp $(top_srcdir)/development/tools/lyxeditor $@
