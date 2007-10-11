Description

This is the readme of the script source code.
The file LyXPackageScripts-x-x.rar contains the NSIS (Nullsoft Install System) scripts and
used plugins.
You can download the file for every release from:
http://developer.berlios.de/projects/lyxwininstall/

---
General

To compile the installer scripts you need at least NSIS version 2.30.

The installer scripts needs the plugins "FindProc" and "InetLoad".
To use them copy the files "FindProcDLL.dll" and "InetLoad.dll" from the
"LyXPackage" folder to NSIS'"Plugins" directory.

Only when you have built your own file tree:
- Copy the files "preferencesxxx", "Aspellxxx.txt", and "session" from the
  specials folder to the \Resources folder
- Copy the files "Console.dll" and "LyXLauncher.exe" from the specials folder
  to the \bin folder where also the "lyx.exe" is.
- Copy all "*.exe" files from the specials\PDFViewWin folder to the \bin
  folder where also the "lyx.exe" is.

---
Internationalization

The subfolder "lyx_languages"
contains the language string files. If you want to help the project then
translate them or create a new language string file based on the file
"english.nsh". Please send the modified files as patch of the category
"Language" to
http://developer.berlios.de/patch/?group_id=5117
or to
uwestoehr<at>web.de.

---
License

The scripts are released under the GNU General Public License (GPL),
see the license file.
