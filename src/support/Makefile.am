include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libsupport.la

CLEANFILES = path_defines.C

INCLUDES = -I$(srcdir)/../ $(BOOST_INCLUDES)

EXTRA_DIST = path_defines.C.in os_unix.C os_win32.C os_os2.C

if USE_COMPRESSION
COMPRESSION = gzstream.C gzstream.h
endif

BUILT_SOURCES = path_defines.C

libsupport_la_SOURCES = \
	BoostFormat.h \
	FileInfo.C \
	FileInfo.h \
	FileMonitor.h \
	FileMonitor.C \
	abort.C \
	atoi.C \
	boost-inst.C \
	chdir.C \
	copy.C \
	copied_ptr.h \
	cow_ptr.h \
	debugstream,h \
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
	$(COMPRESSION) kill.C \
	limited_stack.h \
	lstrings.C \
	lstrings.h \
	lyxalgo.h \
	lyxfunctional.h \
	lyxlib.h \
	lyxmanip.h \
	lyxtime.C \
	lyxtime.h \
	lyxsum.C \
	mkdir.C \
	nt_defines.h \
	os.C \
	os.h \
	os2_defines.h \
	os2_errortable.h \
	path.C \
	path.h \
	path_defines.C \
	path_defines.h \
	putenv.C \
	rename.C \
	rmdir.C \
	snprintf.h \
	snprintf.c \
	sstream.h \
	std_istream.h \
	std_ostream.h \
	std_sstream.h \
	std_string.h \
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


path_defines.C: build_path_defines

build_path_defines: path_defines.C.in
	@rm -f tmp_path_defines ;\
	ABS_SRCDIR=`cd $(top_srcdir) && pwd` ;\
	ABS_BUILDDIR=`cd ../.. && pwd` ;\
	sed "s,%LYX_DIR%,$(pkgdatadir), ;\
		s,%LOCALEDIR%,$(datadir)/locale, ;\
		s,%BUILDDIR%,$${ABS_BUILDDIR}, ;\
		s,%TOP_SRCDIR%,$${ABS_SRCDIR}," \
		$(srcdir)/path_defines.C.in > tmp_path_defines ;\
	if cmp -s tmp_path_defines path_defines.C ; then \
		rm -f tmp_path_defines ;\
	else \
		rm -f path_defines.C ;\
		mv tmp_path_defines path_defines.C ;\
	fi
