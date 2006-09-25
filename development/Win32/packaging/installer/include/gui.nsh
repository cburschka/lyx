/*

Installer User Interface

*/

;--------------------------------
;General

Name "${APP_NAME} ${APP_VERSION}"

;Default installation folder
InstallDir "${SETUP_DEFAULT_DIRECTORY}"

;--------------------------------
;Installer language

;Get from registry if available
!define MUI_LANGDLL_REGISTRY_ROOT SHELL_CONTEXT
!define MUI_LANGDLL_REGISTRY_KEY "${APP_REGKEY_SETUP}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "Setup Language"

;--------------------------------
;Interface settings

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

;--------------------------------
;Pages

;Installer

!define MUI_WELCOMEPAGE_TITLE_3LINES
!define MUI_WELCOMEPAGE_TEXT $(TEXT_WELCOME_${SETUPTYPE_NAME})
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${FILES_LICENSE}"

Page custom PageUser PageUserValidate
Page custom PageReinstall PageReinstallValidate
Page custom PageExternalLaTeX PageExternalLaTeXValidate
Page custom PageExternalImageMagick PageExternalImageMagickValidate
Page custom PageExternalGhostscript PageExternalGhostscriptValidate
Page custom PageViewer PageViewerValidate
Page custom PageLanguage PageLanguageValidate

!define MUI_PAGE_HEADER_TEXT $(TEXT_DICT_TITLE)
!define MUI_PAGE_HEADER_SUBTEXT $(TEXT_DICT_SUBTITLE)
!define MUI_COMPONENTSPAGE_TEXT_TOP $(TEXT_DICT_TOP)
!define MUI_COMPONENTSPAGE_TEXT_COMPLIST $(TEXT_DICT_LIST)
!insertmacro MUI_PAGE_COMPONENTS ;For spell checker dictionaries

!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN "$INSTDIR\${APP_RUN}"
!define MUI_FINISHPAGE_SHOWREADME
!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION CreateDesktopShortcut
!define MUI_FINISHPAGE_SHOWREADME_TEXT $(TEXT_FINISH_DESKTOP)
!define MUI_FINISHPAGE_LINK $(TEXT_FINISH_WEBSITE)
!define MUI_FINISHPAGE_LINK_LOCATION "http://www.lyx.org/"
!insertmacro MUI_PAGE_FINISH

;Uninstaller

!define MUI_WELCOMEPAGE_TITLE_3LINES
!define MUI_WELCOMEPAGE_TEXT $(UNTEXT_WELCOME)
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_COMPONENTS
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages

!insertmacro IncludeLang "english"
!insertmacro IncludeLang "french"
!insertmacro IncludeLang "german"
!insertmacro IncludeLang "italian"

;--------------------------------
;Macros

!macro InitDialogExternal component currentuser_possible

  !insertmacro MUI_INSTALLOPTIONS_WRITE "external_${component}.ini" "Field 1" "Text" $(TEXT_EXTERNAL_${component}_INFO_${SETUPTYPE_NAME})
  !insertmacro MUI_INSTALLOPTIONS_WRITE "external_${component}.ini" "Field 3" "Text" $(TEXT_EXTERNAL_${component}_FOLDER)
  !insertmacro MUI_INSTALLOPTIONS_WRITE "external_${component}.ini" "Field 5" "Text" $(TEXT_EXTERNAL_${component}_FOLDER_INFO)
  !insertmacro MUI_INSTALLOPTIONS_WRITE "external_${component}.ini" "Field 6" "Text" $(TEXT_EXTERNAL_${component}_NONE)
  
  !if ${currentuser_possible} == ${TRUE}
    !insertmacro MUI_INSTALLOPTIONS_WRITE "external_${component}.ini" "Field 2" "Text" "$(TEXT_EXTERNAL_${component}_${SETUPTYPE_NAME})"
  !else  
    ${if} $AdminOrPowerUser == ${TRUE}
      !insertmacro MUI_INSTALLOPTIONS_WRITE "external_${component}.ini" "Field 2" "Text" "$(TEXT_EXTERNAL_${component}_${SETUPTYPE_NAME})"
    ${else}
      !insertmacro MUI_INSTALLOPTIONS_WRITE "external_${component}.ini" "Field 2" "Text" "$(TEXT_EXTERNAL_${component}_${SETUPTYPE_NAME}) $(TEXT_EXTERNAL_NOPRIVILEDGES)"
      !insertmacro MUI_INSTALLOPTIONS_WRITE "external_${component}.ini" "Field 2" "Flags" "DISABLED"
    ${endif}
  !endif
  
