include $(top_srcdir)/config/common.am
include $(srcdir)/../Makefile.dialogs

DISTCLEANFILES += $(MOCDIALOGS)

noinst_LTLIBRARIES = libqt2moc.la

INCLUDES = -I$(top_srcdir)/src/ \
	-I$(top_srcdir)/src/frontends/ \
	-I$(top_srcdir)/src/frontends/controllers \
	-I$(top_builddir)/src/frontends/qt2 \
	$(FRONTEND_INCLUDES) \
	$(BOOST_INCLUDES)

AM_CXXFLAGS = -DQT_CLEAN_NAMESPACE -DQT_GENUINE_STR

nodist_libqt2moc_la_SOURCES = \
	$(MOCDIALOGS) \
	FileDialog_private_moc.C \
	emptytable_moc.C \
	iconpalette_moc.C \
	lengthcombo_moc.C \
	qttableview_moc.C \
	qgridview_moc.C \
	QBrowseBox_moc.C \
	BulletsModule_moc.C \
	Toolbar_pimpl_moc.C \
	QCommandBuffer_moc.C \
	QCommandEdit_moc.C \
	QContentPane_moc.C \
	QLPopupMenu_moc.C \
	Qt2Base_moc.C \
	QtView_moc.C \
	io_callback_moc.C \
	$(MOCDIALOGS)

libqt2moc.la: $(libqt2moc_la_OBJECTS) $(libqt2moc_la_DEPENDENCIES)

%_moc.C: ../%.h
	$(MOC) -o $@ $<

distclean-local:
	rm -f *_moc.C
