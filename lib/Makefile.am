AUTOMAKE_OPTIONS = foreign
DISTCLEANFILES = *.orig *.rej *~ *.bak core textclass.lst packages.lst \
		lyxrc.defaults doc/LyXConfig.lyx
pkgdata_SCRIPTS = configure configure.cmd
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in listerrors
SUBDIRS = reLyX
bin_SCRIPTS = listerrors

BIND = bind/*.bind
CLIPART = clipart/*.eps
DOCDIST = doc/LyXConfig.lyx*
DOCINST = doc/*
EXAMPLES = examples/*.lyx
IMAGES = images/*.xpm
KBD = kbd/*.kmap kbd/*.cdef
LAYOUT = layouts/*.layout layouts/*.inc 
LYXSCRIPTS = scripts/*
TEMPL = templates/*.lyx
TEXSUPPORT = tex/*.cls
UI = ui/*.ui

LIBINSTFILES = $(BIND) $(CLIPART) $(DOCINST) $(EXAMPLES) $(IMAGES) $(KBD) \
	$(LAYOUT) $(TEMPL) $(TEXSUPPORT) $(UI) $(LYXSCRIPTS)

LIBDISTFILES = $(BIND) $(CLIPART) $(DOCDIST) $(EXAMPLES) $(IMAGES) $(KBD) \
	$(LAYOUT) $(TEMPL) $(UI) $(TEXSUPPORT) $(LYXSCRIPTS)

pkgdata_DATA = lyxrc.example CREDITS chkconfig.ltx lyxrc.defaults \
               textclass.lst packages.lst external_templates \
               encodings languages

LYXLIBDIRS = bind clipart doc examples images kbd layouts scripts \
             templates tex ui

EXTRA_DIST = CREDITS chkconfig.ltx configure.cmd lyxrc.example \
	external_templates $(LYXLIBDIRS) build-listerrors

libinstalldirs:
	for dir in $(LYXLIBDIRS) ; do \
		$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/$$dir ; \
	done

install-data-local: libinstalldirs
	files=`cd ${srcdir} ; echo $(LIBINSTFILES)` ; \
	for i in $${files} ; do \
	   $(INSTALL_DATA) ${srcdir}/$$i $(DESTDIR)$(pkgdatadir)/$$i ; \
	done

uninstall-local:
	@$(NORMAL_UNINSTALL)
	files=`cd ${srcdir} ; echo $(LIBINSTFILES)` ; \
	for i in $${files} ; do \
	   rm -f $(pkgdatadir)/$$i ; \
	done

dist-hook:
	cd ${distdir} ; rm -rf `find . -name \*CVS\*` ; \
	rm -rf doc/BUGS.lyx ; \
	{ cvs -Q export -r HEAD -d doc lyxdoc || \
	  echo "WARNING: Unable to get LyX Documentation from CVS!" ; true ; }

listerrors: examples/Literate.lyx
	${srcdir}/build-listerrors ${srcdir}
