AUTOMAKE_OPTIONS = foreign 1.4
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libqt2moc.la
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${top_srcdir}/src/ \
	-I${top_srcdir}/src/frontends/ \
	-I${top_srcdir}/src/frontends/controllers \
	${SIGC_CFLAGS} \
	${FRONTEND_INCLUDES} \
	$(BOOST_INCLUDES)

include ../Makefile.dialogs
 
libqt2moc_la_SOURCES := FileDialog_private_moc.C \
	emptytable_moc.C \
	Qt2Base_moc.C \
	$(MOCDIALOGS)
	#$(patsubst, %, %Dialog_moc.C, $(DIALOGS))
 
DISTCLEANFILES = $(libqt2moc_la_SOURCES) *.orig *.rej *~ *.bak core

ETAGS_ARGS = --lang=c++
 
%_moc.C: ../%.h
	$(MOC) -o $@ $<
 