!macroend

!macro InitDialogExternalDir component currentuser_possible

  !insertmacro MUI_INSTALLOPTIONS_WRITE "external_${component}.ini" "Field 4" "State" $Path${component}
  
  !if ${currentuser_possible} == ${FALSE}
    ${if} $AdminOrPowerUser == ${TRUE}
  !endif
  
    ${if} $Path${component} == ""
      !insertmacro MUI_INSTALLOPTIONS_WRITE "external_${component}.ini" "Field 2" "State" "1"
    ${else}
      !insertmacro MUI_INSTALLOPTIONS_WRITE "external_${component}.ini" "Field 3" "State" "1"
    ${endif}
    
  !if ${currentuser_possible} == ${FALSE}
    ${else}
      !insertmacro MUI_INSTALLOPTIONS_WRITE "external_${component}.ini" "Field 3" "State" "1"
    ${endif}
  !endif
  
!macroend

;--------------------------------
;Functions

Function InitDialogs

  ;Extract dialogs
  
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT_AS "dialogs\user.ini" "user.ini"
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT_AS "dialogs\reinstall.ini" "reinstall.ini"
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT_AS "dialogs\external.ini" "external_latex.ini"
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT_AS "dialogs\external.ini" "external_imagemagick.ini"
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT_AS "dialogs\external.ini" "external_ghostscript.ini"
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT_AS "dialogs\viewer.ini" "viewer.ini"
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT_AS "dialogs\langselect.ini" "langselect.ini"
  
  ;Write texts
  
  !insertmacro MUI_INSTALLOPTIONS_WRITE "user.ini" "Field 1" "Text" $(TEXT_USER_INFO)
  !insertmacro MUI_INSTALLOPTIONS_WRITE "user.ini" "Field 2" "Text" $(TEXT_USER_CURRENT)
  !insertmacro MUI_INSTALLOPTIONS_WRITE "user.ini" "Field 3" "Text" $(TEXT_USER_ALL)
  
  !insertmacro MUI_INSTALLOPTIONS_WRITE "reinstall.ini" "Field 1" "Text" $(TEXT_REINSTALL_INFO)
  !insertmacro MUI_INSTALLOPTIONS_WRITE "reinstall.ini" "Field 2" "Text" $(TEXT_REINSTALL_ENABLE)
  
  !insertmacro InitDialogExternal latex ${TRUE}
  !insertmacro InitDialogExternal imagemagick ${FALSE}
  !insertmacro InitDialogExternal ghostscript ${FALSE}
  
  !insertmacro MUI_INSTALLOPTIONS_WRITE "viewer.ini" "Field 1" "Text" $(TEXT_VIEWER_INFO_${SETUPTYPE_NAME})
  !insertmacro MUI_INSTALLOPTIONS_WRITE "viewer.ini" "Field 2" "Text" $(TEXT_VIEWER_${SETUPTYPE_NAME})
  
  !insertmacro MUI_INSTALLOPTIONS_WRITE "langselect.ini" "Field 1" "Text" $(TEXT_LANGUAGE_INFO)

FunctionEnd

Function InitInterface

  Call CheckPriviledges

  Banner::show /NOUNLOAD "Preparing wizard"

  Call InitDialogs
  Call InitLanguage  
  Call InitSizeExternal
  Call InitSizeViewer
  
  Banner::destroy

FunctionEnd
