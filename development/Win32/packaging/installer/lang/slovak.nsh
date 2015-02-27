/*
LyX Installer Language File
Language: Slovak
Author: Kornel Benko
*/

!insertmacro LANGFILE_EXT "Slovak"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Inštalované pre súèasného uívate¾a)"

${LangFileString} TEXT_WELCOME "Tento sprievodca Vám pomáha inštalova LyX.$\r$\n\
				$\r$\n\
				$_CLICK"

#${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Aktualizácia MiKTeX-databázy súborovıch mien..."
${LangFileString} TEXT_CONFIGURE_LYX "Konfigurácia LyXu ($LaTeXInstalled môe naèíta chıbajúce balíky, to môe trochu trva) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Kompilácia Python skriptov..."

${LangFileString} TEXT_FINISH_DESKTOP "Vytvori skratku pre pracovnú plochu"
${LangFileString} TEXT_FINISH_WEBSITE "Navštívte lyx.org pre posledné novinky, podporu a tipy"

${LangFileString} FileTypeTitle "LyX dokument"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Inštalova pre všetkıch uívate¾ov?"
${LangFileString} SecFileAssocTitle "Asociácie súborov"
${LangFileString} SecDesktopTitle "Ikona pracovnej plochy"

${LangFileString} SecCoreDescription "Súbory LyXu."
${LangFileString} SecInstJabRefDescription "Manaér pre bibliografické odkazy a editor pre BibTeX súbory."
#${LangFileString} SecAllUsersDescription "Inštalova LyX pre všetkıch uívate¾ov alebo len pre súèasného uívate¾a."
${LangFileString} SecFileAssocDescription "Súbory s rozšírením .lyx sa automaticky otvárajú v LyXe."
${LangFileString} SecDesktopDescription "Ikona LyXa na pracovnej ploche."
${LangFileString} SecDictionariesDescription "Slovníky pre kontrolu pravopisu ktoré mono naèíta a inštalova."
${LangFileString} SecThesaurusDescription "Slovníky tezaura ktoré mono naèíta a inštalova."

${LangFileString} EnterLaTeXHeader1 'LaTeX distribúcia'
${LangFileString} EnterLaTeXHeader2 'Urèite LaTeX distribúciu, ktorú má LyX pouíva.'
${LangFileString} EnterLaTeXFolder 'Tu môte nastavi cestu k $\"latex.exe$\" a tım nastavi \
					LaTeX-distribúciu, ktorá by mala by pouívaná LyXom.\r\n\
					Bez LaTeXu, LyX nedokáe produkova dokumenty!\r\n\
					\r\n\
					Inštalatér detegoval LaTeX-distribúciu \
					$\"$LaTeXName$\" na vašom systéme. Dolu je zobrazená jeho cesta.'
${LangFileString} EnterLaTeXFolderNone 'Nastavte dolu cestu k $\"latex.exe$\". Tım nastavíte, ktorá \
					LaTeX-distribúcia má by pouívaná LyXom.\r\n\
					Bez LaTeXu LyX nedokáe produkova dokumenty!\r\n\
					\r\n\
					Inštalatér nemohol nájs LaTeX-distribúciu na vašom systéme.'
${LangFileString} PathName 'Cesta na súbor $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Nepoui LaTeX"
${LangFileString} InvalidLaTeXFolder 'Súbor $\"latex.exe$\" nie je na špecifikovanej ceste.'

${LangFileString} LatexInfo 'Teraz štartuje inštalatér LaTeX distribúcie $\"MiKTeX$\".$\r$\n\
				Na inštalovanie programu tlaète $\"Next$\" v okne inštalátora a inštalácia zaène.$\r$\n\
				$\r$\n\
				!!! Prosím pouite všetky štandardné monosti inštalatéra MiKTeX !!!'

${LangFileString} LatexError1 'Nenašla sa iadna LaTeX-distribúcia!$\r$\n\
				LyX sa nemôe uíva bez LaTeX distribúcie ako napríklad $\"MiKTeX$\"!$\r$\n\
				Inštalácia sa preto zrušuje.'

${LangFileString} HunspellFailed 'Naèítanie slovníka pre jazyk $\"$R3$\" zlyhalo.'
${LangFileString} ThesaurusFailed 'Naèítanie tezaura pre jazyk $\"$R3$\" zlyhalo.'

