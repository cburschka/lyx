Building LyX with CMake

For all builds:
- CMake 2.4 or CVS version from www.cmake.org
- install Qt 4 and make sure qmake 4 is found
  (add the folder with qmake to the environment variable PATH)
- by default it builds the Qt4 frontend
- with GNUWIN32_DIR you could point to your gnuwin32 packages
  (eg. -DGNUWIN32_DIR=c:\gnuwin32) by default it searches in your 
  program  folder

Building Visual C++ 2005 project files:
- install Visual C++ 2005
- install Platform SDK 2005, "Core" and "Web Workshop"
- add include and library paths of the SDK to the IDE search paths,
  menu: Tools->Options->VC++ directories->Library files + Include files
- install zlib (www.zlib.net) into %ProgramFiles%/GnuWin32/include+lib
  or %ProgramFiles%/zlib/include+lib
- create a build directory, e.g. .../trunk/../build
- call in the build directory 'cmake ..\trunk\development\cmake'
- start lyx.sln
- Warnings:
	The default warning level of the msvc cmake builds is now /W4.
	The cmake option 
		-DDISABLEWALL=1 
	switches to /W3, 
		-DWALL=1 
	re enables /W4.
	To disable a specific warning add it to MSVC_W_DISABLE of
	cmake/CMakeLists.txt. To make the warning an error add it
	to MSVC_W_ERROR of the same file.

TIPS: - rename Microsoft Visual Studio 8\VC\vcpackages\feacp.dll 
        to disable Intellisense
      - the Release build links much faster
      - for the 'Debug' and 'Release' build all precompiled headers are enabled
        to compile without pch (to check if all necessary headers are included)
          * use 'MinSizeRel' which only precompiles the STL and Boost headers
          * use 'RelWithDebInfo' which does not use any precompiled headers


Building with GCC/Linux:
- create a build directory, e.g. .../trunk/../build
- call in the build directory 'cmake ..\trunk\development\cmake'

Building with GCC/Windows (Win2k only works with MSYS, XP?):
- install zlib (www.zlib.net) into %ProgramFiles%/GnuWin32/include+lib
- create a build directory, e.g. .../trunk/../build
- call: export QMAKESPEC=win32-g++ (MSYS) or set QMAKESPEC=win32-g++ (CMD)
- call in the build directory 'cmake ..\trunk\development\cmake'

Building with Xcode/Mac:
- create a build directory, e.g. .../trunk/../build
- call in the build directory 'cmake .../trunk/development/cmake -G Xcode'
- open .../trunk/../build/lyx-qt4.xcodeproj

TIPS: - Xcode prefers UTF8 when opening source files, though LyX usually uses
        Latin1. To fix that select all source files in Xcode and click "Get Info"
        in the context menu. Change the encoding to Latin1.
      - You can run and debug LyX from Xcode. For LyX to find its resources, there
        are two possibilities:
	a) Put a resource directory, e.g. a link to the lib directory of the 
	   source tree, at .../trunk/../build/bin/Resources
	b) Select the lyx-qt4 executable in Xcode, click on "Get Info" in the 
	   context menu and add "-sysdir a_valid_LyX_resource_directory" 
	   pointing e.g. to a valid Contents/Resources of a LyX.app directory.
      - LyX on Mac doesn't look for fonts in the resource directory if the
        executable is not in an .app bundle. Instead you have to create a
        symbolic link to the fonts directory in the place where the executable
        is: ln -s .../trunk/lib/fonts .../trunk/../build/bin/Debug/
        If you don't do that math character will not show up correctly.
      - CMake properly finds the Qt4 library bundles from Trolltech's binary
        Qt4 package for Mac. So no need to compile Qt on your own.


To generate other build files call 'cmake'
which shows a list of possibilities.


The build process tries to find aspell on Windows
in %ProgramFiles%/GnuWin32/ and in /usr/ or in /usr/local 
under Linux. If it is not found the support is disabled.
