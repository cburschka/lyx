include $(top_srcdir)/config/common.am

CLEANFILES += $(BUILT_SOURCES)

EXTRA_DIST = pch.h \
	os_cygwin.cpp os_unix.cpp os_win32.cpp os_win32.h

noinst_LTLIBRARIES = liblyxsupport.la

liblyxsupport_la_LIBADD = $(LIBSHLWAPI) $(QT4_CORE_LIB) $(BOOST_SIGNALS)
liblyxsupport_la_LDFLAGS = $(QT4_CORE_LDFLAGS)

BUILT_SOURCES = $(PCH_FILE)

AM_CPPFLAGS += $(PCH_FLAGS) -I$(srcdir)/.. $(BOOST_INCLUDES)
AM_CPPFLAGS += $(QT4_CPPFLAGS) $(QT4_CORE_INCLUDES) -I$(srcdir)/minizip

# force the use of C++ compiler for minizip/*.c files, because
# gcc can not go through the included boost files.
liblyxsupport_la_CFLAGS = -x c++

liblyxsupport_la_SOURCES = \
	FileMonitor.h \
	FileMonitor.cpp \
	RandomAccessList.h \
	abort.cpp \
	convert.cpp \
	convert.h \
	copy.cpp \
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
	FileFilterList.cpp \
	FileFilterList.h \
	FileName.cpp \
	FileName.h \
	filetools.cpp \
	filetools.h \
	ForkedCalls.cpp \
	ForkedCalls.h \
	gettext.cpp \
	gettext.h \
	getcwd.cpp \
	gzstream.cpp \
	gzstream.h \
	kill.cpp \
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
	mkdir.cpp \
	os.cpp \
	os.h \
	Path.cpp \
	Path.h \
	Package.cpp \
	Package.h \
	qstring_helpers.h \
	rename.cpp \
	socktools.cpp \
	socktools.h \
	strfwd.h \
	Systemcall.cpp \
	Systemcall.h \
	SystemcallPrivate.cpp \
	SystemcallPrivate.h \
	ProgressInterface.h \
	SignalSlot.cpp \
	SignalSlot.h \
	SignalSlotPrivate.h \
	tempname.cpp \
	textutils.h \
	Translator.h \
	Timeout.cpp \
	Timeout.h \
	types.h \
	userinfo.cpp \
	userinfo.h \
	unicode.cpp \
	unicode.h \
	minizip/crypt.h \
	minizip/ioapi.c \
	minizip/ioapi.h \
	minizip/iowin32.c \
	minizip/iowin32.h \
	minizip/unzip.c \
	minizip/unzip.h \
	minizip/zip.c \
	minizip/zip.h \
	minizip/zipunzip.cpp

############################## Tests ##################################

EXTRA_DIST += \
	tests/test_convert \
	tests/test_filetools \
	tests/test_lstrings \
	tests/regfiles/convert \
	tests/regfiles/filetools \
	tests/regfiles/lstrings


TESTS = \
	test_convert \
	test_filetools \
	test_lstrings

check_PROGRAMS = \
	check_convert \
	check_filetools \
	check_lstrings

check_convert_LDADD = ../debug.o convert.o docstring.o lstrings.o unicode.o \
	$(BOOST_LIBS) $(QT4_CORE_LIB)
check_convert_LDFLAGS = $(QT4_CORE_LDFLAGS)
check_convert_SOURCES = \
	tests/check_convert.cpp \
	tests/boost.cpp

check_filetools_LDADD = ../debug.o libsupport.la $(BOOST_LIBS)
check_filetools_SOURCES = \
	tests/check_filetools.cpp \
	tests/boost.cpp

check_lstrings_LDADD = ../debug.o lstrings.o convert.o docstring.o unicode.o \
	$(QT4_CORE_LIB)
check_lstrings_LDFLAGS = $(QT4_CORE_LDFLAGS)
check_lstrings_SOURCES = \
	tests/check_lstrings.cpp \
	tests/boost.cpp

makeregfiles: ${check_PROGRAMS}
	for all in ${check_PROGRAMS} ; do \
		./$$all > ${srcdir}/regfiles/$$all ; \
	done
