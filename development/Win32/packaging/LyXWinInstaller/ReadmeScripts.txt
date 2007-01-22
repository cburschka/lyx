Description

This is the readme of the script source code.
The file LyXPackageScripts-x-x.rar contains the NSIS (Nullsoft Install System) scripts and
used plugins.

---
General
To compile the installer scripts you need at least NSIS version 2.22.
The installer scripts need the plugins "FindProc" and "InetLoad". To use it copy the files "FindProcDLL.dll" and "InetLoad.dll" from the "LyXPackage" folder to NSIS'"Plugins" directory.
Also copy the file "Galician.nlf" in NSIS' install folder to
~\Contrib\Language files
and the file "Galician.nsh" to
~\Contrib\Modern UI\Language files

---
Internationalization

The subfolder "lyx_languages"
contains the language string files. If you want to help the project then
translate them or create a new language string file based on the file
"english.nsh". Please send the modified files as patch of the category
"Language" to
http://developer.berlios.de/patch/?group_id=5117
or to
uwestoehr@web.de.

---
License

The scripts are released under the GNU General Public License (GPL),
see the license file.
