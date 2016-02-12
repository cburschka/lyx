/*

NSIS Script - LyX 2.2 Installer for Windows
Authors: Joost Verburg, Angus Leeming, Uwe Stöhr
Compatible with NSIS 2.46

*/

# Do a Cyclic Redundancy Check to make sure the installer
# was not corrupted by the download.
CRCCheck force
#
# Make the installer as small as possible.
SetCompressor /SOLID lzma

!ifndef SETUPTYPE
 !define SETUPTYPE BUNDLE
!endif

# installer settings like version numbers
!include settings.nsh

# declarations of LyX's registry keys and installer variant
!include include\declarations.nsh

# definitions of global variables
!include include\variables.nsh

# configuration of standard NSIS header files
!include include\nsis.nsh

# list of files to be included in the installer
!include include\filelist.nsh

# detect third-party programs like Inkscape and LaTeX
!include include\detection.nsh

# set up the installer pages
!include include\gui.nsh

# sets the install sections and checks the system on starting the un/installer
!include include\init.nsh

# install LyX and needed third-party programs like Python etc.
!include setup\install.nsh

# loads a function to modify Windows environment variables 
!include include\EnvVarUpdate.nsh

# uninstall LyX and all programs that were installed together with LyX
!include setup\uninstall.nsh

# configure LyX (set start menu and write registry entries)
!include setup\configure.nsh

# provides downloads of external programs
#!include gui\external.nsh

#--------------------------------
# Output file

Outfile "${SETUP_EXE}"
