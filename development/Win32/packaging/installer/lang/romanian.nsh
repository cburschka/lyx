!insertmacro LANGFILE_EXT "Romanian"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installed for Current User)"

${LangFileString} TEXT_WELCOME "Acest asistent vă va ghida în procesul de instalare a programului LyX. $\r$\n\
				$\r$\n\
				$_CLICK"

#${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Updating MiKTeX filename database..."
${LangFileString} TEXT_CONFIGURE_LYX "Configuring LyX ($LaTeXInstalled may download missing packages, this can take some time) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compiling Python scripts..."

${LangFileString} TEXT_FINISH_DESKTOP "Create desktop shortcut"
${LangFileString} TEXT_FINISH_WEBSITE "Visit lyx.org for the latest news, support and tips"

#${LangFileString} FileTypeTitle "Document LyX"

#${LangFileString} SecInstJabRefTitle "JabRef"
#${LangFileString} SecAllUsersTitle "Doriţi să instalaţi pentru toţi utilizatorii?"
${LangFileString} SecFileAssocTitle "Asocierea fişierelor"
${LangFileString} SecDesktopTitle "Iconiţă pe desktop"

${LangFileString} SecCoreDescription "Fişierele LyX."
#${LangFileString} SecInstJabRefDescription "Gestionarul de Bibliografie şi editorul de fişiere tip BibTeX."
#${LangFileString} SecAllUsersDescription "Instalează LyX pentru toţi utilizatorii sau doar pentru utilizatorul curent."
${LangFileString} SecFileAssocDescription "Fişierele cu extensia .lyx vor fi deschise automat cu LyX."
${LangFileString} SecDesktopDescription "A iconiţă LyX pe desktop."
${LangFileString} SecDictionaries "Dicționare"
${LangFileString} SecDictionariesDescription "Spell-checker dictionaries that can be downloaded and installed."
${LangFileString} SecThesaurus "Tezaur"
${LangFileString} SecThesaurusDescription "Thesaurus dictionaries that can be downloaded and installed."

${LangFileString} EnterLaTeXHeader1 'Distribuţia LaTeX'
${LangFileString} EnterLaTeXHeader2 'Setează distribuţia LaTeX pe care să o folosească LyX.'
${LangFileString} EnterLaTeXFolder 'Opţional puteţi seta calea către fişierul $\"latex.exe$\" şi, implicit, seta distribuţia \
					LaTeX pe care să o folosească LyX.\r\n\
					Dacă nu folosiţi LaTeX, LyX nu va putea să producă fişiere!\r\n\
					\r\n\
					Programul de instalare a detectat distribuţia LaTeX \
					$\"$LaTeXName$\" instalată pe calculatorul dumneavoastră. Dedesupt este afişată calea acestei distribuţii.'
${LangFileString} EnterLaTeXFolderNone 'Setaţi calea către fişierul $\"latex.exe$\" şi, implicit, seta distribuţia \
					LaTeX pe care să o folosească LyX.\r\n\
					Dacă nu folosiţi LaTeX, LyX nu va putea să producă fişiere!\r\n\
					\r\n\
					Programul de instalare nu a detectat nici o distribuţia LaTeX instalată pe calculatorul dumneavoastră.'

${LangFileString} PathName 'Calea către fişierul $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Nu folosi LaTeX"
${LangFileString} InvalidLaTeXFolder 'Fişierul $\"latex.exe$\" nu se află în calea specificată.'

${LangFileString} LatexInfo 'Programul de instalare al distribuţiei LaTeX  $\"MiKTeX$\" va fi lansat. $\r$\n\
				Pentru instalare apăsaţi butonul $\"Next$\" în fereastra prgramului de instalare pînă cînd instalarea înncepe. $\r$\n\
				$\r$\n\
				!!! Vă rugăm să folosiţi setările implicite ale programului de instalat MiKTex !!!'
${LangFileString} LatexError1 'Nu am detectat nici o distribuţia LaTeX instalată pe calculatorul dumneavoastră!$\r$\n\
				LyX nu poate fi folosit fără o distribuţia LaTeX ca  $\"MiKTeX$\"!$\r$\n\
				Instalarea va fi abandonată.'

${LangFileString} HunspellFailed 'Download of dictionary for language $\"$R3$\" failed.'
${LangFileString} ThesaurusFailed 'Download of thesaurus for language $\"$R3$\" failed.'

