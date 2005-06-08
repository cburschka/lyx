!ifndef _LYX_LANGUAGES_GERMAN_NSH_
!define _LYX_LANGUAGES_GERMAN_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_GERMAN}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString SecAllUsersTitle "${LYX_LANG}" "Install for all users?"
LangString SecFileAssocTitle "${LYX_LANG}" "File associations"
LangString SecDesktopTitle "${LYX_LANG}" "Desktop icon"

LangString SecCoreDescription "${LYX_LANG}" "The ${PRODUCT_NAME} files."
LangString SecAllUsersDescription "${LYX_LANG}" "Install for all users or just the current user. (Requires Administrator privileges.)"
LangString SecFileAssocDescription "${LYX_LANG}" "Create associations between the executable and the .lyx extension."
LangString SecDesktopDescription "${LYX_LANG}" "A ${PRODUCT_NAME} icon on the desktop."

LangString ModifyingConfigureFailed "${LYX_LANG}" "Failed attempting to set 'path_prefix' in the configure script"

LangString FinishPageMessage "${LYX_LANG}" "Congratulations! LyX has been installed successfully."
LangString FinishPageRun "${LYX_LANG}" "Launch LyX"

LangString DownloadPageField2 "${LYX_LANG}" "&Do not install"

LangString MinSYSHeader "${LYX_LANG}" "MinSYS"
LangString MinSYSDescription "${LYX_LANG}" "MinSYS is a minimal unix scripting environment (www.mingw.org/msys.shtml) which ${PRODUCT_NAME} needs to run a number of scripts."
LangString EnterMinSYSFolder "${LYX_LANG}" "Please input the path to the folder containing sh.exe"
LangString InvalidMinSYSFolder "${LYX_LANG}" "Unable to find sh.exe"
LangString MinSYSDownloadLabel "${LYX_LANG}" "&Download MinSYS"
LangString MinSYSFolderLabel "${LYX_LANG}" "&Folder containing sh.exe"

LangString PythonHeader "${LYX_LANG}" "Python"
LangString PythonDescription "${LYX_LANG}" "The Python scripting language (www.python.org) must be installed or ${PRODUCT_NAME} will be unable to run a number of scripts."
LangString EnterPythonFolder "${LYX_LANG}" "Please input the path to the folder containing Python.exe"
LangString InvalidPythonFolder "${LYX_LANG}" "Unable to find Python.exe"
LangString PythonDownloadLabel "${LYX_LANG}" "&Download Python"
LangString PythonFolderLabel "${LYX_LANG}" "&Folder containing Python.exe"

LangString MiKTeXHeader "${LYX_LANG}" "MiKTeX"
LangString MiKTeXDescription "${LYX_LANG}" "MiKTeX (www.miktex.org) is an up-to-date TeX implementation for Windows."
LangString EnterMiKTeXFolder "${LYX_LANG}" "Please input the path to the folder containing latex.exe"
LangString InvalidMiKTeXFolder "${LYX_LANG}" "Unable to find latex.exe"
LangString MiKTeXDownloadLabel "${LYX_LANG}" "&Download MiKTeX"
LangString MiKTeXFolderLabel "${LYX_LANG}" "&Folder containing latex.exe"

LangString PerlHeader "${LYX_LANG}" "Perl"
LangString PerlDescription "${LYX_LANG}" "If you plan on using reLyX to convert LaTeX documents to LyX ones, then you should install Perl (www.perl.com)."
LangString EnterPerlFolder "${LYX_LANG}" "Please input the path to the folder containing Perl.exe"
LangString InvalidPerlFolder "${LYX_LANG}" "Unable to find Perl.exe"
LangString PerlDownloadLabel "${LYX_LANG}" "&Download Perl"
LangString PerlFolderLabel "${LYX_LANG}" "&Folder containing perl.exe"

LangString ImageMagickHeader "${LYX_LANG}" "ImageMagick"
LangString ImageMagickDescription "${LYX_LANG}" "The ImageMagick tools (www.imagemagick.org/script/index.php) can be used to convert graphics files to whatever output format is needed."
LangString EnterImageMagickFolder "${LYX_LANG}" "Please input the path to the folder containing convert.exe"
LangString InvalidImageMagickFolder "${LYX_LANG}" "Unable to find convert.exe"
LangString ImageMagickDownloadLabel "${LYX_LANG}" "&Download ImageMagick"
LangString ImageMagickFolderLabel "${LYX_LANG}" "&Folder containing convert.exe"

LangString GhostscriptHeader "${LYX_LANG}" "Ghostscript"
LangString GhostscriptDescription "${LYX_LANG}" "Ghostscript (http://www.cs.wisc.edu/~ghost/) is used to convert images to/from PostScript."
LangString EnterGhostscriptFolder "${LYX_LANG}" "Please input the path to the folder containing gswin32c.exe"
LangString InvalidGhostscriptFolder "${LYX_LANG}" "Unable to find gswin32c.exe"
LangString GhostscriptDownloadLabel "${LYX_LANG}" "&Download Ghostscript"
LangString GhostscriptFolderLabel "${LYX_LANG}" "&Folder containing gswin32c.exe"

LangString SummaryTitle "${LYX_LANG}" "Software summary"
LangString SummaryPleaseInstall "${LYX_LANG}" "Please install your downloaded files and then run LyX's installer once again."
LangString SummaryPathPrefix "${LYX_LANG}" "I shall add a 'path_prefix' string to 'lyxrc.defaults' containing:$\r$\n$PathPrefix"

LangString UnNotInRegistryLabel "${LYX_LANG}" "Unable to find $(^Name) in the registry$\r$\nShortcuts on the desktop and in the Start Menu will not be removed."
LangString UnNotAdminLabel "${LYX_LANG}" "Sorry! You must have administrator privileges$\r$\nto uninstall $(^Name)."
LangString UnReallyRemoveLabel "${LYX_LANG}" "Are you sure you want to completely remove $(^Name) and all of its components?"
LangString UnRemoveSuccessLabel "${LYX_LANG}" "$(^Name) was successfully removed from your computer."

!undef LYX_LANG

!endif ; _LYX_LANGUAGES_GERMAN_NSH_
