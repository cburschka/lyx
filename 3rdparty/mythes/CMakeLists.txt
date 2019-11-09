cmake_minimum_required(VERSION 3.1)

set(LYX_IPO_SUPPORTED FALSE)
if (POLICY CMP0069)
  if (NOT LYX_DEBUG)
    cmake_policy(SET CMP0069 NEW)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT LYX_IPO_SUPPORTED)
  endif()
endif()

set(VERSION "1.2.5")
set(SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}/${VERSION})

file(GLOB support_mythes_sources ${SRCDIR}/*.cxx)
file(GLOB support_mythes_headers ${SRCDIR}/*.hxx)

include_directories(${VERSION})

#============================================================================
# lyxmytheslib
#============================================================================

add_library(mytheslibstatic STATIC ${support_mythes_sources} ${support_mythes_headers} )


set(MYTHESLIB_LIBRARY mytheslibstatic CACHE STRING "Mytheslib library" FORCE)
set(MYTHESLIB_INCLUDE_DIR  ${SRCDIR} CACHE STRING "Mytheslib include dir" FORCE)
set(MYTHESLIB_FOUND CACHE STRING "Mytheslib found" FORCE )

set_target_properties(mytheslibstatic PROPERTIES
	FOLDER "3rd_party"
	INTERPROCEDURAL_OPTIMIZATION ${LYX_IPO_SUPPORTED})
