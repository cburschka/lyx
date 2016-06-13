include $(top_srcdir)/config/common.am

EXTRA_DIST = os_cygwin.cpp os_unix.cpp os_win32.cpp os_win32.h \
	CMakeLists.txt tests/CMakeLists.txt tests/supporttest.cmake

noinst_LIBRARIES = liblyxsupport.a


#########################  Qt stuff  #############################
#

MOCHEADER = \
	ConsoleApplicationPrivate.h \
	SystemcallPrivate.h

MOCEDFILES = $(MOCHEADER:%.h=moc_%.cpp)

CLEANFILES = $(MOCEDFILES)
BUILT_SOURCES = $(MOCEDFILES)

moc_%.cpp: %.h
	$(QT_MOC) -o $@ $<

liblyxsupport_a_DEPENDENCIES = $(MOCEDFILES)

#
##################################################################

AM_CPPFLAGS += -I$(srcdir)/.. \
	$(BOOST_INCLUDES) $(ICONV_INCLUDES) $(ZLIB_INCLUDES) \
	$(QT_CPPFLAGS) $(QT_INCLUDES)

liblyxsupport_a_SOURCES = \
	FileMonitor.h \
	FileMonitor.cpp \
	RandomAccessList.h \
	bind.h \
	Changer.h \
	ConsoleApplication.cpp \
	ConsoleApplication.h \
	ConsoleApplicationPrivate.h \
	convert.cpp \
	convert.h \
	copied_ptr.h \
	debug.cpp \
	debug.h \
	docstream.cpp \
	docstream.h \
	docstring.cpp \
	docstring.h \
	docstring_list.h \
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
	functional.h \
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
	mutex.h \
	mutex.cpp \
	Messages.cpp \
	Messages.h \
	numpunct_lyx_char_type.h \
	os.cpp \
	os.h \
	PathChanger.cpp \
	PathChanger.h \
	Package.cpp \
	Package.h \
	ProgressInterface.h \
	pmprof.h \
	qstring_helpers.cpp \
	qstring_helpers.h \
	regex.h \
	RefChanger.h \
	socktools.cpp \
	socktools.h \
	strfwd.h \
	Systemcall.cpp \
	Systemcall.h \
	SystemcallPrivate.h \
	TempFile.cpp \
	TempFile.h \
	textutils.h \
	Translator.h \
	Timeout.cpp \
	Timeout.h \
	trivstring.cpp \
	trivstring.h \
	types.h \
	unique_ptr.h \
	userinfo.cpp \
	userinfo.h \
	unicode.cpp \
	unicode.h \
	weighted_btree.h
if USE_INCLUDED_MYTHES
liblyxsupport_a_SOURCES += \
	mythes/mythes.cxx \
	mythes/mythes.hxx \
	mythes/license.readme
endif

#if INSTALL_MACOSX
#liblyxsupport_a_SOURCES += \
#	AppleSpellChecker.h \
#	AppleSpellChecker.mm
#endif

if INSTALL_MACOSX
liblyxsupport_a_SOURCES += \
	AppleSpeller.h \
	AppleSpeller.m \
	AppleScript.h \
	AppleScript.m \
	AppleScriptProxy.cpp \
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
	tests/test_trivstring \
	tests/regfiles/convert \
	tests/regfiles/filetools \
	tests/regfiles/lstrings \
	tests/regfiles/trivstring


TESTS = \
	tests/test_convert \
	tests/test_filetools \
	tests/test_lstrings \
	tests/test_trivstring

check_PROGRAMS = \
	check_convert \
	check_filetools \
	check_lstrings \
	check_trivstring

if INSTALL_MACOSX
ADD_FRAMEWORKS = -framework QtCore -framework AppKit -framework ApplicationServices
endif

check_convert_LDADD = liblyxsupport.a $(LIBICONV) $(BOOST_LIBS) $(QT_CORE_LIBS) $(LIBSHLWAPI) @LIBS@
check_convert_LDFLAGS = $(QT_CORE_LDFLAGS) $(ADD_FRAMEWORKS)
check_convert_SOURCES = \
	tests/check_convert.cpp \
	tests/dummy_functions.cpp \
	tests/boost.cpp

check_filetools_LDADD = liblyxsupport.a $(LIBICONV) $(BOOST_LIBS) $(QT_CORE_LIBS) $(LIBSHLWAPI) @LIBS@
check_filetools_LDFLAGS = $(QT_CORE_LDFLAGS) $(ADD_FRAMEWORKS)
check_filetools_SOURCES = \
	tests/check_filetools.cpp \
	tests/dummy_functions.cpp \
	tests/boost.cpp

check_lstrings_LDADD = liblyxsupport.a $(LIBICONV) $(BOOST_LIBS) $(QT_CORE_LIBS) $(LIBSHLWAPI) @LIBS@
check_lstrings_LDFLAGS = $(QT_CORE_LDFLAGS) $(ADD_FRAMEWORKS)
check_lstrings_SOURCES = \
	tests/check_lstrings.cpp \
	tests/dummy_functions.cpp \
	tests/boost.cpp

check_trivstring_LDADD = liblyxsupport.a $(LIBICONV) $(BOOST_LIBS) $(QT_CORE_LIBS) $(LIBSHLWAPI) @LIBS@
check_trivstring_LDFLAGS = $(QT_CORE_LDFLAGS) $(ADD_FRAMEWORKS)
check_trivstring_SOURCES = \
	tests/check_trivstring.cpp \
	tests/dummy_functions.cpp \
	tests/boost.cpp

makeregfiles: ${check_PROGRAMS}
	for all in ${check_PROGRAMS} ; do \
		./$$all > ${srcdir}/tests/regfiles/$$all ; \
	done
