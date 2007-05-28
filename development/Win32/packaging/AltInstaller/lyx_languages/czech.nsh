!ifndef _LYX_LANGUAGES_CZECH_NSH_
!define _LYX_LANGUAGES_CZECH_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_CZECH}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "Tento pomocník vás provede instalací LyXu.\r\n\
					  \r\n\
					  K instalaci LyXu budete potøebovat administrátorská práva.\r\n\
					  \r\n\
					  Je doporuèeno zavøít všechny ostatní aplikace pøed spuštìním instalace. Umožníte tím aktualizovat patøièné systémové soubory bez nutnosti restartovat poèítaè.\r\n\
					  \r\n\
					  $_CLICK"

LangString FileTypeTitle "${LYX_LANG}" "LyX-dokumentù"

LangString SecInstGSviewTitle "${LYX_LANG}" "GSview"
LangString SecInstJabRefTitle "${LYX_LANG}" "JabRef"
LangString SecAllUsersTitle "${LYX_LANG}" "Instalovat pro všechny uživatele?"
LangString SecFileAssocTitle "${LYX_LANG}" "Asociovat soubory"
LangString SecDesktopTitle "${LYX_LANG}" "Ikonu na plochu"

LangString SecCoreDescription "${LYX_LANG}" "Soubory LyXu."
LangString SecInstGSviewDescription "${LYX_LANG}" "Prohlížeè Postscriptu and PDF-dokumentù."
LangString SecInstJabRefDescription "${LYX_LANG}" "Správce pro bibliografii a editor souborù BibTeXu."
LangString SecAllUsersDescription "${LYX_LANG}" "Instalovat LyX pro všechny uživatele nebo pouze pro souèasného uživatele."
LangString SecFileAssocDescription "${LYX_LANG}" "Soubory s pøíponou .lyx se automaticky otevøou v LyXu."
LangString SecDesktopDescription "${LYX_LANG}" "Ikonu LyXu na plochu."

LangString LangSelectHeader "${LYX_LANG}" "Výbìr jazyka prostøedí LyXu"
LangString AvailableLang "${LYX_LANG}" " Dostupné jazyky "

LangString MissProgHeader "${LYX_LANG}" "Kontrola potøebných programù"
LangString MissProgCap "${LYX_LANG}" "Následující programy budou pøiinstalovány"
LangString MissProgLatex "${LYX_LANG}" "MiKTeX, distribuce LaTeXu"
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript, interpretr PostScriptu a PDF souborù"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick, konvertor obrázkù"
LangString MissProgAspell "${LYX_LANG}" "Aspell, kontrola pravopisu"
LangString MissProgMessage "${LYX_LANG}" 'Žádné další programy není tøeba pøiinstalovat.'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'Distribuce LaTeXu'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'Nastavte distribuci LaTeXu, kterou má LyX používat.'
LangString EnterLaTeXFolder "${LYX_LANG}" 'Mùžete zde volitelnì nastavit cestu k souboru "latex.exe" a tím urèit \
					   distribuci LaTeXu, kterou bude používat Lyx.\r\n\
					   Pokud nepoužijete LaTeX, LyX nemùže vysázet výsledné dokumenty!\r\n\
					   \r\n\
					   Instalátor nalezl na vašem poèítaèi LaTeXovou distribuci \
					   "$LaTeXName". Níže je uvedena její cesta.'
LangString EnterLaTeXFolderNone "${LYX_LANG}" 'Níže nastavte cestu k souboru "latex.exe". Urèíte tím, která \
					       distribuce LaTeXu bude používána LyXem.\r\n\
					       Pokud nepoužijete LaTeX, LyX nemùže vysázet výsledné dokumenty!\r\n\
					       \r\n\
					       Instalátor nemohl nalézt distribuci LaTeXu na vašem poèítaèi.'
