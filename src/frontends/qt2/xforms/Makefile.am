AUTOMAKE_OPTIONS = foreign 1.4
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libqt2xforms.la
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${top_srcdir}/src/ -I${top_srcdir}/src/frontends/ \
	${SIGC_CFLAGS} -I$(top_srcdir)/images ${FRONTEND_INCLUDES} $(BOOST_INCLUDES) \
	-I$(top_srcdir)/src/frontends/controllers

DISTCLEANFILES = $(XFORMS_SOURCES) $(BUILTSOURCES) *.orig *.rej *~ *.bak core

LIBS=
ETAGS_ARGS = --lang=c++

XFORMS_SOURCES = \
	bmtable.h \
	bmtable.c \
	combox.h \
	combox.C \
	Color.h \
	Color.C \
	DropDown.h \
	DropDown.C \
	FormBase.h \
	FormBase.C \
	FormBaseDeprecated.h \
	FormBaseDeprecated.C \
	FormDocument.h \
	FormDocument.C \
	FormParagraph.h \
	FormParagraph.C \
	FormMathsBitmap.h \
	FormMathsBitmap.C \
	FormMathsDeco.h \
	FormMathsDeco.C \
	FormMathsDelim.h \
	FormMathsDelim.C \
	FormMathsMatrix.h \
	FormMathsMatrix.C \
	FormMathsPanel.h \
	FormMathsPanel.C \
	FormMathsStyle.h \
	FormMathsStyle.C \
	FormMathsSpace.h \
	FormMathsSpace.C \
	FormPreferences.h \
	FormPreferences.C \
	FormTabular.h \
	FormTabular.C \
	FormInset.h \
	FormInset.C \
	MathsSymbols.h \
	MathsSymbols.C \
	Menubar_pimpl.h \
	Menubar_pimpl.C \
	RadioButtonGroup.h \
	RadioButtonGroup.C \
	Toolbar_pimpl.h \
	Toolbar_pimpl.C \
	Tooltips.h \
	Tooltips.C \
	form_document.h \
	form_document.C \
	form_paragraph.h \
	form_paragraph.C \
	form_preferences.h \
	form_preferences.C \
	form_tabular.h \
	form_tabular.C \
	form_maths_deco.h \
	form_maths_deco.C \
	form_maths_delim.h \
	form_maths_delim.C \
	form_maths_matrix.h \
	form_maths_matrix.C \
	form_maths_panel.h \
	form_maths_panel.C \
	form_maths_space.h \
	form_maths_space.C \
	form_maths_style.h \
	form_maths_style.C \
	input_validators.h \
	input_validators.C \
	xformsBC.h \
	xformsBC.C \
	xforms_helpers.h \
	xforms_helpers.C
 
$(XFORMS_SOURCES):
	for i in $(XFORMS_SOURCES); do ln -sf "${top_srcdir}/src/frontends/xforms/$$i" . ; done
 
ETAGS_ARGS = --lang=c++

libqt2xforms_la_SOURCES = \
	$(XFORMS_SOURCES)
