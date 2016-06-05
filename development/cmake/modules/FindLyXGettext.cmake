# - Slightly modified Find GNU gettext tools
# This module looks for the GNU gettext tools. This module defines the 
# following values:
#  GETTEXT_MSGMERGE_EXECUTABLE: the full path to the msgmerge tool.
#  GETTEXT_MSGFMT_EXECUTABLE: the full path to the msgfmt tool.
#  GETTEXT_FOUND: True if gettext has been found.
#
# Additionally it provides the following macros:
# GETTEXT_CREATE_TRANSLATIONS ( outputFile [ALL] file1 ... fileN )
#    This will create a target "translations" which will convert the 
#    given input po files into the binary output mo file. If the 
#    ALL option is used, the translations will also be created when
#    building the default target.

set(hints HINTS "${GNUWIN32_DIR}/gettext-tools")
FIND_PROGRAM(GETTEXT_MSGMERGE_EXECUTABLE msgmerge ${hints})
FIND_PROGRAM(GETTEXT_MSGFMT_EXECUTABLE   msgfmt   ${hints})
FIND_PROGRAM(GETTEXT_XGETTEXT_EXECUTABLE xgettext ${hints})
FIND_PROGRAM(GETTEXT_MSGUNIQ_EXECUTABLE  msguniq  ${hints})

MACRO(GETTEXT_CREATE_TRANSLATIONS _potFile _firstPoFile)

   SET(_gmoFiles)
   # remove only the last extension e.g
   # 	LyX2.0.pot ==> LyX2.0
   # and not LyX2.0.pot ==> LyX2
   # as it would be with NAME_WE
   # GET_FILENAME_COMPONENT(_potBasename ${_potFile} NAME_WE)
   GET_FILENAME_COMPONENT(_potname ${_potFile} NAME)
   if (_potname MATCHES "^\(.+\)\\.[^\\.]+$")
       set(_potBasename ${CMAKE_MATCH_1})
   else()
       set(_potBasename ${_potname})
   endif()
   GET_FILENAME_COMPONENT(_absPotFile ${_potFile} ABSOLUTE)

   SET(_addToAll)
   set(_firstArg)
   IF(${_firstPoFile} STREQUAL "ALL")
      SET(_addToAll "ALL")
   else()
      set(_firstArg ${_firstPoFile})
   ENDIF(${_firstPoFile} STREQUAL "ALL")

   FOREACH (_currentPoFile ${_firstArg} ${ARGN})
      GET_FILENAME_COMPONENT(_absFile ${_currentPoFile} ABSOLUTE)
      GET_FILENAME_COMPONENT(_abs_PATH ${_absFile} PATH)
      GET_FILENAME_COMPONENT(_lang ${_absFile} NAME_WE)
      SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)

      ADD_CUSTOM_COMMAND( 
         OUTPUT ${_gmoFile} 
         COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet --update --backup=none ${_absFile} ${_absPotFile}
         COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -c --statistics -o ${_gmoFile}.1 ${_absFile}
         COMMAND ${CMAKE_COMMAND} -E rename ${_gmoFile}.1 ${_gmoFile}
         DEPENDS ${_absPotFile} ${_absFile}
      )

      INSTALL(FILES ${_gmoFile} DESTINATION ${LYX_LOCALEDIR}/${_lang}/LC_MESSAGES RENAME ${_potBasename}.mo)
      SET(_gmoFiles ${_gmoFiles} ${_gmoFile})

   ENDFOREACH (_currentPoFile )

   ADD_CUSTOM_TARGET(translations ${_addToAll} DEPENDS ${_gmoFiles})
   set_target_properties(translations PROPERTIES FOLDER "i18n")

ENDMACRO(GETTEXT_CREATE_TRANSLATIONS )

IF (GETTEXT_MSGMERGE_EXECUTABLE AND GETTEXT_MSGFMT_EXECUTABLE )
   SET(GETTEXT_FOUND TRUE)
ELSE (GETTEXT_MSGMERGE_EXECUTABLE AND GETTEXT_MSGFMT_EXECUTABLE )
   SET(GETTEXT_FOUND FALSE)
   IF (GetText_REQUIRED)
      MESSAGE(FATAL_ERROR "GetText not found")
   ENDIF (GetText_REQUIRED)
ENDIF (GETTEXT_MSGMERGE_EXECUTABLE AND GETTEXT_MSGFMT_EXECUTABLE )

mark_as_advanced(GETTEXT_MSGMERGE_EXECUTABLE GETTEXT_MSGFMT_EXECUTABLE GETTEXT_XGETTEXT_EXECUTABLE GETTEXT_MSGUNIQ_EXECUTABLE)

