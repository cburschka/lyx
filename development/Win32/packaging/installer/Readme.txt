To build the installer do the following:

1. extract the source zip-file to e.g. the path "C:\LyX"
2. open the file settings.nsh with a text editor
   and adapt there the following paths to the one on your PC:
 !define FILES_LYX "C:\LyX\LyXPackage\LyX"
 !define FILES_DEPS "C:\LyX\LyX2.0x\lyx-windows-deps-msvc2008"
 !define FILES_QT "C:\LyX\LyXPackage\LyX"
3. install NSIS (http://nsis.sourceforge.net/Download)
4. open the file FindProc.zip (that is part of this bundle),
   extract from it the file FindProc.dll to the Plugins folder of
   NSIS's installation folder
5. open the file InetLoad.zip (that is part of this bundle),
   extract from it the file InetLoad.dll to the Plugins folder of
   NSIS's installation folder
6. right-click on the file lyx-standard.nsi and choose "Compile NSIS script"
   to compile the standard installer
7. right-click on the file lyx-bundle.nsi and choose "Compile NSIS script"
   to compile the bundle installer