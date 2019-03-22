/*
LyX Installer Language File
Language: Slovak
Author: Kornel Benko
*/

!insertmacro LANGFILE_EXT "Slovak"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Inštalované pre súčasného užívateľa)"

${LangFileString} TEXT_WELCOME "Tento sprievodca Vám pomáha inštalovať LyX.$\r$\n\
				$\r$\n\
				$_CLICK"

#${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Aktualizácia MiKTeX-databázy súborových mien..."
${LangFileString} TEXT_CONFIGURE_LYX "Konfigurácia LyXu ($LaTeXInstalled môže načítať chýbajúce balíky, to môže trochu trvať) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Kompilácia Python skriptov..."

${LangFileString} TEXT_FINISH_DESKTOP "Vytvoriť skratku pre pracovnú plochu"
${LangFileString} TEXT_FINISH_WEBSITE "Navštívte lyx.org pre posledné novinky, podporu a tipy"

#${LangFileString} FileTypeTitle "LyX dokument"

#${LangFileString} SecInstJabRefTitle "JabRef"
#${LangFileString} SecAllUsersTitle "Inštalovať pre všetkých užívateľov?"
${LangFileString} SecFileAssocTitle "Asociácie súborov"
${LangFileString} SecDesktopTitle "Ikona pracovnej plochy"

${LangFileString} SecCoreDescription "Súbory LyXu."
#${LangFileString} SecInstJabRefDescription "Manažér pre bibliografické odkazy a editor pre BibTeX súbory."
#${LangFileString} SecAllUsersDescription "Inštalovať LyX pre všetkých užívateľov alebo len pre súčasného užívateľa."
${LangFileString} SecFileAssocDescription "Súbory s rozšírením .lyx sa automaticky otvárajú v LyXe."
${LangFileString} SecDesktopDescription "Ikona LyXa na pracovnej ploche."
${LangFileString} SecDictionaries "Slovníky"
${LangFileString} SecDictionariesDescription "Slovníky pre kontrolu pravopisu ktoré možno načítať a inštalovať."
${LangFileString} SecThesaurus "Tezaurus"
${LangFileString} SecThesaurusDescription "Slovníky tezaura ktoré možno načítať a inštalovať."

${LangFileString} EnterLaTeXHeader1 'LaTeX distribúcia'
${LangFileString} EnterLaTeXHeader2 'Určite LaTeX distribúciu, ktorú má LyX používať.'
${LangFileString} EnterLaTeXFolder 'Tu môžte nastaviť cestu k $\"latex.exe$\" a tým nastaviť \
					LaTeX-distribúciu, ktorá by mala byť používaná LyXom.\r\n\
					Bez LaTeXu, LyX nedokáže produkovať dokumenty!\r\n\
					\r\n\
					Inštalatér objavil LaTeX-distribúciu \
					$\"$LaTeXName$\" na vašom systéme. Dolu je zobrazená jeho cesta.'
${LangFileString} EnterLaTeXFolderNone 'Nastavte dolu cestu k $\"latex.exe$\". Tým nastavíte, ktorá \
					LaTeX-distribúcia má byť používaná LyXom.\r\n\
					Bez LaTeXu LyX nedokáže produkovať dokumenty!\r\n\
					\r\n\
					Inštalatér nemohol nájsť LaTeX-distribúciu na vašom systéme.'
${LangFileString} PathName 'Cesta na súbor $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Nepoužiť LaTeX"
${LangFileString} InvalidLaTeXFolder 'Súbor $\"latex.exe$\" nie je na špecifikovanej ceste.'

${LangFileString} LatexInfo 'Teraz štartuje inštalatér LaTeX distribúcie $\"MiKTeX$\".$\r$\n\
				Na inštalovanie programu stlačte $\"Next$\" v okne inštalátora až inštalácia začne.$\r$\n\
				$\r$\n\
				!!! Prosím použite všetky štandardné možnosti inštalatéra MiKTeX !!!'

${LangFileString} LatexError1 'Nenašla sa žiadna LaTeX-distribúcia!$\r$\n\
				LyX sa nemôže užívať bez LaTeX distribúcie ako napríklad $\"MiKTeX$\"!$\r$\n\
				Inštalácia sa preto zrušuje.'

${LangFileString} HunspellFailed 'Načítanie slovníka pre jazyk $\"$R3$\" zlyhalo.'
${LangFileString} ThesaurusFailed 'Načítanie tezaura pre jazyk $\"$R3$\" zlyhalo.'

#${LangFileString} JabRefInfo 'Teraz sa štartuje inštalatér programu $\"JabRef$\".$\r$\n\
#				Môžte použiť všetky štandardné možnosti inštalatéra JabRef.'
#${LangFileString} JabRefError 'Program $\"JabRef$\" nebol úspešne inštalovaný!$\r$\n\
#				Predsa inštalácia pokračuje.$\r$\n\
#				Skúste inštalovať JabRef neskôr ešte raz.'

