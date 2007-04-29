; installer settings
; You should need to change only these macros for new releases.

!define PRODUCT_DIR "D:\LyXPackage1.5"
!define PRODUCT_NAME "LyX"
!define PRODUCT_VERSION "1.5svn-30-04-2007"
!define PRODUCT_VERSION_SHORT "150svn"
!define PRODUCT_SUBFOLDER "lyx15"
!define PRODUCT_LICENSE_FILE "${PRODUCT_DIR}\License.txt"
!define PRODUCT_SOURCEDIR "${PRODUCT_DIR}\LyX"
!define PRODUCT_EXE "$INSTDIR\bin\lyx.exe"
!define PRODUCT_BAT "$INSTDIR\bin\lyx.bat"
!define PRODUCT_EXT ".lyx"
!define PRODUCT_MIME_TYPE "application/lyx"
!define PRODUCT_UNINSTALL_EXE "$INSTDIR\LyXWinUninstall.exe"
!define PRODUCT_HELP_LINK "http://www.lyx.org/internet/mailing.php"
!define PRODUCT_ABOUT_URL "http://www.lyx.org/about/"
!define PRODUCT_INFO_URL "http://www.lyx.org/"

BrandingText "LyXWinInstaller v3.14 - ${INSTALLER_VERSION}"

; to check later if this installer version is running at the same time
!define INSTALLER_EXE "LyXWin${PRODUCT_VERSION_SHORT}${INSTALLER_VERSION}-3-14.exe"
!define INSTALLER2_EXE "LyXWin${PRODUCT_VERSION_SHORT}${INSTALLER2_VERSION}-3-14.exe"
!define INSTALLER3_EXE "LyXWin${PRODUCT_VERSION_SHORT}${INSTALLER3_VERSION}-3-14.exe"
!define VERSION_BITMAP "${PRODUCT_DIR}\icons\lyx_logo_vert${PRODUCT_VERSION_SHORT}.bmp"

; Replaced by HKLM or HKCU depending on SetShellVarContext.
!define PRODUCT_ROOT_KEY "SHCTX"

!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\lyx.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX${PRODUCT_VERSION_SHORT}"

!define ClassFileDir "${PRODUCT_SOURCEDIR}\Resources\tex"

!define ImageMagickVersion "6.3.3"
!define ImageMagickDir "$INSTDIR\etc\ImageMagick" ; for some odd reason the ImageMagick folder may not be a subfolder of $INSTDIR\bin!
!define GhostscriptDir "$INSTDIR\etc\Ghostscript"
!define GhostscriptVersion "8.56"
!define AiksaurusDir "$APPDATA\Aiksaurus"
!define AiksaurusInstall "external\Aiksaurus"
!define AspellDir "$APPDATA\Aspell"
!define AspellDisplay "Aspell 0.60.4 Data"
!define AspellInstall "external\Aspell"
!define AspellUninstall "Uninstall-AspellData.exe"
!define AspellDictPath "${AspellDir}\Dictionaries"
!define AspellPersonalPath "${AspellDir}\Personal"
!define AspellLocationExact "http://developer.berlios.de/project/showfiles.php?group_id=5117&release_id=9651"
!define AspellLocation "http://developer.berlios.de/projects/lyxwininstall/"
!define DVIPostFileDir "${PRODUCT_SOURCEDIR}\external\dvipost"
; the following variable is needed for a possible CD-version
;!define LaTeXPackagesDir "${PRODUCT_SOURCEDIR}\latex"
!define MiKTeXRepo "ftp://ftp.tu-chemnitz.de/pub/tex/systems/win32/miktex/tm/packages/"
!define MiKTeXConfigFolder "MiKTeX\2.5\miktex\config"

;--------------------------------
; make some of the information above available to NSIS.

Name "${PRODUCT_NAME}"
OutFile "${INSTALLER_EXE}"
InstallDir "$PROGRAMFILES\${PRODUCT_NAME} ${PRODUCT_VERSION}"

; -------------------------------
; Variables used by all installer versions

Var LatexPath
Var PythonPath
Var ImageMagickPath
Var PathPrefix
Var Answer
Var UserName
Var CreateDesktopIcon
Var StartmenuFolder
Var ProductRootKey
Var AppPre
Var AppSuff
Var AppPath
Var String
Var Search
Var Pointer
Var UserList
Var FileName
Var OldString
Var NewString

