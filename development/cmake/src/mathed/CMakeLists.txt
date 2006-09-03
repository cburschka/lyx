project(mathed)

file(GLOB mathed_sources ${TOP_SRC_DIR}/src/mathed/*.C)
file(GLOB mathed_headers ${TOP_SRC_DIR}/src/mathed/*.h)

list(REMOVE_ITEM mathed_sources
	${TOP_SRC_DIR}/src/mathed/math_xyarrowinset.C
	${TOP_SRC_DIR}/src/mathed/math_mboxinset.C
	${TOP_SRC_DIR}/src/mathed/formulamacro.C)

include_directories(${TOP_SRC_DIR}/src/mathed)

add_library(mathed STATIC ${mathed_sources} ${mathed_headers})

project_source_group("${GROUP_CODE}" mathed_sources mathed_headers)

