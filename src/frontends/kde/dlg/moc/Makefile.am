AUTOMAKE_OPTIONS = foreign 1.4
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libkdedlgmoc.la
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${top_srcdir}/src/ -I${top_srcdir}/src/frontends/ \
	${FRONTEND_INCLUDES} ${BOOST_INCLUDES}

libkdedlgmoc_la_SOURCES = copyrightdlgdata_moc.C \
		indexdlgdata_moc.C \
		paradlgdata_moc.C \
		parageneraldlgdata_moc.C \
		paraabovedlgdata_moc.C \
		parabelowdlgdata_moc.C \
		paraextradlgdata_moc.C \
		printdlgdata_moc.C \
		tabcreatedlgdata_moc.C \
		emptytable_moc.C \
		tabstack_moc.C

DISTCLEANFILES = $(libkdedlgmoc_la_SOURCES) *.orig *.rej *~ *.bak core

LIBS=
ETAGS_ARGS = --lang=c++
	$(BUILTSOURCES)

copyrightdlgdata_moc.C: ../copyrightdlgdata.h
	$(MOC) $< -o $@
emptytable_moc.C: ../emptytable.h
	$(MOC) $< -o $@
indexdlgdata_moc.C: ../indexdlgdata.h
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
