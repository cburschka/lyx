AUTOMAKE_OPTIONS = foreign 1.4
DISTCLEANFILES= *.orig *.rej *~ *.bak core
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libcontrollers.la
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${top_srcdir}/src/ \
	-I${top_srcdir}/src/frontends/ \
	${SIGC_CFLAGS} $(BOOST_INCLUDES)
LIBS =
SUBDIRS = 
ETAGS_ARGS = --lang=c++
libcontrollers_la_SOURCES=\
	ButtonController.C \
	ButtonController.h \
	ButtonPolicies.C \
	ButtonPolicies.h \
	ControlBase.C \
	ControlBase.h \
	ControlBibitem.C \
	ControlBibitem.h \
	ControlBibtex.C \
	ControlBibtex.h \
	ControlCitation.C \
	ControlCitation.h \
	ControlCommand.C \
	ControlCommand.h \
	ControlConnections.C \
	ControlConnections.h \
	ControlLog.C \
	ControlLog.h \
	ControlVCLog.C \
	ControlVCLog.h \
	ViewBase.h

# just copied from old lyx repository
dist-hook:
	for subdir in $(LYXDATADIRS) ; do \
	test -d $(distdir)/$$subdir \
	|| mkdir $(distdir)/$$subdir \
	|| exit 1; \
	chmod 777 $(distdir)/$$subdir; \
	list=`(cd $(srcdir)/$$subdir && ls -1 | grep -v CVS)`; \
	echo $$list ; \
	for fil in $$list ; do \
	cp -p $(srcdir)/$$subdir/$$fil $(distdir)/$$subdir ; \
	done ; \
	done
