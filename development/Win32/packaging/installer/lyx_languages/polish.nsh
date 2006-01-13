!ifndef _LYX_LANGUAGES_POLISH_NSH_
!define _LYX_LANGUAGES_POLISH_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_POLISH}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString SecAllUsersTitle "${LYX_LANG}" "Instalacja dla wszystkich u¿ytkowników?"
LangString SecFileAssocTitle "${LYX_LANG}" "Skojarzenie plików .lyx"
LangString SecDesktopTitle "${LYX_LANG}" "Ikona na pulpicie"

LangString SecCoreDescription "${LYX_LANG}" "Pliki ${PRODUCT_NAME}-a."
LangString SecAllUsersDescription "${LYX_LANG}" "Instalacja dla wszystkich u¿ytkowników lub tylko dla bie¿¹cego u¿ytkownika. Instalacja dla wszystkich wymaga praw administratora."
LangString SecFileAssocDescription "${LYX_LANG}" "Skojarzenie LyX-a z plikami o rozszerzeniu .lyx."
LangString SecDesktopDescription "${LYX_LANG}" "Ikona ${PRODUCT_NAME}-a na pulpicie."

LangString ModifyingConfigureFailed "${LYX_LANG}" "Nieudana próba ustawienia zmiennej 'path_prefix' w pliku skryptu konfigracyjnego"
LangString CreateCmdFilesFailed "${LYX_LANG}" "Niedana próba utworzenia plików lyx.cmd i reLyX.cmd"
LangString RunConfigureFailed "${LYX_LANG}" "Niedana próba wykonania skryptu konfiguracyjnego"

LangString FinishPageMessage "${LYX_LANG}" "LyX powinien byæ ju¿ teraz zainstalowany. Jednak niektórzy u¿ytkownicy raportuj¹, ¿e skrypt konfiguracyjny nie wykonuje siê. W efekcie LyX nie uruchamia siê zg³aszaj¹c brak pliku textclass.lst. Dla naprawy instalacji LyX-a uruchom polecenie 'sh configure' w katalogu $\'<LyX>/Resources/lyx$\'."
LangString FinishPageRun "${LYX_LANG}" "Uruchom LyX"

LangString DownloadPageField2 "${LYX_LANG}" "&Nie instaluj"

LangString MinSYSHeader "${LYX_LANG}" "MinSYS"
LangString MinSYSDescription "${LYX_LANG}" "MinSYS jest minimalnym œrodowiskiem uniksowym (www.mingw.org/msys.shtml), którego wymaga ${PRODUCT_NAME} do uruchomienia niektórych skryptów."
LangString EnterMinSYSFolder "${LYX_LANG}" "Proszê podaæ œcie¿kê do folderu zawieraj¹cego sh.exe"
LangString InvalidMinSYSFolder "${LYX_LANG}" "Nie mo¿na znaleŸæ sh.exe"
LangString MinSYSDownloadLabel "${LYX_LANG}" "&Pobierz MinSYS"
LangString MinSYSFolderLabel "${LYX_LANG}" "&Folder zawieraj¹cy sh.exe"

LangString PythonHeader "${LYX_LANG}" "Python"
LangString PythonDescription "${LYX_LANG}" "Jêzyk skryptowy Python (www.python.org) musi byæ zainstalowany, inaczej ${PRODUCT_NAME} nie bêdzie móg³ uruchomiæ niektórych skryptów."
LangString EnterPythonFolder "${LYX_LANG}" "Proszê podaæ œcie¿kê do folderu zawieraj¹cego Python.exe"
LangString InvalidPythonFolder "${LYX_LANG}" "Nie mo¿na znaleŸæ Python.exe"
LangString PythonDownloadLabel "${LYX_LANG}" "&Pobierz Python"
LangString PythonFolderLabel "${LYX_LANG}" "&Folder zawieraj¹cy Python.exe"

LangString MiKTeXHeader "${LYX_LANG}" "MiKTeX"
LangString MiKTeXDescription "${LYX_LANG}" "MiKTeX (www.miktex.org) jest implementacj¹ TeX-a dla Windows."
LangString EnterMiKTeXFolder "${LYX_LANG}" "Proszê podaæ œcie¿kê do folderu zawieraj¹cego latex.exe"
LangString InvalidMiKTeXFolder "${LYX_LANG}" "Nie mo¿na znaleŸæ latex.exe"
LangString MiKTeXDownloadLabel "${LYX_LANG}" "&Pobierz MiKTeX"
LangString MiKTeXFolderLabel "${LYX_LANG}" "&Folder zawieraj¹cy latex.exe"

