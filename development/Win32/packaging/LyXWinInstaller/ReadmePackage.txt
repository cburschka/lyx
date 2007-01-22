Description

This is the readme of the complete installer package.
The file LyXPackageComplete-x-x.rar contains the NSIS (Nullsoft Install System) scripts and
all used program files. It is needed if you want to build the LyXWinInstaller by yourself.

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
The licenses of all used programs can be found in the folder ~\LyX\bin of this package.

---
ImageMagick

License: ImageMagick license

The folder ~\LyX\etc\ImageMagick contains all used files from ImageMagick 6.2.9

---
Ghostscript

License: GPL

The folder ~\LyX\etc\Ghostscript contains all used files from GPL Ghostscript 8.54

---
Aspell

License: GPL

The files of Aspell 0.60-4 are in the folder ~\LyX\external\Aspell
Dictionaries will be downloaded at runtime, the different licenses of the dictionaries
will be displayed before they are installed.

---
Python

License: Python license

The following files from Python 2.4 are used:

python.exe and python24.dll are in the folder ~\LyX\bin.

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
MSYS

License: GPL

The following files from MSYS are used:

cat.exe
cp.exe
cut.exe
expr.exe
find.exe
grep.exe
ln.exe
ls.exe
mkdir.exe
mv.exe
rm.exe
sed.exe
sh.exe
touch.exe
tr.exe
uname.exe

They are all in the folder ~\LyX\bin of this package.

The following used MSYS files are in the folder ~\LyX\etc:

config.site
fstab.sample
profile
termcap