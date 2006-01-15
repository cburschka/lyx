!ifndef _LYX_LANGUAGES_SWEDISH_NSH_
!define _LYX_LANGUAGES_SWEDISH_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_SWEDISH}

!define SwedishInputFolderPath "Var god ange sökväg till katalogen som innehåller"
!define SwedishUnableToFind "Kan inte hitta"
!define SwedishDownload "&Ladda ner"
!define SwedishFolderContaining "&Katalog innehållande"

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString SecAllUsersTitle "${LYX_LANG}" "Installera för alla användare?"
LangString SecFileAssocTitle "${LYX_LANG}" "Program associerade till olika filtyper" 
LangString SecDesktopTitle "${LYX_LANG}" "Skrivbordsikon"

LangString SecCoreDescription "${LYX_LANG}" "${PRODUCT_NAME}-filer."
LangString SecAllUsersDescription "${LYX_LANG}" "Installera för alla användare, eller enbart för den aktuella användare. (Kräver administratörsrättigheter.)"
LangString SecFileAssocDescription "${LYX_LANG}" "Skapa en association mellan programmet och filtilläget .lyx."
LangString SecDesktopDescription "${LYX_LANG}" "Skapa en ${PRODUCT_NAME}-ikon på skrivbordet."

LangString ModifyingConfigureFailed "${LYX_LANG}" "Misslyckades med att sätta sökvägsprefixet i konfigurationsprogrammet"
LangString CreateCmdFilesFailed "${LYX_LANG}" "Misslyckades med att skapa filerna lyx.cmd och reLyX.cmd"
LangString RunConfigureFailed "${LYX_LANG}" "Misslyckades med att köra konfigurationsprogrammet"

LangString FinishPageMessage "${LYX_LANG}" "LyX borde nu ha installerats. Dessvärre har en del användare rapporterat att konfigurationsskriptet ibland misslyckats. Om detta händer är inte LyX ordentligt konfigurerat och vägrar starta med ett felmeddelande om att filen 'textclass.lst' saknas. För att åtgärda detta problem, öppna ett kommandofönster (kör programmet 'cmd'), och gå till katalogen 'LyX/Resources/lyx directory'. I den katalogen kör du sedan kommandot 'sh configure' vilket kommer att konfigurar LyX."
LangString FinishPageRun "${LYX_LANG}" "Starta LyX"

LangString DownloadPageField2 "${LYX_LANG}" "Installera &ej"

LangString MinSYSHeader "${LYX_LANG}" "MinSYS"
LangString MinSYSDescription "${LYX_LANG}" "MinSYS är en minimal, UNIX-liknande skriptmiljö (www.mingw.org/msys.shtml) vilken ${PRODUCT_NAME} använder för ett flertal funktioner."
LangString EnterMinSYSFolder "${LYX_LANG}" "${SwedishInputFolderPath} sh.exe"
LangString InvalidMinSYSFolder "${LYX_LANG}" "${SwedishUnableToFind} sh.exe"
LangString MinSYSDownloadLabel "${LYX_LANG}" "${SwedishDownload} MinSYS"
LangString MinSYSFolderLabel "${LYX_LANG}" "${SwedishFolderContaining} sh.exe"

LangString PythonHeader "${LYX_LANG}" "Python"
LangString PythonDescription "${LYX_LANG}" "Skriptspråket Python (www.python.org) bör installeras, annars kan ett flertal skript inte användas av ${PRODUCT_NAME}." 
LangString EnterPythonFolder "${LYX_LANG}" "${SwedishInputFolderPath} Python.exe"
LangString InvalidPythonFolder "${LYX_LANG}" "${SwedishUnableToFind} Python.exe"
LangString PythonDownloadLabel "${LYX_LANG}" "${SwedishDownload} Python"
LangString PythonFolderLabel "${LYX_LANG}" "${SwedishFolderContaining} Python.exe"

LangString MiKTeXHeader "${LYX_LANG}" "MiKTeX"
LangString MiKTeXDescription "${LYX_LANG}" "MiKTeX (www.miktex.org) är en modern implementering av TeX för Windows."
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
LangString ImageMagickDescription "${LYX_LANG}" "Verktygen i paketet ImageMagick (www.imagemagick.org/script/index.php) används av LyX för att konvertera bilder till rätt filformat."
LangString EnterImageMagickFolder "${LYX_LANG}" "${SwedishInputFolderPath} convert.exe"
LangString InvalidImageMagickFolder "${LYX_LANG}" "${SwedishUnableToFind} convert.exe"
LangString ImageMagickDownloadLabel "${LYX_LANG}" "${SwedishDownload} ImageMagick"
LangString ImageMagickFolderLabel "${LYX_LANG}" "${SwedishFolderContaining} convert.exe"

LangString GhostscriptHeader "${LYX_LANG}" "Ghostscript"
LangString GhostscriptDescription "${LYX_LANG}" "Ghostscript (www.cs.wisc.edu/~ghost/) används för att konvertera bilder till/från PostScript."
LangString EnterGhostscriptFolder "${LYX_LANG}" "${SwedishInputFolderPath} gswin32c.exe"
LangString InvalidGhostscriptFolder "${LYX_LANG}" "${SwedishUnableToFind} gswin32c.exe"
LangString GhostscriptDownloadLabel "${LYX_LANG}" "${SwedishDownload} Ghostscript"
LangString GhostscriptFolderLabel "${LYX_LANG}" "${SwedishFolderContaining} gswin32c.exe"

LangString SummaryTitle "${LYX_LANG}" "Sammanfattning av programvara"
LangString SummaryPleaseInstall "${LYX_LANG}" "Var god installera först den kompletterande programvara du laddat ner. Kör sedan LyX installationsprogram igen."
LangString SummaryPathPrefix "${LYX_LANG}" "Ett sökvägsprefix kommer att läggas till i filen 'lyxrc.defaults'. Denna innehåller följande:"

LangString UILangageTitle "${LYX_LANG}" "Språk i LyX användargränsnitt"
LangString UILangageDescription "${LYX_LANG}" "Detta språk används i menyer, meddelanden, m.m."
LangString UILanguageAvailableLanguages "${LYX_LANG}" " Tillgängliga språk "

LangString UnNotInRegistryLabel "${LYX_LANG}" "${SwedishUnableToFind} $(^Name) i registret\r$\nGenvägar på skrivbordet och i startmenyn kommer inte att tas bort."
LangString UnNotAdminLabel "${LYX_LANG}" "Tyvärr! Du måste ha administratörsrättigheter för\r$\natt avinstallera $(^Name)."
LangString UnReallyRemoveLabel "${LYX_LANG}" "Är du säker på att du verkligen vill avinstallera $(^Name) och alla dess komponenter fullständigt?"
LangString UnRemoveSuccessLabel "${LYX_LANG}" "Avinstallationen av $(^Name) från din dator var framgångsrik."

!undef SwedishInputFolderPath
!undef SwedishUnableToFind
!undef SwedishDownload
!undef SwedishFolderContaining

!undef LYX_LANG

!endif ; _LYX_LANGUAGES_SWEDISH_NSH_