LangString PerlHeader "${LYX_LANG}" "Perl"
LangString PerlDescription "${LYX_LANG}" "Jeœli przewidujesz u¿ycie programu reLyX do konwersji dokumentów LaTeX-a do LyX-a, to nale¿y zaistalowaæ jêzyk Perl (www.perl.com)."
LangString EnterPerlFolder "${LYX_LANG}" "Proszê podaæ œcie¿kê do folderu zawieraj¹cego Perl.exe"
LangString InvalidPerlFolder "${LYX_LANG}" "Nie mo¿na znaleŸæ Perl.exe"
LangString PerlDownloadLabel "${LYX_LANG}" "&Pobierz Perl"
LangString PerlFolderLabel "${LYX_LANG}" "&Folder zawieraj¹cy perl.exe"

LangString ImageMagickHeader "${LYX_LANG}" "ImageMagick"
LangString ImageMagickDescription "${LYX_LANG}" "Narzêdzia pakietu ImageMagick (www.imagemagick.org/script/index.php) s¹ u¿ywane do konwersji plików graficznych do wymaganych formatów."
LangString EnterImageMagickFolder "${LYX_LANG}" "Proszê podaæ œcie¿kê do folderu zawieraj¹cego convert.exe"
LangString InvalidImageMagickFolder "${LYX_LANG}" "Nie mo¿na znaleŸæ convert.exe"
LangString ImageMagickDownloadLabel "${LYX_LANG}" "&Pobierz ImageMagick"
LangString ImageMagickFolderLabel "${LYX_LANG}" "&Folder zawieraj¹cy convert.exe"

LangString GhostscriptHeader "${LYX_LANG}" "Ghostscript"
LangString GhostscriptDescription "${LYX_LANG}" "Ghostscript (www.cs.wisc.edu/~ghost/) jest u¿ywany do konwersji z i do formatu PostScript."
LangString EnterGhostscriptFolder "${LYX_LANG}" "Proszê podaæ œcie¿kê do folderu zawieraj¹cego gswin32c.exe"
LangString InvalidGhostscriptFolder "${LYX_LANG}" "Nie mo¿na znaleŸæ gswin32c.exe"
LangString GhostscriptDownloadLabel "${LYX_LANG}" "&Pobierz Ghostscript"
LangString GhostscriptFolderLabel "${LYX_LANG}" "&Folder zawieraj¹cy gswin32c.exe"

LangString SummaryTitle "${LYX_LANG}" "Podsumowanie oprogramowania"
LangString SummaryPleaseInstall "${LYX_LANG}" "Proszê zainstalowaæ pobrane pliki i uruchomiæ instalator LyX-a ponownie."
LangString SummaryPathPrefix "${LYX_LANG}" "Do zmiennej 'path_prefix' w pliku 'lyxrc.defaults' zostan¹ dodane nastêpuj¹ce œcie¿ki:"

LangString UILangageTitle "${LYX_LANG}" "Jêzyk interfejsu LyX-a"
LangString UILangageDescription "${LYX_LANG}" "Który bêdzie u¿yty w menu, komunikatach itp."
LangString UILanguageAvailableLanguages "${LYX_LANG}" " Available Languages "

LangString UnNotInRegistryLabel "${LYX_LANG}" "Nie mo¿na znaleŸæ $(^Name) w rejestrze$\r$\nSkróty na pulpicie i w menu Start nie zostan¹ usuniête."
LangString UnNotAdminLabel "${LYX_LANG}" "Musisz posiadaæ prawa administratora$\r$\ndo deinstalacji programu $(^Name)."
LangString UnReallyRemoveLabel "${LYX_LANG}" "Czy na pewno chcesz usun¹æ $(^Name) i wszystkie jego komponenty?"
LangString UnRemoveSuccessLabel "${LYX_LANG}" "$(^Name) zosta³ pomyœlnie usuniêty z Twojego komputera."

!undef LYX_LANG

!endif ; _LYX_LANGUAGES_POLISH_NSH_

