include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libsupport.la

INCLUDES = -I$(srcdir)/.. $(BOOST_INCLUDES)

EXTRA_DIST = lyxstring.C lyxstring.h \
	     os_unix.C os_win32.C os_os2.C

if USE_LYXSTRING
LYXSTRING = lyxstring.C lyxstring.h
endif

localedir = $(datadir)/locale

libsupport_la_SOURCES = \
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
	chdir.C \
	copy.C \
	filetools.C \
	filetools.h \
	forkedcall.C \
	forkedcall.h \
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
	$(LYXSTRING) lyxsum.C \
	mkdir.C \
	nt_defines.h \
	os2_defines.h \
	os2_errortable.h \
	package.C \
	package.h \
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
	translator.h \
	types.h \
	unlink.C

package.lo: ../config.h \
	package.C package.h DebugStream.h FileInfo.h \
	filetools.h LAssert.h LOstream.h lstrings.h lyxlib.h os.h \
	$(top_srcdir)/src/BoostFormat.h $(top_srcdir)/src/debug.h \
	$(top_srcdir)/src/gettext.h $(top_srcdir)/src/Lsstream.h \
	$(top_srcdir)/src/LString.h
	$(CXXCOMPILE) \
		-DLYX_DIR=\"$(pkgdatadir)\" \
		-DTOP_SRCDIR=\"$(top_srcdir)\" \
		-DLOCALEDIR=\"$(localedir)\" \
		-c $(top_srcdir)/src/support/package.C && \
	echo "timestamp" > $@


