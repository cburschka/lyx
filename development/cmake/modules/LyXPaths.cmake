#
#  Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#  
#  1. Redistributions of source code must retain the copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. The name of the author may not be used to endorse or promote products 
#     derived from this software without specific prior written permission.
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

if(NOT WIN32)
    	set(_prog_path ~/bin)
else()

if(WINDEPS)
	FIND_PROGRAM(LYX_QMAKE NAMES qmake)
	if(NOT LYX_QMAKE)
		message("------ qmake not found. Add YOUR_PATH\\lyx-windows-deps-msvc-qt4\\qt-4\\bin")
		message("------ to PATH: set PATH=%PATH%;YOUR_PATH\\lyx-windows-deps-msvc-qt4\\qt-4\\bin")
		message(FATAL_ERROR "Exit.")
	else()
		GET_FILENAME_COMPONENT(LYX_QMAKE_PATH ${LYX_QMAKE} PATH)
		set(WINDEPSDIR "${LYX_QMAKE_PATH}/../..")
	endif()
	
	set(GNUWIN32_DIR ${WINDEPSDIR})
	
	# don't stop in FindQt4.cmake because qmake will
	# report wrong paths
	set(QT_HEADERS_DIR "${WINDEPSDIR}/qt-4/include" CACHE TYPE STRING FORCE)
	set(QT_LIBRARY_DIR "${WINDEPSDIR}/qt-4/lib" CACHE TYPE STRING FORCE)
	set(QT_BINARY_DIR  "${WINDEPSDIR}/qt-4/bin" CACHE TYPE STRING FORCE)
	set(QT_MKSPECS_DIR "${WINDEPSDIR}/qt-4/mkspecs" CACHE TYPE STRING FORCE)
	set(QT_PLUGINS_DIR "${WINDEPSDIR}/qt-4/plugins" CACHE TYPE STRING FORCE)
	set(QT_MOC_EXECUTABLE "${WINDEPSDIR}/qt-4/bin/moc.exe" CACHE TYPE STRING FORCE)
	set(QT_UIC_EXECUTABLE "${WINDEPSDIR}/qt-4/bin/uic.exe" CACHE TYPE STRING FORCE)
	set(QT_RCC_EXECUTABLE "${WINDEPSDIR}/qt-4/bin/rcc.exe" CACHE TYPE STRING FORCE)
endif()

if(LYX_3RDPARTY_BUILD)
    find_package(GNUWIN32)
    file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" _prog_path)
else()
    find_package(GNUWIN32 REQUIRED)

    file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" _prog_path)

    set(_zlib_path ${_prog_path}/zlib)
    set(_iconv_path ${_prog_path}/iconv)

    set(_gnuwin32_dir ${GNUWIN32_DIR})

    set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} 
            ${_gnuwin32_dir}/include
            ${_zlib_path}/include
            ${_iconv_path}/include)
            
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} 
            ${_gnuwin32_dir}/lib
            ${_zlib_path}/lib
            ${_iconv_path}/lib)

    #chek_include_files path
    set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${_gnuwin32_dir}/include)
endif()

endif()


if(WIN32)
	set(locale_dir Resources/locale)
else()	
	set(locale_dir share/locale)
endif()

set(PREFIX ${_prog_path}/LyX)
set(LOCAL_DIR ${PREFIX}/${locale_dir})

#message(" PREFIX : ${PREFIX}")
#message(" LOCAL_DIR : ${LOCAL_DIR}")
#message(" TOP_SRC_DIR : ${TOP_SRC_DIR}")
