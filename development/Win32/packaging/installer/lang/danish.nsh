!insertmacro LANGFILE_EXT "Danish"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installed for Current User)"

${LangFileString} TEXT_WELCOME "Denne guide vil installere LyX p� din computer.$\r$\n\
				$\r$\n\
				$_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Updating MiKTeX filename database..."
${LangFileString} TEXT_CONFIGURE_LYX "Configuring LyX ($LaTeXInstalled may download missing packages, this can take some time) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compiling Python scripts..."

${LangFileString} TEXT_FINISH_DESKTOP "Create desktop shortcut"
${LangFileString} TEXT_FINISH_WEBSITE "Visit lyx.org for the latest news, support and tips"

${LangFileString} FileTypeTitle "LyX-Dokument"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Installer til alle brugere?"
${LangFileString} SecFileAssocTitle "Fil-associationer"
${LangFileString} SecDesktopTitle "Skrivebordsikon"

${LangFileString} SecCoreDescription "Filerne til LyX."
${LangFileString} SecInstJabRefDescription "Bibliography reference manager and editor for BibTeX files."
${LangFileString} SecAllUsersDescription "Installer LyX til alle brugere, eller kun den aktuelle bruger."
${LangFileString} SecFileAssocDescription "Opret association mellem LyX og .lyx filer."
${LangFileString} SecDesktopDescription "Et LyX ikon p� skrivebordet"
${LangFileString} SecDictionariesDescription "Spell-checker dictionaries that can be downloaded and installed."
${LangFileString} SecThesaurusDescription "Thesaurus dictionaries that can be downloaded and installed."

${LangFileString} EnterLaTeXHeader1 'LaTeX-distribution'
${LangFileString} EnterLaTeXHeader2 'V�lg den LaTeX-distribution som LyX skal bruge.'
${LangFileString} EnterLaTeXFolder 'Du kan her v�lge stien til filen $\"latex.exe$\" og derved v�lge hvilken \
					LaTeX-distribution som bruges LyX.\r\n\
					Hvis du ikke v�lger LaTeX, kan LyX ikke generere dokumenter!\r\n\
					\r\n\
					Installationsprogrammet har detekteret LaTeX-distributionen \
					$\"$LaTeXName$\" p� din computer i nedenst�ende sti:'
${LangFileString} EnterLaTeXFolderNone 'V�lg nedenfor stien til filen $\"latex.exe$\". Herved kan du v�lge hvilken \
					LaTeX-distribution LyX skal bruge.\r\n\
					Hvis du ikke v�lger LaTeX, kan LyX ikke generere dokumenter!\r\n\
					\r\n\
					Installationsprogrammet kunne ikke finde nogen LaTeX-distribution.'
${LangFileString} PathName 'Sti til filen $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Brug ikke LaTeX"
${LangFileString} InvalidLaTeXFolder 'Kunne ikke finde $\"latex.exe$\".'

${LangFileString} LatexInfo 'Installationen af LaTeX-distributionen $\"MiKTeX$\" startes.$\r$\n\
				For at installere programmet tryk p� $\"Next$\"-knappen i installationsvinduet indtil installationen begynder.$\r$\n\
				$\r$\n\
				!!! Brug alle MiKTeX installationsprogrammets foresl�de valg !!!'
${LangFileString} LatexError1 'Ingen LaTeX-distribution fundet!$\r$\n\
				LyX kan ikke bruges uden en LaTeX-distribution sasom $\"MiKTeX$\"!$\r$\n\
				Installationen afbrydes.'

${LangFileString} HunspellFailed 'Download of dictionary for language $\"$R3$\" failed.'
${LangFileString} ThesaurusFailed 'Download of thesaurus for language $\"$R3$\" failed.'

${LangFileString} JabRefInfo 'Installationen af programmet $\"JabRef$\" startes.$\r$\n\
				Du kan roligt bruge alle $\"default$\" valgmuligheder ved JabRef-installationen.'
${LangFileString} JabRefError 'Programmet $\"JabRef$\" kunne ikke installeres!$\r$\n\
				Installationen vil dog forts�tte.$\r$\n\
				Pr�v at installere JabRef igen senere.'

${LangFileString} LatexConfigInfo "Den f�lgende konfiguration af LyX vil tage et stykke tid."

${LangFileString} MiKTeXPathInfo "For at alle brugere senere kan konfigurere  MiKTeX$\r$\n\
					er det n�dvendigt at give skriverettigheder for alle MiKTeX brugere i installations mappen$\r$\n\
					$MiKTeXPath $\r$\n\
					og dennes undermapper."
${LangFileString} MiKTeXInfo 'LaTeX-distributionen $\"MiKTeX$\" vil blive brugt sammen med LyX.$\r$\n\
				Det anbefales at installere tilg�ngelige MiKTeX-opdateringer ved hj�lp af programmet $\"MiKTeX Update Wizard$\"$\r$\n\
				f�r LyX startes for f�rste gang.$\r$\n\
				Vil du tjekke om der findes MiKTeX opdateringer?'

${LangFileString} ModifyingConfigureFailed "Forsoget p� at indstille 'path_prefix' i konfigurationen mislykkedes"
${LangFileString} RunConfigureFailed "Mislykket forsog p� at afvikle konfigurations-scriptet"
${LangFileString} NotAdmin "Du skal have administrator-rettigheder for at installere LyX!"
${LangFileString} InstallRunning "Installationsprogrammet k�rer allerede!"
${LangFileString} AlreadyInstalled "LyX ${APP_SERIES_KEY2} er allerede installeret!$\r$\n\
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

${LangFileString} FinishPageMessage "Tillykke!! LyX er installeret.$\r$\n\
					$\r$\n\
					(N�r LyX startes f�rste gang, kan det tage noget tid.)"
${LangFileString} FinishPageRun "Start LyX"

${LangFileString} UnNotInRegistryLabel "Kunne ikke finde LyX i registreringsdatabsen.$\r$\n\
					Genvejene p� skrivebordet og i Start-menuen bliver ikke fjernet"
${LangFileString} UnInstallRunning "Du ma afslutte LyX forst!"
${LangFileString} UnNotAdminLabel "Du skal have administrator-rettigheder for at afinstallere LyX!"
${LangFileString} UnReallyRemoveLabel "Er du sikker p�, at du vil slette LyX og alle tilh�rende komponenter?"
${LangFileString} UnLyXPreferencesTitle 'LyX$\'s user preferences'

${LangFileString} SecUnMiKTeXDescription 'Afinstallerer LaTeX-distributionen $\"MiKTeX$\".'
${LangFileString} SecUnJabRefDescription 'Afinstallerer bibliografi programmet $\"JabRef$\".'
${LangFileString} SecUnPreferencesDescription 'Sletter LyX$\'s konfigurations mappe$\r$\n\
						$\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						for alle brugere.'
${LangFileString} SecUnProgramFilesDescription "Afinstallerer LyX og alle dets komponenter."

