To compile using MicroSoft Visual Studio 2003.NET, do this:

1) Checkout Qt/Free from CVS next to lyx-devel and compile.
   See http://kde-cygwin.sourceforge.net/qt3-win32/compile-net.php
   for more info. If you get a linking error about qt-mt3.lib, then
   copy lib/qt-mt333.lib to qt-mt3.lib and rerun configure.

2) Make sure QTDIR is set in your global environment, and QTDIR\bin
   is in your path. Control panel, system, advanced, environment
   variables in your friend on Windows XP.

3) Apply the win32_kludge.diff using GNU patch. Download from
   http://gnuwin32.sourceforge.net/packages/patch.htm
   and type "patch -p0 < development\win32\win32_kludge.diff"
   in cmd box with current working directory lyx-devel. 
   Make sure that patch is in your path. (c:\program files\gnuwin32\bin)

4) Open the development\win32\lyx.sln file in Visual Studio,
   compile and run. You can probably also use the free version
   of Microsoft's compiler, but I haven't tried that.

Development:

To get a stack-trace for assertions in LyX, make a breakpoint
in src/Boost.C.
To see the output from LyX, add "2> c:\errors.txt" to the command
line in  Project, Build properties, Debugging, Command arguments


To run LyX, you need a bunch of generated files from configure.

Run "fake.bat" to copy chkconfig.ltx, LaTeXConfig.lyx and lyxrc.defaults
from this directory to their destination.

At start-up, you might get an assertion about a unix path in
boost::filesystem. You can safely ignore that.
