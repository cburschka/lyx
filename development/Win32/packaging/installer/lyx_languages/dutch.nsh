!ifndef _LYX_LANGUAGES_DUTCH_NSH_
!define _LYX_LANGUAGES_DUTCH_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_DUTCH}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString SecAllUsersTitle "${LYX_LANG}" "Installeer voor alle gebruikers?"
LangString SecFileAssocTitle "${LYX_LANG}" "Bestand associaties"
LangString SecDesktopTitle "${LYX_LANG}" "Bureaublad pictogram"

LangString SecCoreDescription "${LYX_LANG}" "De ${PRODUCT_NAME} bestanden."
LangString SecAllUsersDescription "${LYX_LANG}" "Installeer voor alle gebruikers of uitsluitend de huidige gebruiker. (Systeembeheer rechten zijn noodzakelijk)"
LangString SecFileAssocDescription "${LYX_LANG}" "Associeer het LyX programma met de .lyx extensie."
LangString SecDesktopDescription "${LYX_LANG}" "Een ${PRODUCT_NAME} pictogram op het Bureaublad."

LangString ModifyingConfigureFailed "${LYX_LANG}" "Mislukte poging om 'path_prefix' te registreren tijdens de configuratie"
LangString RunConfigureFailed "${LYX_LANG}" "Mislukte configuratie poging"

LangString FinishPageMessage "${LYX_LANG}" "Gefeliciteerd! LyX is succesvol geinstalleerd."
LangString FinishPageRun "${LYX_LANG}" "Start LyX"

LangString DownloadPageField2 "${LYX_LANG}" "&Niet installeren"

LangString MinSYSHeader "${LYX_LANG}" "MinSYS"
LangString MinSYSDescription "${LYX_LANG}" "MinSYS (een minimale Unix script omgeving, www.mingw.org/msys.shtml) wordt gebruikt door ${PRODUCT_NAME} om een aantal taken uit te voeren."
LangString EnterMinSYSFolder "${LYX_LANG}" "Voer het pad in naar de map met sh.exe"
LangString InvalidMinSYSFolder "${LYX_LANG}" "sh.exe is niet gevonden"
LangString MinSYSDownloadLabel "${LYX_LANG}" "&Haal MinSYS op"
LangString MinSYSFolderLabel "${LYX_LANG}" "&Map met sh.exe"

LangString PythonHeader "${LYX_LANG}" "Python"
LangString PythonDescription "${LYX_LANG}" "Python (een script taal, www.python.org) moet worden geinstalleerd, anders is ${PRODUCT_NAME} niet in staat een aantal taken uit te voeren."
LangString EnterPythonFolder "${LYX_LANG}" "Voer het pad in naar de map met Python.exe"
LangString InvalidPythonFolder "${LYX_LANG}" "Python.exe is niet gevonden"
LangString PythonDownloadLabel "${LYX_LANG}" "&Haal Python op"
LangString PythonFolderLabel "${LYX_LANG}" "&Map met Python.exe"

LangString MiKTeXHeader "${LYX_LANG}" "MiKTeX"
LangString MiKTeXDescription "${LYX_LANG}" "MiKTeX (www.miktex.org) is een up-to-date TeX implementatie voor Windows."
LangString EnterMiKTeXFolder "${LYX_LANG}" "Voer het pad in naar de map met latex.exe"
LangString InvalidMiKTeXFolder "${LYX_LANG}" "latex.exe is niet gevonden"
LangString MiKTeXDownloadLabel "${LYX_LANG}" "&Haal MiKTeX op"
LangString MiKTeXFolderLabel "${LYX_LANG}" "&Map met latex.exe"

LangString PerlHeader "${LYX_LANG}" "Perl"
LangString PerlDescription "${LYX_LANG}" "Als u van plan bent  reLyX te gebruiken om LaTeX documenten naar LyX documenten te converteren, dan moet u Perl (www.perl.com) installeren."
LangString EnterPerlFolder "${LYX_LANG}" "Voer het pad in naar de map met Perl.exe"
LangString InvalidPerlFolder "${LYX_LANG}" "Perl.exe is niet gevonden"
LangString PerlDownloadLabel "${LYX_LANG}" "&Haal Perl op"
LangString PerlFolderLabel "${LYX_LANG}" "&Map met perl.exe"

LangString ImageMagickHeader "${LYX_LANG}" "ImageMagick"
LangString ImageMagickDescription "${LYX_LANG}" "ImageMagick (www.imagemagick.org/script/index.php) kan worden gebruikt om grafische bestanden te converteren (afhankelijk van gewenste output)."
LangString EnterImageMagickFolder "${LYX_LANG}" "Voer het pad in naar de map met convert.exe"
LangString InvalidImageMagickFolder "${LYX_LANG}" "convert.exe is niet gevonden"
LangString ImageMagickDownloadLabel "${LYX_LANG}" "&Haal ImageMagick op"
LangString ImageMagickFolderLabel "${LYX_LANG}" "&Map met convert.exe"

LangString GhostscriptHeader "${LYX_LANG}" "Ghostscript"
LangString GhostscriptDescription "${LYX_LANG}" "Ghostscript (http://www.cs.wisc.edu/~ghost/) wordt gebruikt om afbeeldingen van/naar PostScript te converteren."
LangString EnterGhostscriptFolder "${LYX_LANG}" "Voer het pad in naar de map met gswin32c.exe"
LangString InvalidGhostscriptFolder "${LYX_LANG}" "gswin32c.exe is niet gevonden"
LangString GhostscriptDownloadLabel "${LYX_LANG}" "&Haal Ghostscript op"
LangString GhostscriptFolderLabel "${LYX_LANG}" "&Map met gswin32c.exe"

LangString SummaryTitle "${LYX_LANG}" "Software samenvatting"
LangString SummaryPleaseInstall "${LYX_LANG}" "Installeer de opgehaalde bestanden en start vervolgens het LyX installatie programma nogmaals  om de installatie af te ronden."
LangString SummaryPathPrefix "${LYX_LANG}" "Ik voeg een 'path_prefix' tekst aan 'lyxrc.defaults' toe met:"

LangString UnNotInRegistryLabel "${LYX_LANG}" "$(^Name) is niet gevonden in het Windows register$\r$\nSnelkoppelingen op het Bureaublad en in het Start Menu worden niet verwijderd."
LangString UnNotAdminLabel "${LYX_LANG}" "Sorry! U heeft systeembeheer rechten nodig$\r$\nom $(^Name) te verwijderen."
LangString UnReallyRemoveLabel "${LYX_LANG}" "Weet u zeker dat u $(^Name) en alle componenten volledig wil verwijderen van deze computer?"
LangString UnRemoveSuccessLabel "${LYX_LANG}" "$(^Name) was successvol verwijderd van uw computer."

!undef LYX_LANG

!endif ; _LYX_LANGUAGES_DUTCH_NSH_