${LangFileString} JabRefInfo 'Teraz sa štartuje inštalatér programu $\"JabRef$\".$\r$\n\
				Môte poui všetky štandardné monosti inštalatéra JabRef.'
${LangFileString} JabRefError 'Program $\"JabRef$\" nebol úspešne inštalovanı!$\r$\n\
				Predsa inštalácia pokraèuje.$\r$\n\
				Skúste inštalova JabRef neskôr ešte raz.'

${LangFileString} LatexConfigInfo "Nasledujúca konfigurácia LyXu trochu potrvá."

${LangFileString} MiKTeXPathInfo "Aby kadı pouívate¾ bol neskôr schopnı pouíva MiKTeX pre svoje potreby$\r$\n\
					je treba nastavi povolenia pre všetkıch pouívate¾ov do inštalaèného adresára MiKTeX $\r$\n\
					$MiKTeXPath $\r$\n\
					i podriadenıch adresárov."
${LangFileString} MiKTeXInfo 'LaTeX-distribúcia $\"MiKTeX$\" sa bude pouíva spolu s LyXom.$\r$\n\
				Doporuèuje sa inštalova dostupné MiKTeX aktualizácie pomocou programu $\"MiKTeX Update Wizard$\"$\r$\n\
				pred prvım pouitím LyXu.$\r$\n\
				Chcete teraz skontrolova MiKTeX aktualizácie?'

${LangFileString} ModifyingConfigureFailed "Nedal sa nastavi 'path_prefix' v konfiguraènom skripte"
#${LangFileString} RunConfigureFailed "Nedal sa spusti konfiguraènı skript"
${LangFileString} NotAdmin "Pre inštaláciu LyXu potrebujete administrátorské práva!"
${LangFileString} InstallRunning "Inštalaènı program u beí!"
${LangFileString} StillInstalled "LyX ${APP_SERIES_KEY2} je u inštalovanı! LyX treba najprv odinštalova."
${LangFileString} NewerInstalled "Pokúšate sa inštalova verziu LyXu ktorá je staršia ako tá ktorá je inštalovaná.$\r$\n\
				  Keï to naozaj chcete, odinštalujte najprv existujúci LyX $OldVersionNumber."

${LangFileString} MultipleIndexesNotAvailable "Podpora pouívania viacerıch indexov v dokumente nebude dostupná, lebo\r$\n\
						MiKTeX bol inštalovanı pod administrátorskımi právami a vy inštalujete LyX bez nich."
${LangFileString} MetafileNotAvailable "Podpora obrázkov vo formáte EMF alebo WMF nebude dostupná$\r$\n\
					lebo to vyaduje inštaláciu softvérovej tlaèiarne pre Windows$\r$\n\
					èo je moné len pod administrátorskımi právami."

${LangFileString} FinishPageMessage "Gratulácia! LyX bol úspešne inštalovanı.$\r$\n\
					$\r$\n\
					(Prvı LyX štart môe trva nieko¾ko sekúnd.)"
${LangFileString} FinishPageRun "Spusti LyX"

${LangFileString} UnNotInRegistryLabel "Nemôem nájs LyX v registre.$\r$\n\
					Skratky na pracovnej ploche a v štartovacom Menu sa nedajú odstráni."
${LangFileString} UnInstallRunning "Najprv treba zavrie LyX!"
${LangFileString} UnNotAdminLabel "Pre odinštaláciu LyX potrebujete administrátorské práva!"
${LangFileString} UnReallyRemoveLabel "Ste si istı, e chcete kompletne odinštalova LyX a všetky jeho súèiastky?"
${LangFileString} UnLyXPreferencesTitle 'LyXove uívate¾ské nastavenia'

${LangFileString} SecUnMiKTeXDescription "Odinštaluje LaTeX-distribúciu MiKTeX."
${LangFileString} SecUnJabRefDescription "Odinštaluje manaéra bibliografie JabRef."
${LangFileString} SecUnPreferencesDescription 'Odstráni konfiguraènı adresár LyXu $\r$\n\
						$\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						pre všetkıch uívate¾ov (keï máte administrátorské práva).'
${LangFileString} SecUnProgramFilesDescription "Odinštaluj LyX a všetky jeho súèiastky."

