project(support)

configure_file(package.C.cmake ${CMAKE_CURRENT_BINARY_DIR}/package.C)

set(support_sources
	FileMonitor.h
	FileMonitor.C
	RandomAccessList.h
	abort.C
	chdir.C
	convert.C
	convert.h
	copy.C
	copied_ptr.h
	cow_ptr.h
	debugstream.h
	environment.h
	environment.C
	filefilterlist.C
	filefilterlist.h
	filename.C
	filename.h
	filetools.C
	filetools.h
	forkedcall.C
	forkedcall.h
	forkedcallqueue.C
	forkedcallqueue.h
	forkedcontr.C
	forkedcontr.h
	fs_extras.C
	fs_extras.h
	getcwd.C
	kill.C
	limited_stack.h
	lstrings.C
	lstrings.h
	lyxalgo.h
	lyxlib.h
	lyxmanip.h
	lyxtime.C
	lyxtime.h
	lyxsum.C
	mkdir.C
	os.C
	os.h
	path.C
	path.h
	package.h
	rename.C
	socktools.C
	socktools.h
	std_istream.h
	std_ostream.h
	systemcall.C
	systemcall.h
	tempname.C
	textutils.h
	translator.h
	types.h
	userinfo.C
	userinfo.h
	unlink.C
)



lyx_add_path(support_sources ${TOP_SRC_DIR}/src/support)

include_directories(${TOP_SRC_DIR}/src/support)

add_library(support STATIC ${support_sources} ${CMAKE_CURRENT_BINARY_DIR}/package.C)

target_link_libraries(support boost_signals)

if(WIN32)
	target_link_libraries(support shlwapi)
endif(WIN32)



