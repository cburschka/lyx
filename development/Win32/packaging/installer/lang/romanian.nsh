!insertmacro LANGFILE_EXT "Romanian"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installed for Current User)"

${LangFileString} TEXT_WELCOME "Acest asistent vã va ghida în procesul de instalare a programului LyX. $\r$\n\
				$\r$\n\
				$_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Updating MiKTeX filename database..."
${LangFileString} TEXT_CONFIGURE_LYX "Configuring LyX ($LaTeXInstalled may download missing packages, this can take some time) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compiling Python scripts..."

${LangFileString} TEXT_FINISH_DESKTOP "Create desktop shortcut"
${LangFileString} TEXT_FINISH_WEBSITE "Visit lyx.org for the latest news, support and tips"

${LangFileString} FileTypeTitle "Document LyX"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Doriþi sã instalaþi pentru toþi utilizatorii?"
${LangFileString} SecFileAssocTitle "Asocierea fiºierelor"
${LangFileString} SecDesktopTitle "Iconiþã pe desktop"

${LangFileString} SecCoreDescription "Fiºierele LyX."
${LangFileString} SecInstJabRefDescription "Gestionarul de Bibliografie ºi editorul de fiºiere tip BibTeX."
${LangFileString} SecAllUsersDescription "Instaleazã LyX pentru toþi utilizatorii sau doar pentru utilizatorul curent."
${LangFileString} SecFileAssocDescription "Fiºierele cu extensia .lyx vor fi deschise automat cu LyX."
${LangFileString} SecDesktopDescription "A iconiþã LyX pe desktop."
${LangFileString} SecDictionariesDescription "Spell-checker dictionaries that can be downloaded and installed."
${LangFileString} SecThesaurusDescription "Thesaurus dictionaries that can be downloaded and installed."

${LangFileString} EnterLaTeXHeader1 'Distribuþia LaTeX'
${LangFileString} EnterLaTeXHeader2 'Seteazã distribuþia LaTeX pe care sã o foloseascã LyX.'
${LangFileString} EnterLaTeXFolder 'Opþional puteþi seta calea cãtre fiºierul $\"latex.exe$\" ºi, implicit, seta distribuþia \
					LaTeX pe care sã o foloseascã LyX.\r\n\
					Dacã nu folosiþi LaTeX, LyX nu va putea sã producã fiºiere!\r\n\
					\r\n\
					Programul de instalare a detectat distribuþia LaTeX \
					$\"$LaTeXName$\" instalatã pe calculatorul dumneavoastrã. Dedesupt este afiºatã calea acestei distribuþii.'
${LangFileString} EnterLaTeXFolderNone 'Setaþi calea cãtre fiºierul $\"latex.exe$\" ºi, implicit, seta distribuþia \
					LaTeX pe care sã o foloseascã LyX.\r\n\
					Dacã nu folosiþi LaTeX, LyX nu va putea sã producã fiºiere!\r\n\
					\r\n\
					Programul de instalare nu a detectat nici o distribuþia LaTeX instalatã pe calculatorul dumneavoastrã.'

${LangFileString} PathName 'Calea cãtre fiºierul $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Nu folosi LaTeX"
${LangFileString} InvalidLaTeXFolder 'Fiºierul $\"latex.exe$\" nu se aflã în calea specificatã.'

${LangFileString} LatexInfo 'Programul de instalare al distribuþiei LaTeX  $\"MiKTeX$\" va fi lansat. $\r$\n\
				Pentru instalare apãsaþi butonul $\"Next$\" în fereastra prgramului de instalare pînã cînd instalarea înncepe. $\r$\n\
				$\r$\n\
				!!! Vã rugãm sã folosiþi setãrile implicite ale programului de instalat MiKTex !!!'
${LangFileString} LatexError1 'Nu am detectat nici o distribuþia LaTeX instalatã pe calculatorul dumneavoastrã!$\r$\n\
				LyX nu poate fi folosit fãrã o distribuþia LaTeX ca  $\"MiKTeX$\"!$\r$\n\
				Instalarea va fi abandonatã.'

${LangFileString} HunspellFailed 'Download of dictionary for language $\"$R3$\" failed.'
${LangFileString} ThesaurusFailed 'Download of thesaurus for language $\"$R3$\" failed.'

