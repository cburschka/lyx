
# CPack packaging
#
# http://cmake.org/cmake/help/cmake-2-8-docs.html#module:CPack

if(LYX_BUNDLE)
	if(APPLE)
		set(CPACK_GENERATOR DragNDrop)
		set(CPACK_DMG_BACKGROUND_IMAGE "${TOP_CMAKE_PATH}/../MacOSX/dmg-background.png")
		set(CPACK_DMG_DS_STORE "${CMAKE_BINARY_DIR}/ds_store/.DS_Store")
	elseif(UNIX)
		set(CPACK_GENERATOR STGZ)
	elseif(WIN32)
		set(CPACK_GENERATOR ZIP)
	endif()
endif()

# Overwrite package name dependent on platform
# (Ignores the top project)
if(APPLE)
	# true on all systems whre __APPLE__ is defined in header files
	set(CPACK_PACKAGE_NAME "LyX${LYX_MAJOR_VERSION}${LYX_MINOR_VERSION}")
elseif(UNIX)
	# True also for cygwin
	set(CPACK_PACKAGE_NAME "lyx${LYX_MAJOR_VERSION}${LYX_MINOR_VERSION}")
else()
	set(CPACK_PACKAGE_NAME "LyX${LYX_MAJOR_VERSION}${LYX_MINOR_VERSION}")
endif()

set(CPACK_PACKAGE_VERSION_MAJOR "${LYX_MAJOR_VERSION}")
set(CPACK_PACKAGE_VERSION_MINOR "${LYX_MINOR_VERSION}")
FILE(STRINGS "${TOP_CMAKE_PATH}/LyX_summary.txt" CPACK_PACKAGE_DESCRIPTION_SUMMARY)


set(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${LYX_INSTALL_SUFFIX}")

if (APPLE)
	# We don't need absolute paths
	set(CPACK_SET_DESTDIR "OFF")
elseif(WIN32)
    set(CPACK_GENERATOR ZIP)
    set(CPACK_BINARY_ZIP 1)
    if(MINGW)
        set(runtime)
        if(LYX_XMINGW)
            execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-libgcc-file-name OUTPUT_VARIABLE GCC_LIBGCC OUTPUT_STRIP_TRAILING_WHITESPACE)
            list(APPEND runtime /usr/${LYX_XMINGW}/lib/libwinpthread-1.dll)
        else()
            set(GCC_LIBGCC ${CMAKE_CXX_COMPILER})
        endif()
        get_filename_component(MINGW_RUNTIME_PATH ${GCC_LIBGCC} PATH)        
        macro(add_runtime_dll _DLL)
            file(GLOB GCC_RUNTIME ${MINGW_RUNTIME_PATH}/${_DLL})
            list(APPEND runtime ${GCC_RUNTIME})
        endmacro()
        add_runtime_dll(libgcc*.dll)
        add_runtime_dll(libstd*.dll)
        add_runtime_dll(libwin*.dll)
        if(NOT runtime)
            message(FATAL_ERROR "No mingw runtime found in ${MINGW_RUNTIME_PATH}")
        endif()

        if(LYX_USE_QT MATCHES "QT5")
            get_target_property(qmakebin Qt5::qmake IMPORTED_LOCATION)
            get_filename_component(QT_BINARY_DIR ${qmakebin} PATH)
            install(FILES
                ${runtime}
                ${QT_BINARY_DIR}/Qt5Core.dll
                ${QT_BINARY_DIR}/Qt5Network.dll
                ${QT_BINARY_DIR}/Qt5Gui.dll
                ${QT_BINARY_DIR}/Qt5Widgets.dll
                ${QT_BINARY_DIR}/Qt5Concurrent.dll
                ${QT_BINARY_DIR}/Qt5OpenGL.dll
                ${QT_BINARY_DIR}/Qt5PrintSupport.dll
                ${QT_BINARY_DIR}/Qt5Svg.dll
                ${QT_BINARY_DIR}/Qt5WinExtras.dll
                DESTINATION bin
                CONFIGURATIONS Release)
            install(FILES
                ${QT_BINARY_DIR}/../plugins/platforms/qminimal.dll
                ${QT_BINARY_DIR}/../plugins/platforms/qwindows.dll
                DESTINATION bin/platforms
                CONFIGURATIONS Release)
            install(FILES
                ${QT_BINARY_DIR}/../plugins/printsupport/windowsprintersupport.dll
                DESTINATION bin/printsupport
                CONFIGURATIONS Release)
            install(FILES
                ${QT_BINARY_DIR}/../plugins/imageformats/qgif.dll
                ${QT_BINARY_DIR}/../plugins/imageformats/qjpeg.dll
                ${QT_BINARY_DIR}/../plugins/imageformats/qsvg.dll
                ${QT_BINARY_DIR}/../plugins/imageformats/qico.dll
                DESTINATION bin/imageformats
                CONFIGURATIONS Release)
            install(FILES
                ${QT_BINARY_DIR}/../plugins/iconengines/qsvgicon.dll
                DESTINATION bin/iconengines
                CONFIGURATIONS Release)
        else()
            install(FILES
                ${runtime}
                ${QT_BINARY_DIR}/QtCore4.dll
                ${QT_BINARY_DIR}/QtGui4.dll
                ${QT_BINARY_DIR}/QtNetwork4.dll
                ${QT_PLUGINS_DIR}/imageformats/qgif4.dll
                ${QT_PLUGINS_DIR}/imageformats/qico4.dll
                ${QT_PLUGINS_DIR}/imageformats/qmng4.dll
                ${QT_PLUGINS_DIR}/imageformats/qsvg4.dll
                ${QT_PLUGINS_DIR}/imageformats/qtga4.dll
                ${QT_PLUGINS_DIR}/imageformats/qtiff4.dll
                DESTINATION bin CONFIGURATIONS Release)
        endif()
    endif()
