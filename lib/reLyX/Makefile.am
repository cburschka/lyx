AUTOMAKE_OPTIONS = foreign
DISTCLEANFILES= *.orig *.rej *~ *.bak reLyX reLyX.1
MAINTAINERCLEANFILES = Makefile.in
bin_SCRIPTS = reLyX noweb2lyx
LYXDATADIRS = reLyX reLyX/Text
LYXDISTDIRS = Text
man_MANS = reLyX.1
EXTRA_DIST = BUGS BasicLyX.pm CHANGES CleanTeX.pm LastLyX.pm MANIFEST \
	MakePreamble.pm ReadCommands.pm RelyxTable.pm reLyX.pod \
	reLyXmain.pl syntax.default test.ltx test.lyx reLyX.man \
	RelyxFigure.pm Verbatim.pm \
	$(LYXDISTDIRS)

LIBINSTFILES = *.pm *.pl README BUGS CHANGES reLyX.pod syntax.default Text/*.pm

reLyX.1:
	cp -p $(srcdir)/reLyX.man reLyX.1

libinstalldirs:
	for dir in $(LYXDATADIRS) ; do \
	  $(mkinstalldirs) $(pkgdatadir)/$$dir ; \
	done

install-data-local:  libinstalldirs
	files=`cd ${srcdir} ; echo $(LIBINSTFILES)` ; \
	for i in $${files} ; do \
	   $(INSTALL_DATA) ${srcdir}/$$i $(pkgdatadir)/reLyX/$$i ; \
	done
