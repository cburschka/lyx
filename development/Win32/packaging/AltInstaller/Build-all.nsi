# this script compiles the three different installer variants at once
# this is necessary when the installer should e.g. be built by CMake

SetCompressor /SOLID lzma

#--------------------------------
# The following commands use makensis to compile the three different installer variants.

#--------------------------------
#Compile the Update Installer

!system '"${NSISDIR}\makensis.exe" "LyXInstaller-Update.nsi"'

#--------------------------------
#Compile the Small Installer

!system '"${NSISDIR}\makensis.exe" "LyXInstaller-Small.nsi"'

#--------------------------------
#Compile the Complete Installer

!system '"${NSISDIR}\makensis.exe" "LyXInstaller-Complete.nsi"'