#${LangFileString} JabRefInfo 'Acum va fi lansat programul de instalare al aplicaţiei $\"JabRef$\".$\r$\n\
#				Este indicat să folosiţi setările implicite ale programului de instalat  JabRef.'
#${LangFileString} JabRefError 'Programul $\"JabRef$\" nu a putut fi instalat!$\r$\n\
#				Programul de instalare LyX va continua totuşi.$\r$\n\
#				Încercaţi să instalaţi JabRef mai tîrziu.'

#${LangFileString} LatexConfigInfo "Configurarea programului LyX va dura o perioadă de timp."

#${LangFileString} MiKTeXPathInfo "Pentru ca utilizatorii să poată modifica setările MiKTeX$\r$\n\
#					este necesar să setaţi dreptul de scriere la directorul$\r$\n\
#					$MiKTeXPath $\r$\n\
#					unde a fost instalat MiKTex şi toate subdirectoarele pentru toţi utilizatorii."
${LangFileString} MiKTeXInfo 'Distribuţia LaTeX $\"MiKTeX$\" va fi folosită cu LyX.$\r$\n\
				Este recomandat să instalaţi actualizări ale programului MiKTex folosind programul $\"MiKTeX Update Wizard$\"$\r$\n\
				înainte de a folosi LyX pentru prima oară.$\r$\n\
				Doriţi să verificaţi dacă sunt disponibile actualizări pentru MiKTeX?'

${LangFileString} ModifyingConfigureFailed "Nu am putut seta 'path_prefix' în procesul de configurare"
#${LangFileString} RunConfigureFailed "Nu am putut executa scriptul de configurare"
${LangFileString} NotAdmin "Trebuie să aveţi drepturi de administrator pentru instalarea programului LyX!"
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

#${LangFileString} FinishPageMessage "Felicitări! LyX a fost instalat cu succes.$\r$\n\
#					$\r$\n\
#					(Prima oară cînd porniţi LyX s-ar putea să dureze cîteva secunde.)"
${LangFileString} FinishPageRun "Lansează LyX"

${LangFileString} UnNotInRegistryLabel "Nu am găsit LyX în registri.$\r$\n\
					Scurtăturile de pe desktop şi Start Menu nu vor fi şterse."
${LangFileString} UnInstallRunning "Trebuie să inchideţi LyX prima oară!"
${LangFileString} UnNotAdminLabel "Trebuie să aveţi drepturi de administrator pentru dezinstalarea programului LyX!"
${LangFileString} UnReallyRemoveLabel "Sunteţi sigur că doriţi să dezinstalaţi programul LyX şi toate componentele lui?"
${LangFileString} UnLyXPreferencesTitle 'Preferinţele utilizatorului pentru LyX'

${LangFileString} SecUnMiKTeXDescription 'Dezinstalează Distribuţia LaTeX $\"MiKTeX$\".'
#${LangFileString} SecUnJabRefDescription "Dezinstalează gestionarul de bibliografie JabRef."
${LangFileString} SecUnPreferencesDescription 'Şterge directorul cu setările LyX$\r$\n\
						$\"$AppPre\username\$\r$\n\
						$AppSuff\$\r$\n\
						${APP_DIR_USERDATA}$\")$\r$\n\
						pentru toţi utilizatorii.'
${LangFileString} SecUnProgramFilesDescription "Dezinstalaţi programul LyX şi toate componentele lui."

# MikTeX Update Stuff
${LangFileString} MiKTeXUpdateMsg "Datorită schimbărilor în MiKTeX, instalarea versiunilor LyX mai târziu de 2.2.3 poate provoca o actualizare parțială care face MiKTeX inutilizabil.$\r$\n\
$\r$\n\
Datorită schimbărilor în MiKTeX, instalarea versiunilor LyX mai târziu de 2.2.3 poate provoca o actualizare parțială care face MiKTeX inutilizabil.$\r$\n\
$\r$\n\
Datorită schimbărilor în MiKTeX, instalarea versiunilor LyX mai târziu de 2.2.3 poate provoca o actualizare parțială care face MiKTeX inutilizabil.$\r$\n\
$\r$\n\
(Ne pare rău pentru traducerea automată.)"

${LangFileString} MiKTeXUpdateHeader "AVERTISMENT: MiKTeX Modificări"
