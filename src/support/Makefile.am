include $(top_srcdir)/config/common.am

CLEANFILES += $(BUILT_SOURCES)

EXTRA_DIST = Package.cpp.in pch.h \
	os_cygwin.cpp os_unix.cpp os_win32.cpp os_win32.h

pkglib_LTLIBRARIES = liblyxsupport.la

liblyxsupport_la_LIBADD = $(LIBSHLWAPI) $(QT4_CORE_LIB) $(BOOST_SIGNALS)
liblyxsupport_la_LDFLAGS = $(QT4_CORE_LDFLAGS)

BUILT_SOURCES = $(PCH_FILE) Package.cpp

AM_CPPFLAGS += $(PCH_FLAGS) -I$(srcdir)/.. $(BOOST_INCLUDES)
AM_CPPFLAGS += $(QT4_CPPFLAGS) $(QT4_CORE_INCLUDES)

liblyxsupport_la_SOURCES = \
	FileMonitor.h \
	FileMonitor.cpp \
	RandomAccessList.h \
	abort.cpp \
	chdir.cpp \
	convert.cpp \
	convert.h \
	copy.cpp \
	copied_ptr.h \
	cow_ptr.h \
	debugstream.h \
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
	Forkedcall.cpp \
	Forkedcall.h \
	ForkedCallQueue.cpp \
	ForkedCallQueue.h \
	ForkedcallsController.cpp \
	ForkedcallsController.h \
	fs_extras.cpp \
	fs_extras.h \
	getcwd.cpp \
	gzstream.cpp \
	gzstream.h \
	kill.cpp \
	limited_stack.h \
	lstrings.cpp \
	lstrings.h \
	lyxalgo.h \
	lyxlib.h \
	lyxmanip.h \
	lyxtime.cpp \
	lyxtime.h \
	lyxsum.cpp \
	mkdir.cpp \
	os.cpp \
	os.h \
	Path.cpp \
	Path.h \
	Package.cpp \
	Package.h \
	qstring_helpers.cpp \
	qstring_helpers.h \
	rename.cpp \
	socktools.cpp \
	socktools.h \
	std_istream.h \
	std_ostream.h \
	Systemcall.cpp \
	Systemcall.h \
	tempname.cpp \
	textutils.cpp \
	textutils.h \
	Translator.h \
	types.h \
	userinfo.cpp \
	userinfo.h \
	unicode.cpp \
	unicode.h \
	unlink.cpp


Package.cpp: build_package

# Solaris sed does not like spaces bewteen the ;-delimited commands
build_package: Package.cpp.in
	@rm -f tmp_package ;\
	sed "s,@LYX_DIR@,$(LYX_ABS_INSTALLED_DATADIR),;\
s,@LOCALEDIR@,$(LYX_ABS_INSTALLED_LOCALEDIR),;\
s,@TOP_SRCDIR@,$(LYX_ABS_TOP_SRCDIR),;\
s,@PROGRAM_SUFFIX@,$(program_suffix)," \
		$(srcdir)/Package.cpp.in > tmp_package ;\
	if cmp -s tmp_package Package.cpp ; then \
		rm -f tmp_package ;\
	else \
		rm -f Package.cpp ;\
		mv tmp_package Package.cpp ;\
	fi


############################## Tests ##################################

EXTRA_DIST += test_convert test_filetools test_lstrings regfiles

TESTS = \
	test_convert \
	test_filetools \
	test_lstrings

check_PROGRAMS = \
	check_convert \
	check_filetools \
	check_lstrings

check_convert_LDADD = ../debug.o convert.o docstring.o lstrings.o unicode.o \
	qstring_helpers.o $(BOOST_LIBS) $(QT4_CORE_LIB)
check_convert_LDFLAGS = $(QT4_CORE_LDFLAGS)
check_convert_SOURCES = \
	tests/check_convert.cpp \
	tests/boost.cpp

check_filetools_LDADD = ../debug.o libsupport.la $(BOOST_LIBS)
check_filetools_SOURCES = \
	tests/check_filetools.cpp \
	tests/boost.cpp

check_lstrings_LDADD = ../debug.o lstrings.o convert.o docstring.o unicode.o \
	qstring_helpers.o $(QT4_CORE_LIB)
check_lstrings_LDFLAGS = $(QT4_CORE_LDFLAGS)
check_lstrings_SOURCES = \
	tests/check_lstrings.cpp \
	tests/boost.cpp

makeregfiles: ${check_PROGRAMS}
	for all in ${check_PROGRAMS} ; do \
		./$$all > ${srcdir}/regfiles/$$all ; \
	done
