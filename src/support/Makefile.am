include $(top_srcdir)/config/common.am

CLEANFILES += $(BUILT_SOURCES)

EXTRA_DIST = pch.h \
	os_cygwin.cpp os_unix.cpp os_win32.cpp os_win32.h

noinst_LTLIBRARIES = liblyxsupport.la

liblyxsupport_la_LIBADD = $(LIBSHLWAPI) $(QT4_CORE_LIB) $(BOOST_SIGNALS)
liblyxsupport_la_LDFLAGS = $(QT4_CORE_LDFLAGS)

BUILT_SOURCES = $(PCH_FILE)

#########################  Qt stuff  #############################
#

MOCHEADER = SignalSlotPrivate.h

MOCEDFILES = $(MOCHEADER:%.h=%_moc.cpp)

CLEANFILES += $(MOCEDFILES)
BUILT_SOURCES += $(MOCEDFILES)

%_moc.cpp: %.h
	$(MOC4) -o $@ $<

liblyxsupport_la_DEPENDENCIES = $(MOCEDFILES)

#
##################################################################

AM_CPPFLAGS += $(PCH_FLAGS) -I$(srcdir)/.. $(BOOST_INCLUDES)
AM_CPPFLAGS += $(QT4_CPPFLAGS) $(QT4_CORE_INCLUDES) 

liblyxsupport_la_SOURCES = \
	FileMonitor.h \
	FileMonitor.cpp \
	RandomAccessList.h \
	convert.cpp \
	convert.h \
	copied_ptr.h \
	debug.cpp \
	debug.h \
	docstream.cpp \
	docstream.h \
	docstring.cpp \
	docstring.h \
	environment.h \
	environment.cpp \
	ExceptionMessage.h \
	FileName.cpp \
	FileName.h \
	FileNameList.h \
	filetools.cpp \
	filetools.h \
	foreach.h \
	ForkedCalls.cpp \
	ForkedCalls.h \
	gettext.cpp \
	gettext.h \
	gzstream.cpp \
	gzstream.h \
	kill.cpp \
	lassert.h \
	lassert.cpp \
	limited_stack.h \
	lstrings.cpp \
	lstrings.h \
	lyxalgo.h \
	lyxlib.h \
	lyxtime.cpp \
	lyxtime.h \
	lyxsum.cpp \
	Messages.cpp \
	Messages.h \
	os.cpp \
	os.h \
	Path.cpp \
	Path.h \
	Package.cpp \
	Package.h \
	qstring_helpers.cpp \
	qstring_helpers.h \
	socktools.cpp \
	socktools.h \
	strfwd.h \
	Systemcall.cpp \
	Systemcall.h \
	SignalSlot.cpp \
	SignalSlot.h \
	SignalSlotPrivate.cpp \
	SignalSlotPrivate.h \
	textutils.h \
	Translator.h \
	Timeout.cpp \
	Timeout.h \
	types.h \
	userinfo.cpp \
	userinfo.h \
	unicode.cpp \
	unicode.h \
	weighted_btree.h

if INSTALL_MACOSX
liblyxsupport_la_SOURCES += \
	linkback/LinkBack.h \
	linkback/LinkBack.m \
	linkback/LinkBackProxy.h \
	linkback/LinkBackProxy.m \
	linkback/LinkBackServer.h \
	linkback/LinkBackServer.m
endif

############################## Tests ##################################

EXTRA_DIST += \
	tests/test_convert \
	tests/test_filetools \
	tests/test_lstrings \
	tests/regfiles/convert \
	tests/regfiles/filetools \
	tests/regfiles/lstrings


TESTS = \
	tests/test_convert \
	tests/test_filetools \
	tests/test_lstrings

check_PROGRAMS = \
	check_convert \
	check_filetools \
	check_lstrings

check_convert_LDADD = liblyxsupport.la  \
	$(BOOST_LIBS) $(QT4_CORE_LIB)
check_convert_LDFLAGS = $(QT4_CORE_LDFLAGS)
check_convert_SOURCES = \
	tests/check_convert.cpp \
	tests/boost.cpp

check_filetools_LDADD = liblyxsupport.la $(BOOST_LIBS)
check_filetools_SOURCES = \
	tests/check_filetools.cpp \
	tests/boost.cpp

check_lstrings_LDADD = liblyxsupport.la $(BOOST_LIBS) $(QT4_CORE_LIB)
check_lstrings_LDFLAGS = $(QT4_CORE_LDFLAGS)
check_lstrings_SOURCES = \
	tests/check_lstrings.cpp \
	tests/boost.cpp

makeregfiles: ${check_PROGRAMS}
	for all in ${check_PROGRAMS} ; do \
		./$$all > ${srcdir}/tests/regfiles/$$all ; \
	done
