AUTOMAKE_OPTIONS = foreign
DISTCLEANFILES= *.orig *.rej *~ *.bak core
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libsupport.la
LIBS =
ETAGS_ARGS = --lang=c++
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${srcdir}/../ $(BOOST_INCLUDES)

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
	path.C \
	path.h \
	putenv.C \
	rename.C \
	rmdir.C \
	snprintf.h \
	snprintf.c \
	sstream.h \
	$(REGEX) syscall.C \
	syscall.h \
	syscontr.C \
	syscontr.h \
	syssingleton.C \
	tempname.C \
	translator.h \
	textutils.h \
	unlink.C
