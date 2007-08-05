# installer settings
# You should need to change only these macros for new releases.

!define PRODUCT_DIR "D:\LyXPackage1.5"
!define PRODUCT_NAME "LyX"
!define PRODUCT_VERSION "1.5.1"
!define PRODUCT_VERSION_SHORT "151"
!define PRODUCT_SUBFOLDER "lyx15"
!define PRODUCT_LICENSE_FILE "${PRODUCT_DIR}\License.txt"
!define PRODUCT_SOURCEDIR "${PRODUCT_DIR}\LyX"
!define PRODUCT_EXE "$INSTDIR\bin\lyx.exe"
!define PRODUCT_BAT "$INSTDIR\bin\lyx.bat"
!define PRODUCT_EXT ".lyx"
!define PRODUCT_REGNAME "LyX.Document"
!define PRODUCT_MIME_TYPE "application/lyx"
!define PRODUCT_UNINSTALL_EXE "$INSTDIR\LyXWinUninstall.exe"
!define PRODUCT_HELP_LINK "http://www.lyx.org/internet/mailing.php"
!define PRODUCT_ABOUT_URL "http://www.lyx.org/about/"
!define PRODUCT_INFO_URL "http://www.lyx.org/"
!define LAUNCHER_EXE "$INSTDIR\bin\LyXLauncher.exe"

BrandingText "LyXWinInstaller v3.18 - ${INSTALLER_VERSION}"

# to check later if this installer version is running at the same time
!define INSTALLER_EXE "LyXWin${PRODUCT_VERSION_SHORT}${INSTALLER_VERSION}-3-18.exe"
!define INSTALLER2_EXE "LyXWin${PRODUCT_VERSION_SHORT}${INSTALLER2_VERSION}-3-18.exe"
!define INSTALLER3_EXE "LyXWin${PRODUCT_VERSION_SHORT}${INSTALLER3_VERSION}-3-18.exe"
!define VERSION_BITMAP "${PRODUCT_DIR}\icons\lyx_logo_vert${PRODUCT_VERSION_SHORT}.bmp"

# Replaced by HKLM or HKCU depending on SetShellVarContext.
!define PRODUCT_ROOT_KEY "SHCTX"

; registry preparations
!define SHCNE_ASSOCCHANGED 0x08000000
!define SHCNF_IDLIST 0
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\lyx.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX${PRODUCT_VERSION_SHORT}"

# definitions for the Update installer
!if ${INSTALLER_VERSION} == "Update"
 !define PRODUCT_UNINST_KEY_OLD "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX150svn"
 !define PRODUCT_VERSION_OLD "LyX 1.5rc2-27-05-2007"
 !define PRODUCT_UNINSTALL_EXE_OLD "$INSTDIR\LyXWinUninstall.exe"
 !define PRODUCT_VERSION_SHORT_OLD "150"
 !define JabRefVersion "2.2" # could be an older version
 !define MiKTeXDeliveredVersion "2.6" # could be an older version
!endif

# definitions for the Small installer
!if ${INSTALLER_VERSION} == "Small"
 !define JabRefVersion "none" # dummy entry to avoid NSIS warnig
 !define MiKTeXDeliveredVersion "none" # dummy entry to avoid NSIS warnig
!endif

# definitions for the Complete installer
!if ${INSTALLER_VERSION} == "Complete"
 !define GSviewInstall "external\gsv48w32.exe"
 !define JabRefInstall "external\JabRef-2.2-Setup.exe"
 !define MiKTeXInstall "$INSTDIR\external\basic-miktex-2.6.2742.exe"
 !define JabRefVersion "2.2"
 !define MiKTeXDeliveredVersion "2.6"
!endif

!define ClassFileDir "${PRODUCT_SOURCEDIR}\Resources\tex"

!define ImageMagickVersion "6.3.5"
# for some odd reason the ImageMagick folder may not be a subfolder of $INSTDIR\bin!
!define ImageMagickDir "$INSTDIR\etc\ImageMagick"
!define GhostscriptDir "$INSTDIR\etc\Ghostscript"
!define GhostscriptVersion "8.60"
!define AiksaurusDir "$APPDATA\Aiksaurus"
!define AiksaurusInstall "external\Aiksaurus"
!define AspellDir "$APPDATA\Aspell"
!define AspellDisplay "Aspell 0.60.4 Data"
!define AspellInstall "external\Aspell"
!define AspellUninstall "Uninstall-AspellData.exe"
!define AspellDictPath "${AspellDir}\Dictionaries"
!define AspellPersonalPath "${AspellDir}\Personal"
!define AspellLocationExact "http://developer.berlios.de/project/showfiles.php?group_id=5117&release_id=12973"
!define AspellLocation "http://developer.berlios.de/projects/lyxwininstall/"
!define DVIPostFileDir "${PRODUCT_SOURCEDIR}\external\dvipost"
# the following variable is needed for a possible CD-version
#!define LaTeXPackagesDir "${PRODUCT_SOURCEDIR}\latex"
!define MiKTeXRepo "ftp://ftp.tu-chemnitz.de/pub/tex/systems/win32/miktex/tm/packages/"

#--------------------------------
# make some of the information above available to NSIS.

Name "${PRODUCT_NAME}"
OutFile "${INSTALLER_EXE}"
InstallDir "$PROGRAMFILES\${PRODUCT_NAME} ${PRODUCT_VERSION}"

