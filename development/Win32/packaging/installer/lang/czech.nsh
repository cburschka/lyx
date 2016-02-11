!insertmacro LANGFILE_EXT "Czech"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installed for Current User)"

${LangFileString} TEXT_WELCOME "Tento pomocník vás provede instalací LyXu.$\r$\n\
				$\r$\n\
				$_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Updating MiKTeX filename database..."
${LangFileString} TEXT_CONFIGURE_LYX "Configuring LyX ($LaTeXInstalled may download missing packages, this can take some time) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compiling Python scripts..."

${LangFileString} TEXT_FINISH_DESKTOP "Create desktop shortcut"
${LangFileString} TEXT_FINISH_WEBSITE "Visit lyx.org for the latest news, support and tips"

${LangFileString} FileTypeTitle "LyX-dokumentù"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Instalovat pro všechny uživatele?"
${LangFileString} SecFileAssocTitle "Asociovat soubory"
${LangFileString} SecDesktopTitle "Ikonu na plochu"

${LangFileString} SecCoreDescription "Soubory LyXu."
${LangFileString} SecInstJabRefDescription "Správce pro bibliografii a editor souborù BibTeXu."
${LangFileString} SecAllUsersDescription "Instalovat LyX pro všechny uživatele nebo pouze pro souèasného uživatele."
${LangFileString} SecFileAssocDescription "Soubory s pøíponou .lyx se automaticky otevøou v LyXu."
${LangFileString} SecDesktopDescription "Ikonu LyXu na plochu."
${LangFileString} SecDictionariesDescription "Spell-checker dictionaries that can be downloaded and installed."
${LangFileString} SecThesaurusDescription "Thesaurus dictionaries that can be downloaded and installed."

${LangFileString} EnterLaTeXHeader1 'Distribuce LaTeXu'
${LangFileString} EnterLaTeXHeader2 'Nastavte distribuci LaTeXu, kterou má LyX používat.'
${LangFileString} EnterLaTeXFolder 'Mùžete zde volitelnì nastavit cestu k souboru $\"latex.exe$\" a tím urèit \
					distribuci LaTeXu, kterou bude používat Lyx.\r\n\
					Pokud nepoužijete LaTeX, LyX nemùže vysázet výsledné dokumenty!\r\n\
					\r\n\
					Instalátor nalezl na vašem poèítaèi LaTeXovou distribuci \
					$\"$LaTeXName$\". Níže je uvedena její cesta.'
${LangFileString} EnterLaTeXFolderNone 'Níže nastavte cestu k souboru $\"latex.exe$\". Urèíte tím, která \
					distribuce LaTeXu bude používána LyXem.\r\n\
					Pokud nepoužijete LaTeX, LyX nemùže vysázet výsledné dokumenty!\r\n\
					\r\n\
					Instalátor nemohl nalézt distribuci LaTeXu na vašem poèítaèi.'
${LangFileString} PathName 'Cesta k souboru $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Nepoužívat LaTeX"
${LangFileString} InvalidLaTeXFolder 'Soubor $\"latex.exe$\" není v zadané cestì.'

${LangFileString} LatexInfo 'Nyní bude spuštìn instalátor LaTeXové-distribuce $\"MiKTeX$\".$\r$\n\
				V instalátoru pokraèujte tlaèítkem $\"Next$\" dokud instalace nezaène.$\r$\n\
				$\r$\n\
				!!! Užijte všechny pøedvolby instalátoru MiKTeX !!!'
${LangFileString} LatexError1 'Žádná LaTeXová-distribuce nebyla nalezena!$\r$\n\
				Nemùžete používat LyX bez distribuce LaTeXu jako je napø. $\"MiKTeX$\"!$\r$\n\
				Instalace bude proto pøerušena.'

${LangFileString} HunspellFailed 'Download of dictionary for language $\"$R3$\" failed.'
${LangFileString} ThesaurusFailed 'Download of thesaurus for language $\"$R3$\" failed.'

