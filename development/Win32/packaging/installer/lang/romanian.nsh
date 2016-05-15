!insertmacro LANGFILE_EXT "Romanian"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installed for Current User)"

${LangFileString} TEXT_WELCOME "Acest asistent v� va ghida �n procesul de instalare a programului LyX. $\r$\n\
				$\r$\n\
				$_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Updating MiKTeX filename database..."
${LangFileString} TEXT_CONFIGURE_LYX "Configuring LyX ($LaTeXInstalled may download missing packages, this can take some time) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compiling Python scripts..."

${LangFileString} TEXT_FINISH_DESKTOP "Create desktop shortcut"
${LangFileString} TEXT_FINISH_WEBSITE "Visit lyx.org for the latest news, support and tips"

${LangFileString} FileTypeTitle "Document LyX"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Dori�i s� instala�i pentru to�i utilizatorii?"
${LangFileString} SecFileAssocTitle "Asocierea fi�ierelor"
${LangFileString} SecDesktopTitle "Iconi�� pe desktop"

${LangFileString} SecCoreDescription "Fi�ierele LyX."
${LangFileString} SecInstJabRefDescription "Gestionarul de Bibliografie �i editorul de fi�iere tip BibTeX."
${LangFileString} SecAllUsersDescription "Instaleaz� LyX pentru to�i utilizatorii sau doar pentru utilizatorul curent."
${LangFileString} SecFileAssocDescription "Fi�ierele cu extensia .lyx vor fi deschise automat cu LyX."
${LangFileString} SecDesktopDescription "A iconi�� LyX pe desktop."
${LangFileString} SecDictionariesDescription "Spell-checker dictionaries that can be downloaded and installed."
${LangFileString} SecThesaurusDescription "Thesaurus dictionaries that can be downloaded and installed."

${LangFileString} EnterLaTeXHeader1 'Distribu�ia LaTeX'
${LangFileString} EnterLaTeXHeader2 'Seteaz� distribu�ia LaTeX pe care s� o foloseasc� LyX.'
${LangFileString} EnterLaTeXFolder 'Op�ional pute�i seta calea c�tre fi�ierul $\"latex.exe$\" �i, implicit, seta distribu�ia \
					LaTeX pe care s� o foloseasc� LyX.\r\n\
					Dac� nu folosi�i LaTeX, LyX nu va putea s� produc� fi�iere!\r\n\
					\r\n\
					Programul de instalare a detectat distribu�ia LaTeX \
					$\"$LaTeXName$\" instalat� pe calculatorul dumneavoastr�. Dedesupt este afi�at� calea acestei distribu�ii.'
${LangFileString} EnterLaTeXFolderNone 'Seta�i calea c�tre fi�ierul $\"latex.exe$\" �i, implicit, seta distribu�ia \
					LaTeX pe care s� o foloseasc� LyX.\r\n\
					Dac� nu folosi�i LaTeX, LyX nu va putea s� produc� fi�iere!\r\n\
					\r\n\
					Programul de instalare nu a detectat nici o distribu�ia LaTeX instalat� pe calculatorul dumneavoastr�.'

${LangFileString} PathName 'Calea c�tre fi�ierul $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Nu folosi LaTeX"
${LangFileString} InvalidLaTeXFolder 'Fi�ierul $\"latex.exe$\" nu se afl� �n calea specificat�.'

${LangFileString} LatexInfo 'Programul de instalare al distribu�iei LaTeX  $\"MiKTeX$\" va fi lansat. $\r$\n\
				Pentru instalare ap�sa�i butonul $\"Next$\" �n fereastra prgramului de instalare p�n� c�nd instalarea �nncepe. $\r$\n\
				$\r$\n\
				!!! V� rug�m s� folosi�i set�rile implicite ale programului de instalat MiKTex !!!'
${LangFileString} LatexError1 'Nu am detectat nici o distribu�ia LaTeX instalat� pe calculatorul dumneavoastr�!$\r$\n\
				LyX nu poate fi folosit f�r� o distribu�ia LaTeX ca  $\"MiKTeX$\"!$\r$\n\
				Instalarea va fi abandonat�.'

${LangFileString} HunspellFailed 'Download of dictionary for language $\"$R3$\" failed.'
${LangFileString} ThesaurusFailed 'Download of thesaurus for language $\"$R3$\" failed.'

${LangFileString} JabRefInfo 'Acum va fi lansat programul de instalare al aplica�iei $\"JabRef$\".$\r$\n\
				Este indicat s� folosi�i set�rile implicite ale programului de instalat  JabRef.'
${LangFileString} JabRefError 'Programul $\"JabRef$\" nu a putut fi instalat!$\r$\n\
				Programul de instalare LyX va continua totu�i.$\r$\n\
				�ncerca�i s� instala�i JabRef mai t�rziu.'

${LangFileString} LatexConfigInfo "Configurarea programului LyX va dura o perioad� de timp."

${LangFileString} MiKTeXPathInfo "Pentru ca utilizatorii s� poat� modifica set�rile MiKTeX$\r$\n\
					este necesar s� seta�i dreptul de scriere la directorul$\r$\n\
					$MiKTeXPath $\r$\n\
					unde a fost instalat MiKTex �i toate subdirectoarele pentru to�i utilizatorii."
${LangFileString} MiKTeXInfo 'Distribu�ia LaTeX $\"MiKTeX$\" va fi folosit� cu LyX.$\r$\n\
				Este recomandat s� instala�i actualiz�ri ale programului MiKTex folosind programul $\"MiKTeX Update Wizard$\"$\r$\n\
				�nainte de a folosi LyX pentru prima oar�.$\r$\n\
				Dori�i s� verifica�i dac� sunt disponibile actualiz�ri pentru MiKTeX?'

${LangFileString} ModifyingConfigureFailed "Nu am putut seta 'path_prefix' �n procesul de configurare"
${LangFileString} RunConfigureFailed "Nu am putut executa scriptul de configurare"
${LangFileString} NotAdmin "Trebuie s� ave�i drepturi de administrator pentru instalarea programului LyX!"
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

${LangFileString} FinishPageMessage "Felicit�ri! LyX a fost instalat cu succes.$\r$\n\
					$\r$\n\
					(Prima oar� c�nd porni�i LyX s-ar putea s� dureze c�teva secunde.)"
${LangFileString} FinishPageRun "Lanseaz� LyX"

${LangFileString} UnNotInRegistryLabel "Nu am g�sit LyX �n registri.$\r$\n\
					Scurt�turile de pe desktop �i Start Menu nu vor fi �terse."
${LangFileString} UnInstallRunning "Trebuie s� inchide�i LyX prima oar�!"
${LangFileString} UnNotAdminLabel "Trebuie s� ave�i drepturi de administrator pentru dezinstalarea programului LyX!"
${LangFileString} UnReallyRemoveLabel "Sunte�i sigur c� dori�i s� dezinstala�i programul LyX �i toate componentele lui?"
${LangFileString} UnLyXPreferencesTitle 'Preferin�ele utilizatorului pentru LyX'

${LangFileString} SecUnMiKTeXDescription 'Dezinstaleaz� Distribu�ia LaTeX $\"MiKTeX$\".'
${LangFileString} SecUnJabRefDescription "Dezinstaleaz� gestionarul de bibliografie JabRef."
${LangFileString} SecUnPreferencesDescription '�terge directorul cu set�rile LyX$\r$\n\
						$\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						pentru to�i utilizatorii.'
${LangFileString} SecUnProgramFilesDescription "Dezinstala�i programul LyX �i toate componentele lui."

