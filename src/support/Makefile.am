include $(top_srcdir)/config/common.am

SUBDIRS = . tests

CLEANFILES += $(BUILT_SOURCES)

EXTRA_DIST = package.C.in os_unix.C os_win32.C os_os2.C

noinst_LTLIBRARIES = libsupport.la

if USE_COMPRESSION
COMPRESSION = gzstream.C gzstream.h
endif

BUILT_SOURCES = package.C

AM_CPPFLAGS += $(PCH_FLAGS) -I$(srcdir)/.. $(BOOST_INCLUDES)

libsupport_la_SOURCES = \
	FileMonitor.h \
	FileMonitor.C \
	abort.C \
	chdir.C \
	convert.C \
	convert.h \
	copy.C \
	copied_ptr.h \
	cow_ptr.h \
	debugstream.h \
	environment.h \
	environment.C \
	filefilterlist.C \
	filefilterlist.h \
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
	fs_extras.C \
	fs_extras.h \
	getcwd.C \
	$(COMPRESSION) kill.C \
	limited_stack.h \
	lstrings.C \
	lstrings.h \
	lyxalgo.h \
	lyxlib.h \
	lyxmanip.h \
	lyxtime.C \
	lyxtime.h \
	lyxsum.C \
	mkdir.C \
	os.C \
	os.h \
	os2_defines.h \
	os2_errortable.h \
	path.C \
	path.h \
	package.C \
	package.h \
	rename.C \
	socktools.C \
	socktools.h \
	std_istream.h \
	std_ostream.h \
	systemcall.C \
	systemcall.h \
	tempname.C \
	textutils.h \
	translator.h \
	types.h \
	userinfo.C \
	userinfo.h \
	unlink.C


package.C: build_package

# Solaris sed does not like spaces bewteen the ;-delimited commands
build_package: package.C.in
	@rm -f tmp_package ;\
	ABS_SRCDIR=`cd $(top_srcdir) && pwd` ;\
	ABS_BUILDDIR=`cd ../.. && pwd` ;\
	sed "s,%LYX_DIR%,$(pkgdatadir),;\
s,%LOCALEDIR%,$(datadir)/locale,;\
s,%TOP_SRCDIR%,$${ABS_SRCDIR}," \
		$(srcdir)/package.C.in > tmp_package ;\
	if cmp -s tmp_package package.C ; then \
		rm -f tmp_package ;\
	else \
		rm -f package.C ;\
		mv tmp_package package.C ;\
	fi
