include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libsupport.la

INCLUDES = -I$(srcdir)/../ $(BOOST_INCLUDES)

EXTRA_DIST = lyxstring.C lyxstring.h \
	     os_unix.C os_win32.C os_os2.C

if USE_LYXSTRING
LYXSTRING = lyxstring.C lyxstring.h
endif

libsupport_la_SOURCES = \
	BoostFormat.h \
	DebugStream.C \
	DebugStream.h \
	FileInfo.C \
	FileInfo.h \
	FileMonitor.h \
	FileMonitor.C \
	LAssert.C \
	LAssert.h \
	LIstream.h \
	LOstream.h \
	os.C \
	os.h \
	abort.C \
	atoi.C \
	boost-inst.C \
	chdir.C \
	copy.C \
	filename.C \
	filename.h \
	filetools.C \
	filetools.h \
	forkedcall.C \
	forkedcall.h \
	forkedcallqueue.C \
	forkedcallqueue.h \
	forkedcontr.C \
	forkedcontr.h \
	getcwd.C \
	kill.C \
	limited_stack.h \
	lstrings.C \
	lstrings.h \
	lyxalgo.h \
	lyxfunctional.h \
	lyxlib.h \
	lyxmanip.h \
	lyxtime.C \
	lyxtime.h \
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
	snprintf.h \
	snprintf.c \
	sstream.h \
	systemcall.C \
	systemcall.h \
	tempname.C \
	textutils.h \
	tostr.h \
	tostr.C \
	translator.h \
	types.h \
	userinfo.C \
	userinfo.h \
	unlink.C
