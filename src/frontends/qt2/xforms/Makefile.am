include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libqt2xforms.la

INCLUDES = -I$(top_srcdir)/src/ -I$(top_srcdir)/src/frontends/ \
	$(SIGC_CFLAGS) -I$(top_srcdir)/images \
	$(FRONTEND_INCLUDES) $(BOOST_INCLUDES) \
	-I$(top_srcdir)/src/frontends/controllers

if USE_BASIC_IMAGE_LOADER
else
XFORMSGIMAGE = xformsGImage.C xformsGImage.h
endif

nodist_libqt2xforms_la_SOURCES = \
	bmtable.h \
	bmtable.c \
	combox.h \
	combox.C \
	Color.h \
	Color.C \
	DropDown.h \
	DropDown.C \
	FeedbackController.h \
	FeedbackController.C \
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
	$(XFORMSGIMAGE) xformsBC.h \
	xformsBC.C \
	xforms_helpers.h \
	xforms_helpers.C

libqt2xforms.la: link_files $(libqt2xforms_la_OBJECTS)

link_files:
	for i in $(nodist_libqt2xforms_la_SOURCES); do \
		ln -sf "$(top_srcdir)/src/frontends/xforms/$$i" . ; \
	done
