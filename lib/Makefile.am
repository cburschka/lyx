AUTOMAKE_OPTIONS = foreign
DISTCLEANFILES = *.orig *.rej *~ *.bak core textclass.lst packages.lst \
		lyxrc.defaults doc/LyXConfig.lyx
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
SUBDIRS = reLyX

M4=m4
CHMOD=chmod

BIND = bind/*.bind
CLIPART = clipart/*.eps
DOCDIST = doc/LyXConfig.lyx*
DOCINST = doc/*.lyx doc/*.eps doc/*.lyx.in
EXAMPLES = examples/*.lyx examples/*.fen
HELP = help/*.hlp
IMAGES = images/*.xpm
KBD = kbd/*.kmap kbd/*.cdef
LAYOUT = layouts/*.layout layouts/*.inc
LYXSCRIPTS = configure configure.cmd scripts/*
TEMPL = templates/*.lyx
TEXSUPPORT = tex/*.cls tex/*.sty
UI = ui/*.ui

LIBINSTFILES = $(BIND) $(CLIPART) $(DOCINST) $(EXAMPLES) $(HELP) $(IMAGES) \
	$(KBD) $(LAYOUT) $(TEMPL) $(TEXSUPPORT) $(UI)

LIBDISTFILES = $(BIND) $(CLIPART) $(DOCDIST) $(EXAMPLES) $(HELP) $(IMAGES) \
	$(KBD) $(LAYOUT) $(TEMPL) $(UI) $(TEXSUPPORT) $(LYXSCRIPTS)

pkgdata_DATA = lyxrc.example CREDITS chkconfig.ltx lyxrc.defaults \
	       textclass.lst packages.lst external_templates \
	       encodings languages symbols

LYXLIBDIRS = bind clipart doc examples help images kbd layouts scripts \
	     templates tex ui xfonts

EXTRA_DIST = CREDITS chkconfig.ltx configure.cmd lyxrc.example \
	external_templates $(LYXLIBDIRS) \
	encodings languages symbols configure configure.m4

$(srcdir)/configure: $(srcdir)/configure.m4
	$(M4) $< > $@
	$(CHMOD) 755 $@

lyxrc.defaults: $(srcdir)/configure
	$(SHELL) $<

libinstalldirs:
	for dir in $(LYXLIBDIRS) ; do \
		$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/$$dir ; \
	done

install-xfonts:
	test -f xfonts/fonts.dir || exit 0 ; \
	files=`sed -e 's/^[0-9]*$$//' -e 's/^\([^ ]*\) .*$$/\1/' xfonts/fonts.dir` ; echo [$$files] ;\
	for i in $${files} ; do \
	  ln -sf `kpsewhich $$i` $(DESTDIR)$(pkgdatadir)/xfonts/$$i ; \
	done ; \
	$(INSTALL_DATA) xfonts/fonts.dir $(DESTDIR)$(pkgdatadir)/xfonts/fonts.dir ; \
	$(INSTALL_DATA) xfonts/fonts.scale $(DESTDIR)$(pkgdatadir)/xfonts/fonts.scale

install-data-local: libinstalldirs install-xfonts
	files=`cd $(srcdir) ; echo $(LIBINSTFILES)` ; \
	for i in $${files} ; do \
	   $(INSTALL_DATA) $(srcdir)/$$i $(DESTDIR)$(pkgdatadir)/$$i ; \
	done

install-exec-local: libinstalldirs
	files=`cd $(srcdir) ; echo $(LYXSCRIPTS)` ; \
	for i in $${files} ; do \
	   $(INSTALL_SCRIPT) $(srcdir)/$$i $(DESTDIR)$(pkgdatadir)/$$i ; \
	done

uninstall-local:
	@$(NORMAL_UNINSTALL)
	files=`cd $(srcdir) ; echo $(LIBINSTFILES) $(LYXSCRIPTS)` ; \
	for i in $${files} ; do \
	   rm -f $(pkgdatadir)/$$i ; \
	done ; \
	dirs=`cd $(srcdir) ; echo $(LYXLIBDIRS)` ; \
	for i in $${dirs} ; do \
	   rm -rf $(pkgdatadir)/$$i ; \
	done

dist-hook:
	cd $(distdir) ; rm -rf `find . -name \*CVS\*` ; \
	rm -rf doc/.cvsignore; \
	[ -f doc/UserGuide.lyx ] || { cvs -Q export -r HEAD -d doc lyxdoc || \
	  echo "WARNING: Unable to get LyX Documentation from CVS!" ; true ; }
