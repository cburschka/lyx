!ifndef _LYX_LANGUAGES_DANISH_NSH_
!define _LYX_LANGUAGES_DANISH_NSH_

!ifdef LYX_LANG
 !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_DANISH}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "Denne guide vil installere LyX på din computer.\r\n\
                                         \r\n\
                                         Du skal have administratorrettigheder for at installerer LyX.\r\n\
                                         \r\n\
                                         Det anbefales at du lukker alle kørende programmer inden start af setup-guiden. Dette vil tillade guiden at opdatere de nødvendige systemfiler uden at skulle genstarte computeren.\r\n\
                                         \r\n\
                                         $_CLICK"

LangString FileTypeTitle "${LYX_LANG}" "LyX-Document"

LangString SecInstGSviewTitle "${LYX_LANG}" "GSview"
LangString SecInstJabRefTitle "${LYX_LANG}" "JabRef"
LangString SecAllUsersTitle "${LYX_LANG}" "Installer til alle brugere?"
LangString SecFileAssocTitle "${LYX_LANG}" "Fil-associationer"
LangString SecDesktopTitle "${LYX_LANG}" "Skrivebordsikon"

LangString SecCoreDescription "${LYX_LANG}" "Filerne til LyX."
LangString SecInstGSviewDescription "${LYX_LANG}" "Program til gennemsyn af Postscript- og PDF-dokumenter."
LangString SecInstJabRefDescription "${LYX_LANG}" "Bibliography reference manager and editor for BibTeX files."
LangString SecAllUsersDescription "${LYX_LANG}" "Installer LyX til alle brugere, eller kun den aktuelle bruger."
LangString SecFileAssocDescription "${LYX_LANG}" "Opret association mellem LyX og .lyx filer."
LangString SecDesktopDescription "${LYX_LANG}" "Et LyX ikon på skrivebordet"

LangString LangSelectHeader "${LYX_LANG}" "Valg af sprog i LyX's menuer"
LangString AvailableLang "${LYX_LANG}" " Tilgængelige Sprog "

LangString MissProgHeader "${LYX_LANG}" "Tjek for nødvendige programmer"
LangString MissProgCap "${LYX_LANG}" "Følgende nødvendige program(mer) vil også blive installeret"
LangString MissProgLatex "${LYX_LANG}" "MiKTeX, en LaTeX distribution"
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript, en PostScript og PDF oversætter"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick, en billed-konverter"
LangString MissProgAspell "${LYX_LANG}" "Aspell, en stavekontrol"
LangString MissProgMessage "${LYX_LANG}" 'Ingen andre programmer behøves.'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'LaTeX-distribution'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'Vælg den LaTeX-distribution som LyX skal bruge.'
LangString EnterLaTeXFolder "${LYX_LANG}" 'You can optionally set here the path to the file "latex.exe" and therewith set the \
					   LaTeX-distribution that should be used by LyX.\r\n\
					   If you don$\'t use LaTeX, LyX cannot output documents!\r\n\
					   \r\n\
					   The installer has detected the LaTeX-distribution \
					   "$LaTeXName" on your system. Displayed below is its path.'
LangString EnterLaTeXFolderNone "${LYX_LANG}" 'Set below the path to the file "latex.exe". Therewith you set which \
					       LaTeX-distribution should be used by LyX.\r\n\
					       If you don$\'t use LaTeX, LyX cannot output documents!\r\n\
					       \r\n\
					       The installer couldn$\'t find a LaTeX-distribution on your system.'
LangString PathName "${LYX_LANG}" 'Path to the file "latex.exe"'
LangString DontUseLaTeX "${LYX_LANG}" "Brug ikke LaTeX"
LangString InvalidLaTeXFolder "${LYX_LANG}" 'Kunne ikke finde "latex.exe".'

LangString LatexInfo "${LYX_LANG}" 'Installationen af LaTeX-distributionen "MiKTeX" startes.$\r$\n\
                                   For at installere programmet tryk på "Next"-knappen i installationsvinduet indtil installationen begynder.$\r$\n\
                                   $\r$\n\
                                   !!! Brug alle MiKTeX installationsprogrammets foreslåede valg !!!'
LangString LatexError1 "${LYX_LANG}" 'Ingen LaTeX-distribution fundet!$\r$\n\
                                     LyX kan ikke bruges uden en LaTeX-distribution såsom "MiKTeX"!$\r$\n\
                                     Installationen afbrydes.'

LangString GSviewInfo "${LYX_LANG}" 'Installationen af programmet "GSview" startes.$\r$\n\
                                    For at installere programmet tryk på "Setup"-knappen i det første installationsvindue$\r$\n\
                                    vælg sprog og tryk på "Next"-knappen i det efterfølgende installationsvindue.$\r$\n\
                                    Du kan bruge alle GSview installationsprogrammets foreslåede valg.'
LangString GSviewError "${LYX_LANG}" 'Programmet "GSview" kunne ikke installeres!$\r$\n\
                                     Installationen af LyX fortsættes alligevel.$\r$\n\
                                     Prøv at installere GSview på et senere tidspunkt.'

LangString JabRefInfo "${LYX_LANG}" 'Now the installer of the program "JabRef" will be launched.$\r$\n\
				     You can use all default options of the JabRef-installer.'
