AUTOMAKE_OPTIONS = foreign
DISTCLEANFILES = *.orig *.rej *~ *.bak textclass.lst packages.lst \
		lyxrc.defaults doc/LaTeXConfig.lyx
pkgdata_SCRIPTS = configure configure.cmd
MAINTAINERCLEANFILES = Makefile.in
SUBDIRS    = reLyX

BIND = bind/*.bind
CLIPART = clipart/*.eps
DOCDIST = doc/LaTeXConfig.lyx*
DOCINST = doc/*
EXAMPLES = examples/*.lyx
IMAGES = images/*.xpm images/*.xbm
KBD = kbd/*.kmap kbd/*.cdef
LAYOUT = layouts/*.layout layouts/*.inc 
TEMPL = templates/*.lyx
TEXSUPPORT = tex/*.cls

LIBINSTFILES = $(BIND) $(CLIPART) $(DOCINST) $(EXAMPLES) $(IMAGES) $(KBD) \
	$(LAYOUT) $(TEMPL) $(TEXSUPPORT)

LIBDISTFILES = $(BIND) $(CLIPART) $(DOCDIST) $(EXAMPLES) $(IMAGES) $(KBD) \
	$(LAYOUT) $(TEMPL) $(TEXSUPPORT)

pkgdata_DATA = lyxrc.example CREDITS chkconfig.ltx lyxrc.defaults \
               textclass.lst packages.lst

LYXLIBDIRS = bind clipart doc examples images kbd layouts templates tex

EXTRA_DIST = CREDITS chkconfig.ltx configure.cmd lyxrc.example \
	$(LYXLIBDIRS)

libinstalldirs:
	for dir in $(LYXLIBDIRS) ; do \
		$(mkinstalldirs) $(pkgdatadir)/$$dir ; \
	done

install-data-local: libinstalldirs
	files=`cd ${srcdir} ; echo $(LIBINSTFILES)` ; \
	for i in $${files} ; do \
	   $(INSTALL_DATA) ${srcdir}/$$i $(pkgdatadir)/$$i ; \
	done

dist-hook:
	cd ${distdir} ; rm -rf `find . -name \*CVS\*` ; \
	rm -rf doc/BUGS.lyx ; \
	cvs -Q export -r HEAD -d doc lyxdoc
