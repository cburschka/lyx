AUTOMAKE_OPTIONS = foreign
noinst_LIBRARIES = libsupport.a
libsupport_a_LIBADD = @LIBOBJS@
ETAGS_ARGS = --c++
INCLUDES = -I${srcdir}/../

if USE_LYXSTRING
LYXSTRING = lyxstring.C lyxstring.h
endif

libsupport_a_SOURCES = \
	DebugStream.C \
	DebugStream.h \
	FileInfo.C \
	FileInfo.h \
	LAssert.h \
	filetools.C \
	filetools.h \
	lstrings.C \
	lstrings.h \
	lyxlib.h \
	$(LYXSTRING) lyxsum.C \
	path.h \
	syscall.C \
	syscall.h \
	syscontr.C \
	syscontr.h \
	syssingleton.C \
	textutils.h

