AUTOMAKE_OPTIONS = foreign 1.4
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libqt2.o
BOOST_INCLUDES = -I$(top_srcdir)/boost
SUBDIRS=ui moc
INCLUDES = -I${top_srcdir}/src/ -I${top_srcdir}/src/frontends/ \
	${SIGC_CFLAGS} ${FRONTEND_INCLUDES} $(BOOST_INCLUDES) \
	-I$(top_srcdir)/src/frontends/controllers
AM_CXXFLAGS = -DQT_CLEAN_NAMESPACE -DQT_GENUINE_STR

DISTCLEANFILES = $(BUILTSOURCES) *.orig *.rej *~ *.bak core

include Makefile.dialogs

libqt2_o_LIBADD = \
	ui/libqt2ui.o \
	moc/libqt2moc.o \
	ui/moc/libqt2uimoc.o \
	../xforms/combox.o \
	../xforms/Alert_pimpl.o \
	../xforms/Color.o \
	../xforms/DropDown.o \
	../xforms/FormBase.o \
	../xforms/FormBaseDeprecated.o	\
	../xforms/FormMathsBitmap.o \
	../xforms/FormMathsDeco.o \
	../xforms/FormMathsDelim.o \
	../xforms/FormMathsMatrix.o \
	../xforms/FormMathsPanel.o \
	../xforms/FormMathsStyle.o \
	../xforms/FormMathsSpace.o \
	../xforms/FormPreferences.o \
	../xforms/FormTabular.o \
	../xforms/FormInset.o \
	../xforms/MathsSymbols.o \
	../xforms/Menubar_pimpl.o \
	../xforms/RadioButtonGroup.o \
	../xforms/Toolbar_pimpl.o \
	../xforms/bmtable.o \
	../xforms/form_preferences.o \
	../xforms/form_tabular.o \
	../xforms/form_maths_deco.o \
	../xforms/form_maths_delim.o \
	../xforms/form_maths_matrix.o \
	../xforms/form_maths_panel.o \
	../xforms/form_maths_space.o \
	../xforms/form_maths_style.o \
	../xforms/input_validators.o \
	../xforms/xformsBC.o \
	../xforms/xforms_helpers.o \
	@LYX_LIBS@ @FRONTEND_LDFLAGS@ @FRONTEND_LIBS@
 
LIBS=
#LDFLAGS= $(libqt2_la_OBJADD)
ETAGS_ARGS = --lang=c++

libqt2_o_SOURCES = \
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
