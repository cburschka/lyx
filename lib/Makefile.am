AUTOMAKE_OPTIONS = foreign
DISTCLEANFILES = *.orig *.rej *~ *.bak textclass.lst packages.lst \
		lyxrc.defaults doc/LaTeXConfig.lyx
pkgdata_SCRIPTS = configure
MAINTAINERCLEANFILES = Makefile.in
SUBDIRS    = reLyX

LYXDATADIRS = bind clipart doc examples images kbd layouts templates tex
EXTRA_DIST = CREDITS chkconfig.ltx configure.cmd lyxrc.example \
	$(LYXDATADIRS)

CLIPART = clipart/*.eps
LAYOUT = layouts/*.layout layouts/*.inc 
DOC = doc/*.lyx doc/LaTeXConfig.lyx.in
KBD = kbd/*.kmap kbd/*.cdef
TEMPL = templates/*.lyx
BIND = bind/*.bind
EXAMPLES = examples/*.lyx
TEXSUPPORT = tex/*.cls
IMAGES = images/*.xpm images/*.xbm

pkgdata_DATA = lyxrc.example CREDITS chkconfig.ltx lyxrc.defaults \
               textclass.lst packages.lst

LIBINSTFILES = $(LAYOUT) $(CLIPART) $(DOC) $(KBD) $(DOC) $(TEMPL) $(BIND) \
               $(EXAMPLES) $(TEXSUPPORT) $(IMAGES)

libinstalldirs:
	for dir in $(LYXDATADIRS) ; do \
	  $(mkinstalldirs) $(pkgdatadir)/$$dir ; \
	done

install-data-local:  libinstalldirs
	files=`cd ${srcdir} ; echo $(LIBINSTFILES)` ; \
	for i in $${files} ; do \
	   $(INSTALL_DATA) ${srcdir}/$$i $(pkgdatadir)/$$i ; \
	done 
