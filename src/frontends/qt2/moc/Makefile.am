include $(top_srcdir)/config/common.am

DISTCLEANFILES += $(libqt2moc_la_SOURCES)

noinst_LTLIBRARIES = libqt2moc.la

INCLUDES = -I$(top_srcdir)/src/ \
	-I$(top_srcdir)/src/frontends/ \
	-I$(top_srcdir)/src/frontends/controllers \
	-I$(top_builddir)/src/frontends/qt2 \
	$(SIGC_CFLAGS) \
	$(FRONTEND_INCLUDES) \
	$(BOOST_INCLUDES)

AM_CXXFLAGS = -DQT_CLEAN_NAMESPACE -DQT_GENUINE_STR

include $(srcdir)/../Makefile.dialogs

#libqt2moc_la_SOURCES = FileDialog_private_moc.C \
#	emptytable_moc.C \
#	lengthcombo_moc.C \
#	qttableview_moc.C \
#	Qt2Base_moc.C

nodist_libqt2moc_la_SOURCES = $(MOCDIALOGS)

libqt2moc.la: $(libqt2moc_la_OBJECTS) $(libqt2moc_la_DEPENDENCIES)

%_moc.C: ../%.h
	$(MOC) -o $@ $<
