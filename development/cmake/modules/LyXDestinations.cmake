# This file is part of lyx.
#
# Helper function to get path to destination directories
#
# Copyright (c) 2016 Kornel Benko <kornel@lyx.org>
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
# This file is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with lyx; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# LYX_DATA_SUBDIR is defined in ${TOP_SRC_DIR}/CMakeLists.txt
# 

function(get_locale_destination _result)
  set(_dir)
  if(WIN32)
    # "Resources/locale"
    set(_dir "${LYX_DATA_SUBDIR}locale")
  elseif(APPLE)
    # "Contents/Resources/locale"
    set(_dir "${LYX_DATA_SUBDIR}locale")
  elseif(UNIX)
    set(_dir "share/locale")
  else()
    message(FATAL_ERROR "Unhandled platform")
  endif()
  set(${_result} ${_dir} PARENT_SCOPE)
endfunction()

function(get_data_destination _result)
  set(_dir)
  if(WIN32)
    set(_dir "${LYX_DATA_SUBDIR}")
  elseif(APPLE)
    # "Contents/Resources/"
    set(_dir "${LYX_DATA_SUBDIR}")
  elseif(UNIX)
    set(_dir "${LYX_DATA_SUBDIR}")
  else()
    message(FATAL_ERROR "Unhandled platform")
  endif()
  set(${_result} ${_dir} PARENT_SCOPE)
endfunction()

function(get_font_destination _result)
  set(_dir)
  if(WIN32)
    set(_dir "${LYX_DATA_SUBDIR}fonts/")
  elseif(APPLE)
    # "Contents/Resources/fonts/"
    set(_dir "${LYX_DATA_SUBDIR}fonts/")
  elseif(UNIX)
    set(_dir "fonts/truetype/${_lyx}/")
  else()
    message(FATAL_ERROR "Unhandled platform")
  endif()
  set(${_result} ${_dir} PARENT_SCOPE)
endfunction()

function(get_tex_destination _result)
  set(_dir)
  if(WIN32)
    set(_dir "${LYX_DATA_SUBDIR}tex/")
  elseif(APPLE)
    # "Contents/Resources/tex/" 
    set(_dir "${LYX_DATA_SUBDIR}tex/")
  elseif(UNIX)
    set(_dir "texmf/tex/latex/${_lyx}/")
  else()
    message(FATAL_ERROR "Unhandled platform")
  endif()
  set(${_result} ${_dir} PARENT_SCOPE)
endfunction()

