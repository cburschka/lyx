AUTOMAKE_OPTIONS = foreign 1.4
DISTCLEANFILES= *.orig *.rej *~ *.bak core
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libqt2.la
BOOST_INCLUDES = -I$(top_srcdir)/boost
SUBDIRS=ui moc
INCLUDES = -I${top_srcdir}/src/ -I${top_srcdir}/src/frontends/ \
	${SIGC_CFLAGS} ${FRONTEND_INCLUDES} $(BOOST_INCLUDES) \
	-I$(top_srcdir)/src/frontends/controllers
AM_CXXFLAGS = -DQT_CLEAN_NAMESPACE -DQT_GENUINE_STR

DISTCLEANFILES = $(BUILTSOURCES) *.orig *.rej *~ *.bak core

include Makefile.dialogs

libqt2_la_DEPENDENCIES = ui/libqt2ui.la moc/libqt2moc.la ui/moc/libqt2uimoc.la
 
libqt2_la_OBJADD = \
	ui/libqt2ui.la \
	moc/libqt2moc.la \
	ui/moc/libqt2uimoc.la \
	../controllers/ButtonControllerBase.lo \
	../xforms/combox.lo \
	../xforms/Color.lo \
	../xforms/FormBaseDeprecated.lo	\
	../xforms/FormBrowser.lo \
	../xforms/FormDocument.lo \
	../xforms/FormIndex.lo \
	../xforms/FormInset.lo \
	../xforms/FormPreferences.lo \
	../xforms/FormRef.lo \
	../xforms/FormTabular.lo \
	../xforms/FormToc.lo \
	../xforms/FormUrl.lo \
	../xforms/MathsSymbols.lo \
	../xforms/Menubar_pimpl.lo \
	../xforms/RadioButtonGroup.lo \
	../xforms/Toolbar_pimpl.lo \
	../xforms/bmtable.lo \
	../xforms/form_bibitem.lo \
	../xforms/form_bibtex.lo \
	../xforms/form_browser.lo \
	../xforms/form_document.lo \
	../xforms/form_error.lo \
	../xforms/form_external.lo \
	../xforms/form_index.lo \
	../xforms/form_graphics.lo \
	../xforms/form_include.lo \
	../xforms/form_index.lo \
	../xforms/form_minipage.lo \
	../xforms/form_preamble.lo \
	../xforms/form_preferences.lo \
	../xforms/form_ref.lo \
	../xforms/form_tabular.lo \
	../xforms/form_toc.lo \
	../xforms/form_url.lo \
	../xforms/input_validators.lo \
	../xforms/xformsBC.lo \
	../xforms/xforms_helpers.lo

LIBS=
LDFLAGS= $(libqt2_la_OBJADD)
ETAGS_ARGS = --lang=c++

libqt2_la_SOURCES = \
	$(DIALOGSOURCES) \
	Dialogs.C \
	GUIRunTime.C \
	FileDialog.C \
	FileDialog_private.C \
	Timeout_pimpl.h \
	Timeout_pimpl.C \
	Qt2Base.C	\
	Qt2Base.h	\
	Qt2BC.C	\
	Qt2BC.h	\
	emptytable.C \
	emptytable.h \
	lengthcombo.C \
	lengthcombo.h
