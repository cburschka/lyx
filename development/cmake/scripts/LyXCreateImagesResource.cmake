#  Copyright (c) 2006-2011 Peter Kümmel, <syntheticpp@gmx.net>
#                2012, Kornel Benko, <kornel@lyx.org>
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#  1. Redistributions of source code must retain the copyright
#         notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the copyright
#         notice, this list of conditions and the following disclaimer in the
#         documentation and/or other materials provided with the distribution.
#  3. The name of the author may not be used to endorse or promote products
#         derived from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
#  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
#  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
#  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# To call this script, one has to provide following parameters
# IMAGES_DIR                # root for the directory-tree for .png, .svgz and .git image files 
# RESOURCE_NAME             # full path of the resulting resource-file
# MAPPED_DIR                # Path-prefix to be removed from the file name entries

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

if(NOT RESOURCE_NAME)
  message(FATAL_ERROR "RESOURCE_NAME not given")
endif()

message(STATUS "Generating ${RESOURCE_NAME}")

if(NOT IS_DIRECTORY ${IMAGES_DIR})
  message(FATAL_ERROR "Directory ${IMAGES_DIR} does not exist")
endif()

if(NOT EXISTS ${MAPPED_DIR})
  message(FATAL_ERROR "Directory ${MAPPED_DIR} does not exist")
endif()

file(GLOB_RECURSE images_png      ${IMAGES_DIR}/*.png)
file(GLOB_RECURSE images_gif      ${IMAGES_DIR}/*.gif)
file(GLOB_RECURSE images_svgz      ${IMAGES_DIR}/*.svgz)

set(images ${images_png} ${images_gif} ${images_svgz})

file(REMOVE ${RESOURCE_NAME})
if(EXISTS ${RESOURCE_NAME})
  message(FATAL_ERROR "Cannot remove file ${RESOURCE_NAME}")
endif()

file(WRITE ${RESOURCE_NAME} "<!DOCTYPE RCC><RCC version=\"1.0\">\n")
file(APPEND ${RESOURCE_NAME} "<qresource>\n")

foreach (_current_FILE ${images})
  get_filename_component(_abs_FILE ${_current_FILE} ABSOLUTE)
  string(REGEX REPLACE "${MAPPED_DIR}" "" _file_name ${_abs_FILE})
  file(APPEND ${RESOURCE_NAME} "	 <file alias=\"${_file_name}\">${_abs_FILE}</file>\n")
endforeach (_current_FILE)

# copy lyx.svg too (and use versioning)
get_filename_component(_abs_FILE "${IMAGES_DIR}/lyx.svg" ABSOLUTE)
string(REGEX REPLACE "${MAPPED_DIR}" "" _file_name ${_abs_FILE})
file(APPEND ${RESOURCE_NAME} "         <file alias=\"lyx${SUFFIX}\">${_abs_FILE}</file>\n")

file(APPEND ${RESOURCE_NAME} "</qresource>\n")
file(APPEND ${RESOURCE_NAME} "</RCC>\n")

if(NOT EXISTS ${RESOURCE_NAME})
  message(FATAL_ERROR "File ${RESOURCE_NAME} could not be created")
endif()
