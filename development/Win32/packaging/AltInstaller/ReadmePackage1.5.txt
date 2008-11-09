Description

This is the readme of the complete installer package.
The file LyXPackageComplete-x-x.rar contains the NSIS (Nullsoft Install System) scripts and
all used program files. It is needed if you want to build the LyXWinInstaller by yourself.
You can download the file for every release from:
http://developer.berlios.de/projects/lyxwininstall/

---
General

To compile the installer scripts you need at least NSIS version 2.30.

The installer scripts needs the plugins "FindProc" and "InetLoad".
To use them, copy the files "FindProcDLL.dll" and "InetLoad.dll" from the
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
The licenses of all used programs can be found in the folder ~\LyX\bin of this package.

---
ImageMagick

License: ImageMagick license

The folder ~\LyX\etc\ImageMagick contains all used files from ImageMagick 6.4.5

---
Ghostscript

License: GPL

The folder ~\LyX\etc\Ghostscript contains all used files from GPL Ghostscript 8.63

---
Aspell

License: GPL

The files of Aspell 0.60-4 are in the folder ~\LyX\external\Aspell
Dictionaries will be downloaded at runtime, the different licenses of the dictionaries
will be displayed before they are installed.

---
Python

License: Python license

The following files from Python 2.6 are used:

python.exe and python26.dll are in the folder ~\LyX\bin.

The subfolders of ~\LyX\bin contain all other used python files.

---
MiKTeX

License: GPL

The small version of MiKTeX is included together with its installer. MiKTeX will be
installed and uninstalled using its own installer.

---
GSview

License: AFPL

GSview is included completely. It will be installed and uninstalled using its own
installer.

---
JabRef

License: GPL

GSview is included completely. It will be installed and uninstalled using its own
installer.