${LangFileString} JabRefInfo 'Acum va fi lansat programul de instalare al aplicaþiei $\"JabRef$\".$\r$\n\
				Este indicat sã folosiþi setãrile implicite ale programului de instalat  JabRef.'
${LangFileString} JabRefError 'Programul $\"JabRef$\" nu a putut fi instalat!$\r$\n\
				Programul de instalare LyX va continua totuºi.$\r$\n\
				Încercaþi sã instalaþi JabRef mai tîrziu.'

${LangFileString} LatexConfigInfo "Configurarea programului LyX va dura o perioadã de timp."

${LangFileString} MiKTeXPathInfo "Pentru ca utilizatorii sã poatã modifica setãrile MiKTeX$\r$\n\
					este necesar sã setaþi dreptul de scriere la directorul$\r$\n\
					$MiKTeXPath $\r$\n\
					unde a fost instalat MiKTex ºi toate subdirectoarele pentru toþi utilizatorii."
${LangFileString} MiKTeXInfo 'Distribuþia LaTeX $\"MiKTeX$\" va fi folositã cu LyX.$\r$\n\
				Este recomandat sã instalaþi actualizãri ale programului MiKTex folosind programul $\"MiKTeX Update Wizard$\"$\r$\n\
				înainte de a folosi LyX pentru prima oarã.$\r$\n\
				Doriþi sã verificaþi dacã sunt disponibile actualizãri pentru MiKTeX?'

${LangFileString} ModifyingConfigureFailed "Nu am putut seta 'path_prefix' în procesul de configurare"
${LangFileString} RunConfigureFailed "Nu am putut executa scriptul de configurare"
${LangFileString} NotAdmin "Trebuie sã aveþi drepturi de administrator pentru instalarea programului LyX!"
${LangFileString} InstallRunning "Programul de instalare este deja pornit!"
${LangFileString} AlreadyInstalled "LyX ${APP_SERIES_KEY2} este deja instalat!$\r$\n\
				Installing over existing installations is not recommended if the installed version$\r$\n\
				is a test release or if you have problems with your existing LyX installation.$\r$\n\
				In these cases better reinstall LyX.$\r$\n\
				Dou you nevertheles want to install LyX over the existing version?"
${LangFileString} NewerInstalled "You are trying to install an older version of LyX than what you have installed.$\r$\n\
				  If you really want this, you must uninstall the existing LyX $OldVersionNumber before."

${LangFileString} MultipleIndexesNotAvailable "The support for using several indexes in a document will not be available because$\r$\n\
						MiKTeX was installed with administrator privileges but you are installing LyX without them."
${LangFileString} MetafileNotAvailable "The LyX support for images in the format EMF or WMF will not be$\r$\n\
					available because that requires to install a software printer to$\r$\n\
					Windows which is only possible with administrator privileges."

${LangFileString} FinishPageMessage "Felicitãri! LyX a fost instalat cu succes.$\r$\n\
					$\r$\n\
					(Prima oarã cînd porniþi LyX s-ar putea sã dureze cîteva secunde.)"
${LangFileString} FinishPageRun "Lanseazã LyX"

${LangFileString} UnNotInRegistryLabel "Nu am gãsit LyX în registri.$\r$\n\
					Scurtãturile de pe desktop ºi Start Menu nu vor fi ºterse."
${LangFileString} UnInstallRunning "Trebuie sã inchideþi LyX prima oarã!"
${LangFileString} UnNotAdminLabel "Trebuie sã aveþi drepturi de administrator pentru dezinstalarea programului LyX!"
${LangFileString} UnReallyRemoveLabel "Sunteþi sigur cã doriþi sã dezinstalaþi programul LyX ºi toate componentele lui?"
${LangFileString} UnLyXPreferencesTitle 'Preferinþele utilizatorului pentru LyX'

${LangFileString} SecUnMiKTeXDescription 'Dezinstaleazã Distribuþia LaTeX $\"MiKTeX$\".'
${LangFileString} SecUnJabRefDescription "Dezinstaleazã gestionarul de bibliografie JabRef."
${LangFileString} SecUnPreferencesDescription 'ªterge directorul cu setãrile LyX$\r$\n\
						$\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						pentru toþi utilizatorii.'
${LangFileString} SecUnProgramFilesDescription "Dezinstalaþi programul LyX ºi toate componentele lui."

