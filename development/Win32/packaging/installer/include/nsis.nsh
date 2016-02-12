/*

nsis.nsh

Configuration of standard NSIS header files

*/

#--------------------------------
# Multi-User settings

!define MULTIUSER_EXECUTIONLEVEL Highest
!define MULTIUSER_INSTALLMODE_COMMANDLINE
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_KEY "${APP_REGKEY}"
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_VALUENAME ""

!define MULTIUSER_INSTALLMODE_INSTDIR "${APP_DIR}"
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_KEY "${APP_REGKEY}"
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_VALUENAME ""

!define MULTIUSER_INSTALLMODE_FUNCTION InitUser
!define MULTIUSER_MUI

#--------------------------------
# Standard header files

!include MUI2.nsh
!include LogicLib.nsh
!include Sections.nsh
!include WinVer.nsh
!include LangFile.nsh
!include MultiUser.nsh
!include InstallOptions.nsh
!include x64.nsh

# Set of various macros and functions
!include include\LyXUtils.nsh

# Functions to check and configure the LaTeX-system
!include include\LaTeX.nsh

!if ${SETUPTYPE} != BUNDLE
 # Function for page to manually select LaTeX's installation folder
 !include gui\LaTeXFolder.nsh
!endif # end if != BUNDLE

# Functions to download spell-checker and thesaurus dictionaries
!include include\dictionaries.nsh
