!ifndef _LYX_LANGUAGES_SWEDISH_NSH_
!define _LYX_LANGUAGES_SWEDISH_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_SWEDISH}

!define SwedishInputFolderPath "Var god mata in sökvägen till katalogen som innehåller"
!define SwedishUnableToFind "Kan inte hitta"
!define SwedishDownload "&Ladda ner"
!define SwedishFolderContaining "&Katalog innehållande"

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString SecAllUsersTitle "${LYX_LANG}" "Installera för alla användare?"
LangString SecFileAssocTitle "${LYX_LANG}" "Filassociationer"
LangString SecDesktopTitle "${LYX_LANG}" "Skrivbordsikon"

LangString SecCoreDescription "${LYX_LANG}" "${PRODUCT_NAME}-filer."
LangString SecAllUsersDescription "${LYX_LANG}" "Installera för alla användare, eller enbart för den aktuella användare. (Kräver administratörsrättigheter.)"
LangString SecFileAssocDescription "${LYX_LANG}" "Skapa en association mellan programmet och filtilläget .lyx."
LangString SecDesktopDescription "${LYX_LANG}" "En ${PRODUCT_NAME}ikon på skrivbordet."

LangString ModifyingConfigureFailed "${LYX_LANG}" "Misslyckades med att sätta  'path_prefix' i konfigurationsskriptet"
LangString CreateCmdFilesFailed "${LYX_LANG}" "Failed atempting to create lyx.cmd and reLyX.cmd"
LangString RunConfigureFailed "${LYX_LANG}" "Misslyckades med att köra konfigurationsskriptet"

LangString FinishPageMessage "${LYX_LANG}" "Gratulerar! LyX har installerats framgångsrikt."
LangString FinishPageRun "${LYX_LANG}" "Kör LyX"

LangString DownloadPageField2 "${LYX_LANG}" "Installera &ej"

LangString MinSYSHeader "${LYX_LANG}" "MinSYS"
LangString MinSYSDescription "${LYX_LANG}" "MinSYS är en minimal UNIX-liknande skriptmiljö (www.mingw.org/msys.shtml) som ${PRODUCT_NAME} behöver för att använda ett flertal skript."
LangString EnterMinSYSFolder "${LYX_LANG}" "${SwedishInputFolderPath} sh.exe"
LangString InvalidMinSYSFolder "${LYX_LANG}" "${SwedishUnableToFind} sh.exe"
LangString MinSYSDownloadLabel "${LYX_LANG}" "${SwedishDownload} MinSYS"
LangString MinSYSFolderLabel "${LYX_LANG}" "${SwedishFolderContaining} sh.exe"

LangString PythonHeader "${LYX_LANG}" "Python"
LangString PythonDescription "${LYX_LANG}" "Skriptspråket Python (www.python.org) bör installeras, annars kan inte ${PRODUCT_NAME} använda ett flertal skript."
LangString EnterPythonFolder "${LYX_LANG}" "${SwedishInputFolderPath} Python.exe"
LangString InvalidPythonFolder "${LYX_LANG}" "${SwedishUnableToFind} Python.exe"
LangString PythonDownloadLabel "${LYX_LANG}" "${SwedishDownload} Python"
LangString PythonFolderLabel "${LYX_LANG}" "${SwedishFolderContaining} Python.exe"

LangString MiKTeXHeader "${LYX_LANG}" "MiKTeX"
LangString MiKTeXDescription "${LYX_LANG}" "MiKTeX (www.miktex.org) aktuell (nyligen uppdaterad) implementation av TeX för Windows."
LangString EnterMiKTeXFolder "${LYX_LANG}" "${SwedishInputFolderPath} latex.exe"
LangString InvalidMiKTeXFolder "${LYX_LANG}" "${SwedishUnableToFind} latex.exe"
LangString MiKTeXDownloadLabel "${LYX_LANG}" "${SwedishDownload} MiKTeX"
LangString MiKTeXFolderLabel "${LYX_LANG}" "${SwedishFolderContaining} latex.exe"

LangString PerlHeader "${LYX_LANG}" "Perl"
LangString PerlDescription "${LYX_LANG}" "Om du räknar med att använda reLyX för att konvertera LaTeX-dokument till LyX-dokument, måste du installera Perl (www.perl.com)."
LangString EnterPerlFolder "${LYX_LANG}" "${SwedishInputFolderPath} Perl.exe"
LangString InvalidPerlFolder "${LYX_LANG}" "${SwedishUnableToFind} Perl.exe"
LangString PerlDownloadLabel "${LYX_LANG}" "${SwedishDownload} Perl"
LangString PerlFolderLabel "${LYX_LANG}" "${SwedishFolderContaining} perl.exe"

LangString ImageMagickHeader "${LYX_LANG}" "ImageMagick"
LangString ImageMagickDescription "${LYX_LANG}" "Verktygen i paketet ImageMagick (www.imagemagick.org/script/index.php) kan användas av LyX för att konvertera mellan de flesta grafikformat."
LangString EnterImageMagickFolder "${LYX_LANG}" "${SwedishInputFolderPath} convert.exe"
LangString InvalidImageMagickFolder "${LYX_LANG}" "${SwedishUnableToFind} convert.exe"
LangString ImageMagickDownloadLabel "${LYX_LANG}" "${SwedishDownload} ImageMagick"
LangString ImageMagickFolderLabel "${LYX_LANG}" "${SwedishFolderContaining} convert.exe"

LangString GhostscriptHeader "${LYX_LANG}" "Ghostscript"
LangString GhostscriptDescription "${LYX_LANG}" "Ghostscript (http://www.cs.wisc.edu/~ghost/) används till att konvertera bilder till/från PostScript."
LangString EnterGhostscriptFolder "${LYX_LANG}" "${SwedishInputFolderPath} gswin32c.exe"
LangString InvalidGhostscriptFolder "${LYX_LANG}" "${SwedishUnableToFind} gswin32c.exe"
LangString GhostscriptDownloadLabel "${LYX_LANG}" "${SwedishDownload} Ghostscript"
LangString GhostscriptFolderLabel "${LYX_LANG}" "${SwedishFolderContaining} gswin32c.exe"

LangString SummaryTitle "${LYX_LANG}" "Mjukvarusammanfattning"
LangString SummaryPleaseInstall "${LYX_LANG}" "Var god installera först dina nedladdade filer och kör sedan LyXs installationsprogram igen."
LangString SummaryPathPrefix "${LYX_LANG}" "En 'path_prefix'-sträng kommer att läggas till i filen 'lyxrc.defaults' innehållande följande:"

LangString UILangageTitle "${LYX_LANG}" "The language of LyX's interface"
LangString UILangageDescription "${LYX_LANG}" "As used for menus, messages, etc."

LangString UnNotInRegistryLabel "${LYX_LANG}" "${SwedishUnableToFind} $(^Name) i registret\r$\nGenvägar på skrivbordet och i startmeny kommer inte att tas bort."
LangString UnNotAdminLabel "${LYX_LANG}" "Tyvärr! Du måste ha administratörsrättigheter för\r$\natt avinstallera $(^Name)."
LangString UnReallyRemoveLabel "${LYX_LANG}" "Är du säker på att du verkligen vill fullständigt avinstallera $(^Name) och alla dess komponenter?"
LangString UnRemoveSuccessLabel "${LYX_LANG}" "$(^Name) har framgångsrikt avinstallerats från din dator."

!undef SwedishInputFolderPath
!undef SwedishUnableToFind
!undef SwedishDownload
!undef SwedishFolderContaining

!undef LYX_LANG

!endif ; _LYX_LANGUAGES_SWEDISH_NSH_
