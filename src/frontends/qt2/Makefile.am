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
	FileDialog.C \
	FileDialog_private.C \
	LyXScreenFactory.C \
	Menubar_pimpl.C \
	Menubar_pimpl.h \
	Timeout_pimpl.C \
	Timeout_pimpl.h \
	Toolbar_pimpl.C \
	Toolbar_pimpl.h \
	QContentPane.C \
	QContentPane.h \
	QLPainter.C \
	QLPainter.h \
	QLyXKeySym.C \
	QLyXKeySym.h \
	Qt2Base.C \
	Qt2Base.h \
	Qt2BC.C	\
	Qt2BC.h	\
	QtView.h \
	QtView.C \
	QWorkArea.h \
	QWorkArea.C \
	WorkAreaFactory.C \
	emptytable.C \
	emptytable.h \
	guiapi.C \
	lengthcombo.C \
	lengthcombo.h \
	lyx_gui.C \
	qfont_loader.h \
	qfont_loader.C \
	qfont_metrics.C \
	qlkey.h \
	qscreen.h \
	qscreen.C \
	qttableview.h \
	qttableview.C

libqt2.la: $(libqt2_la_OBJECTS) $(libqt2_la_DEPENDENCIES)
