include $(top_srcdir)/config/common.am

SUBDIRS = ui moc

EXTRA_DIST = Makefile.dialogs QDocument.C QDocument.h \
	QDocumentDialog.C QDocumentDialog.h

noinst_LTLIBRARIES = libqt2.la

INCLUDES = -I$(top_srcdir)/src/ -I$(top_srcdir)/src/frontends/ \
	-I$(top_srcdir)/images \
	$(FRONTEND_INCLUDES) $(BOOST_INCLUDES) \
	-I$(top_srcdir)/src/frontends/controllers

AM_CXXFLAGS = -DQT_CLEAN_NAMESPACE -DQT_GENUINE_STR

include $(srcdir)/Makefile.dialogs

libqt2_la_SOURCES = \
	Alert_pimpl.C \
	qgridview.h \
	qgridview.cpp \
	BulletsModule.C \
	BulletsModule.h \
	QBrowseBox.C \
	QBrowseBox.h \
	Dialogs.C \
	Dialogs2.C \
	Dialogs_impl.h \
	FileDialog.C \
	FileDialog_private.C \
	LyXKeySymFactory.C \
	LyXScreenFactory.C \
	Menubar_pimpl.C \
	Menubar_pimpl.h \
	Timeout_pimpl.C \
	Timeout_pimpl.h \
	Toolbar_pimpl.C \
	Toolbar_pimpl.h \
	QCommandBuffer.C \
	QCommandBuffer.h \
	QCommandEdit.C \
	QCommandEdit.h \
	QContentPane.C \
	QContentPane.h \
	QLImage.C \
	QLImage.h \
	QLPainter.C \
	QLPainter.h \
	QLPopupMenu.C \
	QLPopupMenu.h \
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
	iconpalette.h \
	iconpalette.C \
	io_callback.h \
	io_callback.C \
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
	qttableview.C \
	$(DIALOGSOURCES)

libqt2.la: $(libqt2_la_OBJECTS) $(libqt2_la_DEPENDENCIES)
