!insertmacro LANGFILE_EXT "Czech"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installed for Current User)"

${LangFileString} TEXT_WELCOME "Tento pomocník vás provede instalací LyXu.$\r$\n\
				$\r$\n\
				$_CLICK"

#${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Updating MiKTeX filename database..."
${LangFileString} TEXT_CONFIGURE_LYX "Configuring LyX ($R9 may download missing packages, this can take some time) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compiling Python scripts..."

${LangFileString} TEXT_FINISH_DESKTOP "Create desktop shortcut"
${LangFileString} TEXT_FINISH_WEBSITE "Visit lyx.org for the latest news, support and tips"

#${LangFileString} FileTypeTitle "LyX-dokumentů"

#${LangFileString} SecInstJabRefTitle "JabRef"
#${LangFileString} SecAllUsersTitle "Instalovat pro všechny uživatele?"
${LangFileString} SecFileAssocTitle "Asociovat soubory"
${LangFileString} SecDesktopTitle "Ikonu na plochu"

${LangFileString} SecCoreDescription "Soubory LyXu."
#${LangFileString} SecInstJabRefDescription "Správce pro bibliografii a editor souborů BibTeXu."
#${LangFileString} SecAllUsersDescription "Instalovat LyX pro všechny uživatele nebo pouze pro současného uživatele."
${LangFileString} SecFileAssocDescription "Soubory s příponou .lyx se automaticky otevřou v LyXu."
${LangFileString} SecDesktopDescription "Ikonu LyXu na plochu."
${LangFileString} SecDictionaries "Slovníky"
${LangFileString} SecDictionariesDescription "Spell-checker dictionaries that can be downloaded and installed."
${LangFileString} SecThesaurus "Tezaurus"
${LangFileString} SecThesaurusDescription "Thesaurus dictionaries that can be downloaded and installed."

${LangFileString} EnterLaTeXHeader1 'Distribuce LaTeXu'
${LangFileString} EnterLaTeXHeader2 'Nastavte distribuci LaTeXu, kterou má LyX používat.'
${LangFileString} EnterLaTeXFolder 'Můžete zde volitelně nastavit cestu k souboru $\"latex.exe$\" a tím určit \
					distribuci LaTeXu, kterou bude používat Lyx.$\r$\n\
					Pokud nepoužijete LaTeX, LyX nemůže vysázet výsledné dokumenty!$\r$\n\
					$\r$\n\
					Instalátor nalezl na vašem počítači LaTeXovou distribuci \
					$\"$R9$\". Níže je uvedena její cesta.'
${LangFileString} EnterLaTeXFolderNone 'Níže nastavte cestu k souboru $\"latex.exe$\". Určíte tím, která \
					distribuce LaTeXu bude používána LyXem.$\r$\n\
					Pokud nepoužijete LaTeX, LyX nemůže vysázet výsledné dokumenty!$\r$\n\
					$\r$\n\
					Instalátor nemohl nalézt distribuci LaTeXu na vašem počítači.'
${LangFileString} PathName 'Cesta k souboru $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Nepoužívat LaTeX"
${LangFileString} InvalidLaTeXFolder 'Soubor $\"latex.exe$\" není v zadané cestě.'

${LangFileString} LatexInfo 'Nyní bude spuštěn instalátor LaTeXové-distribuce $\"MiKTeX$\".$\r$\n\
				V instalátoru pokračujte tlačítkem $\"Next$\" dokud instalace nezačne.$\r$\n\
				$\r$\n\
				!!! Užijte všechny předvolby instalátoru MiKTeX !!!'
${LangFileString} LatexError1 'Žádná LaTeXová-distribuce nebyla nalezena!$\r$\n\
				Nemůžete používat LyX bez distribuce LaTeXu jako je např. $\"MiKTeX$\"!$\r$\n\
				Instalace bude proto přerušena.'

${LangFileString} HunspellFailed 'Download of dictionary for language $\"$R9$\" failed.'
${LangFileString} ThesaurusFailed 'Download of thesaurus for language $\"$R9$\" failed.'

#${LangFileString} JabRefInfo 'Nyní bude spuštěn instalátor programu $\"JabRef$\".$\r$\n\
#				Můžete použít všechny předvolby instalátoru JabRef.'
#${LangFileString} JabRefError 'Program $\"JabRef$\" nebyl úspěšně nainstalován.$\r$\n\
#				Instalátor bude navzdory tomu pokračovat.$\r$\n\
#				Pokuste se nainstalovat JabRef později.'

