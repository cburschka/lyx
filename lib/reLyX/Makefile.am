AUTOMAKE_OPTIONS = foreign
DISTCLEANFILES= *.orig *.rej *~ *.bak reLyX reLyX.1 config.log config.status
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in $(srcdir)/configure

man_MANS = reLyX.1

EXTRA_DIST = BUGS BasicLyX.pm CHANGES CleanTeX.pm LastLyX.pm MANIFEST \
	MakePreamble.pm ReadCommands.pm RelyxTable.pm reLyX.pod \
	reLyXmain.pl syntax.default test.ltx test.lyx reLyX.man \
	RelyxFigure.pm Verbatim.pm Text

bin_SCRIPTS = reLyX noweb2lyx

relyxdatadir = $(pkgdatadir)/reLyX
relyxdata_DATA = \
	BasicLyX.pm \
	CleanTeX.pm \
	LastLyX.pm \
	MakePreamble.pm \
	ReadCommands.pm \
	RelyxFigure.pm \
	RelyxTable.pm \
	Verbatim.pm \
	reLyXmain.pl \
	README \
	BUGS \
	CHANGES \
	reLyX.pod \
	syntax.default

relyxtextdir = $(pkgdatadir)/reLyX/Text
relyxtext_DATA = \
	Text/manpage.3pm \
	Text/TeX.pm

reLyX.1:
	cp -p $(srcdir)/reLyX.man reLyX.1
