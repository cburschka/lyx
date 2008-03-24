=========================
Building LyX with CMake
=========================

July, 2007



All systems
===========

    * CMake 2.4 or CVS version from www.cmake.org
    * Install Qt 4 and make sure qmake 4 is found
      (add the folder with qmake to the environment variable PATH).
      
    * Faster build process: with file merging enabled compilation 
      is up to 5 times faster: '-Dmerge=1'.
      To force a complete regeneration of the created files use
      '-Dmerge_rebuild=1'.



Windows only
=============

    Install the windows supplementary modules
     * as described in INSTALL.scons
     * install win32libs with the 'KDE on Windows' installer
       http://download.cegit.de/kde-windows/installer/
       - use the msvc packages
       - a release version of Qt is also available by the installer
    
    If cmake couldn't find these modules set GNUWIN32_DIR, eg. 
    -DGNUWIN32_DIR=c:\gnuwin32. By default cmake searches in your 
    program folder


Building Visual C++ 2005 project files
--------------------------------------

    * install Visual C++ 2005
    * install Platform SDK 2005, "Core" and "Web Workshop"
    * add include and library paths of the SDK to the IDE search paths.
      Menu entry: Tools->Options->'VC++ directories'->'Library files' and 'Include files'
    * create a build directory, e.g. ..\trunk\..\build
    * call in the build directory 'cmake ..\trunk\development\cmake'
    * start lyx.sln
    
    * Warnings: The default warning level of the msvc cmake builds 
      is /W3. To enable /W4 use
        '-DWALL=1 '
      and 
        '-DDISABLEWALL=1'
      switches back to to /W3, 
      To disable a specific warning add it to MSVC_W_DISABLE in
      cmake/CMakeLists.txt. To make the warning an error add it
      to MSVC_W_ERROR.
      
    * Memory leak detection
      For MSVC the usage of 'Visual Leak Detection' could be enabled
      (http://dmoulding.googlepages.com/vld): -Dvld=1
      
      Building vld requires the 'Debugging Tools For Windows' (~16MB)
      http://www.microsoft.com/whdc/devtools/debugging/default.mspx
      Eventually you have to copy dbghelp.dll from your Visual Studio
      Common7\IDE folder into the binary directory. 
      'vld.ini' is part of lyx.proj where you can configure vld.
      
      The docu for vld could be found in development/Win32/vld, or
      http://www.codeproject.com/tools/visualleakdetector.asp
      
      
Some tips:

    * rename Microsoft Visual Studio 8\VC\vcpackages\feacp.dll 
      to disable Intellisense
    * the Release build links much faster 
    * for the 'Debug' and 'Release' build all precompiled headers are enabled
      to compile without pch (non file merge mode) This is usefull to check 
      if all necessary headers are included.
        * use 'MinSizeRel' which only precompiles the STL and Boost headers
        * use 'RelWithDebInfo' which does not use any precompiled headers



GCC/Windows (Win2k only works with MSYS, XP?)
----------------------------------------------

    * create a build directory, e.g. .../trunk/../build
    * call: export QMAKESPEC=win32-g++ (MSYS) or set QMAKESPEC=win32-g++ (CMD)
    * call in the build directory 'cmake ..\trunk\development\cmake'



Building with GCC/Linux
------------------------

    * create a build directory, e.g. .../trunk/../build
    * call in the build directory 'cmake ../trunk/development/cmake'
    * compiler and linker options could be suppressd by '-Dquiet=1' 



Building with Xcode/Mac
-----------------------

    * create a build directory, e.g. .../trunk/../build
    * call in the build directory 'cmake .../trunk/development/cmake -G Xcode'
    * open .../trunk/../build/lyx-qt4.xcodeproj


Some tips:

    * Xcode prefers UTF8 when opening source files, though LyX usually uses
      Latin1. To fix that select all source files in Xcode and click "Get Info"
      in the context menu. Change the encoding to Latin1.
    * You can run and debug LyX from Xcode. For LyX to find its resources, there
      are two possibilities:
        a) Put a resource directory, e.g. a link to the lib directory of the 
           source tree, at .../trunk/../build/bin/Resources
        b) Select the lyx-qt4 executable in Xcode, click on "Get Info" in the 
           context menu and add "-sysdir a_valid_LyX_resource_directory" 
           pointing e.g. to a valid Contents/Resources of a LyX.app directory.
    * LyX on Mac doesn't look for fonts in the resource directory if the
      executable is not in an .app bundle. Instead you have to create a
      symbolic link to the fonts directory in the place where the executable
      is: ln -s .../trunk/lib/fonts .../trunk/../build/bin/Debug/
      If you don't do that math character will not show up correctly.
    * CMake properly finds the Qt4 library bundles from Trolltech's binary
      Qt4 package for Mac. So no need to compile Qt on your own.




To generate other build files call 'cmake' 
which shows a list of possibilities.


The build process tries to find aspell on Windows
in %ProgramFiles%/GnuWin32/ and in /usr/ or in /usr/local 
under Linux. If it could not find aspell, spell checking
will be disabled.