LangString PathName "${LYX_LANG}" 'Cesta k souboru "latex.exe"'
LangString DontUseLaTeX "${LYX_LANG}" "Nepoužívat LaTeX"
LangString InvalidLaTeXFolder "${LYX_LANG}" 'Soubor "latex.exe" není v zadané cestì.'

LangString LatexInfo "${LYX_LANG}" 'Nyní bude spuštìn instalátor LaTeXové-distribuce "MiKTeX".$\r$\n\
			            V instalátoru pokraèujte tlaèítkem "Next" dokud instalace nezaène.$\r$\n\
				    $\r$\n\
				    !!! Užijte všechny pøedvolby instalátoru MiKTeX !!!'
LangString LatexError1 "${LYX_LANG}" 'Žádná LaTeXová-distribuce nebyla nalezena!$\r$\n\
                      		      Nemùžete používat LyX bez distribuce LaTeXu jako je napø. "MiKTeX"!$\r$\n\
				      Instalace bude proto pøerušena.'
			    
LangString GSviewInfo "${LYX_LANG}" 'Nyní bude spuštìn instalátor programu "GSview".$\r$\n\
			             K jeho instalaci zmáèknìte nejprve tlaèítko "Setup" v prvním oknì,$\r$\n\
				     vyberte jazyk a stisknìtì tlaèítko "Next" v následujícím oknì.$\r$\n\
				     Mùžete použít všechny pøedvolby tohoto instalátoru.'
LangString GSviewError "${LYX_LANG}" 'Program "GSview" nebyl úspìšnì nainstalován!$\r$\n\
		                      Instalátor bude navzdory tomu pokraèovat.$\r$\n\
				      Pokuste se nainstalovat GSview pozdìji.'
				      
LangString JabRefInfo "${LYX_LANG}" 'Nyní bude spuštìn instalátor programu "JabRef".$\r$\n\
				     Mùžete použít všechny pøedvolby instalátoru JabRef.'
LangString JabRefError "${LYX_LANG}" 'Program "JabRef" nebyl úspìšnì nainstalován.$\r$\n\
		                      Instalátor bude navzdory tomu pokraèovat.$\r$\n\
				      Pokuste se nainstalovat JabRef pozdìji.'
				      
LangString LatexConfigInfo "${LYX_LANG}" "Následující konfigurování LyXu chvíli potrvá."

LangString AspellInfo "${LYX_LANG}" 'Nyní budou staženy a nainstalovány slovníky pro kontrolu pravopisu programu "Aspell".$\r$\n\
				     Každý slovník má odlišnou licenci, která bude zobrazena pøed instalací.'
LangString AspellDownloadFailed "${LYX_LANG}" "Nelze stáhnout žádný slovníku Aspellu!"
LangString AspellInstallFailed "${LYX_LANG}" "Nelze nainstalovat žádný slovník Aspellu!"
LangString AspellPartAnd "${LYX_LANG}" " a "
LangString AspellPartSep "${LYX_LANG}" ", "
LangString AspellAll "${LYX_LANG}" 'Slovník jazyka je již naistalován'
LangString AspellNone "${LYX_LANG}" 'Nebyl nainstalován žádný slovník pro Aspell.$\r$\n\
				     Slovníky pro kontrolu pravopisu lze stáhnout z$\r$\n\
				     ${AspellLocation}$\r$\n\
				     Chcete je stáhnout nyní?'
LangString AspellPartStart "${LYX_LANG}" "Byl úspìšnì nainstalován"
LangString AspellPart1 "${LYX_LANG}" "anglický slovník"
LangString AspellPart2 "${LYX_LANG}" "slovník jazyka: $LangName"
LangString AspellPart3 "${LYX_LANG}" "a$\r$\n\
				      slovník jazyka: $LangNameSys"
LangString AspellPart4 "${LYX_LANG}" ' pro kontrolu pravopisu programem "Aspell".$\r$\n\
				      Více slovníkù mùže být staženo z$\r$\n\
				      ${AspellLocation}$\r$\n\
				      Chcete nyní stáhnout jiný slovník ?'

