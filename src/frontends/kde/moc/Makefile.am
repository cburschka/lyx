AUTOMAKE_OPTIONS = foreign 1.4
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libkdemoc.la
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${top_srcdir}/src/ \
	-I${top_srcdir}/src/frontends/ \
	-I${top_srcdir}/src/frontends/controllers \
	${SIGC_CFLAGS} \
	${FRONTEND_INCLUDES} \
	$(BOOST_INCLUDES)

libkdemoc_la_SOURCES = FileDialog_private_moc.C \
		citationdlg_moc.C \
		copyrightdlg_moc.C \
		docdlg_moc.C \
		indexdlg_moc.C \
		logdlg_moc.C \
		paradlg_moc.C \
		parageneraldlg_moc.C \
		paraextradlg_moc.C \
		printdlg_moc.C \
		refdlg_moc.C \
		tabcreatedlg_moc.C \
		urldlg_moc.C \
		vclogdlg_moc.C

# FIXME
 
#tocdlg_moc.C \
 
DISTCLEANFILES = $(libkdemoc_la_SOURCES) *.orig *.rej *~ *.bak core

LIBS=
ETAGS_ARGS = --lang=c++

FileDialog_private_moc.C: ../FileDialog_private.h
	$(MOC) $< -o $@
citationdlg_moc.C: ../citationdlg.h
	$(MOC) $< -o $@
copyrightdlg_moc.C: ../copyrightdlg.h
	$(MOC) $< -o $@
docdlg_moc.C: ../docdlg.h
	$(MOC) $< -o $@
indexdlg_moc.C: ../indexdlg.h
	$(MOC) $< -o $@
logdlg_moc.C: ../logdlg.h
	$(MOC) $< -o $@
paradlg_moc.C: ../paradlg.h
	$(MOC) $< -o $@
parageneraldlg_moc.C: ../parageneraldlg.h
	$(MOC) $< -o $@
paraextradlg_moc.C: ../paraextradlg.h
	$(MOC) $< -o $@
printdlg_moc.C: ../printdlg.h
	$(MOC) $< -o $@
refdlg_moc.C: ../refdlg.h
	$(MOC) $< -o $@
tabcreatedlg_moc.C: ../tabcreatedlg.h
	$(MOC) $< -o $@
tocdlg_moc.C: ../tocdlg.h
	$(MOC) $< -o $@
urldlg_moc.C: ../urldlg.h
	$(MOC) $< -o $@
vclogdlg_moc.C: ../vclogdlg.h
	$(MOC) $< -o $@
