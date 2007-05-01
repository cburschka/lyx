!ifndef _LYX_LANGUAGES_POLISH_NSH_
!define _LYX_LANGUAGES_POLISH_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_POLISH}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "Kreator przeprowadzi Ciebie przez proces instalacji LyX-a.\r\n\
					  \r\n\
					  Wymagane s¹ prawa administratora aby zainstalowaæ LyX-a.\r\n\
					  \r\n\
					  Zalecane jest pozamykanie wszystkich innych aplikacji przed uruchomieniem programu instalacyjnego. Pozwoli to zaktualizowaæ pliki systemowe bez wymagania restartu komputera.\r\n\
					  \r\n\
					  $_CLICK"

LangString FileTypeTitle "${LYX_LANG}" "LyX-Document"

LangString SecInstGSviewTitle "${LYX_LANG}" "GSview"
LangString SecInstJabRefTitle "${LYX_LANG}" "JabRef"
LangString SecAllUsersTitle "${LYX_LANG}" "Instalacja dla wszystkich u¿ytkowników?"
LangString SecFileAssocTitle "${LYX_LANG}" "Skojarzenie plików .lyx"
LangString SecDesktopTitle "${LYX_LANG}" "Ikona na pulpicie"

LangString SecCoreDescription "${LYX_LANG}" "Pliki LyX-a."
LangString SecInstGSviewDescription "${LYX_LANG}" "Program do podgl¹du dokumentów Postscript i PDF."
LangString SecInstJabRefDescription "${LYX_LANG}" "Bibliography reference manager and editor for BibTeX files."
LangString SecAllUsersDescription "${LYX_LANG}" "Instalacja dla wszystkich u¿ytkowników lub tylko dla bie¿¹cego u¿ytkownika."
LangString SecFileAssocDescription "${LYX_LANG}" "Skojarzenie LyX-a z plikami o rozszerzeniu .lyx."
LangString SecDesktopDescription "${LYX_LANG}" "Ikona LyX-a na pulpicie."

LangString LangSelectHeader "${LYX_LANG}" "Wybór jêzyka menu LyX-a"
LangString AvailableLang "${LYX_LANG}" " Dostêpne jêzyki "

LangString MissProgHeader "${LYX_LANG}" "Sprawdzenie zainstalowania wymaganych programów"
LangString MissProgCap "${LYX_LANG}" "Zostan¹ dodatkowo zainstalowane nastêpuj¹ce wymagane programy"
LangString MissProgLatex "${LYX_LANG}" "MiKTeX, dystrybucja LaTeX-a"
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript, interpreter PostScript i PDF"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick, konwerter rysunków"
LangString MissProgAspell "${LYX_LANG}" "Aspell, kontrola pisowni"
LangString MissProgMessage "${LYX_LANG}" 'Brak dodatkowych programów do instalacji.'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'Dystrybucja LaTeX-a'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'Wybierz dystrybucjê LaTeX-a, któr¹ chcesz u¿ywaæ z LyX-em.'
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
LangString DontUseLaTeX "${LYX_LANG}" "Nie u¿ywaj LaTeX-a"
LangString InvalidLaTeXFolder "${LYX_LANG}" 'Plik "latex.exe" nie znajduje siê w podanej œcie¿ce.'

LangString LatexInfo "${LYX_LANG}" 'Teraz zostanie uruchomiony instalator dystrybucji LaTeX-a "MiKTeX".$\r$\n\
			            Aby zainstalowaæ ten program naciœnij przycisk "Dalej" w oknie instalatora.$\r$\n\
				    $\r$\n\
				    !!! Proszê u¿yæ wszystkich domyœlnych opcji instalatora MiKTeX-a !!!'
LangString LatexError1 "${LYX_LANG}" 'Nie znaleziono dystrybucji LaTeX-a!$\r$\n\
                      		      LyX nie mo¿e byæ u¿yty bez dystrybucji LaTeX-a takiej jak "MiKTeX"!$\r$\n\
				      Z tego powodu instalacja zostanie przerwana.'
		
