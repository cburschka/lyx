AUTOMAKE_OPTIONS = foreign 1.4
DISTCLEANFILES= *.orig *.rej *~ *.bak core
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libxforms.la
INCLUDES = -I${top_srcdir}/src/ \
	-I${top_srcdir}/src/frontends/ \
	${SIGC_CFLAGS}
LIBS=
LDFLAGS=
LYXDATADIRS = forms
ETAGS_ARGS = --lang=c++
libxforms_la_SOURCES = \
	Dialogs.C \
	FormCopyright.C \
	FormCopyright.h \
	form_copyright.C \
	form_copyright.h \
	FormPreferences.C \
	FormPreferences.h \
	form_preferences.C \
	form_preferences.h \
	FormPrint.C \
	FormPrint.h \
	form_print.C \
	form_print.h \
	input_validators.h \
	input_validators.c \
	xform_macros.h

# These still have to be added. Sooner or later. ARRae-20000411
#	GUI_defaults.C \
#	GUI_initialize.C \
#	GUI_postlyxrc.C \
#	GUI_runtime.C \
#	GUI_applymenu.C

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
