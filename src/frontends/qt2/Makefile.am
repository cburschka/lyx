include $(top_srcdir)/config/common.am

SUBDIRS = xforms ui moc

EXTRA_DIST = Makefile.dialogs QDocument.C QDocument.h \
	QDocumentDialog.C QDocumentDialog.h QParagraph.h QParagraph.C \
	QParagraphDialog.h QParagraphDialog.C

noinst_LTLIBRARIES = libqt2.la

INCLUDES = -I$(top_srcdir)/src/ -I$(top_srcdir)/src/frontends/ \
	$(SIGC_CFLAGS) -I$(top_srcdir)/images \
	$(FRONTEND_INCLUDES) $(BOOST_INCLUDES) \
	-I$(top_srcdir)/src/frontends/controllers

AM_CXXFLAGS = -DQT_CLEAN_NAMESPACE -DQT_GENUINE_STR

include $(srcdir)/Makefile.dialogs

libqt2_la_SOURCES = \
	$(DIALOGSOURCES) \
	Alert_pimpl.C \
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

libqt2.la: $(libqt2_la_OBJECTS) $(libqt2_la_DEPENDENCIES)
