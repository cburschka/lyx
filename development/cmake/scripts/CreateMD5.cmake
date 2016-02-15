#  Copyright (c) 2016, Kornel Benko, <kornel@lyx.org>
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
# FILE_TO_HANDLE
# MD5_OUTPUT

# Use-example in some CMakeLists.txt
#
# set(md5out ...)
# set(fileToCheck ...)
# add_custom_command(
#   OUTPUT ${md5out}
#   COMMAND ${CMAKE_COMMAND}
#      "-DFILE_TO_HANDLE=${fileToCheck}"
#      "-DMD5_OUTPUT=${md5out}"
#      -P ${TOP_SCRIPT_PATH}/CtreateMD5MD5.cmake
#   DEPENDS ")
#
# add_custom_target(lyxmd5 DEPENDS ${md5out})

if (NOT EXISTS "${FILE_TO_HANDLE}")
  message(FATAL_ERROR "File \"${FILE_TO_HANDLE}\" does not exit, cannot create md5")
elseif(IS_DIRECTORY "${FILE_TO_HANDLE}")
  message(FATAL_ERROR "Name \"${FILE_TO_HANDLE}\" refers to a directory, cannot create md5")
else()
  file(MD5 "${FILE_TO_HANDLE}" md5val)
  file(WRITE "${MD5_OUTPUT}" ${md5val})
endif()
