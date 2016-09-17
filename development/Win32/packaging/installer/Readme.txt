To build the installer do the following:

1. extract the source zip-file to e.g. the path "C:\LyX-Installer"
2. open the file settings.nsh with a text editor
   and adapt there the following paths to the one on your PC, e.g.:
 !define FILES_LYX "C:\LyX-Installer\LyXPackage\LyX"
 !define FILES_DEPS "C:\LyX-Installer\LyX2.2x\lyx-windows-deps-msvc2015"
 !define FILES_QT "C:\LyX-Installer\LyXPackage\LyX"
3. install the latest version 3.x of NSIS (http://nsis.sourceforge.net/Download)
4. install the NSIS Large Strings extension (http://nsis.sourceforge.net/Special_Builds)
   (extract the extension ZIP file in your NSIS installation folder and this way overwrite some files) 
5. open the file FindProcDLL.zip (that is part of this bundle),
   extract from it the file FindProc.dll to the folder \Plugins\x86-ansi of
   NSIS's installation folder
6. open the file InetLoad.zip (that is part of this bundle),
   extract from it the file InetLoad.dll to the folder \Plugins\x86-ansi of
   NSIS's installation folder
7. right-click on the file lyx-standard.nsi and choose "Compile NSIS script"
   to compile the standard installer
8. right-click on the file lyx-bundle.nsi and choose "Compile NSIS script"
   to compile the bundle installer