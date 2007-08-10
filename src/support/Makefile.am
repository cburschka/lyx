include $(top_srcdir)/config/common.am

SUBDIRS = . tests

CLEANFILES += $(BUILT_SOURCES)

EXTRA_DIST = Package.cpp.in pch.h \
	os_cygwin.cpp os_unix.cpp os_win32.cpp os_win32.h

pkglib_LTLIBRARIES = liblyxsupport.la

liblyxsupport_la_LIBADD = $(LIBSHLWAPI) $(QT4_CORE_LIB) $(BOOST_SIGNALS)
liblyxsupport_la_LDFLAGS = $(QT4_CORE_LDFLAGS)

BUILT_SOURCES = $(PCH_FILE) package.C

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
	package.C \
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


package.C: build_package

# Solaris sed does not like spaces bewteen the ;-delimited commands
build_package: Package.cpp.in
	@rm -f tmp_package ;\
	sed "s,@LYX_DIR@,$(LYX_ABS_INSTALLED_DATADIR),;\
s,@LOCALEDIR@,$(LYX_ABS_INSTALLED_LOCALEDIR),;\
s,@TOP_SRCDIR@,$(LYX_ABS_TOP_SRCDIR),;\
s,@PROGRAM_SUFFIX@,$(program_suffix)," \
		$(srcdir)/Package.cpp.in > tmp_package ;\
	if cmp -s tmp_package package.C ; then \
		rm -f tmp_package ;\
	else \
		rm -f package.C ;\
		mv tmp_package package.C ;\
	fi
