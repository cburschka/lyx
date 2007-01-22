!ifndef _LYX_LANGUAGES_DUTCH_NSH_
!define _LYX_LANGUAGES_DUTCH_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_DUTCH}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "Dit installatie programma zal LyX op uw systeem installeren.\r\n\
					  \r\n\
					  U heeft systeembeheer nodig om LyX te installeren.\r\n\
					  \r\n\
					  Het is aanbevolen om alle andere programma's af te sluiten voordat u Setup start. Dit zorgt ervoor dat Setup bepaalde systeembestanden kan bijwerken zonder uw systeem opnieuw op te starten.\r\n\
					  \r\n\
					  $_CLICK"

LangString FileTypeTitle "${LYX_LANG}" "LyX-Document"

LangString SecInstGSviewTitle "${LYX_LANG}" "GSview"
LangString SecInstJabRefTitle "${LYX_LANG}" "JabRef"
LangString SecAllUsersTitle "${LYX_LANG}" "Installeer voor alle gebruikers?"
LangString SecFileAssocTitle "${LYX_LANG}" "Bestand associaties"
LangString SecDesktopTitle "${LYX_LANG}" "Bureaublad pictogram"

LangString SecCoreDescription "${LYX_LANG}" "De LyX bestanden."
LangString SecInstGSviewDescription "${LYX_LANG}" "Programma dat gebruikt wordt om Postscript en PDF documenten te lezen."
LangString SecInstJabRefDescription "${LYX_LANG}" "Bibliography reference manager and editor for BibTeX files."
LangString SecAllUsersDescription "${LYX_LANG}" "Installeer LyX voor alle gebruikers of uitsluitend de huidige gebruiker?"
LangString SecFileAssocDescription "${LYX_LANG}" "Associeer het LyX programma met de .lyx extensie."
LangString SecDesktopDescription "${LYX_LANG}" "Een LyX pictogram op het Bureaublad."

LangString LangSelectHeader "${LYX_LANG}" "Taalselectie voor LyX's menu"
LangString AvailableLang "${LYX_LANG}" " Beschikbare Talen "

LangString MissProgHeader "${LYX_LANG}" "Controleer welke programma's nodig zijn"
LangString MissProgCap "${LYX_LANG}" "De volgende aanvullende programmma's zijn vereist, en zullen worden geinstalleerd"
LangString MissProgLatex "${LYX_LANG}" "MiKTeX, LaTeX software"
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript, een PostScript/PDF conversie programma"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick, een grafische conversie programma"
LangString MissProgAspell "${LYX_LANG}" "Aspell, een programma voor spellingscontrole"
LangString MissProgMessage "${LYX_LANG}" 'Aanvullende programma$\'s zijn niet vereist.'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'LaTeX software'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'Geef aan welke LaTeX-software LyX moet gebruiken.'
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
LangString DontUseLaTeX "${LYX_LANG}" "Gebruik geen LaTeX"
LangString InvalidLaTeXFolder "${LYX_LANG}" '"latex.exe" is niet gevonden.'

LangString LatexInfo "${LYX_LANG}" 'Het installatieprogramma van "MiKTeX" word gestart.$\r$\n\
			            Om het programma te installeren klik op de "Next"-knop in het installatie venster totdat de installatie begint.$\r$\n\
				    $\r$\n\
				    !!! Gebruik de standaard opties tijdens de installatie van MiKTeX !!!'
LangString LatexError1 "${LYX_LANG}" 'Er is geen LaTeX software gevonden!$\r$\n\
                      		      LyX kan niet worden gebruikt zonder LaTeX software zoals "MiKTeX"!$\r$\n\
				      De installatie wordt daarom afgebroken.'
		    
LangString GSviewInfo "${LYX_LANG}" 'Het installatieprogramma van "GSview" wordt gestart.$\r$\n\
			             Klik op de "Setup"-knop in het eerste venster van het installatie programma om de software te installeren$\r$\n\
				     kies een taal, en klik daarna op de "Next"-knop in de volgende vensters van het installatie programma.$\r$\n\
				     U kunt alle standaard opties van het GSview installatie programma gebruiken.'
LangString GSviewError "${LYX_LANG}" 'Het programma "GSview" kon niet worden geinstalleerd!$\r$\n\
		                      De installatie van LyX gaat echter door.$\r$\n\
				      Probeer GSview later te installeren.'

LangString JabRefInfo "${LYX_LANG}" 'Now the installer of the program "JabRef" will be launched.$\r$\n\
				     You can use all default options of the JabRef-installer.'
LangString JabRefError "${LYX_LANG}" 'The program "JabRef" could not successfully be installed!$\r$\n\
		                      The installer will continue anyway.$\r$\n\
				      Try to install JabRef again later.'
				      
LangString LatexConfigInfo "${LYX_LANG}" "De volgende configuratie van LyX zal enige tijd duren."

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
				     
LangString ModifyingConfigureFailed "${LYX_LANG}" "Mislukte poging om 'path_prefix' te registreren tijdens de configuratie"
LangString CreateCmdFilesFailed "${LYX_LANG}" "lyx.bat kon niet worden aangemaakt"
LangString RunConfigureFailed "${LYX_LANG}" "Mislukte configuratie poging"
LangString NotAdmin "${LYX_LANG}" "U heeft systeembeheerrechten nodig om LyX te installeren!"
LangString InstallRunning "${LYX_LANG}" "Het installatieprogramma is al gestart!"
LangString StillInstalled "${LYX_LANG}" "LyX is reeds geinstalleerd! Verwijder LyX eerst."

LangString FinishPageMessage "${LYX_LANG}" "Gefeliciteerd! LyX is succesvol geinstalleerd.\r\n\
					    \r\n\
					    (De eerste keer dat u LyX start kan dit enige seconden duren.)"
LangString FinishPageRun "${LYX_LANG}" "Start LyX"

LangString UnNotInRegistryLabel "${LYX_LANG}" "LyX is niet gevonden in het Windows register.$\r$\n\
					       Snelkoppelingen op het Bureaublad en in het Start Menu worden niet verwijderd."
LangString UnInstallRunning "${LYX_LANG}" "U moet LyX eerst afsluiten!"
LangString UnNotAdminLabel "${LYX_LANG}" "U heeft systeembeheer rechten nodig om LyX te verwijderen!"
LangString UnReallyRemoveLabel "${LYX_LANG}" "Weet u zeker dat u LyX en alle componenten volledig wil verwijderen van deze computer?"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'LyX$\'s user preferences'
LangString UnGSview "${LYX_LANG}" 'Klik op de "Uninstall" knop in het volgende venster om het$\r$\n\
				   Postscript en PDF leesprogramma "GSview" te verwijderen.'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "LyX was successvol verwijderd van uw computer."

LangString SecUnAspellDescription "${LYX_LANG}" "Uninstalls the spellchecker Aspell and all of its dictionaries."
LangString SecUnMiKTeXDescription "${LYX_LANG}" "Uninstalls the LaTeX-distribution MiKTeX."
LangString SecUnJabRefDescription "${LYX_LANG}" "Uninstalls the bibliography manager JabRef."
LangString SecUnPreferencesDescription "${LYX_LANG}" 'Deletes LyX$\'s configuration folder$\r$\n\
					   "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
					   for all users.'
LangString SecUnProgramFilesDescription "${LYX_LANG}" "Uninstall LyX and all of its components."

!undef LYX_LANG

!endif ; _LYX_LANGUAGES_DUTCH_NSH_
