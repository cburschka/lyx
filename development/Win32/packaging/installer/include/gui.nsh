/*

gui.nsh

Installer user interface settings

*/

#--------------------------------
# General

Name "${APP_NAME} ${APP_VERSION}"
BrandingText " "

# Default installation folder
InstallDir "${SETUP_DEFAULT_DIRECTORY}"

#--------------------------------
# Interface settings

!define MUI_ABORTWARNING
!define MUI_ICON "${SETUP_ICON}"
!define MUI_UNICON "${SETUP_ICON}"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${SETUP_HEADERIMAGE}"
!define MUI_HEADERIMAGE_RIGHT
!define MUI_WELCOMEFINISHPAGE_BITMAP "${SETUP_WIZARDIMAGE}"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "${SETUP_WIZARDIMAGE}"
!define MUI_CUSTOMFUNCTION_GUIINIT InitInterface
!define MUI_COMPONENTSPAGE_NODESC

#--------------------------------
# Pages

# Installer

!define MUI_WELCOMEPAGE_TITLE_3LINES
!define MUI_WELCOMEPAGE_TEXT $(TEXT_WELCOME_${SETUPTYPE_NAME})
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${FILES_LICENSE}"
!insertmacro MULTIUSER_PAGE_INSTALLMODE

Page custom PageReinstall PageReinstallValidate
Page custom PageExternalLaTeX PageExternalLaTeXValidate
Page custom PageExternalImageMagick PageExternalImageMagickValidate
Page custom PageExternalGhostscript PageExternalGhostscriptValidate
Page custom PageLanguage PageLanguageValidate

!define MUI_PAGE_HEADER_TEXT $(TEXT_DICT_TITLE)
!define MUI_PAGE_HEADER_SUBTEXT $(TEXT_DICT_SUBTITLE)
!define MUI_COMPONENTSPAGE_TEXT_TOP $(TEXT_DICT_TOP)
!define MUI_COMPONENTSPAGE_TEXT_COMPLIST $(TEXT_DICT_LIST)
!insertmacro MUI_PAGE_COMPONENTS # For spell checker dictionaries

!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN "$INSTDIR\${APP_RUN}"
!define MUI_FINISHPAGE_SHOWREADME
!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION CreateDesktopShortcut
!define MUI_FINISHPAGE_SHOWREADME_TEXT $(TEXT_FINISH_DESKTOP)
!define MUI_FINISHPAGE_LINK $(TEXT_FINISH_WEBSITE)
!define MUI_FINISHPAGE_LINK_LOCATION "http://www.lyx.org/"
!define MUI_PAGE_CUSTOMFUNCTION_SHOW CheckDesktopShortcut
!insertmacro MUI_PAGE_FINISH

# Uninstaller

!define MUI_WELCOMEPAGE_TITLE_3LINES
!define MUI_WELCOMEPAGE_TEXT $(UNTEXT_WELCOME)
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_COMPONENTS
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_UNPAGE_FINISH

#--------------------------------
# Installer Languages

!macro LANG LANG_NAME
  # NSIS language file
  !insertmacro MUI_LANGUAGE "${LANG_NAME}"
  # LyX language file
  !insertmacro LANGFILE_INCLUDE_WITHDEFAULT "lang\${LANG_NAME}.nsh" "lang\English.nsh"
!macroend

!insertmacro LANG "english"
!insertmacro LANG "french"
!insertmacro LANG "german"
!insertmacro LANG "italian"

#--------------------------------
# Version information

VIProductVersion "${APP_VERSION_NUMBER}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${APP_NAME}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "${APP_INFO}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${APP_VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "${APP_COPYRIGHT}"

Function InitInterface

  # Warn the user when no Administrator or Power user privileges are available
  # These privileges are required to install ImageMagick or Ghostscript

  ${If} $MultiUser.Privileges != "Admin"
  ${andif} $MultiUser.Privileges != "Power"
    MessageBox MB_OK|MB_ICONEXCLAMATION $(TEXT_NO_PRIVILEDGES)
  ${EndIf}

FunctionEnd