else()
	# needed by rpm
	set(CPACK_SET_DESTDIR "ON")
endif()

FILE(READ "${TOP_CMAKE_PATH}/LyX_license.txt" CPACK_RPM_PACKAGE_LICENSE)
set(CPACK_RPM_PACKAGE_GROUP "Applications/Publishing")
set(CPACK_RPM_PACKAGE_VENDOR "The LyX team")
#
# the next ones are needed by deb
set(CPACK_PACKAGE_CONTACT "${PACKAGE_BUGREPORT}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${LYX_PROJECT}-${LYX_INSTALL_SUFFIX}")

# We depend on python scripting
set(CPACK_DEBIAN_PACKAGE_DEPENDS "python (>= 2.6)")

# use dpkg-shlibdeps to generate additional info for package dependency list.
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_DEBUG ON)

#
# for the next ones, cpack insists on data with values in some file
set(CPACK_PACKAGE_DESCRIPTION_FILE "${TOP_CMAKE_PATH}/LyX_description.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")

# Use the revision number saved in ${LYX_PACKAGE_RELEASE}
# as the release in rpm-package-build.
# This way we maycan omit the otherwise needed "--force" parameter when
# installing from that rpm package.
set(CPACK_RPM_PACKAGE_RELEASE ${LYX_PACKAGE_RELEASE})
set(CPACK_PACKAGE_VERSION_PATCH ${LYX_REVISION_VERSION})

# we do not have a readme or welcome data,
# so we do not provide infofiles for
# CPACK_RESOURCE_FILE_README and CPACK_RESOURCE_FILE_WELCOME

# sources
set(CPACK_SOURCE_INSTALLED_DIRECTORIES "${TOP_SRC_DIR};/") # http://www.mail-archive.com/cmake@cmake.org/msg33720.html


set(CPACK_STRIP_FILES 1)

# Finaly give some hints about files not to be included in source package
SET(CPACK_SOURCE_IGNORE_FILES
"/CVS/"
"/\\\\.svn/"
"/\\\\.bzr/"
"/\\\\.hg/"
"/\\\\.git/"
"/\\\\.project$"
"/\\\\.cproject$"
"/\\\\.history$"
"\\\\.#"
"\\\\.swp$"
"~$"
"/#"
"/_CPack_Packages/"
"/CMakeCache.txt$"
"/CPackSourceConfig\\\\.cmake$"
"/configCompiler\\\\.h$"
"/install_manifest\\\\.txt$"
"/cmake_install\\\\.cmake$"
"/POTFILES"
"\\\\.tar\\\\.gz$"
"\\\\.tar\\\\.bz2$"
"\\\\.tar\\\\.Z$"
"\\\\.deb$"
"\\\\.rpm$"
"\\\\.rej$"
"\\\\.orig$"
"/core$"
"/CMakeFiles/"
"/Makefile\\\\.in$"
"/Makefile$"
"/autom4te\\\\.cache/"
"/\\\\.deps/"
"/build/"
"/lyx-2\\\\."
)

if(LYX_CPACK)
	include(CPack)
endif()


#Now it is possible to create some packages
# cd <BuildDir>
# make package
#############################################################################################
# So, e.g. for variables
#    CMAKE_PROJECT_NAME                     : lyx
#    CPACK_PACKAGE_VERSION_MAJOR            : 2
#    CPACK_PACKAGE_VERSION_MINOR            : 0
#    CPACK_PACKAGE_VERSION_PATCH            : 1
#    CMAKE_SYSTEM_NAME                      : Linux
#    CPACK_BINARY_DEB:BOOL                  : ON
#
# the package name builds as "lyx-2.0.1-Linux.deb"
#
############################## rpm ################################### deb ##################
# create    # cpack -G RPM --config CPackConfig.cmake   # cpack -G DEB --config CPackConfig.cmake
# creates =># lyx-2.0.1-Linux.rpm                       # lyx-2.0.1-Linux.deb
# list      # rpm -qlp lyx-2.0.1-Linux.rpm              # dpkg-deb -c lyx-2.0.1-Linux.deb
# install   # rpm -U lyx-2.0.1-Linux.rpm                # dpkg -i lyx-2.0.1-Linux.deb
#

