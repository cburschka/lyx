!ifndef _LYX_LANGUAGES_DUTCH_NSH_
!define _LYX_LANGUAGES_DUTCH_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_DUTCH}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "Dit installatie programma zal LyX op uw systeem installeren.\r\n\
					  \r\n\
					  U heeft systeem-beheerrechten nodig om LyX te installeren.\r\n\
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
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript, een PostScript/PDF converteer programma"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick, een programma om afbeeldingen te converteren"
LangString MissProgAspell "${LYX_LANG}" "Aspell, een programma voor spellingscontrole"
LangString MissProgMessage "${LYX_LANG}" 'Aanvullende programma$\'s zijn niet vereist.'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'LaTeX software'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'Geef aan welke LaTeX-software LyX moet gebruiken.'
LangString EnterLaTeXFolder "${LYX_LANG}" 'U kunt hier opgeven in welke map het programma "latex.exe" zich bevindt en op deze manier bepalen \
					   welke LaTeX softare gebruikt wordt door LyX.\r\n\
					   Zonder LaTeX kan LyX geen documenten generen!\r\n\
					   \r\n\
					   Het installatie programma heeft de LaTeX software \
					   "$LaTeXName" op uw systeem gevonden in de volgende map.'
LangString EnterLaTeXFolderNone "${LYX_LANG}" 'U kunt hier opgeven in welke map het programma "latex.exe" zich bevindt en op deze manier bepalen \
					   welke LaTeX software gebruikt wordt door LyX.\r\n\
					   Zonder LaTeX kan LyX geen documenten generen!\r\n\
					       \r\n\
					       Het installatie programma heeft geen LaTeX software op uw systeem gevonden.'
LangString PathName "${LYX_LANG}" 'Map met het programma "latex.exe"'
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

LangString JabRefInfo "${LYX_LANG}" 'Het installatie programma van "JabRef" wordt nu gestart.$\r$\n\
				     U kunt de standaard opties gebruiken.'
LangString JabRefError "${LYX_LANG}" 'Het programma "JabRef" kon niet worden geinstalleerd!$\r$\n\
		                      De LyX installatie gaat desondanks verder.$\r$\n\
				      Probeer JabRef later te installeren.'
				      
LangString LatexConfigInfo "${LYX_LANG}" "De volgende configuratie van LyX zal enige tijd duren."

LangString AspellInfo "${LYX_LANG}" 'Er worden nu woordenlijst gedownload en geinstalleer voor het spellingscontrole programma "Aspell".$\r$\n\
				     Elke woordenlijst heeft een eigen licensie die zal worden getoond voor installatie.'
LangString AspellDownloadFailed "${LYX_LANG}" "Er kon geen Aspell woordenlijst worden gedownload!"
LangString AspellInstallFailed "${LYX_LANG}" "Er kon geen Aspell woordenlijst worden geinstalleerd!"
LangString AspellPartAnd "${LYX_LANG}" " en "
LangString AspellPartSep "${LYX_LANG}" ", "
LangString AspellAll "${LYX_LANG}" 'Woordenlijst reeds geinstalleerd voor: '
LangString AspellNone "${LYX_LANG}" 'Er is geen woordenlijst geinstalleerd voor het spellingscontrole programma "Aspell".$\r$\n\
				     Woordenlijsten kunnen worden gedownload van$\r$\n\
				     ${AspellLocation}$\r$\n\
				     Wilt u de woordenlijsten nu downloaden?'
LangString AspellPartStart "${LYX_LANG}" "Succesvolle installatie van "
LangString AspellPart1 "${LYX_LANG}" "een engelse woordenlijst"
LangString AspellPart2 "${LYX_LANG}" "een woordenlijst voor $LangName"
LangString AspellPart3 "${LYX_LANG}" "een$\r$\n\
				      woordenlijst voor $LangNameSys"
