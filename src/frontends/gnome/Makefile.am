include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libgnome.la

INCLUDES = $(FRONTEND_INCLUDES) -I$(top_srcdir)/src/ \
	-I$(top_srcdir)/src/frontends/ \
	-I$(top_srcdir)/src/frontends/xforms \
	-I$(top_srcdir)/src/frontends/controllers \
	$(SIGC_CFLAGS) $(BOOST_INCLUDES)

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
	../xforms/DropDown.lo \
	../xforms/FeedbackController.lo \
	../xforms/FormBaseDeprecated.lo \
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
	../xforms/MathsSymbols.lo \
	../xforms/Menubar_pimpl.lo \
	../xforms/Toolbar_pimpl.lo \
	../xforms/Tooltips.lo \
	../xforms/xforms_helpers.lo \
	../xforms/xformsBC.lo \
	$(XFORMSGIMAGE)

libgnome.la: xforms.lo $(libgnome_la_OBJECTS) $(libgnome_la_DEPENDENCIES)

xforms.lo: $(xforms_objects)
	$(CXXLINK) $(xforms_objects)

###
# KOZ 20020315 Large commented out removed.
###

#LDFLAGS= $(libgnome_la_OBJADD)
LYXDATADIRS =

libgnome_la_SOURCES = \
	Dialogs.C \
	FileDialog.C \
	GError.C \
	GError.h \
	GTabularCreate.C \
	GTabularCreate.h \
	GUrl.C \
	GUrl.h \
	GERT.C \
	GERT.h \
	gnome_helpers.C \
	gnome_helpers.h \
	GnomeBase.C \
	GnomeBase.h \
	gnomeBC.C \
	gnomeBC.h \
	GUIRunTime.C \
	pixbutton.h \
	support.c \
	support.h \
	Timeout_pimpl.C \
	Timeout_pimpl.h

# Trying to make things a litte more tidy. -- Koz-2002-01-11
#	mainapp.C \
#	mainapp.h \
#	Menubar_pimpl.C \
#	Menubar_pimpl.h \
#
#	FormCopyright.C \
#	FormCopyright.h \
#	FormCredits.C \
#	FormCredits.h \
# These still have to be added. Sooner or later. ARRae-20000411
#	GUI_defaults.C \
#	GUI_initialize.C \
#	GUI_postlyxrc.C \
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