#${LangFileString} LatexConfigInfo "Nasledujúca konfigurácia LyXu trochu potrvá."

#${LangFileString} MiKTeXPathInfo "Aby každý používateľ bol neskôr schopný používať MiKTeX pre svoje potreby$\r$\n\
#					je treba nastaviť povolenia pre všetkých používateľov do inštalačného adresára MiKTeX $\r$\n\
#					$MiKTeXPath $\r$\n\
#					i podriadených adresárov."
${LangFileString} MiKTeXInfo 'LaTeX-distribúcia $\"MiKTeX$\" sa bude používať spolu s LyXom.$\r$\n\
				Doporučuje sa inštalovať dostupné MiKTeX aktualizácie pomocou programu $\"MiKTeX Update Wizard$\"$\r$\n\
				pred prvým použitím LyXu.$\r$\n\
				Chcete teraz skontrolovať MiKTeX aktualizácie?'

${LangFileString} ModifyingConfigureFailed "Nedal sa nastaviť 'path_prefix' v konfiguračnom skripte"
#${LangFileString} RunConfigureFailed "Nedal sa spustiť konfiguračný skript"
${LangFileString} NotAdmin "Pre inštaláciu LyXu potrebujete administrátorské práva!"
${LangFileString} InstallRunning "Inštalačný program už beží!"
${LangFileString} AlreadyInstalled "LyX ${APP_SERIES_KEY2} je už inštalovaný!$\r$\n\
				Inštalovať ponad existujúce inštalácie sa nedoporučuje keď inštalovaná verzia$\r$\n\
				je testovné vydanie alebo keď máte problémy s existujúcou inštaláciou.$\r$\n\
				V takýchto prípadoch je lepšie reinštalovať LyX.$\r$\n\
				Napriek tomu chcete inštalovať LyX ponad existujúcu verziu?"
${LangFileString} NewerInstalled "Pokúšate sa inštalovať verziu LyXu ktorá je staršia ako tá ktorá je inštalovaná.$\r$\n\
				  Keď to naozaj chcete, odinštalujte najprv existujúci LyX $OldVersionNumber."

${LangFileString} MultipleIndexesNotAvailable "Podpora používania viacerých indexov v dokumente nebude dostupná, lebo\r$\n\
						MiKTeX bol inštalovaný pod administrátorskými právami a vy inštalujete LyX bez nich."

#${LangFileString} FinishPageMessage "Gratulácia! LyX bol úspešne inštalovaný.$\r$\n\
#					$\r$\n\
#					(Prvý LyX štart môže trvať niekoľko sekúnd.)"
${LangFileString} FinishPageRun "Spustiť LyX"

${LangFileString} UnNotInRegistryLabel "Nemôžem nájsť LyX v registre.$\r$\n\
					Skratky na pracovnej ploche a v štartovacom Menu sa nedajú odstrániť."
${LangFileString} UnInstallRunning "Najprv treba zavrieť LyX!"
${LangFileString} UnNotAdminLabel "Pre odinštaláciu LyXu potrebujete administrátorské práva!"
${LangFileString} UnReallyRemoveLabel "Ste si istý, že chcete kompletne odinštalovať LyX a všetky jeho súčiastky?"
${LangFileString} UnLyXPreferencesTitle 'LyXove užívateľské nastavenia'

${LangFileString} SecUnMiKTeXDescription "Odinštaluje LaTeX-distribúciu MiKTeX."
#${LangFileString} SecUnJabRefDescription "Odinštaluje manažéra bibliografie JabRef."
${LangFileString} SecUnPreferencesDescription 'Odstráni konfiguračný adresár LyXu $\r$\n\
						$\"$AppPre\username\$\r$\n\
						$AppSuff\$\r$\n\
						${APP_DIR_USERDATA}$\")$\r$\n\
						pre všetkých užívateľov (keď máte administrátorské práva).'
${LangFileString} SecUnProgramFilesDescription "Odinštaluj LyX a všetky jeho súčiastky."


# MikTeX Update Stuff
${LangFileString} MiKTeXUpdateMsg "Kvôli zmenám MiKTeX-u, inštalácia verzie LyX-u novšej ako 2.2.3 môže byť dôvodom parciálnej aktualizácie. To by mohlo zrobiť MiKTeX nepoužiteľným.$\r$\n\
$\r$\n\
Ak ste to doteraz ešte neurobili, odporúčame aktualizovať MiKTeX na jeho najnovšiu verziu pred inštaláciou novej LyX- verzie.$\r$\n\
$\r$\n\
Teraz môžte buď s inštaláciou pokračovať, alebo ju zrušiť aby ste mohli aktualizovať MiKTeX."

${LangFileString} MiKTeXUpdateHeader "VAROVANIE: Zmeny MiKTeX-u"