${LangFileString} JabRefInfo 'Nyní bude spuštìn instalátor programu $\"JabRef$\".$\r$\n\
				Mùžete použít všechny pøedvolby instalátoru JabRef.'
${LangFileString} JabRefError 'Program $\"JabRef$\" nebyl úspìšnì nainstalován.$\r$\n\
				Instalátor bude navzdory tomu pokraèovat.$\r$\n\
				Pokuste se nainstalovat JabRef pozdìji.'

${LangFileString} LatexConfigInfo "Následující konfigurování LyXu chvíli potrvá."

${LangFileString} MiKTeXPathInfo "Aby každý uživatel mohl pozdìji pøizpùsobit MiKTeX pro své potøeby$\r$\n\
					je tøeba nastavit práva zápisu pro všechny uživatele do adresáøe MiKTeXu$\r$\n\
					$MiKTeXPath $\r$\n\
					a všech jeho podadresáøù."
${LangFileString} MiKTeXInfo 'LaTeXová distribuce $\"MiKTeX$\" bude použita dohromady s LyXem.$\r$\n\
				Je doporuèeno nainstalovat dostupné aktualizace MiKTeXu pomocí programu $\"MiKTeX Update Wizard$\"$\r$\n\
				pøedtím než poprvé spustíte LyX.$\r$\n\
				Chcete zkontrolovat dostupné aktualizace MiKTeXu nyní?'
				     
${LangFileString} ModifyingConfigureFailed "Nelze nastavit 'path_prefix' v konfiguraèním skriptu"
${LangFileString} RunConfigureFailed "Nelze spustit konfiguraèní skript"
${LangFileString} NotAdmin "Potøebujete administrátorská práva pro nainstalování LyXu!"
${LangFileString} InstallRunning "Instalátor je již spuštìn!"
${LangFileString} AlreadyInstalled "LyX ${APP_SERIES_KEY2} je již nainstalován!$\r$\n\
				Installing over existing installations is not recommended if the installed version$\r$\n\
				is a test release or if you have problems with your existing LyX installation.$\r$\n\
				In these cases better reinstall LyX.$\r$\n\
				Dou you nevertheles want to install LyX over the existing version?"
${LangFileString} NewerInstalled "You are trying to install an older version of LyX than what you have installed.$\r$\n\
				  If you really want this, you must uninstall the existing LyX $OldVersionNumber before."

${LangFileString} MultipleIndexesNotAvailable "The support for using several indexes in a document will not be available because$\r$\n\
						MiKTeX was installed with administrator privileges but you are installing LyX without them."

${LangFileString} FinishPageMessage "Blahopøejeme! LyX byl úspìšnì nainstalován.$\r$\n\
					$\r$\n\
					(První spuštìní LyXu mùže trvat delší dobu.)"
${LangFileString} FinishPageRun "Spustit LyX"

${LangFileString} UnNotInRegistryLabel "Nelze nalézt LyX v registrech.$\r$\n\
					Zástupce na ploše a ve Start menu nebude smazán."
${LangFileString} UnInstallRunning "Nejprve musíte zavøít LyX!"
${LangFileString} UnNotAdminLabel "Musíte mít administrátorská práva pro odinstalování LyXu!"
${LangFileString} UnReallyRemoveLabel "Chcete opravdu smazat LyX a všechny jeho komponenty?"
${LangFileString} UnLyXPreferencesTitle 'Uživatelská nastavení LyXu'

${LangFileString} SecUnMiKTeXDescription "Odinstalovat LaTeXovou-distribuci MiKTeX."
${LangFileString} SecUnJabRefDescription "Odinstalovat manažer bibliografie JabRef."
${LangFileString} SecUnPreferencesDescription 'Smazat konfiguraèní adresáø LyXu$\r$\n\
						$\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						pro všechny uživatele.'
${LangFileString} SecUnProgramFilesDescription "Odinstalovat LyX a všechny jeho komponenty."

