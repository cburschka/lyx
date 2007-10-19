# - Find QT 4
# This module can be used to find Qt4.
# The most important issue is that the Qt4 qmake is available via the system path.
# This qmake is then used to detect basically everything else.
# This module defines a number of key variables and macros. First is 
# QT_USE_FILE which is the path to a CMake file that can be included to compile
# Qt 4 applications and libraries.  By default, the QtCore and QtGui 
# libraries are loaded. This behavior can be changed by setting one or more 
# of the following variables to true:
#                    QT_DONT_USE_QTCORE
#                    QT_DONT_USE_QTGUI
#                    QT_USE_QT3SUPPORT
#                    QT_USE_QTASSISTANT
#                    QT_USE_QTDESIGNER
#                    QT_USE_QTMOTIF
#                    QT_USE_QTMAIN
#                    QT_USE_QTNETWORK
#                    QT_USE_QTNSPLUGIN
#                    QT_USE_QTOPENGL
#                    QT_USE_QTSQL
#                    QT_USE_QTXML
#                    QT_USE_QTSVG
#                    QT_USE_QTTEST
#                    QT_USE_QTUITOOLS
#
# All the libraries required are stored in a variable called QT_LIBRARIES.  
# Add this variable to your TARGET_LINK_LIBRARIES.
#  
#  macro QT4_WRAP_CPP(outfiles inputfile ... )
#  macro QT4_WRAP_UI(outfiles inputfile ... )
#  macro QT4_ADD_RESOURCE(outfiles inputfile ... )
#  macro QT4_AUTOMOC(inputfile ... )
#  macro QT4_GENERATE_MOC(inputfile outputfile )
#  
#  QT_FOUND         If false, don't try to use Qt.
#  QT4_FOUND        If false, don't try to use Qt 4.
#                      
#  QT_QTCORE_FOUND        True if QtCore was found.
#  QT_QTGUI_FOUND         True if QtGui was found.
#  QT_QT3SUPPORT_FOUND    True if Qt3Support was found.
#  QT_QTASSISTANT_FOUND   True if QtAssistant was found.
#  QT_QTDESIGNER_FOUND    True if QtDesigner was found.
#  QT_QTMOTIF_FOUND       True if QtMotif was found.
#  QT_QTNETWORK_FOUND     True if QtNetwork was found.
#  QT_QTNSPLUGIN_FOUND    True if QtNsPlugin was found.
#  QT_QTOPENGL_FOUND      True if QtOpenGL was found.
#  QT_QTSQL_FOUND         True if QtSql was found.
#  QT_QTXML_FOUND         True if QtXml was found.
#  QT_QTSVG_FOUND         True if QtSvg was found.
#  QT_QTTEST_FOUND        True if QtTest was found.
#  QT_QTUITOOLS_FOUND     True if QtUiTools was found.
#                      
#  QT_DEFINITIONS   Definitions to use when compiling code that uses Qt.
#                  
#  QT_INCLUDES      List of paths to all include directories of 
#                   Qt4 QT_INCLUDE_DIR and QT_QTCORE_INCLUDE_DIR are
#                   always in this variable even if NOTFOUND,
#                   all other INCLUDE_DIRS are
#                   only added if they are found.
#   
#  QT_INCLUDE_DIR              Path to "include" of Qt4
#  QT_QT_INCLUDE_DIR           Path to "include/Qt" 
#  QT_QT3SUPPORT_INCLUDE_DIR   Path to "include/Qt3Support" 
#  QT_QTASSISTANT_INCLUDE_DIR  Path to "include/QtAssistant" 
#  QT_QTCORE_INCLUDE_DIR       Path to "include/QtCore"         
#  QT_QTDESIGNER_INCLUDE_DIR   Path to "include/QtDesigner" 
#  QT_QTGUI_INCLUDE_DIR        Path to "include/QtGui" 
#  QT_QTMOTIF_INCLUDE_DIR      Path to "include/QtMotif" 
#  QT_QTNETWORK_INCLUDE_DIR    Path to "include/QtNetwork" 
#  QT_QTNSPLUGIN_INCLUDE_DIR   Path to "include/QtNsPlugin" 
#  QT_QTOPENGL_INCLUDE_DIR     Path to "include/QtOpenGL" 
#  QT_QTSQL_INCLUDE_DIR        Path to "include/QtSql" 
#  QT_QTXML_INCLUDE_DIR        Path to "include/QtXml" 
#  QT_QTSVG_INCLUDE_DIR        Path to "include/QtSvg"
#  QT_QTTEST_INCLUDE_DIR       Path to "include/QtTest"
#                            
#  QT_LIBRARY_DIR              Path to "lib" of Qt4
# 
#  QT_PLUGINS_DIR              Path to "plugins" for Qt4
#                            
# For every library of Qt there are three variables:
#  QT_QTFOO_LIBRARY_RELEASE, which contains the full path to the release version
#  QT_QTFOO_LIBRARY_DEBUG, which contains the full path to the debug version
#  QT_QTFOO_LIBRARY, the full path to the release version if available, otherwise to the debug version
#
# So there are the following variables:
# The Qt3Support library:     QT_QT3SUPPORT_LIBRARY
#                             QT_QT3SUPPORT_LIBRARY_RELEASE
#                             QT_QT3SUPPORT_DEBUG
#
# The QtAssistant library:    QT_QTASSISTANT_LIBRARY
#                             QT_QTASSISTANT_LIBRARY_RELEASE
#                             QT_QTASSISTANT_LIBRARY_DEBUG
#
# The QtCore library:         QT_QTCORE_LIBRARY
#                             QT_QTCORE_LIBRARY_RELEASE
#                             QT_QTCORE_LIBRARY_DEBUG
#
# The QtDesigner library:     QT_QTDESIGNER_LIBRARY
#                             QT_QTDESIGNER_LIBRARY_RELEASE
#                             QT_QTDESIGNER_LIBRARY_DEBUG
#
# The QtGui library:          QT_QTGUI_LIBRARY
#                             QT_QTGUI_LIBRARY_RELEASE
#                             QT_QTGUI_LIBRARY_DEBUG
#
# The QtMotif library:        QT_QTMOTIF_LIBRARY
#                             QT_QTMOTIF_LIBRARY_RELEASE
#                             QT_QTMOTIF_LIBRARY_DEBUG
#
# The QtNetwork library:      QT_QTNETWORK_LIBRARY
#                             QT_QTNETWORK_LIBRARY_RELEASE
#                             QT_QTNETWORK_LIBRARY_DEBUG
#
# The QtNsPLugin library:     QT_QTNSPLUGIN_LIBRARY
#                             QT_QTNSPLUGIN_LIBRARY_RELEASE
#                             QT_QTNSPLUGIN_LIBRARY_DEBUG
#
# The QtOpenGL library:       QT_QTOPENGL_LIBRARY
#                             QT_QTOPENGL_LIBRARY_RELEASE
#                             QT_QTOPENGL_LIBRARY_DEBUG
#
# The QtSql library:          QT_QTSQL_LIBRARY
#                             QT_QTSQL_LIBRARY_RELEASE
#                             QT_QTSQL_LIBRARY_DEBUG
#
# The QtXml library:          QT_QTXML_LIBRARY
#                             QT_QTXML_LIBRARY_RELEASE
#                             QT_QTXML_LIBRARY_DEBUG
#
# The QtSvg library:          QT_QTSVG_LIBRARY
#                             QT_QTSVG_LIBRARY_RELEASE
#                             QT_QTSVG_LIBRARY_DEBUG
#
# The QtTest library:         QT_QTTEST_LIBRARY
#                             QT_QTTEST_LIBRARY_RELEASE
#                             QT_QTTEST_LIBRARY_DEBUG
#
# The qtmain library for Windows QT_QTMAIN_LIBRARY
#                             QT_QTMAIN_LIBRARY_RELEASE
#                             QT_QTMAIN_LIBRARY_DEBUG
#
#The QtUiTools library:       QT_QTUITOOLS_LIBRARY
#                             QT_QTUITOOLS_LIBRARY_RELEASE
#                             QT_QTUITOOLS_LIBRARY_DEBUG
#  
# also defined, but NOT for general use are
#  QT_MOC_EXECUTABLE          Where to find the moc tool.
#  QT_UIC_EXECUTABLE          Where to find the uic tool.
#  QT_UIC3_EXECUTABLE         Where to find the uic3 tool.
#  QT_RCC_EXECUTABLE          Where to find the rcc tool
#  
#  QT_DOC_DIR                 Path to "doc" of Qt4
#  QT_MKSPECS_DIR             Path to "mkspecs" of Qt4
#
#
# These are around for backwards compatibility 
# they will be set
#  QT_WRAP_CPP  Set true if QT_MOC_EXECUTABLE is found
#  QT_WRAP_UI   Set true if QT_UIC_EXECUTABLE is found
#  
# These variables do _NOT_ have any effect anymore (compared to FindQt.cmake)
#  QT_MT_REQUIRED         Qt4 is now always multithreaded
#  
# These variables are set to "" Because Qt structure changed 
# (They make no sense in Qt4)
#  QT_QT_LIBRARY        Qt-Library is now split

