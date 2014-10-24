#  Copyright (c) 2012-2014, Kornel Benko, <kornel@lyx.org>
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
# TOP_SRC_DIR
# TOP_CMAKE_PATH
# TOP_BINARY_DIR
# LYX_DATE:	to be used if not under git control


set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

set(LYX_GIT_COMMIT_HASH "none")

FIND_PROGRAM(LYX_GITVERSION git)
if(LYX_GITVERSION)
  # Override the value from configure.ac, if possible
  EXECUTE_PROCESS(COMMAND ${LYX_GITVERSION} "log" "-1" "--format=%h %ci"
    WORKING_DIRECTORY "${TOP_SRC_DIR}"
    OUTPUT_VARIABLE LYX_DATEtmp OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(LYX_DATEtmp MATCHES "^([0-9a-f]+) ([0-9]+-[0-9]+-[0-9]+)")
    set(LYX_GITHASH ${CMAKE_MATCH_1})
    set(LYX_DATE ${CMAKE_MATCH_2})
    message(STATUS "Git-hash = ${LYX_GITHASH}")
  endif()
  EXECUTE_PROCESS(COMMAND ${LYX_GITVERSION} log -1 "--pretty=format:%H"
    WORKING_DIRECTORY "${TOP_SRC_DIR}" OUTPUT_VARIABLE lyxgitcommit OUTPUT_STRIP_TRAILING_WHITESPACE)
  if (lyxgitcommit MATCHES "^[0-9a-f]+$")
    set(LYX_GIT_COMMIT_HASH ${lyxgitcommit})
  endif()
endif()

foreach(_hf lyx_date lyx_commit_hash)
  if(NOT EXISTS ${TOP_BINARY_DIR}/${_hf}.h)
    configure_file(${TOP_CMAKE_PATH}/${_hf}.h.cmake ${TOP_BINARY_DIR}/${_hf}.h)
  else()
    configure_file(${TOP_CMAKE_PATH}/${_hf}.h.cmake ${TOP_BINARY_DIR}/${_hf}.tmp)
    message(STATUS "Created ${TOP_BINARY_DIR}/${_hf}.tmp")

    EXECUTE_PROCESS(
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TOP_BINARY_DIR}/${_hf}.tmp ${TOP_BINARY_DIR}/${_hf}.h
      ERROR_VARIABLE copy_err
    )

    if(copy_err)
      message(FATAL_ERROR "${CMAKE_COMMAND} -E copy_if_different not working")
    endif()

    EXECUTE_PROCESS(
      COMMAND ${CMAKE_COMMAND} -E remove ${TOP_BINARY_DIR}/${_hf}.tmp
    )
  endif()
endforeach()