LangString JabRefError "${LYX_LANG}" 'The program "JabRef" could not successfully be installed!$\r$\n\
		                      The installer will continue anyway.$\r$\n\
				      Try to install JabRef again later.'

LangString LatexConfigInfo "${LYX_LANG}" "Den følgende konfiguration af LyX vil tage et stykke tid."

LangString AspellInfo "${LYX_LANG}" 'Now dictionaries for the spellchecker "Aspell" will be downloaded and installed.$\r$\n\
				     Every dictionary has a different license that will be displayed before the installation.'
LangString AspellDownloadFailed "${LYX_LANG}" "No Aspell spellchecker dictionary could be downloaded!"
LangString AspellInstallFailed "${LYX_LANG}" "No Aspell spellchecker dictionary could be installed!"
LangString AspellPartAnd "${LYX_LANG}" " and "
LangString AspellPartSep "${LYX_LANG}" ", "
LangString AspellAll "${LYX_LANG}" 'There is already installed a dictionary of the language'
LangString AspellNone "${LYX_LANG}" 'No dictionary for the spellchecker "Aspell" has been installed.$\r$\n\
				     Dictionaries can be downloaded from$\r$\n\
				     ${AspellLocation}$\r$\n\
				     Do you want to download dictionaries now?'
LangString AspellPartStart "${LYX_LANG}" "There was successfully installed "
LangString AspellPart1 "${LYX_LANG}" "an english dictionary"
LangString AspellPart2 "${LYX_LANG}" "a dictionary of the language $LangName"
LangString AspellPart3 "${LYX_LANG}" "a$\r$\n\
				      dictionary of the language $LangNameSys"
LangString AspellPart4 "${LYX_LANG}" ' for the spellchecker "Aspell".$\r$\n\
				      More dictionaries can be downloaded from$\r$\n\
				      ${AspellLocation}$\r$\n\
				      Do you want to download other dictionaries now?'

LangString MiKTeXPathInfo "${LYX_LANG}" "In order that every user is later able to customize MiKTeX for his needs$\r$\n\
					 it is necessary to set write permissions for all users to MiKTeX's installation folder$\r$\n\
					 $MiKTeXPath $\r$\n\
					 and its subfolders."
LangString MiKTeXInfo "${LYX_LANG}" 'The LaTeX-distribution "MiKTeX" will be used together with LyX.$\r$\n\
				     It is recommended to install available MiKTeX-updates using the program "MiKTeX Update Wizard"$\r$\n\
				     before you use LyX for the first time.$\r$\n\
				     Would you now check for MiKTeX updates?'

LangString ModifyingConfigureFailed "${LYX_LANG}" "Forsøget på at indstille 'path_prefix' i konfigurationen mislykkedes"
LangString CreateCmdFilesFailed "${LYX_LANG}" "Kunne ikke oprette lyx.bat"
LangString RunConfigureFailed "${LYX_LANG}" "Mislykket forsøg på at afvikle konfigurations-scriptet"
LangString NotAdmin "${LYX_LANG}" "Du skal have administrator-rettigheder for at installere LyX!"
LangString InstallRunning "${LYX_LANG}" "Installationsprogrammet kører allerede!"
LangString StillInstalled "${LYX_LANG}" "LyX er allerede installeret! Afinstaller LyX først."

LangString FinishPageMessage "${LYX_LANG}" "Tillykke!! LyX er installeret.\r\n\
                                           \r\n\
                                           (Når LyX startes første gang, kan det tage noget tid.)"
LangString FinishPageRun "${LYX_LANG}" "Start LyX"

LangString UnNotInRegistryLabel "${LYX_LANG}" "Kunne ikke finde LyX i registreringsdatabsen.$\r$\n\
                                              Genvejene på skrivebordet og i Start-menuen bliver ikke fjernet"
LangString UnInstallRunning "${LYX_LANG}" "Du må afslutte LyX først!"
LangString UnNotAdminLabel "${LYX_LANG}" "Du skal have administrator-rettigheder for at afinstallere LyX!"
LangString UnReallyRemoveLabel "${LYX_LANG}" "Er du sikker på, at du vil slette LyX og alle tilhørende komponenter?"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'LyX$\'s user preferences'
LangString UnGSview "${LYX_LANG}" 'Vær så god at trykke på "Uninstall"-knappen i det næste vindue for at afinstallere$\r$\n\
                                  Postscript and PDF-programmet "GSview".'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "LyX er blevet fjernet fra din computer."

LangString SecUnAspellDescription "${LYX_LANG}" "Uninstalls the spellchecker Aspell and all of its dictionaries."
LangString SecUnMiKTeXDescription "${LYX_LANG}" "Uninstalls the LaTeX-distribution MiKTeX."
LangString SecUnJabRefDescription "${LYX_LANG}" "Uninstalls the bibliography manager JabRef."
LangString SecUnPreferencesDescription "${LYX_LANG}" 'Deletes LyX$\'s configuration folder$\r$\n\
					   "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
					   for all users.'
LangString SecUnProgramFilesDescription "${LYX_LANG}" "Uninstall LyX and all of its components."

!undef LYX_LANG

!endif ; _LYX_LANGUAGES_DANISH_NSH_
