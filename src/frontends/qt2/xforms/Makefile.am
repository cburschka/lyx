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
	Color.h \
	Color.C \
	DropDown.h \
	DropDown.C \
	Tooltips.h \
	Tooltips.C \
	$(XFORMSGIMAGE) \
	xforms_helpers.h \
	xforms_helpers.C

libqt2xforms.la: link_files $(libqt2xforms_la_OBJECTS)

link_files:
	for i in $(nodist_libqt2xforms_la_SOURCES); do \
		ln -sf "$(top_srcdir)/src/frontends/xforms/$$i" . ; \
	done
