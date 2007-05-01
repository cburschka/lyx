!ifndef _LYX_LANGUAGES_SLOVAK_NSH_
!define _LYX_LANGUAGES_SLOVAK_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_SLOVAK}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "Tento sprievodca Vám pomáha inštalova LyX.\r\n\
					  \r\n\
					  Pre inštaláciu LyX potrebujete administrátorské práva.\r\n\
					  \r\n\
					  Odporúèuje sa zavrie všetkı ostatné aplikácie pred štartom Setup. Tım sa umoòuje nahradi relevantné dáta sústavy bez nového štartu poèitaèe.\r\n\
					  \r\n\
					  $_CLICK"

LangString FileTypeTitle "${LYX_LANG}" "LyX dokument"

LangString SecInstGSviewTitle "${LYX_LANG}" "GSview"
LangString SecInstJabRefTitle "${LYX_LANG}" "JabRef"
LangString SecAllUsersTitle "${LYX_LANG}" "Inštalova pre všetkıch uívate¾ov?"
LangString SecFileAssocTitle "${LYX_LANG}" "Usporiadanie data"
LangString SecDesktopTitle "${LYX_LANG}" "Symbol desktopu"

LangString SecCoreDescription "${LYX_LANG}" "Program LyX."
LangString SecInstGSviewDescription "${LYX_LANG}" "Program na pozeranie dokumentov Postscript a PDF."
LangString SecInstJabRefDescription "${LYX_LANG}" "Manager pre bibliografickı odkaz a editor pre BibTeX files."
LangString SecAllUsersDescription "${LYX_LANG}" "Inštalova LyX pre všetkıch uívatelov alebo len pre terajší uívate¾."
LangString SecFileAssocDescription "${LYX_LANG}" "Data s lyx rozšírením sa automaticky otvárajú v LyX."
LangString SecDesktopDescription "${LYX_LANG}" "Symbol LyX na desktopu."

LangString LangSelectHeader "${LYX_LANG}" "Vyber jazyka pre LyX"
LangString AvailableLang "${LYX_LANG}" " Pohotové jazyky "

LangString MissProgHeader "${LYX_LANG}" "Hladajte vyadované programy"
LangString MissProgCap "${LYX_LANG}" "Nasledujúce vyadované programy sa inštalujú dodatocne"
LangString MissProgLatex "${LYX_LANG}" "MiKTeX, je LaTeX distribúcia"
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript, je interpretátor pre PostScript a PDF"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick, konvertuje obrazy"
LangString MissProgAspell "${LYX_LANG}" "Aspell, je overovanie pravopisu"
LangString MissProgMessage "${LYX_LANG}" 'Netreba inštalova ïalšie programy.'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'LaTeX distribúcia'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'Urcujte LaTeX distribúciu, ktorú má uíva LyX.'
LangString EnterLaTeXFolder "${LYX_LANG}" 'Tu môete pouíva chod k "latex.exe" a tım pouíva chod \
					   LaTeX-distribúciu, ktorá by mohla by pouívaná cestou LyXu.\r\n\
					   Ak nepouívate LaTeX, LyX nemôe produkova dokumenty!\r\n\
					   \r\n\
					   Inštalatér detektoval LaTeX-distribúciu \
					   "$LaTeXName" na vašej sústave. Dolu vidíte jeho chod.'
LangString EnterLaTeXFolderNone "${LYX_LANG}" 'Pouívajte dolu chod k "latex.exe". Tım urcujete, ktorá \
					       LaTeX-distribúcia má by pouívaná LyXom.\r\n\
					       Ak nepouívate LaTeX, LyX nemôe produkova dokumenty!\r\n\
					       \r\n\
					       Inštalatér nemohol nájst LaTeX-distribúciu na vašej sústave.'
LangString PathName "${LYX_LANG}" 'Chod k "latex.exe"'
LangString DontUseLaTeX "${LYX_LANG}" "Neuívajte LaTeX"
LangString InvalidLaTeXFolder "${LYX_LANG}" 'Mapa "latex.exe" nie je na špecifikovanej ceste.'

LangString LatexInfo "${LYX_LANG}" 'Teraz môete štartova LaTeX distribúciu "MiKTeX".$\r$\n\
			            Na inštalovanie programu tlaète gombík "Next" v inštalovacích oknách a sa zaèína inštalácia.$\r$\n\
				    $\r$\n\
				    !!! Prosím uívajte všetkı obsiahnuté nastavenia inštalatéra MiKTeX !!!'

LangString LatexError1 "${LYX_LANG}" 'LaTeX distribúcia sa nemohla nájst!$\r$\n\
                      		      LyX sa nemôe uíva bez LaTeX distribúcie ako "MiKTeX"!$\r$\n\
				      Preto sa inštalácia zastavila.'
			
LangString GSviewInfo "${LYX_LANG}" 'Teraz štartuje inštalatér programu "GSview" .$\r$\n\
			             Na inštaláciu programu tlaète gombík "Setup" v prvom inštalaènom okne$\r$\n\
				     Vyberajte jazyk a tlaète gombík "Next" v nasledujúcich inštalaènıch oknách.$\r$\n\
				     Môete uívat všetkı obsiahnuté nastavenia inštalatéra GSview.'
LangString GSviewError "${LYX_LANG}" 'Program "GSview" sa nemohol inštalova úspešne!$\r$\n\
		                      Predsa inštalácia pokraèuje.$\r$\n\
				      Skúste inštalova GSview ešte raz neskôr.'

LangString JabRefInfo "${LYX_LANG}" 'Teraz sa štartuje inštalatér programu "JabRef".$\r$\n\
				     Môzete všetkı obsiahnuté nastavenia inštalatéra JabRef.'