LangString AspellPart4 "${LYX_LANG}" ' voor het spellingscontrole programma "Aspell".$\r$\n\
				      Meer woordenlijsten kunnen worden gedownload van$\r$\n\
				      ${AspellLocation}$\r$\n\
				      Wilt u nu andere woordenlijsten downloaden?'

LangString MiKTeXPathInfo "${LYX_LANG}" "Opdat elke gebruiker zijn MiKTeX configuratie kan aanpassen$\r$\n\
					 is het noodzakelijk alle gebruikers schrijfpermissie te geven in MiKTeX's installatie map$\r$\n\
					 $MiKTeXPath $\r$\n\
					 en submappen."
LangString MiKTeXInfo "${LYX_LANG}" 'LyX gebruikt de LaTeX softare "MiKTeX".$\r$\n\
				     Het is aanbevolen MiKTeX-updates te installeren via de "MiKTeX Update Wizard"$\r$\n\
				     voordat u LyX voor de eerste keer gebruikt.$\r$\n\
				     Wilt u controleren of er updates voor MiKTeX beschikbaar zijn?'

LangString UpdateNotAllowed "${LYX_LANG}" "This update package can only update ${PRODUCT_VERSION_OLD}!"
LangString ModifyingConfigureFailed "${LYX_LANG}" "Mislukte poging om 'path_prefix' te registreren tijdens de configuratie"
LangString CreateCmdFilesFailed "${LYX_LANG}" "lyx.bat kon niet worden aangemaakt"
LangString RunConfigureFailed "${LYX_LANG}" "Mislukte configuratie poging"
LangString NotAdmin "${LYX_LANG}" "U heeft systeem-beheerrechten nodig om LyX te installeren!"
LangString InstallRunning "${LYX_LANG}" "Het installatieprogramma is al gestart!"
LangString StillInstalled "${LYX_LANG}" "LyX is reeds geinstalleerd! Verwijder LyX eerst."

LangString FinishPageMessage "${LYX_LANG}" "Gefeliciteerd! LyX is succesvol geinstalleerd.\r\n\
					    \r\n\
					    (De eerste keer dat u LyX start kan dit enige seconden duren.)"
LangString FinishPageRun "${LYX_LANG}" "Start LyX"

LangString UnNotInRegistryLabel "${LYX_LANG}" "LyX is niet gevonden in het Windows register.$\r$\n\
					       Snelkoppelingen op het Bureaublad en in het Start Menu worden niet verwijderd."
LangString UnInstallRunning "${LYX_LANG}" "U moet LyX eerst afsluiten!"
LangString UnNotAdminLabel "${LYX_LANG}" "U heeft systeem-beheerrechten nodig om LyX te verwijderen!"
LangString UnReallyRemoveLabel "${LYX_LANG}" "Weet u zeker dat u LyX en alle componenten volledig wil verwijderen van deze computer?"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'LyX$\'s user preferences'
LangString UnGSview "${LYX_LANG}" 'Klik op de "Uninstall" knop in het volgende venster om het$\r$\n\
				   Postscript en PDF leesprogramma "GSview" te verwijderen.'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "LyX was successvol verwijderd van uw computer."

LangString SecUnAspellDescription "${LYX_LANG}" "Verwijder het spellingscontrole programma Aspell en alle bijbehorende woordelijsten."
LangString SecUnMiKTeXDescription "${LYX_LANG}" "Verwijder de LaTeX software MiKTeX."
LangString SecUnJabRefDescription "${LYX_LANG}" "Verwijder de bibliografie manager JabRef."
LangString SecUnPreferencesDescription "${LYX_LANG}" 'Verwijder LyX$\'s configuratie map$\r$\n\
					   "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
					   voor alle gebruikers.'
LangString SecUnProgramFilesDescription "${LYX_LANG}" "Verwijder LyX en alle bijbehorende onderdelen."

!undef LYX_LANG

!endif # _LYX_LANGUAGES_DUTCH_NSH_