INCLUDE(CheckSymbolExists)
INCLUDE(MacroAddFileDependencies)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

set(QT_USE_FILE ${CMAKE_ROOT}/Modules/UseQt4.cmake)

set(QT_DEFINITIONS "")

# check for qmake
find_program(QT_QMAKE_EXECUTABLE NAMES qmake qmake-qt4 PATHS
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\4.0.0;InstallDir]/bin"
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Versions\\4.0.0;InstallDir]/bin"
  $ENV{QTDIR}/bin)

set(QT4_INSTALLED_VERSION_TOO_OLD FALSE)

##  macro for asking qmake to process pro files
macro(QT_QUERY_QMAKE outvar invar)
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmpQmake/tmp.pro
    "message(CMAKE_MESSAGE<$$${invar}>)")
  execute_process(COMMAND ${QT_QMAKE_EXECUTABLE}
    WORKING_DIRECTORY  
    ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmpQmake
    OUTPUT_VARIABLE _qmake_query_output
    ERROR_VARIABLE _qmake_query_output )
  file(REMOVE_RECURSE 
    "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmpQmake")
  string(REGEX REPLACE ".*CMAKE_MESSAGE<([^>]*).*" "\\1"
		${outvar} "${_qmake_query_output}")
endmacro(QT_QUERY_QMAKE)


if(QT_QMAKE_EXECUTABLE)

  set(QT4_QMAKE_FOUND FALSE)
  
  exec_program(${QT_QMAKE_EXECUTABLE} ARGS "-query QT_VERSION"
		OUTPUT_VARIABLE QTVERSION)
  # check for qt3 qmake and then try and find qmake-qt4 in the path
  if("${QTVERSION}" MATCHES "Unknown")
    set(QT_QMAKE_EXECUTABLE NOTFOUND CACHE FILEPATH "" FORCE)
    find_program(QT_QMAKE_EXECUTABLE NAMES qmake-qt4 PATHS
      "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\4.0.0;InstallDir]/bin"
      "[HKEY_CURRENT_USER\\Software\\Trolltech\\Versions\\4.0.0;InstallDir]/bin"
      $ENV{QTDIR}/bin
      )
    if(QT_QMAKE_EXECUTABLE)
      EXEC_PROGRAM(${QT_QMAKE_EXECUTABLE} 
        ARGS "-query QT_VERSION" OUTPUT_VARIABLE QTVERSION)
    endif()
  endif()
  # check that we found the Qt4 qmake, Qt3 qmake output won't match here
  string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+" qt_version_tmp "${QTVERSION}")
  if(qt_version_tmp)

    # we need at least version 4.0.0
    if(NOT QT_MIN_VERSION)
      set(QT_MIN_VERSION "4.0.0")
    endif()
    
    # now parse the parts of the user given version string into variables
    string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+" req_qt_major_vers "${QT_MIN_VERSION}")
    if(NOT req_qt_major_vers)
      message( FATAL_ERROR "Invalid Qt version string given: \"${QT_MIN_VERSION}\", expected e.g. \"4.0.1\"")
    endif()
    
    # now parse the parts of the user given version string into variables
    string(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1"
			req_qt_major_vers "${QT_MIN_VERSION}")
    string(REGEX REPLACE "^[0-9]+\\.([0-9])+\\.[0-9]+" "\\1"
			req_qt_minor_vers "${QT_MIN_VERSION}")
    string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1"
			req_qt_patch_vers "${QT_MIN_VERSION}")
    
    if(NOT req_qt_major_vers EQUAL 4)
      message( FATAL_ERROR "Invalid Qt version string given: \"${QT_MIN_VERSION}\", major version 4 is required, e.g. \"4.0.1\"")
    endif()
    
    # and now the version string given by qmake
    string(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1"
			found_qt_major_vers "${QTVERSION}")
    string(REGEX REPLACE "^[0-9]+\\.([0-9])+\\.[0-9]+.*" "\\1"	
			found_qt_minor_vers "${QTVERSION}")
    string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1"
			found_qt_patch_vers "${QTVERSION}")
    
    # compute an overall version number which can be compared at once
    math(EXPR req_vers "${req_qt_major_vers}*10000 + ${req_qt_minor_vers}*100 + ${req_qt_patch_vers}")
    math(EXPR found_vers "${found_qt_major_vers}*10000 + ${found_qt_minor_vers}*100 + ${found_qt_patch_vers}")
    
    if(found_vers LESS req_vers)
      set(QT4_QMAKE_FOUND FALSE)
      set(QT4_INSTALLED_VERSION_TOO_OLD TRUE)
    else()
      set(QT4_QMAKE_FOUND TRUE)
    endif()
  endif()

