AUTOMAKE_OPTIONS = foreign 1.4
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

libqt2_la_LIBADD = \
	ui/libqt2ui.la \
	moc/libqt2moc.la \
	ui/moc/libqt2uimoc.la \
	../xforms/combox.lo \
	../xforms/Alert_pimpl.lo \
	../xforms/Color.lo \
	../xforms/DropDown.lo \
	../xforms/FormBase.lo \
	../xforms/FormBaseDeprecated.lo	\
	../xforms/FormMathsBitmap.lo \
	../xforms/FormMathsDeco.lo \
	../xforms/FormMathsDelim.lo \
	../xforms/FormMathsMatrix.lo \
	../xforms/FormMathsPanel.lo \
	../xforms/FormMathsStyle.lo \
	../xforms/FormMathsSpace.lo \
	../xforms/FormPreferences.lo \
	../xforms/FormTabular.lo \
	../xforms/FormInset.lo \
	../xforms/MathsSymbols.lo \
	../xforms/Menubar_pimpl.lo \
	../xforms/RadioButtonGroup.lo \
	../xforms/Toolbar_pimpl.lo \
	../xforms/bmtable.lo \
	../xforms/form_preferences.lo \
	../xforms/form_tabular.lo \
	../xforms/form_maths_deco.lo \
	../xforms/form_maths_delim.lo \
	../xforms/form_maths_matrix.lo \
	../xforms/form_maths_panel.lo \
	../xforms/form_maths_space.lo \
	../xforms/form_maths_style.lo \
	../xforms/input_validators.lo \
	../xforms/xformsBC.lo \
	../xforms/xforms_helpers.lo \
	@LYX_LIBS@ @FRONTEND_LDFLAGS@ @FRONTEND_LIBS@
 
LIBS=
#LDFLAGS= $(libqt2_la_OBJADD)
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
	lengthcombo.h \
	qttableview.h \
	qttableview.C
