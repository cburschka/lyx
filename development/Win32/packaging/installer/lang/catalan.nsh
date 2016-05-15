!insertmacro LANGFILE_EXT "Catalan"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installed for Current User)"

${LangFileString} TEXT_WELCOME "Aquest assistent us guiar� en la instal�laci� del LyX.$\r$\n\
				$\r$\n\
				$_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Updating MiKTeX filename database..."
${LangFileString} TEXT_CONFIGURE_LYX "Configuring LyX ($LaTeXInstalled may download missing packages, this can take some time) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compiling Python scripts..."

${LangFileString} TEXT_FINISH_DESKTOP "Create desktop shortcut"
${LangFileString} TEXT_FINISH_WEBSITE "Visit lyx.org for the latest news, support and tips"

${LangFileString} FileTypeTitle "Document LyX"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Voleu instal�lar-ho per a tots els usuaris?"
${LangFileString} SecFileAssocTitle "Associaci� de fitxers"
${LangFileString} SecDesktopTitle "Icona a l'escriptori"

${LangFileString} SecCoreDescription "Els fitxers del LyX."
${LangFileString} SecInstJabRefDescription "Gestor i editor de refer�ncies bibliogr�fiques per a fitxers BibTeX."
${LangFileString} SecAllUsersDescription "Instal�la el LyX per a tots els usuaris o nom�s per a l'usuari actual."
${LangFileString} SecFileAssocDescription "Els fitxers amb extensi� .lyx s'obriran autom�ticament amb el LyX."
${LangFileString} SecDesktopDescription "Una icona del LyX a l'escriptori."
${LangFileString} SecDictionariesDescription "Spell-checker dictionaries that can be downloaded and installed."
${LangFileString} SecThesaurusDescription "Thesaurus dictionaries that can be downloaded and installed."

${LangFileString} EnterLaTeXHeader1 'Distribuci� LaTeX'
${LangFileString} EnterLaTeXHeader2 'Establiu la distribuci� LaTeX que ha de fer servir el LyX.'
${LangFileString} EnterLaTeXFolder 'Opcionalment podeu establir aqu� el cam� al fitxer $\"latex.exe$\" i posteriorment establir la \
					distribuci� LaTeX que el LyX ha de fer servir.\r\n\
					Si no useu LaTeX, el LyX no podr� generar documents!\r\n\
					\r\n\
					L$\'instal�lador ha detectat la distribuci� LaTeX \
					$\"$LaTeXName$\" al sistema, al cam� que es mostra avall.'
${LangFileString} EnterLaTeXFolderNone 'Indiqueu el cam� al fitxer $\"latex.exe$\". Posteriorment establiu \
					quina distribuci� LaTeX ha de fer servir el LyX.\r\n\
					Si no useu LaTeX, el LyX no podr� generar documents!\r\n\
					\r\n\
					L$\'instal�lador no ha trobat cap distribuci� LaTeX al sistema.'
${LangFileString} PathName 'Cam� al fitxer $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "No usis LaTeX"
${LangFileString} InvalidLaTeXFolder 'El fitxer $\"latex.exe$\" no es troba al cam� indicat.'

${LangFileString} LatexInfo 'Ara s$\'executar� l$\'instal�lador de la distribuci� LaTeX $\"MiKTeX$\".$\r$\n\
				Per instal�lar el programa pitgeu el bot� $\"Seg�ent$\" a les finestres de l$\'instal�lador fins que la instal�laci� comenci.$\r$\n\
				$\r$\n\
				Si us plau, useu les opcions predeterminades de l$\'instal�lador MiKTeX !!!'
${LangFileString} LatexError1 'No s$\'ha trobat cap distribuci� LaTeX!$\r$\n\
				No podeu usar el LyX sense cap distribuci� LaTeX, com ara $\"MiKTeX$\"!$\r$\n\
				Es cancel�lar� la instal�laci�.'

${LangFileString} HunspellFailed 'Download of dictionary for language $\"$R3$\" failed.'
${LangFileString} ThesaurusFailed 'Download of thesaurus for language $\"$R3$\" failed.'

${LangFileString} JabRefInfo 'Ara s$\'executar� l$\'instal�lador del programa $\"JabRef$\".$\r$\n\
				Podeu usar totes les opcions predeterminades de l$\'instal�lador del JabRef.'
${LangFileString} JabRefError 'El programa $\"JabRef$\" no s$\'ha instal�lat correctament!$\r$\n\
				L$\'instal�lador continuar� igualment.$\r$\n\
				Intenteu instal�lar el JabRef altre cop m�s tard.'

${LangFileString} LatexConfigInfo "La configuraci� seg�ent del LyX pot trigar una mica."

${LangFileString} MiKTeXPathInfo "Per a que cada usuari pugui personalitzar el MiKTeX segons les seves necessitats\r$\n\
					�s necessari que la carpeta on es troba instal�lat el MiKTeK$\r$\n\
					$MiKTeXPath $\r$\n\
					i a les seves subcarpetes tinguin permissos d'escriptura per a tots els usuaris."
${LangFileString} MiKTeXInfo 'La distribuci� LaTeX $\"MiKTeX$\" conjuntament amb el LyX.$\r$\n\
				�s recomenable instal�lar les actualitzacions MiKTeX fent servir el programa $\"MiKTeX Update Wizard$\"$\r$\n\
				abans d$\'executar el LyX per primer cop.$\r$\n\
				Voleu comprobar ara si hi ha actualitzacions del MiKTeX?'

${LangFileString} ModifyingConfigureFailed "No es pot establir 'path_prefix' durant el programa de configuraci�"
${LangFileString} RunConfigureFailed "No es pot executar el programa de configuraci�"
${LangFileString} NotAdmin "Necessiteu drets d'administrador per instal�lar el LyX!"
${LangFileString} InstallRunning "L'instal�lador ja s'est� executant!"
${LangFileString} AlreadyInstalled "El LyX ${APP_SERIES_KEY2} ja es troba instal�lat!$\r$\n\
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

${LangFileString} FinishPageMessage "Felicitats! Heu instal�lat correctament el LyX.$\r$\n\
					$\r$\n\
					(La primera execuci� del LyX pot trigar alguns segons.)"
${LangFileString} FinishPageRun "Executa el LyX"

${LangFileString} UnNotInRegistryLabel "No es possible trobar el LyX al registre.$\r$\n\
					No se suprimiran les dreceres de l'escriptori i del men� inici."
${LangFileString} UnInstallRunning "Primer heu de tancar el LyX!"
${LangFileString} UnNotAdminLabel "Necessiteu drets d'administrador per desinstal�lar el LyX!"
${LangFileString} UnReallyRemoveLabel "Esteu segur de voler suprimir completament el LyX i tots els seus components?"
${LangFileString} UnLyXPreferencesTitle 'Prefer�ncies d$\'usuari del LyX'

${LangFileString} SecUnMiKTeXDescription "Desintal�la la distribuci� de LaTeX MiKTeX."
${LangFileString} SecUnJabRefDescription "Desinstal�la el gestor de bibliografia JabRef."
${LangFileString} SecUnPreferencesDescription 'Suprimeix les carptes de configuraci� del LyX$\r$\n\
						$\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						de tots els usuaris.'
${LangFileString} SecUnProgramFilesDescription "Desinstal�la el LyX i tots els seus components."

