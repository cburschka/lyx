include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libgnome.la

INCLUDES = $(FRONTEND_INCLUDES) -I$(top_srcdir)/src/ \
	-I$(top_srcdir)/src/frontends/ \
	-I$(top_srcdir)/src/frontends/xforms \
	-I$(top_srcdir)/src/frontends/controllers \
	$(BOOST_INCLUDES)

if USE_BASIC_IMAGE_LOADER
else
XFORMSGIMAGE = ../xforms/xformsGImage.lo
endif

# alphabetised
xforms_objects = \
	../xforms/Alert_pimpl.lo \
	../xforms/bmtable.lo \
	../xforms/combox.lo \
	../xforms/Color.lo \
	../xforms/ColorHandler.lo \
	../xforms/DropDown.lo \
	../xforms/FeedbackController.lo \
	../xforms/FileDialog.lo \
	../xforms/FontInfo.lo \
	../xforms/FormBaseDeprecated.lo \
	../xforms/FormFiledialog.lo \
	../xforms/form_filedialog.lo \
	../xforms/FormMathsBitmap.lo \
	../xforms/FormMathsDeco.lo \
	../xforms/form_maths_deco.lo \
	../xforms/FormMathsDelim.lo \
	../xforms/form_maths_delim.lo \
	../xforms/FormMathsMatrix.lo \
	../xforms/form_maths_matrix.lo \
	../xforms/FormMathsPanel.lo \
	../xforms/form_maths_panel.lo \
	../xforms/FormMathsSpace.lo \
	../xforms/form_maths_space.lo \
	../xforms/FormMathsStyle.lo \
	../xforms/form_maths_style.lo \
	../xforms/lyxlookup.lo \
	../xforms/MathsSymbols.lo \
	../xforms/Menubar_pimpl.lo \
	../xforms/Toolbar_pimpl.lo \
	../xforms/Tooltips.lo \
	../xforms/xfont_loader.lo \
	../xforms/xfont_metrics.lo \
	../xforms/xforms_helpers.lo \
	../xforms/xforms_resize.lo \
	../xforms/xformsBC.lo \
	../xforms/XFormsView.lo \
	../xforms/XPainter.lo \
	../xforms/XWorkArea.lo $(XFORMSGIMAGE)

libgnome.la: xforms.lo $(libgnome_la_OBJECTS) $(libgnome_la_DEPENDENCIES)

xforms.lo: $(xforms_objects)
	$(CXXLINK) $(xforms_objects)

###
# KOZ 20020315 Large commented out removed.
###

libgnome_la_SOURCES = \
	Dialogs.C \
	GAbout.C \
	GAbout.h \
	GError.C \
	GError.h \
	GERT.C \
	GERT.h \
	GFloat.C \
	GFloat.h \
	GLog.C \
	GLog.h \
	GPreamble.C \
	GPreamble.h \
	GTabularCreate.C \
	GTabularCreate.h \
	GUrl.C \
	GUrl.h \
	gnome_helpers.C \
	gnome_helpers.h \
	GnomeBase.C \
	GnomeBase.h \
	gnomeBC.C \
	gnomeBC.h \
	GUIRunTime.C \
	pixbutton.h \
	Timeout_pimpl.C \
	Timeout_pimpl.h