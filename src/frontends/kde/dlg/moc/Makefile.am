AUTOMAKE_OPTIONS = foreign 1.4
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libkdedlgmoc.la
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${top_srcdir}/src/ -I${top_srcdir}/src/frontends/ \
	${FRONTEND_INCLUDES} ${BOOST_INCLUDES}

libkdedlgmoc_la_SOURCES = citationdlgdata_moc.C \
		copyrightdlgdata_moc.C \
		docdlgdata_moc.C \
		docsettingsdlgdata_moc.C \
		docgeometrydlgdata_moc.C \
		doclanguagedlgdata_moc.C \
		docextradlgdata_moc.C \
		docbulletsdlgdata_moc.C \
		indexdlgdata_moc.C \
		logdlgdata_moc.C \
		paradlgdata_moc.C \
		parageneraldlgdata_moc.C \
		paraabovedlgdata_moc.C \
		parabelowdlgdata_moc.C \
		paraextradlgdata_moc.C \
		printdlgdata_moc.C \
		tabcreatedlgdata_moc.C \
		emptytable_moc.C \
		tabstack_moc.C \
		lengthentry_moc.C

DISTCLEANFILES = $(libkdedlgmoc_la_SOURCES) *.orig *.rej *~ *.bak core

LIBS=
ETAGS_ARGS = --lang=c++
	$(BUILTSOURCES)

citationdlgdata_moc.C: ../citationdlgdata.h
	$(MOC) $< -o $@ 
copyrightdlgdata_moc.C: ../copyrightdlgdata.h
	$(MOC) $< -o $@
docdlgdata_moc.C: ../docdlgdata.h
	$(MOC) $< -o $@
docsettingsdlgdata_moc.C: ../docsettingsdlgdata.h
	$(MOC) $< -o $@
docgeometrydlgdata_moc.C: ../docgeometrydlgdata.h
	$(MOC) $< -o $@
doclanguagedlgdata_moc.C: ../doclanguagedlgdata.h
	$(MOC) $< -o $@
docextradlgdata_moc.C: ../docextradlgdata.h
	$(MOC) $< -o $@
docbulletsdlgdata_moc.C: ../docbulletsdlgdata.h
	$(MOC) $< -o $@
emptytable_moc.C: ../emptytable.h
	$(MOC) $< -o $@
indexdlgdata_moc.C: ../indexdlgdata.h
	$(MOC) $< -o $@
logdlgdata_moc.C: ../logdlgdata.h
	$(MOC) $< -o $@
paradlgdata_moc.C: ../paradlgdata.h
	$(MOC) $< -o $@
parageneraldlgdata_moc.C: ../parageneraldlgdata.h
	$(MOC) $< -o $@
paraabovedlgdata_moc.C: ../paraabovedlgdata.h
	$(MOC) $< -o $@
parabelowdlgdata_moc.C: ../parabelowdlgdata.h
	$(MOC) $< -o $@
paraextradlgdata_moc.C: ../paraextradlgdata.h
	$(MOC) $< -o $@
printdlgdata_moc.C: ../printdlgdata.h
	$(MOC) $< -o $@
tabcreatedlgdata_moc.C: ../tabcreatedlgdata.h
	$(MOC) $< -o $@
tabstack_moc.C: ../tabstack.h
	$(MOC) $< -o $@
lengthentry_moc.C: ../lengthentry.h
	$(MOC) $< -o $@
