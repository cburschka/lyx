include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libsupport.la

INCLUDES = -I${srcdir}/../ $(SIGC_INCLUDES) $(BOOST_INCLUDES)

EXTRA_DIST = lyxstring.C lyxstring.h regex.c lyxregex.h \
             os_unix.C os_win32.C os_os2.C

if USE_LYXSTRING
LYXSTRING = lyxstring.C lyxstring.h
endif
if USE_REGEX
REGEX = regex.c lyxregex.h
endif

libsupport_la_SOURCES = \
	DebugStream.C \
	DebugStream.h \
	FileInfo.C \
	FileInfo.h \
	LAssert.C \
	LAssert.h \
	LIstream.h \
	LOstream.h \
	LRegex.C \
	LRegex.h \
	LSubstring.C \
	LSubstring.h \
	os.C \
	os.h \
	StrPool.C \
	StrPool.h \
	abort.C \
	atoi.C \
	chdir.C \
	copy.C \
	date.C \
	filetools.C \
	filetools.h \
	fmt.C \
	forkedcall.C \
	forkedcall.h \
	forkedcontr.C \
	forkedcontr.h \
	getUserName.C \
	getcwd.C \
	kill.C \
	lstrings.C \
	lstrings.h \
	lxtl.h \
	lyxalgo.h \
	lyxfunctional.h \
	lyxlib.h \
	lyxmanip.h \
	$(LYXSTRING) lyxsum.C \
	mkdir.C \
	nt_defines.h \
	os2_defines.h \
	os2_errortable.h \
	path.C \
	path.h \
	putenv.C \
	rename.C \
	rmdir.C \
	smart_ptr.h \
	snprintf.h \
	snprintf.c \
	sstream.h \
	$(REGEX) systemcall.C \
	systemcall.h \
	tempname.C \
	textutils.h \
	translator.h \
	types.h \
	unlink.C \
	utility.h

if USE_LD_R
libsupport.la: libsupport.o

libsupport.o: $(libsupport_la_OBJECTS) $(libsupport_la_DEPENDENCIES)
	$(CXXLINK) $(libsupport_la_OBJECTS)
endif