endif()

if(QT4_QMAKE_FOUND)

  # ask qmake for the library dir
  # Set QT_LIBRARY_DIR
  if(NOT QT_LIBRARY_DIR)
    exec_program( ${QT_QMAKE_EXECUTABLE}
      ARGS "-query QT_INSTALL_LIBS"
      OUTPUT_VARIABLE QT_LIBRARY_DIR_TMP)
    if(EXISTS "${QT_LIBRARY_DIR_TMP}")
      set(QT_LIBRARY_DIR ${QT_LIBRARY_DIR_TMP} CACHE PATH "Qt library dir")
    else()
      message("Warning: QT_QMAKE_EXECUTABLE reported QT_INSTALL_LIBS as ${QT_LIBRARY_DIR_TMP}")
      message("Warning: ${QT_LIBRARY_DIR_TMP} does NOT exist, Qt must NOT be installed correctly.")
    endif()
  endif()
  
  if(APPLE)
    if(EXISTS ${QT_LIBRARY_DIR}/QtCore.framework)
      set(QT_USE_FRAMEWORKS ON
        CACHE BOOL "Set to ON if Qt build uses frameworks.")
    else()
      set(QT_USE_FRAMEWORKS OFF
        CACHE BOOL "Set to ON if Qt build uses frameworks.")
    endif()
    
    mark_as_advanced(QT_USE_FRAMEWORKS)
  endif()
  
  # ask qmake for the binary dir
  if(NOT QT_BINARY_DIR)
    exec_program(${QT_QMAKE_EXECUTABLE}
      ARGS "-query QT_INSTALL_BINS"
      OUTPUT_VARIABLE qt_bins )
    set(QT_BINARY_DIR ${qt_bins} CACHE INTERNAL "")
  endif()

  # ask qmake for the include dir
  if(NOT QT_HEADERS_DIR)
    EXEC_PROGRAM( ${QT_QMAKE_EXECUTABLE}
      ARGS "-query QT_INSTALL_HEADERS" 
      OUTPUT_VARIABLE qt_headers )
    set(QT_HEADERS_DIR ${qt_headers} CACHE INTERNAL "")
  endif()


  # ask qmake for the documentation directory
  if(NOT QT_DOC_DIR)
    exec_program( ${QT_QMAKE_EXECUTABLE}
      ARGS "-query QT_INSTALL_DOCS"
      OUTPUT_VARIABLE qt_doc_dir )
    set(QT_DOC_DIR ${qt_doc_dir} CACHE PATH "The location of the Qt docs")
  endif()

  # ask qmake for the mkspecs directory
  if(NOT QT_MKSPECS_DIR)
    exec_program( ${QT_QMAKE_EXECUTABLE}
      ARGS "-query QMAKE_MKSPECS"
      OUTPUT_VARIABLE qt_mkspecs_dir )
    set(QT_MKSPECS_DIR ${qt_mkspecs_dir} CACHE PATH
			"The location of the Qt mkspecs")
  endif()

  # ask qmake for the plugins directory
  if(NOT QT_PLUGINS_DIR)
    exec_program( ${QT_QMAKE_EXECUTABLE}
      ARGS "-query QT_INSTALL_PLUGINS"
      OUTPUT_VARIABLE qt_plugins_dir )
    set(QT_PLUGINS_DIR ${qt_plugins_dir} CACHE PATH
			"The location of the Qt plugins")
  endif()
  ########################################
  #
  #       Setting the INCLUDE-Variables
  #
  ########################################

  find_path(QT_QTCORE_INCLUDE_DIR QtGlobal
    ${QT_HEADERS_DIR}/QtCore
    ${QT_LIBRARY_DIR}/QtCore.framework/Headers
    NO_DEFAULT_PATH)

  # Set QT_INCLUDE_DIR by removing "/QtCore" in the string ${QT_QTCORE_INCLUDE_DIR}
  if(QT_QTCORE_INCLUDE_DIR AND NOT QT_INCLUDE_DIR)
    if(QT_USE_FRAMEWORKS)
      set(QT_INCLUDE_DIR ${QT_HEADERS_DIR})
    else()
      string(REGEX REPLACE "/QtCore$" "" qt4_include_dir ${QT_QTCORE_INCLUDE_DIR})
      set(QT_INCLUDE_DIR ${qt4_include_dir} CACHE PATH "")
    endif()
  endif()

  if(NOT QT_INCLUDE_DIR)
    if(NOT Qt4_FIND_QUIETLY AND Qt4_FIND_REQUIRED)
      message( FATAL_ERROR "Could NOT find QtGlobal header")
    endif()
  endif()

  #############################################
  #
  # Find out what window system we're using
  #
  #############################################
  # Save required includes variable
  set(CMAKE_REQUIRED_INCLUDES_SAVE ${CMAKE_REQUIRED_INCLUDES})
  # Add QT_INCLUDE_DIR to CMAKE_REQUIRED_INCLUDES
  set(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES};${QT_INCLUDE_DIR}")
  # Check for Window system symbols (note: only one should end up being set)
  check_symbol_exists(Q_WS_X11 "QtCore/qglobal.h" Q_WS_X11)
  check_symbol_exists(Q_WS_MAC "QtCore/qglobal.h" Q_WS_MAC)
  check_symbol_exists(Q_WS_WIN "QtCore/qglobal.h" Q_WS_WIN)

  if(QT_QTCOPY_REQUIRED)
    check_symbol_exists(QT_IS_QTCOPY "QtCore/qglobal.h" QT_KDE_QT_COPY)
    if(NOT QT_IS_QTCOPY)
      message(FATAL_ERROR "qt-copy is required, but hasn't been found")
    endif()
  endif()

  # Restore CMAKE_REQUIRED_INCLUDES variable
  set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES_SAVE})
  #
  #############################################

  IF (QT_USE_FRAMEWORKS)
    set(QT_DEFINITIONS ${QT_DEFINITIONS} -F${QT_LIBRARY_DIR} -L${QT_LIBRARY_DIR} )
  endif()

  # Set QT_QT3SUPPORT_INCLUDE_DIR
  FIND_PATH(QT_QT3SUPPORT_INCLUDE_DIR Qt3Support
    PATHS
    ${QT_INCLUDE_DIR}/Qt3Support
    ${QT_LIBRARY_DIR}/Qt3Support.framework/Headers
    NO_DEFAULT_PATH
    )

  # Set QT_QT_INCLUDE_DIR
  FIND_PATH(QT_QT_INCLUDE_DIR qglobal.h
    PATHS
    ${QT_INCLUDE_DIR}/Qt
    ${QT_LIBRARY_DIR}/QtCore.framework/Headers
    NO_DEFAULT_PATH
    )

  # Set QT_QTGUI_INCLUDE_DIR
  FIND_PATH(QT_QTGUI_INCLUDE_DIR QtGui
    PATHS
    ${QT_INCLUDE_DIR}/QtGui
    ${QT_LIBRARY_DIR}/QtGui.framework/Headers
    NO_DEFAULT_PATH
    )

  # Set QT_QTSVG_INCLUDE_DIR
  FIND_PATH(QT_QTSVG_INCLUDE_DIR QtSvg
    PATHS
    ${QT_INCLUDE_DIR}/QtSvg
    ${QT_LIBRARY_DIR}/QtSvg.framework/Headers
    NO_DEFAULT_PATH
    )

  # Set QT_QTTEST_INCLUDE_DIR
  FIND_PATH(QT_QTTEST_INCLUDE_DIR QtTest
    PATHS
    ${QT_INCLUDE_DIR}/QtTest
    ${QT_LIBRARY_DIR}/QtTest.framework/Headers
    NO_DEFAULT_PATH
    )

  # Set QT_QTUITOOLS_INCLUDE_DIR
  FIND_PATH(QT_QTUITOOLS_INCLUDE_DIR QtUiTools
    PATHS
    ${QT_INCLUDE_DIR}/QtUiTools
    ${QT_LIBRARY_DIR}/QtUiTools.framework/Headers
    NO_DEFAULT_PATH
    )



  # Set QT_QTMOTIF_INCLUDE_DIR
  IF(Q_WS_X11)
    FIND_PATH(QT_QTMOTIF_INCLUDE_DIR QtMotif PATHS ${QT_INCLUDE_DIR}/QtMotif NO_DEFAULT_PATH )
  endif()

  # Set QT_QTNETWORK_INCLUDE_DIR
  FIND_PATH(QT_QTNETWORK_INCLUDE_DIR QtNetwork
    PATHS
    ${QT_INCLUDE_DIR}/QtNetwork
    ${QT_LIBRARY_DIR}/QtNetwork.framework/Headers
    NO_DEFAULT_PATH
    )

  # Set QT_QTNSPLUGIN_INCLUDE_DIR
  FIND_PATH(QT_QTNSPLUGIN_INCLUDE_DIR QtNsPlugin
    PATHS
    ${QT_INCLUDE_DIR}/QtNsPlugin
    ${QT_LIBRARY_DIR}/QtNsPlugin.framework/Headers
    NO_DEFAULT_PATH
    )

  # Set QT_QTOPENGL_INCLUDE_DIR
  FIND_PATH(QT_QTOPENGL_INCLUDE_DIR QtOpenGL
    PATHS
    ${QT_INCLUDE_DIR}/QtOpenGL
    ${QT_LIBRARY_DIR}/QtOpenGL.framework/Headers
    NO_DEFAULT_PATH
    )

  # Set QT_QTSQL_INCLUDE_DIR
  FIND_PATH(QT_QTSQL_INCLUDE_DIR QtSql
    PATHS
    ${QT_INCLUDE_DIR}/QtSql
    ${QT_LIBRARY_DIR}/QtSql.framework/Headers
    NO_DEFAULT_PATH
    )

  # Set QT_QTXML_INCLUDE_DIR
  FIND_PATH(QT_QTXML_INCLUDE_DIR QtXml
    PATHS
    ${QT_INCLUDE_DIR}/QtXml
    ${QT_LIBRARY_DIR}/QtXml.framework/Headers
    NO_DEFAULT_PATH
    )

  # Set QT_QTASSISTANT_INCLUDE_DIR
  FIND_PATH(QT_QTASSISTANT_INCLUDE_DIR QtAssistant
    PATHS
    ${QT_INCLUDE_DIR}/QtAssistant
    ${QT_HEADERS_DIR}/QtAssistant
    NO_DEFAULT_PATH
    )

  # Set QT_QTDESIGNER_INCLUDE_DIR
  FIND_PATH(QT_QTDESIGNER_INCLUDE_DIR QDesignerComponents
    PATHS
    ${QT_INCLUDE_DIR}/QtDesigner
    ${QT_HEADERS_DIR}/QtDesigner
    NO_DEFAULT_PATH
    )

  # Make variables changeble to the advanced user
  MARK_AS_ADVANCED( QT_LIBRARY_DIR QT_INCLUDE_DIR QT_QT_INCLUDE_DIR QT_DOC_DIR)

  # Set QT_INCLUDES
  set( QT_INCLUDES ${QT_INCLUDE_DIR} ${QT_QT_INCLUDE_DIR} ${QT_MKSPECS_DIR}/default )

  # Set QT_QTCORE_LIBRARY by searching for a lib with "QtCore."  as part of the filename
  FIND_LIBRARY(QT_QTCORE_LIBRARY_RELEASE NAMES QtCore QtCore4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH )
  FIND_LIBRARY(QT_QTCORE_LIBRARY_DEBUG NAMES QtCore_debug QtCored QtCored4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH)

  # Set QT_QT3SUPPORT_LIBRARY
  FIND_LIBRARY(QT_QT3SUPPORT_LIBRARY_RELEASE NAMES Qt3Support Qt3Support4 PATHS ${QT_LIBRARY_DIR}        NO_DEFAULT_PATH)
  FIND_LIBRARY(QT_QT3SUPPORT_LIBRARY_DEBUG   NAMES Qt3Support_debug Qt3Supportd4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH)

  # Set QT_QTGUI_LIBRARY
  FIND_LIBRARY(QT_QTGUI_LIBRARY_RELEASE NAMES QtGui QtGui4 PATHS ${QT_LIBRARY_DIR}        NO_DEFAULT_PATH)
  FIND_LIBRARY(QT_QTGUI_LIBRARY_DEBUG   NAMES QtGui_debug QtGuid4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH)

  # Set QT_QTMOTIF_LIBRARY
  IF(Q_WS_X11)
    FIND_LIBRARY(QT_QTMOTIF_LIBRARY_RELEASE NAMES QtMotif PATHS ${QT_LIBRARY_DIR}       NO_DEFAULT_PATH)
    FIND_LIBRARY(QT_QTMOTIF_LIBRARY_DEBUG   NAMES QtMotif_debug PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH)
  endif()

  # Set QT_QTNETWORK_LIBRARY
  FIND_LIBRARY(QT_QTNETWORK_LIBRARY_RELEASE NAMES QtNetwork QtNetwork4 PATHS ${QT_LIBRARY_DIR}        NO_DEFAULT_PATH)
  FIND_LIBRARY(QT_QTNETWORK_LIBRARY_DEBUG   NAMES QtNetwork_debug QtNetworkd4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH)

  # Set QT_QTNSPLUGIN_LIBRARY
  FIND_LIBRARY(QT_QTNSPLUGIN_LIBRARY_RELEASE NAMES QtNsPlugin PATHS ${QT_LIBRARY_DIR}       NO_DEFAULT_PATH)
  FIND_LIBRARY(QT_QTNSPLUGIN_LIBRARY_DEBUG   NAMES QtNsPlugin_debug PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH)

  # Set QT_QTOPENGL_LIBRARY
  FIND_LIBRARY(QT_QTOPENGL_LIBRARY_RELEASE NAMES QtOpenGL QtOpenGL4 PATHS ${QT_LIBRARY_DIR}        NO_DEFAULT_PATH)
  FIND_LIBRARY(QT_QTOPENGL_LIBRARY_DEBUG   NAMES QtOpenGL_debug QtOpenGLd4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH)

  # Set QT_QTSQL_LIBRARY
  FIND_LIBRARY(QT_QTSQL_LIBRARY_RELEASE NAMES QtSql QtSql4 PATHS ${QT_LIBRARY_DIR}        NO_DEFAULT_PATH)
  FIND_LIBRARY(QT_QTSQL_LIBRARY_DEBUG   NAMES QtSql_debug QtSqld4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH)

  # Set QT_QTXML_LIBRARY
  FIND_LIBRARY(QT_QTXML_LIBRARY_RELEASE NAMES QtXml QtXml4 PATHS ${QT_LIBRARY_DIR}        NO_DEFAULT_PATH)
  FIND_LIBRARY(QT_QTXML_LIBRARY_DEBUG   NAMES QtXml_debug QtXmld4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH)

  # Set QT_QTSVG_LIBRARY
  FIND_LIBRARY(QT_QTSVG_LIBRARY_RELEASE NAMES QtSvg QtSvg4 PATHS ${QT_LIBRARY_DIR}        NO_DEFAULT_PATH)
  FIND_LIBRARY(QT_QTSVG_LIBRARY_DEBUG   NAMES QtSvg_debug QtSvgd4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH)

  # Set QT_QTUITOOLS_LIBRARY
  FIND_LIBRARY(QT_QTUITOOLS_LIBRARY_RELEASE NAMES QtUiTools QtUiTools4 PATHS ${QT_LIBRARY_DIR}        NO_DEFAULT_PATH)
  FIND_LIBRARY(QT_QTUITOOLS_LIBRARY_DEBUG   NAMES QtUiTools_debug QtUiToolsd4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH)

  # Set QT_QTTEST_LIBRARY
  FIND_LIBRARY(QT_QTTEST_LIBRARY_RELEASE NAMES QtTest QtTest4 PATHS ${QT_LIBRARY_DIR}                      NO_DEFAULT_PATH)
  FIND_LIBRARY(QT_QTTEST_LIBRARY_DEBUG   NAMES QtTest_debug QtTest_debug4 QtTestd4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH)

  MARK_AS_ADVANCED(QT_QT3SUPPORT_LIBRARY QT_QTGUI_LIBRARY )

  IF( NOT QT_QTCORE_LIBRARY_DEBUG AND NOT QT_QTCORE_LIBRARY_RELEASE )
    IF( NOT Qt4_FIND_QUIETLY AND Qt4_FIND_REQUIRED)
      message( FATAL_ERROR "Could NOT find QtCore. Check ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log for more details.")
    endif()
  endif()

  # Set QT_QTASSISTANT_LIBRARY
  FIND_LIBRARY(QT_QTASSISTANT_LIBRARY_RELEASE NAMES QtAssistantClient QtAssistantClient4 QtAssistant QtAssistant4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH)
  FIND_LIBRARY(QT_QTASSISTANT_LIBRARY_DEBUG   NAMES QtAssistantClientd QtAssistantClientd4 QtAssistantClient_debug QtAssistant_debug QtAssistantd4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH)

  # Set QT_QTDESIGNER_LIBRARY
  FIND_LIBRARY(QT_QTDESIGNER_LIBRARY_RELEASE NAMES QtDesigner QtDesigner4 PATHS ${QT_LIBRARY_DIR}        NO_DEFAULT_PATH)
  FIND_LIBRARY(QT_QTDESIGNER_LIBRARY_DEBUG   NAMES QtDesigner_debug QtDesignerd4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH)

  # Set QT_QTMAIN_LIBRARY
  IF(WIN32)
    FIND_LIBRARY(QT_QTMAIN_LIBRARY_RELEASE NAMES qtmain  PATHS ${QT_LIBRARY_DIR}  NO_DEFAULT_PATH)
    FIND_LIBRARY(QT_QTMAIN_LIBRARY_DEBUG   NAMES qtmaind PATHS ${QT_LIBRARY_DIR}  NO_DEFAULT_PATH)
  endif()

  ############################################
  #
  # Check the existence of the libraries.
  #
  ############################################

  MACRO (_QT4_ADJUST_LIB_VARS basename)
    IF (QT_${basename}_LIBRARY_RELEASE OR QT_${basename}_LIBRARY_DEBUG)   

      # if only the release version was found, set the debug variable also to the release version
      IF (QT_${basename}_LIBRARY_RELEASE AND NOT QT_${basename}_LIBRARY_DEBUG)
        set(QT_${basename}_LIBRARY_DEBUG ${QT_${basename}_LIBRARY_RELEASE})
        set(QT_${basename}_LIBRARY       ${QT_${basename}_LIBRARY_RELEASE})
        set(QT_${basename}_LIBRARIES     ${QT_${basename}_LIBRARY_RELEASE})
      endif()

      # if only the debug version was found, set the release variable also to the debug version
      IF (QT_${basename}_LIBRARY_DEBUG AND NOT QT_${basename}_LIBRARY_RELEASE)
        set(QT_${basename}_LIBRARY_RELEASE ${QT_${basename}_LIBRARY_DEBUG})
        set(QT_${basename}_LIBRARY         ${QT_${basename}_LIBRARY_DEBUG})
        set(QT_${basename}_LIBRARIES       ${QT_${basename}_LIBRARY_DEBUG})
      endif()
      IF (QT_${basename}_LIBRARY_DEBUG AND QT_${basename}_LIBRARY_RELEASE)
        # if the generator supports configuration types then set
        # optimized and debug libraries, or if the CMAKE_BUILD_TYPE has a value
        IF (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
          set(QT_${basename}_LIBRARY       optimized ${QT_${basename}_LIBRARY_RELEASE} debug ${QT_${basename}_LIBRARY_DEBUG})
        else()
          # if there are no configuration types and CMAKE_BUILD_TYPE has no value
          # then just use the release libraries
          set(QT_${basename}_LIBRARY       ${QT_${basename}_LIBRARY_RELEASE} )
        endif()
        set(QT_${basename}_LIBRARIES       optimized ${QT_${basename}_LIBRARY_RELEASE} debug ${QT_${basename}_LIBRARY_DEBUG})
      endif()

      set(QT_${basename}_LIBRARY ${QT_${basename}_LIBRARY} CACHE FILEPATH "The Qt ${basename} library")

      IF (QT_${basename}_LIBRARY)
        set(QT_${basename}_FOUND 1)
      endif()
    endif()

      #add the include directory to QT_INCLUDES
    IF (QT_${basename}_INCLUDE_DIR)
      set(QT_INCLUDES ${QT_INCLUDES} "${QT_${basename}_INCLUDE_DIR}")
    endif()

    # Make variables changeble to the advanced user
    MARK_AS_ADVANCED(QT_${basename}_LIBRARY QT_${basename}_LIBRARY_RELEASE QT_${basename}_LIBRARY_DEBUG QT_${basename}_INCLUDE_DIR)
  ENDMACRO (_QT4_ADJUST_LIB_VARS)


  # Set QT_xyz_LIBRARY variable and add 
  # library include path to QT_INCLUDES
  _QT4_ADJUST_LIB_VARS(QTCORE)
  _QT4_ADJUST_LIB_VARS(QTGUI)
  _QT4_ADJUST_LIB_VARS(QT3SUPPORT)
  _QT4_ADJUST_LIB_VARS(QTASSISTANT)
  _QT4_ADJUST_LIB_VARS(QTDESIGNER)
  _QT4_ADJUST_LIB_VARS(QTNETWORK)
  _QT4_ADJUST_LIB_VARS(QTNSPLUGIN)
  _QT4_ADJUST_LIB_VARS(QTOPENGL)
  _QT4_ADJUST_LIB_VARS(QTSQL)
  _QT4_ADJUST_LIB_VARS(QTXML)
  _QT4_ADJUST_LIB_VARS(QTSVG)
  _QT4_ADJUST_LIB_VARS(QTUITOOLS)
  _QT4_ADJUST_LIB_VARS(QTTEST)

  # platform dependent libraries
  IF(Q_WS_X11)
    _QT4_ADJUST_LIB_VARS(QTMOTIF)
  endif()
  IF(WIN32)
    _QT4_ADJUST_LIB_VARS(QTMAIN)
  endif()

  #######################################
  #
  #       Check the executables of Qt 
  #          ( moc, uic, rcc )
  #
  #######################################


  # find moc and uic using qmake
  QT_QUERY_QMAKE(QT_MOC_EXECUTABLE_INTERNAL "QMAKE_MOC")
  QT_QUERY_QMAKE(QT_UIC_EXECUTABLE_INTERNAL "QMAKE_UIC")

  FILE(TO_CMAKE_PATH 
    "${QT_MOC_EXECUTABLE_INTERNAL}" QT_MOC_EXECUTABLE_INTERNAL)
  FILE(TO_CMAKE_PATH 
    "${QT_UIC_EXECUTABLE_INTERNAL}" QT_UIC_EXECUTABLE_INTERNAL)

  set(QT_MOC_EXECUTABLE 
    ${QT_MOC_EXECUTABLE_INTERNAL} CACHE FILEPATH "The moc executable")
  set(QT_UIC_EXECUTABLE 
    ${QT_UIC_EXECUTABLE_INTERNAL} CACHE FILEPATH "The uic executable")

  FIND_PROGRAM(QT_UIC3_EXECUTABLE
    NAMES uic3
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH
    )

  FIND_PROGRAM(QT_RCC_EXECUTABLE 
    NAMES rcc
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH
    )

  IF (QT_MOC_EXECUTABLE)
    set(QT_WRAP_CPP "YES")
  endif()

  IF (QT_UIC_EXECUTABLE)
    set(QT_WRAP_UI "YES")
  endif()



  MARK_AS_ADVANCED( QT_UIC_EXECUTABLE QT_UIC3_EXECUTABLE QT_MOC_EXECUTABLE QT_RCC_EXECUTABLE )

  ######################################
  #
  #       Macros for building Qt files
  #
  ######################################

  MACRO (QT4_GET_MOC_INC_DIRS _moc_INC_DIRS)
    set(${_moc_INC_DIRS})
    GET_DIRECTORY_PROPERTY(_inc_DIRS INCLUDE_DIRECTORIES)

    FOREACH(_current ${_inc_DIRS})
      set(${_moc_INC_DIRS} ${${_moc_INC_DIRS}} "-I" ${_current})
    ENDFOREACH(_current ${_inc_DIRS})
  ENDMACRO(QT4_GET_MOC_INC_DIRS)


  MACRO (QT4_GENERATE_MOC infile outfile )
    # get include dirs
    QT4_GET_MOC_INC_DIRS(moc_includes)

    GET_FILENAME_COMPONENT(infile ${infile} ABSOLUTE)

    ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
      COMMAND ${QT_MOC_EXECUTABLE}
      ARGS ${moc_includes} -o ${outfile} ${infile}
      DEPENDS ${infile})

    MACRO_ADD_FILE_DEPENDENCIES(${infile} ${outfile})
  ENDMACRO (QT4_GENERATE_MOC)


  # QT4_WRAP_CPP(outfiles inputfile ... )
  # TODO  perhaps add support for -D, -U and other minor options

  MACRO (QT4_WRAP_CPP outfiles )
    # get include dirs
    QT4_GET_MOC_INC_DIRS(moc_includes)

    FOREACH (it ${ARGN})
      GET_FILENAME_COMPONENT(it ${it} ABSOLUTE)
      GET_FILENAME_COMPONENT(outfile ${it} NAME_WE)

      set(outfile ${CMAKE_CURRENT_BINARY_DIR}/moc_${outfile}.cxx)
      ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
        COMMAND ${QT_MOC_EXECUTABLE}
        ARGS ${moc_includes} -o ${outfile} ${it}
        DEPENDS ${it})
      set(${outfiles} ${${outfiles}} ${outfile})
    ENDFOREACH(it)

  ENDMACRO (QT4_WRAP_CPP)


  # QT4_WRAP_UI(outfiles inputfile ... )

  MACRO (QT4_WRAP_UI outfiles )

    FOREACH (it ${ARGN})
      GET_FILENAME_COMPONENT(outfile ${it} NAME_WE)
      GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
      set(outfile ${CMAKE_CURRENT_BINARY_DIR}/ui_${outfile}.h)
      ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
        COMMAND ${QT_UIC_EXECUTABLE}
        ARGS -o ${outfile} ${infile}
        MAIN_DEPENDENCY ${infile})
      set(${outfiles} ${${outfiles}} ${outfile})
    ENDFOREACH (it)

  ENDMACRO (QT4_WRAP_UI)


  # QT4_ADD_RESOURCE(outfiles inputfile ... )
  # TODO  perhaps consider adding support for compression and root options to rcc

  MACRO (QT4_ADD_RESOURCES outfiles )

    FOREACH (it ${ARGN})
      GET_FILENAME_COMPONENT(outfilename ${it} NAME_WE)
      GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
      set(outfile ${CMAKE_CURRENT_BINARY_DIR}/qrc_${outfilename}.cxx)
      ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
        COMMAND ${QT_RCC_EXECUTABLE}
        ARGS -o ${outfile} ${infile}
        MAIN_DEPENDENCY ${infile} )
      set(${outfiles} ${${outfiles}} ${outfile})
    ENDFOREACH (it)

  ENDMACRO (QT4_ADD_RESOURCES)


  MACRO(QT4_AUTOMOC)
    QT4_GET_MOC_INC_DIRS(_moc_INCS)

    set(_matching_FILES )
    FOREACH (_current_FILE ${ARGN})

      GET_FILENAME_COMPONENT(_abs_FILE ${_current_FILE} ABSOLUTE)
      # if "SKIP_AUTOMOC" is set to true, we will not handle this file here.
      # here. this is required to make bouic work correctly:
      # we need to add generated .cpp files to the sources (to compile them),
      # but we cannot let automoc handle them, as the .cpp files don't exist yet when
      # cmake is run for the very first time on them -> however the .cpp files might
      # exist at a later run. at that time we need to skip them, so that we don't add two
      # different rules for the same moc file
      GET_SOURCE_FILE_PROPERTY(_skip ${_abs_FILE} SKIP_AUTOMOC)

      IF ( NOT _skip AND EXISTS ${_abs_FILE} )

        FILE(READ ${_abs_FILE} _contents)

        GET_FILENAME_COMPONENT(_abs_PATH ${_abs_FILE} PATH)

        STRING(REGEX MATCHALL "#include +[^ ]+\\.moc[\">]" _match "${_contents}")
        IF(_match)
          FOREACH (_current_MOC_INC ${_match})
            STRING(REGEX MATCH "[^ <\"]+\\.moc" _current_MOC "${_current_MOC_INC}")

            GET_filename_component(_basename ${_current_MOC} NAME_WE)
            # set(_header ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.h)
            set(_header ${_abs_PATH}/${_basename}.h)
            set(_moc    ${CMAKE_CURRENT_BINARY_DIR}/${_current_MOC})
            ADD_CUSTOM_COMMAND(OUTPUT ${_moc}
              COMMAND ${QT_MOC_EXECUTABLE}
              ARGS ${_moc_INCS} ${_header} -o ${_moc}
              DEPENDS ${_header}
              )

            MACRO_ADD_FILE_DEPENDENCIES(${_abs_FILE} ${_moc})
          ENDFOREACH (_current_MOC_INC)
        endif()
      endif()
    ENDFOREACH (_current_FILE)
  ENDMACRO(QT4_AUTOMOC)



  ######################################
  #
  #       decide if Qt got found
  #
  ######################################

  # if the includes,libraries,moc,uic and rcc are found then we have it
  IF( QT_LIBRARY_DIR AND QT_INCLUDE_DIR AND QT_MOC_EXECUTABLE AND QT_UIC_EXECUTABLE AND QT_RCC_EXECUTABLE)
    set( QT4_FOUND "YES" )
    IF( NOT Qt4_FIND_QUIETLY)
      message(STATUS "Found Qt-Version ${QTVERSION}")
    endif()
  else()
    set( QT4_FOUND "NO")
    IF( Qt4_FIND_REQUIRED)
      message( FATAL_ERROR "Qt libraries, includes, moc, uic or/and rcc NOT found!")
    endif()
  endif()
  set(QT_FOUND ${QT4_FOUND})
  
  #######################################
  #
  #       Qt configuration
  #
  #######################################
  IF(EXISTS "${QT_MKSPECS_DIR}/qconfig.pri")
    FILE(READ ${QT_MKSPECS_DIR}/qconfig.pri _qconfig_FILE_contents)
    STRING(REGEX MATCH "QT_CONFIG[^\n]+" QT_QCONFIG ${_qconfig_FILE_contents})
    STRING(REGEX MATCH "CONFIG[^\n]+" QT_CONFIG ${_qconfig_FILE_contents})
  endif()


  ###############################################
  #
  #       configuration/system dependent settings  
  #
  ###############################################

  set(QT_GUI_LIB_DEPENDENCIES "")
  set(QT_CORE_LIB_DEPENDENCIES "")
  
  # shared build needs -DQT_SHARED
  IF(NOT QT_CONFIG MATCHES "static")
    # warning currently only qconfig.pri on Windows potentially contains "static"
    # so QT_SHARED might not get defined properly on Mac/X11 (which seems harmless right now)
    # Trolltech said they'd consider exporting it for all platforms in future releases.
    set(QT_DEFINITIONS ${QT_DEFINITIONS} -DQT_SHARED)
  endif()
  
  ## system png
  IF(QT_QCONFIG MATCHES "system-png")
    FIND_LIBRARY(QT_PNG_LIBRARY NAMES png)
    set(QT_GUI_LIB_DEPENDENCIES ${QT_GUI_LIB_DEPENDENCIES} ${QT_PNG_LIBRARY})
    MARK_AS_ADVANCED(QT_PNG_LIBRARY)
  endif()
  
  # for X11, get X11 library directory
  IF(Q_WS_X11)
    QT_QUERY_QMAKE(QMAKE_LIBDIR_X11 "QMAKE_LIBDIR_X11")
  endif()

  ## X11 SM
  IF(QT_QCONFIG MATCHES "x11sm")
    # ask qmake where the x11 libs are
    FIND_LIBRARY(QT_X11_SM_LIBRARY NAMES SM PATHS ${QMAKE_LIBDIR_X11})
    FIND_LIBRARY(QT_X11_ICE_LIBRARY NAMES ICE PATHS ${QMAKE_LIBDIR_X11})
    set(QT_GUI_LIB_DEPENDENCIES ${QT_GUI_LIB_DEPENDENCIES} ${QT_X11_SM_LIBRARY} ${QT_X11_ICE_LIBRARY})
    MARK_AS_ADVANCED(QT_X11_SM_LIBRARY)
    MARK_AS_ADVANCED(QT_X11_ICE_LIBRARY)
  endif()
  
  ## Xi
  IF(QT_QCONFIG MATCHES "tablet")
    FIND_LIBRARY(QT_XI_LIBRARY NAMES Xi PATHS ${QMAKE_LIBDIR_X11})
    set(QT_GUI_LIB_DEPENDENCIES ${QT_GUI_LIB_DEPENDENCIES} ${QT_XI_LIBRARY})
    MARK_AS_ADVANCED(QT_XI_LIBRARY)
  endif()

  ## Xrender
  IF(QT_QCONFIG MATCHES "xrender")
    FIND_LIBRARY(QT_XRENDER_LIBRARY NAMES Xrender PATHS ${QMAKE_LIBDIR_X11})
    set(QT_GUI_LIB_DEPENDENCIES ${QT_GUI_LIB_DEPENDENCIES} ${QT_XRENDER_LIBRARY})
    MARK_AS_ADVANCED(QT_XRENDER_LIBRARY)
  endif()
  
  ## Xrandr
  IF(QT_QCONFIG MATCHES "xrandr")
    FIND_LIBRARY(QT_XRANDR_LIBRARY NAMES Xrandr PATHS ${QMAKE_LIBDIR_X11})
    set(QT_GUI_LIB_DEPENDENCIES ${QT_GUI_LIB_DEPENDENCIES} ${QT_XRANDR_LIBRARY})
    MARK_AS_ADVANCED(QT_XRANDR_LIBRARY)
  endif()
  
  ## Xcursor
  IF(QT_QCONFIG MATCHES "xcursor")
    FIND_LIBRARY(QT_XCURSOR_LIBRARY NAMES Xcursor PATHS ${QMAKE_LIBDIR_X11})
    set(QT_GUI_LIB_DEPENDENCIES ${QT_GUI_LIB_DEPENDENCIES} ${QT_XCURSOR_LIBRARY})
    MARK_AS_ADVANCED(QT_XCURSOR_LIBRARY)
  endif()
  
  ## Xinerama
  IF(QT_QCONFIG MATCHES "xinerama")
    FIND_LIBRARY(QT_XINERAMA_LIBRARY NAMES Xinerama PATHS ${QMAKE_LIBDIR_X11})
    set(QT_GUI_LIB_DEPENDENCIES ${QT_GUI_LIB_DEPENDENCIES} ${QT_XINERAMA_LIBRARY})
    MARK_AS_ADVANCED(QT_XINERAMA_LIBRARY)
  endif()
  
  ## system-freetype
  IF(QT_QCONFIG MATCHES "system-freetype")
    FIND_LIBRARY(QT_FREETYPE_LIBRARY NAMES freetype)
    set(QT_GUI_LIB_DEPENDENCIES ${QT_GUI_LIB_DEPENDENCIES} ${QT_FREETYPE_LIBRARY})
    MARK_AS_ADVANCED(QT_FREETYPE_LIBRARY)
  endif()
  
  ## fontconfig
  IF(QT_QCONFIG MATCHES "fontconfig")
    FIND_LIBRARY(QT_FONTCONFIG_LIBRARY NAMES fontconfig)
    set(QT_GUI_LIB_DEPENDENCIES ${QT_GUI_LIB_DEPENDENCIES} ${QT_FONTCONFIG_LIBRARY})
    MARK_AS_ADVANCED(QT_FONTCONFIG_LIBRARY)
  endif()
  
  ## system-zlib
  IF(QT_QCONFIG MATCHES "system-zlib")
    FIND_LIBRARY(QT_ZLIB_LIBRARY NAMES z)
    set(QT_CORE_LIB_DEPENDENCIES ${QT_CORE_LIB_DEPENDENCIES} ${QT_ZLIB_LIBRARY})
    MARK_AS_ADVANCED(QT_ZLIB_LIBRARY)
  endif()

  IF(Q_WS_X11)
    # X11 libraries Qt absolutely depends on
    QT_QUERY_QMAKE(QT_LIBS_X11 "QMAKE_LIBS_X11")
    SEPARATE_ARGUMENTS(QT_LIBS_X11)
    FOREACH(QT_X11_LIB ${QT_LIBS_X11})
      STRING(REGEX REPLACE "-l" "" QT_X11_LIB "${QT_X11_LIB}")
      set(QT_TMP_STR "QT_X11_${QT_X11_LIB}_LIBRARY")
      FIND_LIBRARY(${QT_TMP_STR} NAMES "${QT_X11_LIB}" PATHS ${QMAKE_LIBDIR_X11})
      set(QT_GUI_LIB_DEPENDENCIES ${QT_GUI_LIB_DEPENDENCIES} ${${QT_TMP_STR}})
    ENDFOREACH(QT_X11_LIB)

    QT_QUERY_QMAKE(QT_LIBS_THREAD "QMAKE_LIBS_THREAD")
    set(QT_CORE_LIB_DEPENDENCIES ${QT_CORE_LIB_DEPENDENCIES} ${QT_LIBS_THREAD})
    
    QT_QUERY_QMAKE(QMAKE_LIBS_DYNLOAD "QMAKE_LIBS_DYNLOAD")
    set(QT_CORE_LIB_DEPENDENCIES ${QT_CORE_LIB_DEPENDENCIES} ${QMAKE_LIBS_DYNLOAD})

  endif()
  
  IF(Q_WS_WIN)
    set(QT_GUI_LIB_DEPENDENCIES ${QT_GUI_LIB_DEPENDENCIES} Imm32 Winmm)
    set(QT_CORE_LIB_DEPENDENCIES ${QT_CORE_LIB_DEPENDENCIES} Ws2_32)
  endif()
  
  IF(Q_WS_MAC)
    set(QT_GUI_LIB_DEPENDENCIES ${QT_GUI_LIB_DEPENDENCIES} "-framework Carbon" "-framework QuickTime")
    set(QT_CORE_LIB_DEPENDENCIES ${QT_CORE_LIB_DEPENDENCIES} "-framework ApplicationServices")
  endif()

  #######################################
  #
  #       compatibility settings 
  #
  #######################################
  # Backwards compatibility for CMake1.4 and 1.2
  set(QT_MOC_EXE ${QT_MOC_EXECUTABLE} )
  set(QT_UIC_EXE ${QT_UIC_EXECUTABLE} )

  set( QT_QT_LIBRARY "")

else()

  IF(Qt4_FIND_REQUIRED)
    IF(QT4_INSTALLED_VERSION_TOO_OLD)
      message(FATAL_ERROR "The installed Qt version ${QTVERSION} is too old, at least version ${QT_MIN_VERSION} is required")
    else()
      message( FATAL_ERROR "Qt qmake not found!")
    endif()
  else()
    IF(QT4_INSTALLED_VERSION_TOO_OLD AND NOT Qt4_FIND_QUIETLY)
      message(STATUS "The installed Qt version ${QTVERSION} is too old, at least version ${QT_MIN_VERSION} is required")
    endif()
  endif()
  
endif()

