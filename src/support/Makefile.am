AUTOMAKE_OPTIONS = foreign
noinst_LTLIBRARIES = libsupport.la
LIBS =
ETAGS_ARGS = --c++
INCLUDES = -I${srcdir}/../

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
	LAssert.h \
	LIstream.h \
	LOstream.h \
	LRegex.C \
	LRegex.h \
	LSubstring.C \
	LSubstring.h \
	abort.C \
	block.h \
	chdir.C \
	date.C \
	filetools.C \
	filetools.h \
	getUserName.C \
	getcwd.C \
	kill.C \
	lstrings.C \
	lstrings.h \
	lyxlib.h \
	$(LYXSTRING) lyxsum.C \
	path.h \
	$(REGEX) syscall.C \
	syscall.h \
	syscontr.C \
	syscontr.h \
	syssingleton.C \
	textutils.h