LangString JabRefError "${LYX_LANG}" 'Program "JabRef" sa nemohol inštalova úspešne!$\r$\n\
		                      Predsa inštalácia pokraèuje.$\r$\n\
				      Skúste inštalova JabRef ešte raz neskôr.'
				
LangString LatexConfigInfo "${LYX_LANG}" "Na nasledujúcu konfiguráciu LyXu treba poèka chví¾u."

LangString AspellInfo "${LYX_LANG}" 'Teraz sa inštalujú slovníky pre skúšobnı program "Aspell".$\r$\n\
				     Kadı slovník má inú licensiu, ktorá sa predstavuje pred downloadom a inštaláciou.'
LangString AspellDownloadFailed "${LYX_LANG}" "Aspell slovník sa nemohol downloadova!"
LangString AspellInstallFailed "${LYX_LANG}" "Aspell slovník sa nemohol inštalova!"
LangString AspellPartAnd "${LYX_LANG}" " a "
LangString AspellPartSep "${LYX_LANG}" ", "
LangString AspellAll "${LYX_LANG}" 'U je inštalovanı slovník jazyku'
LangString AspellNone "${LYX_LANG}" 'Slovník pre "Aspell" nebol inštalovanı.$\r$\n\
				     Slovníky je moné downloadova od$\r$\n\
				     ${AspellLocation}$\r$\n\
				     Chcete teraz downloadova slovníky?'
LangString AspellPartStart "${LYX_LANG}" "Teraz je úspešne inštalovanı "
LangString AspellPart1 "${LYX_LANG}" "anglickı slovník"
LangString AspellPart2 "${LYX_LANG}" "slovník jazyku $LangName"
LangString AspellPart3 "${LYX_LANG}" "a$\r$\n\
				      slovník jazyku $LangNameSys"
LangString AspellPart4 "${LYX_LANG}" ' pre "Aspell".$\r$\n\
				      Viac slovníkov je mono downloadova od$\r$\n\
				      ${AspellLocation}$\r$\n\
				      Chcete teraz downloadova iné slovníky?'

LangString MiKTeXPathInfo "${LYX_LANG}" "Aby kadı pouívatel bol neskôr schopnı pouívat MiKTeX pre svoje potreby$\r$\n\
					 je treba napísat povolenia pre všetkıch pouívatelov do inštalacného papiera MiKTeX $\r$\n\
					 $MiKTeXPath $\r$\n\
					 aj do podriadenıch papierov."
LangString MiKTeXInfo "${LYX_LANG}" 'LaTeX distribúcia "MiKTeX" sa pouíva spolu s LyXom.$\r$\n\
				     Doporuèuje sa inštalovat platné MiKTeX aktualizácie pomocou programu "MiKTeX Update Wizard"$\r$\n\
				     pred prvım pouívaním LyXu.$\r$\n\
				     Chcete teraz testova MiKTeX aktualizácie?'

LangString UpdateNotAllowed "${LYX_LANG}" "This update package can only update ${PRODUCT_VERSION_OLD}!"
LangString ModifyingConfigureFailed "${LYX_LANG}" "Nemohol uvies v platnost 'path_prefix' (zoznam s cestami programu) v konfiguraènom skripte"
LangString CreateCmdFilesFailed "${LYX_LANG}" "Nemohol prípravi lyx.bat"
LangString RunConfigureFailed "${LYX_LANG}" "Nemohol vykona konfiguraènı skript"
LangString NotAdmin "${LYX_LANG}" "Pre inštaláciu LyX potrebujete administrátorské práva!"
LangString InstallRunning "${LYX_LANG}" "Inštalatér u behá!"
LangString StillInstalled "${LYX_LANG}" "LyX je u inštalovanı! Najprv treba odinštalova LyX."

LangString FinishPageMessage "${LYX_LANG}" "Gratulácia! LyX bol úspešne inštalovanı.\r\n\
					    \r\n\
					    (Prvı LyX štart môe trva niekolko sekund.)"
LangString FinishPageRun "${LYX_LANG}" "Štartova LyX"

LangString UnNotInRegistryLabel "${LYX_LANG}" "Nemôe nájs LyX v registre.$\r$\n\
					       Symboly na desktope a v štartovacom Menu sa nedajú odstrani."
LangString UnInstallRunning "${LYX_LANG}" "Najprv zavrí LyX!"
LangString UnNotAdminLabel "${LYX_LANG}" "Pre odinštaláciu LyX potrebujete administrátorské práva!"
LangString UnReallyRemoveLabel "${LYX_LANG}" "Ste si istı, e chcete kompletne odinštalova LyX a všetkı jeho suèiastky?"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'LyX prednosti uívae¾a'
LangString UnGSview "${LYX_LANG}" 'Prosím tlaète gombík "Uninstall" v nasledujúcom okne aby Ste inštalovali $\r$\n\
				   Postscript a PDF-viewer "GSview".'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "LyX je úspešne odstránenı od Vášho poèitaèa."

LangString SecUnAspellDescription "${LYX_LANG}" "Odinštaluje Aspell a všetkı jeho slovníky."
LangString SecUnMiKTeXDescription "${LYX_LANG}" "Odinštaluje LaTeX distribúciu MiKTeX."
LangString SecUnJabRefDescription "${LYX_LANG}" "Odinštaluje bibliografickı manager JabRef."
LangString SecUnPreferencesDescription "${LYX_LANG}" 'Odstráni konfiguracné papiere LyXu $\r$\n\
					   "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
					   pre všetkıch uivate¾ov.'
LangString SecUnProgramFilesDescription "${LYX_LANG}" "Odinštaluj LyX a všetkı jeho suèiastky."

!undef LYX_LANG

!endif # _LYX_LANGUAGES_SLOVAK_NSH_





