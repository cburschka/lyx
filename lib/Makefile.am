AUTOMAKE_OPTIONS = foreign
DISTCLEANFILES = *.orig *.rej *~ *.bak textclass.lst packages.lst \
		lyxrc.defaults doc/LaTeXConfig.lyx
pkgdata_SCRIPTS = configure configure.cmd
MAINTAINERCLEANFILES = Makefile.in
SUBDIRS    = reLyX

BIND = bind/*.bind
CLIPART = clipart/*.eps
DOC = doc/LaTeXConfig.lyx.in 
EXAMPLES = examples/*.lyx
IMAGES = images/*.xpm images/*.xbm
KBD = kbd/*.kmap kbd/*.cdef
LAYOUT = layouts/*.layout layouts/*.inc 
TEMPL = templates/*.lyx
TEXSUPPORT = tex/*.cls

pkgdata_DATA = lyxrc.example CREDITS chkconfig.ltx lyxrc.defaults \
               textclass.lst packages.lst

LIBINSTFILES = $(BIND) $(CLIPART) $(EXAMPLES) $(IMAGES) $(KBD) \
	$(LAYOUT) $(TEMPL) $(TEXSUPPORT)

EXTRA_DIST = CREDITS chkconfig.ltx configure.cmd lyxrc.example \
	$(LIBINSTFILES) $(DOC)

install-data-local:
	files=`cd ${srcdir} ; echo $(LIBINSTFILES)` ; \
	for i in $${files} ; do \
	   $(INSTALL_DATA) ${srcdir}/$$i $(pkgdatadir)/$$i ; \
	done \
	for i in doc/* ; do
		$(INSTALL_DATA) ${srcdir}/$$i $(pkgdatadir)/$$i ; \
	done

dist-hook:
	cd ${distdir} ; cvs -q export -r HEAD -d doc lyxdoc
