!ifndef _LYX_LANGUAGES_DANISH_NSH_
!define _LYX_LANGUAGES_DANISH_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_DANISH}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString SecAllUsersTitle "${LYX_LANG}" "Installer til alle brugere?"
LangString SecFileAssocTitle "${LYX_LANG}" "Fil-associationer"
LangString SecDesktopTitle "${LYX_LANG}" "Skrivebordsikon"

LangString SecCoreDescription "${LYX_LANG}" "Filerne til ${PRODUCT_NAME}."
LangString SecAllUsersDescription "${LYX_LANG}" "Installer til alle brugere, eller kun den aktuelle bruger. (kræver administrator-rettigheder.)"
LangString SecFileAssocDescription "${LYX_LANG}" "Opret association mellem LyX og .lyx filer."
LangString SecDesktopDescription "${LYX_LANG}" "Et ${PRODUCT_NAME} ikon på skrivebordet"

LangString ModifyingConfigureFailed "${LYX_LANG}" "Forsøget på at indstille 'path_prefix' i konfigurationen mislykkedes"
LangString RunConfigureFailed "${LYX_LANG}" "Mislykket forsøg på at afvikle konfigurations-scriptet"

LangString FinishPageMessage "${LYX_LANG}" "Tillykke!! LyX er installeret."
LangString FinishPageRun "${LYX_LANG}" "Start LyX"

LangString DownloadPageField2 "${LYX_LANG}" "&Installer ikke"

LangString MinSYSHeader "${LYX_LANG}" "MinSYS"
LangString MinSYSDescription "${LYX_LANG}" "MinSYS (www.mingw.org/msys.shtml) er et værktøj som ${PRODUCT_NAME} skal buge til at afvikle en række scripts."
LangString EnterMinSYSFolder "${LYX_LANG}" "Angiv stien til den mappe, som indeholder sh.exe"
LangString InvalidMinSYSFolder "${LYX_LANG}" "Kunne ikke finde sh.exe"
LangString MinSYSDownloadLabel "${LYX_LANG}" "&Hent MinSYS"
LangString MinSYSFolderLabel "${LYX_LANG}" "&Mappen som indeholder sh.exe"

LangString PythonHeader "${LYX_LANG}" "Python"
LangString PythonDescription "${LYX_LANG}" "Python er et script-sprog (www.python.org) som skal være installeret. Ellers er der et antal scripts som ${PRODUCT_NAME} ikke kan afvikle."
LangString EnterPythonFolder "${LYX_LANG}" "Angiv stien til den mappe som indeholder Python.exe"
LangString InvalidPythonFolder "${LYX_LANG}" "Kunne ikke finde Python.exe"
LangString PythonDownloadLabel "${LYX_LANG}" "&hent Python"
LangString PythonFolderLabel "${LYX_LANG}" "&Mappen som indeholder Python.exe"

LangString MiKTeXHeader "${LYX_LANG}" "MiKTeX"
LangString MiKTeXDescription "${LYX_LANG}" "MiKTeX (www.miktex.org) er en ajourført TeX implementering til Windows."
LangString EnterMiKTeXFolder "${LYX_LANG}" "Angiv stien til mappen som indeholder latex.exe"
LangString InvalidMiKTeXFolder "${LYX_LANG}" "Kunne ikke finde latex.exe"
LangString MiKTeXDownloadLabel "${LYX_LANG}" "&Hent MiKTeX"
LangString MiKTeXFolderLabel "${LYX_LANG}" "&Mappen som indeholder latex.exe"

LangString PerlHeader "${LYX_LANG}" "Perl"
LangString PerlDescription "${LYX_LANG}" "Hvis du forventer at skulle bruge reLyX til at konvertere LaTeX- til LyX-dokumenter, skal du installere Perl (www.perl.com)."
LangString EnterPerlFolder "${LYX_LANG}" "Angiv stien til mappen som indeholder Perl.exe"
LangString InvalidPerlFolder "${LYX_LANG}" "Kunne ikke finde Perl.exe"
LangString PerlDownloadLabel "${LYX_LANG}" "&Hent Perl"
LangString PerlFolderLabel "${LYX_LANG}" "&Mappen som indeholder perl.exe"

LangString ImageMagickHeader "${LYX_LANG}" "ImageMagick"
LangString ImageMagickDescription "${LYX_LANG}" "ImageMagick (www.imagemagick.org/script/index.php) bruges til at konvertere grafik-filer til forskellige slutformater."
LangString EnterImageMagickFolder "${LYX_LANG}" "Angiv stien til mappen som indeholder convert.exe"
LangString InvalidImageMagickFolder "${LYX_LANG}" "Kunne ikke finde convert.exe"
LangString ImageMagickDownloadLabel "${LYX_LANG}" "&Hent ImageMagick"
LangString ImageMagickFolderLabel "${LYX_LANG}" "&Mappen som indeholder convert.exe"

LangString GhostscriptHeader "${LYX_LANG}" "Ghostscript"
LangString GhostscriptDescription "${LYX_LANG}" "Ghostscript (http://www.cs.wisc.edu/~ghost/) bruges til at konvertere billeder til og fra PostScript."
LangString EnterGhostscriptFolder "${LYX_LANG}" "Angiv stien til mappen som indeholder gswin32c.exe"
LangString InvalidGhostscriptFolder "${LYX_LANG}" "Kunne ikke finde gswin32c.exe"
LangString GhostscriptDownloadLabel "${LYX_LANG}" "&Hent Ghostscript"
LangString GhostscriptFolderLabel "${LYX_LANG}" "&Mappen som indeholder gswin32c.exe"

LangString SummaryTitle "${LYX_LANG}" "Software - sammendrag"
LangString SummaryPleaseInstall "${LYX_LANG}" "Installer de filer du har downloaded, og kør så LyX's installationsprogram igen."
LangString SummaryPathPrefix "${LYX_LANG}" "Jeg tilføjer en 'path_prefix' streng til 'lyxrc.defaults' som indeholder:"

LangString UnNotInRegistryLabel "${LYX_LANG}" "Kunne ikke finde $(^Name) i registreringsdatabsen$\r$\nGenvejene på skrivebordet og i Start-menuen bliver ikke fjernet"
LangString UnNotAdminLabel "${LYX_LANG}" "Beklager! Du skal have administrator-rettigheder$\r$\nfor at afinstallere$(^Name)."
LangString UnReallyRemoveLabel "${LYX_LANG}" "Er du sikker på, at du vil slette $(^Name) og alle tilhørende komponenter?"
LangString UnRemoveSuccessLabel "${LYX_LANG}" "$(^Name) er blevet fjernet fra din computer."

!undef LYX_LANG

!endif ; _LYX_LANGUAGES_DANISH_NSH_
