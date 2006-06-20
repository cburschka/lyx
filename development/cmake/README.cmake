Building LyX with CMake

For all builds:
- CMake 2.4.2 or CVS version from www.cmake.org
- install Qt 4 and make sure qmake 4 is found
- by default it builds the Qt4 frontend
- to build the Qt3 frontend set QTDIR and call cmake with '-Dqt3=1'

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

TIPS: - rename Microsoft Visual Studio 8\VC\vcpackages\feacp.dll 
        to disable Intellisense
      - the Release build links much faster


Building with GCC/Linux:
- create a build directory, e.g. .../trunk/../build
- call in the build directory 'cmake ..\trunk\development\cmake'

Building with GCC/Windows (Win2k only works with MSYS, XP?):
- install zlib (www.zlib.net) into %ProgramFiles%/GnuWin32/include+lib
- create a build directory, e.g. .../trunk/../build
- call: export QMAKESPEC=win32-g++ (MSYS) or set QMAKESPEC=win32-g++ (CMD)
- call in the build directory 'cmake ..\trunk\development\cmake'


To generate other build files call 'cmake'
which shows a list of possibilities.


The build process tries to find aspell and iconv on Windows 
in %ProgramFiles%/GnuWin32/ and in /usr/ or in /usr/local 
under Linux. If they are not found the support is disabled.
