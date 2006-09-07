project(aspell)


if(${LINK} MATCHES "shared" OR ASPELL_LINK)
	set(ASPELL_SHARED_LIB 1)
	set(ASPELL_LINK SHARED CACHE TYPE STRING)
else(${LINK} MATCHES "shared" OR ASPELL_LINK)
	MESSAGE(STATUS)
	MESSAGE(STATUS "Use -DLINK=shared to build the libraries as shared")
	MESSAGE(STATUS)
	set(ASPELL_LINK STATIC)
endif(${LINK} MATCHES "shared" OR ASPELL_LINK)


########################################################################
#
# Aspell Library
#

set(libaspell_sources
  ../common/cache.cpp
  ../common/string.cpp
  ../common/getdata.cpp
  ../common/itemize.cpp
  ../common/file_util.cpp
  ../common/string_map.cpp
  ../common/string_list.cpp
  ../common/config.cpp
  ../common/posib_err.cpp
  ../common/errors.cpp
  ../common/error.cpp
  ../common/fstream.cpp
  ../common/iostream.cpp
  ../common/info.cpp
  ../common/can_have_error.cpp
  ../common/convert.cpp
  ../common/convert_filter.cpp
  ../common/speller.cpp
  ../common/checker.cpp
  ../common/filter.cpp
  ../common/objstack.cpp 
  ../common/strtonum.cpp
  ../common/gettext_init.cpp
  ../common/file_data_util.cpp
  ../modules/speller/default/readonly_ws.cpp
  ../modules/speller/default/suggest.cpp
  ../modules/speller/default/data.cpp
  ../modules/speller/default/multi_ws.cpp
  ../modules/speller/default/phonetic.cpp
  ../modules/speller/default/writable.cpp
  ../modules/speller/default/speller_impl.cpp
  ../modules/speller/default/checker_impl.cpp
  ../modules/speller/default/phonet.cpp
  ../modules/speller/default/typo_editdist.cpp
  ../modules/speller/default/editdist.cpp
  ../modules/speller/default/primes.cpp
  ../modules/speller/default/lang_impl.cpp
  ../modules/speller/default/leditdist.cpp
  ../modules/speller/default/affix.cpp
  ../lib/word_list-c.cpp
  ../lib/info-c.cpp
  ../lib/mutable_container-c.cpp
  ../lib/error-c.cpp
  ../lib/string_map-c.cpp
  ../lib/new_config.cpp
  ../lib/config-c.cpp
  ../lib/string_enumeration-c.cpp
  ../lib/can_have_error-c.cpp
  ../lib/dummy.cpp
  ../lib/new_filter.cpp
  ../lib/new_fmode.cpp
  ../lib/string_list-c.cpp
  ../lib/find_speller.cpp
  ../lib/speller-c.cpp
  ../lib/speller-cm.cpp
  ../lib/string_pair_enumeration-c.cpp
  ../lib/new_checker.cpp
  ../lib/checker-c.cpp
  ../lib/document_checker-c.cpp
  ../lib/convert-c.cpp
  ../lib/language-c.cpp
  ../lib/language_types-c.cpp
  ../lib/language-cm.cpp
  ../lib/munch_list.cpp
  
  
# This is for filters which are ALWAYS static.  
# The url filter is always usefull and fairly simple.
# The genconv filter is mostly defined in the main aspell libarary
#   since it is used by other filters.  The actual genconv filter
#   is just a small wrapper.

  ../modules/filter/url.cpp
  ../modules/filter/genconv.cpp
 
)

file(GLOB commons_headers ../common/*.hpp)

if(COMPILE_IN_FILTERS)

### Add your filter sources here,
### starting with file containing filter class definition followed by
### file containing filter member implementation.
set(libaspell_sources 
  ${libaspell_sources}
  ../modules/filter/email.cpp
  ../modules/filter/tex.cpp
  ../modules/filter/sgml.cpp
  ../modules/filter/context.cpp
  ../modules/filter/nroff.cpp
  ../modules/filter/texinfo.cpp
)
endif(COMPILE_IN_FILTERS)


include_directories(
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/../win32
	${CMAKE_SOURCE_DIR}/../gen
	${CMAKE_SOURCE_DIR}/../lib
	${CMAKE_SOURCE_DIR}/../common
	${CMAKE_SOURCE_DIR}/../modules
	${CMAKE_SOURCE_DIR}/../modules/filter
	${CMAKE_SOURCE_DIR}/../modules/speller
	${CMAKE_SOURCE_DIR}/../modules/speller/default
	${CMAKE_SOURCE_DIR}/../interfaces/cc
)

if(MSVC)
	# debug   library name: aspelld.lib
	# release library name: aspell.lib
	set(CMAKE_DEBUG_POSTFIX "d")
	if(MSVC_IDE)
	else(MSVC_IDE)
		SET(CMAKE_EXE_LINKER_FLAGS /MANIFEST)
	endif(MSVC_IDE)

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Zi  -wd4522 -wd4521 -wd4996 -wd4800")
	set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -wd4522 -wd4521 -wd4996 -wd4800")
	SET(CMAKE_C_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	SET(CMAKE_C_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
endif(MSVC)

if(WIN32)
	### msvc win32 support
	set(libaspell_sources ${libaspell_sources} ../win32/libstub.cpp)
	add_definitions(-DWIN32PORT)
endif(WIN32)

add_library(aspell ${ASPELL_LINK} ${libaspell_sources} ${commons_headers})


if(WIN32)
	target_link_libraries(aspell ole32)
endif(WIN32)


########################################################################
#
# Aspell Program
#

set(aspell_SOURCES 
  ../prog/aspell.cpp 
  ../prog/check_funs.cpp 
  ../prog/checker_string.cpp
  )
  
add_executable(aspellexe ${aspell_SOURCES})

if(WIN32)
	target_link_libraries(aspellexe aspell ole32)
endif(WIN32)


########################################################################
#
# Install
#

set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/../gnuwin32)

install(TARGETS aspellexe aspell
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)
	
install(FILES ../interfaces/cc/aspell.h 
	DESTINATION include)
	