LangString GSviewInfo "${LYX_LANG}" 'Teraz zostanie uruchomiony program instalacyjny "GSview".$\r$\n\
			             Aby go zainstalowaæ naciœnij przycisk "Setup" w pierwszym oknie instalatora,$\r$\n\
				     w kolejnym oknie wybierz jêzyk i przyciœnij przycisk "Next".$\r$\n\
				     Mo¿esz u¿yæ wszystkich domyœlnych opcji instalacji.'
LangString GSviewError "${LYX_LANG}" 'Program "GSview" nie zosta³ pomyœlnie zainstalowany!$\r$\n\
		                      Mimo to instalacja bêdzie kontynuowana.$\r$\n\
				      Spróbuj póŸniej zainstalowaæ program GSview.'

LangString JabRefInfo "${LYX_LANG}" 'Now the installer of the program "JabRef" will be launched.$\r$\n\
				     You can use all default options of the JabRef-installer.'
LangString JabRefError "${LYX_LANG}" 'The program "JabRef" could not successfully be installed!$\r$\n\
		                      The installer will continue anyway.$\r$\n\
				      Try to install JabRef again later.'

LangString LatexConfigInfo "${LYX_LANG}" "Dalsza konfiguracja LyX-a chwilê potrwa."

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

LangString UpdateNotAllowed "${LYX_LANG}" "This update package can only update ${PRODUCT_VERSION_OLD}!"
LangString ModifyingConfigureFailed "${LYX_LANG}" "Nieudana próba ustawienia zmiennej 'path_prefix' w pliku skryptu konfigracyjnego"
LangString CreateCmdFilesFailed "${LYX_LANG}" "Niedana próba utworzenia plików lyx.bat"
LangString RunConfigureFailed "${LYX_LANG}" "Niedana próba wykonania skryptu konfiguracyjnego"
LangString NotAdmin "${LYX_LANG}" "Musisz mieæ prawa administratora aby zainstalowaæ LyX-a!"
LangString InstallRunning "${LYX_LANG}" "Instalator jest ju¿ uruchomiony!"
LangString StillInstalled "${LYX_LANG}" "LyX jest ju¿ zainstalowany! Aby kontynowaæ musisz go najpierw usun¹æ."

LangString FinishPageMessage "${LYX_LANG}" "Gratulacje! LyX zosta³ pomyœlnie zainstalowany.\r\n\
					    \r\n\
					    (Pierwsze uruchomienie mo¿e potrwaæ kilka sekund.)"
LangString FinishPageRun "${LYX_LANG}" "Uruchom LyX-a"

LangString UnNotInRegistryLabel "${LYX_LANG}" "Nie mo¿na znaleŸæ LyX-a w rejestrze.$\r$\n\
	                                       Skróty na pulpicie i w menu Start nie zostan¹ usuniête."
LangString UnInstallRunning "${LYX_LANG}" "Musisz najpierw zamkn¹æ LyX-a!"
LangString UnNotAdminLabel "${LYX_LANG}" "Musisz posiadaæ prawa administratora do deinstalacji programu LyX."
LangString UnReallyRemoveLabel "${LYX_LANG}" "Czy na pewno chcesz usun¹æ LyX-a i wszystkie jego komponenty?"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'LyX$\'s user preferences'
LangString UnGSview "${LYX_LANG}" 'Naciœnij przycisk "Odinstaluj" w nastêpnym oknie aby odinstalowaæ$\r$\n\
				   przegl¹darkê Postscript i PDF "GSview".'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "LyX zosta³ pomyœlnie usuniêty z Twojego komputera."

LangString SecUnAspellDescription "${LYX_LANG}" "Uninstalls the spellchecker Aspell and all of its dictionaries."
LangString SecUnMiKTeXDescription "${LYX_LANG}" "Uninstalls the LaTeX-distribution MiKTeX."
LangString SecUnJabRefDescription "${LYX_LANG}" "Uninstalls the bibliography manager JabRef."
LangString SecUnPreferencesDescription "${LYX_LANG}" 'Deletes LyX$\'s configuration folder$\r$\n\
					   "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
					   for all users.'
LangString SecUnProgramFilesDescription "${LYX_LANG}" "Uninstall LyX and all of its components."

!undef LYX_LANG

!endif # _LYX_LANGUAGES_POLISH_NSH_