#${LangFileString} LatexConfigInfo "Následující konfigurování LyXu chvíli potrvá."

#${LangFileString} MiKTeXPathInfo "Aby každý uživatel mohl později přizpůsobit MiKTeX pro své potřeby$\r$\n\
#					je třeba nastavit práva zápisu pro všechny uživatele do adresáře MiKTeXu$\r$\n\
#					$MiKTeXPath $\r$\n\
#					a všech jeho podadresářů."
${LangFileString} MiKTeXInfo 'LaTeXová distribuce $\"MiKTeX$\" bude použita dohromady s LyXem.$\r$\n\
				Je doporučeno nainstalovat dostupné aktualizace MiKTeXu pomocí programu $\"MiKTeX Update Wizard$\"$\r$\n\
				předtím než poprvé spustíte LyX.$\r$\n\
				Chcete zkontrolovat dostupné aktualizace MiKTeXu nyní?'
				     
${LangFileString} ModifyingConfigureFailed "Nelze nastavit 'path_prefix' v konfiguračním skriptu"
#${LangFileString} RunConfigureFailed "Nelze spustit konfigurační skript"
${LangFileString} NotAdmin "Potřebujete administrátorská práva pro nainstalování LyXu!"
${LangFileString} InstallRunning "Instalátor je již spuštěn!"
${LangFileString} AlreadyInstalled "LyX ${APP_VERSION} je již nainstalován!$\r$\n\
				Installing over existing installations is not recommended if the installed version$\r$\n\
				is a test release or if you have problems with your existing LyX installation.$\r$\n\
				In these cases better reinstall LyX.$\r$\n\
				Dou you nevertheles want to install LyX over the existing version?"
${LangFileString} NewerInstalled "You are trying to install an older version of LyX than what you have installed.$\r$\n\
				  If you really want this, you must uninstall the existing LyX $R9 before."

${LangFileString} MultipleIndexesNotAvailable "The support for using several indexes in a document will not be available because$\r$\n\
						MiKTeX was installed with administrator privileges but you are installing LyX without them."

#${LangFileString} FinishPageMessage "Blahopřejeme! LyX byl úspěšně nainstalován.$\r$\n\
#					$\r$\n\
#					(První spuštění LyXu může trvat delší dobu.)"
${LangFileString} FinishPageRun "Spustit LyX"

${LangFileString} UnNotInRegistryLabel "Nelze nalézt LyX v registrech.$\r$\n\
					Zástupce na ploše a ve Start menu nebude smazán."
${LangFileString} UnInstallRunning "Nejprve musíte zavřít LyX!"
${LangFileString} UnNotAdminLabel "Musíte mít administrátorská práva pro odinstalování LyXu!"
${LangFileString} UnReallyRemoveLabel "Chcete opravdu smazat LyX a všechny jeho komponenty?"
${LangFileString} UnLyXPreferencesTitle 'Uživatelská nastavení LyXu'

${LangFileString} SecUnMiKTeXDescription "Odinstalovat LaTeXovou-distribuci MiKTeX."
#${LangFileString} SecUnJabRefDescription "Odinstalovat manažer bibliografie JabRef."
${LangFileString} SecUnPreferencesDescription 'Smazat konfigurační adresář LyXu$\r$\n\
						$\"$R9\username\$\r$\n\
						$R8\$\r$\n\
						LyX${VERSION_MAJOR}.${VERSION_MINOR}$\")$\r$\n\
						pro všechny uživatele.'
${LangFileString} SecUnProgramFilesDescription "Odinstalovat LyX a všechny jeho komponenty."


# MikTeX Update Stuff
${LangFileString} MiKTeXUpdateMsg "Kvůli změnám MiKTeX-u mohly starší instalace LyX-u (2.2.3) způsobit částečnou aktualizaci, která MiKTeX-u znemožní normální fungovaní.$\r$\n\
$\r$\n\
Pokud jste tak již neučinili, doporučujeme abyste si sami aktualizovali poslední verzi MiKTeX-u před instalací LyX-u.$\r$\n\
$\r$\n\
Nyní můžete buďto pokračovat v instalaci, anebo ji stornovat za účelem aktualizace MikTeX-u."

${LangFileString} MiKTeXUpdateHeader "Upozornění: Změny MiKTeX-u"
