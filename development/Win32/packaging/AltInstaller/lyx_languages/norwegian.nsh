!ifndef _LYX_LANGUAGES_NORWEGIAN_NSH_
!define _LYX_LANGUAGES_NORWEGIAN_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_NORWEGIAN}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "Denne veiviseren installerer LyX på datamaskinen din.\r\n\
					  \r\n\
					  Du trenger administratorprivilegier for å installere LyX.\r\n\
					  \r\n\
					  Du bør lukke alle andre programmer først, dermed kan installasjonsprogrammet oppdatere relevante systemfiler uten å måtte restarte maskinen.\r\n\
					  \r\n\
					  $_CLICK"

LangString FileTypeTitle "${LYX_LANG}" "LyX-Document"

LangString SecInstGSviewTitle "${LYX_LANG}" "GSview"
LangString SecInstJabRefTitle "${LYX_LANG}" "JabRef"
LangString SecAllUsersTitle "${LYX_LANG}" "Installere for alle brukere?"
LangString SecFileAssocTitle "${LYX_LANG}" "Fil-assosiasjoner"
LangString SecDesktopTitle "${LYX_LANG}" "Skrivebordsikon"

LangString SecCoreDescription "${LYX_LANG}" "LyX-filene."
LangString SecInstGSviewDescription "${LYX_LANG}" "Program for å vise Postscript- og PDF-dokumenter."
LangString SecInstJabRefDescription "${LYX_LANG}" "Bibliography reference manager and editor for BibTeX files."
LangString SecAllUsersDescription "${LYX_LANG}" "Installere LyX for alle brukere, eller kun den aktuelle brukeren."
LangString SecFileAssocDescription "${LYX_LANG}" "Files med endelsen .lyx åpnes automatisk i LyX."
LangString SecDesktopDescription "${LYX_LANG}" "Et LyX-ikon på skrivebordet."

LangString LangSelectHeader "${LYX_LANG}" "Utvalg av LyX's menyspråk"
LangString AvailableLang "${LYX_LANG}" " Tilgjengelige Språk "

LangString MissProgHeader "${LYX_LANG}" "Kryss av for nødvendige programmer"
LangString MissProgCap "${LYX_LANG}" "Følgende programmer installeres i tillegg"
LangString MissProgLatex "${LYX_LANG}" "MiKTeX, en LaTeX-distribusjon"
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript, en interpreter for PostScript og PDF"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick, konverterer bilder"
LangString MissProgAspell "${LYX_LANG}" "Aspell, stavesjekking"
LangString MissProgMessage "${LYX_LANG}" 'Ingen ekstra programmer trenger installeres'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'LaTeX-distribusjon'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'Velg LaTeX-distribusjonen LyX skal bruke.'
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
LangString DontUseLaTeX "${LYX_LANG}" "Ikke bruk LaTeX"
LangString InvalidLaTeXFolder "${LYX_LANG}" 'Filen "latex.exe" fins ikke i den oppgitte mappa.'

LangString LatexInfo "${LYX_LANG}" 'Nå starter installasjon av LaTeX-distribusjonen "MiKTeX"$\r$\n\
			            For å installere programmet, klikk "Neste"-knappen i installasjonsveiviseren til installasjonen begynner.$\r$\n\
				    $\r$\n\
				    !!! Vennligst bruk standardopsjonene for MiKTeX-instasjonen !!!'
LangString LatexError1 "${LYX_LANG}" 'Fant ikke noen LaTeX-distribusjon!$\r$\n\
                      		      LyX kan ikke brukes uten en LaTeX-distribusjion som "MiKTeX"!$\r$\n\
				      Derfor avbrytes installasjonen.'
		    
LangString GSviewInfo "${LYX_LANG}" 'Installasjonsveiviser for programmet "GSview" kjøres i gang.$\r$\n\
			             For å installere programmet, klikk "Setup"-knappen i det første installasjonsvinduet$\r$\n\
				     velg et språk, og klikk deretter "Neste"-knappen i det følgende installasjonsvinduet.$\r$\n\
				     Du kan bruke alle standardvalg for GSview-veiviseren.'
LangString GSviewError "${LYX_LANG}" 'Programmet "GSview" kunne ikke installeres!$\r$\n\
		                      Installasjonen fortsetter likevel.$\r$\n\
				      Prøv å installere GSview omigjen senere.'

LangString JabRefInfo "${LYX_LANG}" 'Now the installer of the program "JabRef" will be launched.$\r$\n\
				     You can use all default options of the JabRef-installer.'
LangString JabRefError "${LYX_LANG}" 'The program "JabRef" could not successfully be installed!$\r$\n\
		                      The installer will continue anyway.$\r$\n\
				      Try to install JabRef again later.'
				      
LangString LatexConfigInfo "${LYX_LANG}" "Konfigurasjon av LyX vil ta en stund."

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

LangString UpdateNotAllowed "${LYX_LANG}" "This update package can only update LyX ${PRODUCT_VERSION_OLD}!"				     
LangString ModifyingConfigureFailed "${LYX_LANG}" "Forsøket på å stille inn 'path_prefix' i konfigurasjonsscriptet mislyktes"
LangString CreateCmdFilesFailed "${LYX_LANG}" "Fikk ikke opprettet lyx.bat"
LangString RunConfigureFailed "${LYX_LANG}" "Fikk ikke kjørt konfigurasjonsscriptet"
LangString NotAdmin "${LYX_LANG}" "Du trenger administratorrettigheter for å installere LyX!"
LangString InstallRunning "${LYX_LANG}" "Installasjonsprogrammet er allerede i gang!"
LangString StillInstalled "${LYX_LANG}" "LyX er allerede installert! Fjern LyX først."

LangString FinishPageMessage "${LYX_LANG}" "Gratulerer!! LyX er installert.\r\n\
					    \r\n\
					    (Første gangs oppstart av LyX kan ta noen sekunder.)"
LangString FinishPageRun "${LYX_LANG}" "Start LyX"

LangString UnNotInRegistryLabel "${LYX_LANG}" "Fant ikke LyX i registeret.$\r$\n\
					       Snarveier på skrivebordet og i startmenyen fjernes ikke."
LangString UnInstallRunning "${LYX_LANG}" "Du må avslutte LyX først!"
LangString UnNotAdminLabel "${LYX_LANG}" "Du må ha administratorrettigheter for å fjerne LyX!"
LangString UnReallyRemoveLabel "${LYX_LANG}" "Er du sikker på at du vil fjerne LyX og alle tilhørende komponenter?"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'LyX$\'s user preferences'
LangString UnGSview "${LYX_LANG}" 'Vennligst klikk "Uninstall"-knappen i neste vindu for å fjerne$\r$\n\
				   Postscript and PDF-leseren "GSview".'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "LyX ble fjernet fra din datamaskin."

LangString SecUnAspellDescription "${LYX_LANG}" "Uninstalls the spellchecker Aspell and all of its dictionaries."
LangString SecUnMiKTeXDescription "${LYX_LANG}" "Uninstalls the LaTeX-distribution MiKTeX."
LangString SecUnJabRefDescription "${LYX_LANG}" "Uninstalls the bibliography manager JabRef."
LangString SecUnPreferencesDescription "${LYX_LANG}" 'Deletes LyX$\'s configuration folder$\r$\n\
					   "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
					   for all users.'
LangString SecUnProgramFilesDescription "${LYX_LANG}" "Uninstall LyX and all of its components."

!undef LYX_LANG

!endif # _LYX_LANGUAGES_NORWEGIAN_NSH_