LangString MiKTeXPathInfo "${LYX_LANG}" "Aby každý uživatel mohl pozdìji pøizpùsobit MiKTeX pro své potøeby$\r$\n\
					 je tøeba nastavit práva zápisu pro všechny uživatele do adresáøe MiKTeXu$\r$\n\
					 $MiKTeXPath $\r$\n\
					 a všech jeho podadresáøù."
LangString MiKTeXInfo "${LYX_LANG}" 'LaTeXová distribuce "MiKTeX" bude použita dohromady s LyXem.$\r$\n\
				     Je doporuèeno nainstalovat dostupné aktualizace MiKTeXu pomocí programu "MiKTeX Update Wizard"$\r$\n\
				     pøedtím než poprvé spustíte LyX.$\r$\n\
				     Chcete zkontrolovat dostupné aktualizace MiKTeXu nyní?'
				     
LangString UpdateNotAllowed "${LYX_LANG}" "Tento aktualizaèní balíèek mùže být použít jen na ${PRODUCT_VERSION_OLD}!"
LangString ModifyingConfigureFailed "${LYX_LANG}" "Nelze nastavit 'path_prefix' v konfiguraèním skriptu"
LangString CreateCmdFilesFailed "${LYX_LANG}" "Nelze vytvoøit lyx.bat"
LangString RunConfigureFailed "${LYX_LANG}" "Nelze spustit konfiguraèní skript"
LangString NotAdmin "${LYX_LANG}" "Potøebujete administrátorská práva pro nainstalování LyXu!"
LangString InstallRunning "${LYX_LANG}" "Instalátor je již spuštìn!"
LangString StillInstalled "${LYX_LANG}" "LyX je již nainstalován! Nejprve LyX odinstalujte."

LangString FinishPageMessage "${LYX_LANG}" "Blahopøejeme! LyX byl úspìšnì nainstalován.\r\n\
					    \r\n\
					    (První spuštìní LyXu mùže trvat delší dobu.)"
LangString FinishPageRun "${LYX_LANG}" "Spustit LyX"

LangString UnNotInRegistryLabel "${LYX_LANG}" "Nelze nalézt LyX v registrech.$\r$\n\
					       Zástupce na ploše a ve Start menu nebude smazán."
LangString UnInstallRunning "${LYX_LANG}" "Nejprve musíte zavøít LyX!"
LangString UnNotAdminLabel "${LYX_LANG}" "Musíte mít administrátorská práva pro odinstalování LyXu!"
LangString UnReallyRemoveLabel "${LYX_LANG}" "Chcete opravdu smazat LyX a všechny jeho komponenty?"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'Uživatelská nastavení LyXu'
LangString UnGSview "${LYX_LANG}" 'Zvolte tlaèítko "Uninstall" v dalším oknì pro odinstalování$\r$\n\
				   prohlížeèe "GSview".'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "LyX byl úspìšnì odstranìn z vašeho poèítaèe."

LangString SecUnAspellDescription "${LYX_LANG}" 'Odinstalovat program pro kontrolu pravopisu Aspell a všechny jeho slovníky.'
LangString SecUnMiKTeXDescription "${LYX_LANG}" "Odinstalovat LaTeXovou-distribuci MiKTeX."
LangString SecUnJabRefDescription "${LYX_LANG}" "Odinstalovat manažer bibliografie JabRef."
LangString SecUnPreferencesDescription "${LYX_LANG}" 'Smazat konfiguraèní adresáø LyXu$\r$\n\
					   "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
					   pro všechny uživatele.'
LangString SecUnProgramFilesDescription "${LYX_LANG}" "Odinstalovat LyX a všechny jeho komponenty."

!undef LYX_LANG

!endif ; _LYX_LANGUAGES_CZECH_NSH_
