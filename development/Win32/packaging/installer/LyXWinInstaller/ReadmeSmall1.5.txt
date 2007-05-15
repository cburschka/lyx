Description

This is the readme of the variant "small" of the LyXWinInstaller. Please read
this file carefully to prevent troubles with the installer.
The LyXWinInstaller installs the wordprocessor "LyX" (www.lyx.org) and some
needed programs on Windows 2000/XP/XPx64/Vista.
! Win95, Win98, and WinME are NOT supported. !

---
General

The installer variant "small" has the same functionality as the "complete" version
but doesn't include the LaTeX-distribution MiKTeX. It checks your system for an
installed version of MiKTeX (www.miktex.org) or TeXLive (http://www.tug.org/texlive/).

The installer provides an option to to choose a LaTeX-distribution from a network
drive or a live-CD/DVD and an option to not use LyX with LaTeX.

---
Prerequisites

To use the installer you need administrator privileges.
An open internet connection is recommended as several LaTeX-packages and
spellchecker dictionaris can than be downloaded and installed automatically.

It is not essential but would be an advantage if you have the PDF-viewer
"Adobe Reader" installed before running the installer.

---
Installed Applications

The variant "small" of the LyXWinInstaller analyses your system and installs,
if needed, a fully functional version of:

- Aspell, a spellchecker
- Ghostscript, an interpreter for Postscript and PDF
- ImageMagick, an image converter
- (optional) GSview, a viewer for Postscript and PDF-documents

ImageMagick and Ghostscript won't appear in the list of the installed programs
in Window's system control because they are installed in a special version to 
use them with LyX. But if you decide to install another version of the programs
you can do this without problems and without uninstalling LyX.
If you want to install GSview manually later you can do this without a new
installation of Ghostscript.

---
After the Installation

To view/edit external files like images LyX uses the default program set in the registry
for a file extension. This installer checks for many programs so that you only need to type
the name of their executables in LyX's preferences to change this.
For example the default viewer for PNG-images on Windows is "Paint". To change it to Gimp,
set "gimp-2.2" as viewer/editor for the PNG file format in LyX's preferences.

---
Known Problems

You can't install LyX to a folder that has accents or umlauts in its name.

If you use LyX with a computer user account that has accents or umlauts in its name,
change LyX's paths in the menu Edit -> Preferences so that they don't contain accents
and restart LyX.

The live-variant of TeX-Live don't work together with LyX due to incompatible Ghostscript
and dv2dt/dt2dv versions.
TeX-Live will only work together with LyX when it is installed without Ghostscript.

---
The Uninstaller

LyXWinInstall's uninstaller uninstalls LyX. Aspell, Ghostscript, GSview, ImageMagick, JabRef,
and MiKTeX are only uninstalled when they were installed together with LyX. You can uninstall
Aspell, GSview, JabRef, and MiKTeX also separately via Window's Software menu in the system
control.

---
Internationalization

The installer language string files can be found in the subfolder
"lyx_languages" of the sourcecode package:
https://developer.berlios.de/project/showfiles.php?group_id=5117

If you want to help the project you can translate them or create a new language
string file based on the file "english.nsh".
Please send the modified files as patch to
http://developer.berlios.de/patch/?group_id=5117
or to
uwestoehr<at>web.de.

---
License

The scripts of the LyXWinInstaller are released under the GNU General Public
License (GPL). A package with the sourcecode can be found
in the section "LyxWinInstallerSource" of
http://developer.berlios.de/project/showfiles.php?group_id=5117

The license files of all used programs can be found in the subdirectory "bin"
of LyX's installation folder.
