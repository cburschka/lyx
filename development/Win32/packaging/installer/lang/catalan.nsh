!insertmacro LANGFILE_EXT "Catalan"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installed for Current User)"

${LangFileString} TEXT_WELCOME "Aquest assistent us guiarà en la instal·lació del LyX.$\r$\n\
				$\r$\n\
				$_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Updating MiKTeX filename database..."
${LangFileString} TEXT_CONFIGURE_LYX "Configuring LyX ($LaTeXInstalled may download missing packages, this can take some time) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compiling Python scripts..."

${LangFileString} TEXT_FINISH_DESKTOP "Create desktop shortcut"
${LangFileString} TEXT_FINISH_WEBSITE "Visit lyx.org for the latest news, support and tips"

${LangFileString} FileTypeTitle "Document LyX"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Voleu instal·lar-ho per a tots els usuaris?"
${LangFileString} SecFileAssocTitle "Associació de fitxers"
${LangFileString} SecDesktopTitle "Icona a l'escriptori"

${LangFileString} SecCoreDescription "Els fitxers del LyX."
${LangFileString} SecInstJabRefDescription "Gestor i editor de referències bibliogràfiques per a fitxers BibTeX."
${LangFileString} SecAllUsersDescription "Instal·la el LyX per a tots els usuaris o només per a l'usuari actual."
${LangFileString} SecFileAssocDescription "Els fitxers amb extensió .lyx s'obriran automàticament amb el LyX."
${LangFileString} SecDesktopDescription "Una icona del LyX a l'escriptori."
${LangFileString} SecDictionariesDescription "Spell-checker dictionaries that can be downloaded and installed."
${LangFileString} SecThesaurusDescription "Thesaurus dictionaries that can be downloaded and installed."

${LangFileString} EnterLaTeXHeader1 'Distribució LaTeX'
${LangFileString} EnterLaTeXHeader2 'Establiu la distribució LaTeX que ha de fer servir el LyX.'
${LangFileString} EnterLaTeXFolder 'Opcionalment podeu establir aquí el camí al fitxer $\"latex.exe$\" i posteriorment establir la \
					distribució LaTeX que el LyX ha de fer servir.\r\n\
					Si no useu LaTeX, el LyX no podrà generar documents!\r\n\
					\r\n\
					L$\'instal·lador ha detectat la distribució LaTeX \
					$\"$LaTeXName$\" al sistema, al camí que es mostra avall.'
${LangFileString} EnterLaTeXFolderNone 'Indiqueu el camí al fitxer $\"latex.exe$\". Posteriorment establiu \
					quina distribució LaTeX ha de fer servir el LyX.\r\n\
					Si no useu LaTeX, el LyX no podrà generar documents!\r\n\
					\r\n\
					L$\'instal·lador no ha trobat cap distribució LaTeX al sistema.'
${LangFileString} PathName 'Camí al fitxer $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "No usis LaTeX"
${LangFileString} InvalidLaTeXFolder 'El fitxer $\"latex.exe$\" no es troba al camí indicat.'

${LangFileString} LatexInfo 'Ara s$\'executarà l$\'instal·lador de la distribució LaTeX $\"MiKTeX$\".$\r$\n\
				Per instal·lar el programa pitgeu el botó $\"Següent$\" a les finestres de l$\'instal·lador fins que la instal·lació comenci.$\r$\n\
				$\r$\n\
				Si us plau, useu les opcions predeterminades de l$\'instal·lador MiKTeX !!!'
${LangFileString} LatexError1 'No s$\'ha trobat cap distribució LaTeX!$\r$\n\
				No podeu usar el LyX sense cap distribució LaTeX, com ara $\"MiKTeX$\"!$\r$\n\
				Es cancel·larà la instal·lació.'

${LangFileString} HunspellFailed 'Download of dictionary for language $\"$R3$\" failed.'
${LangFileString} ThesaurusFailed 'Download of thesaurus for language $\"$R3$\" failed.'

${LangFileString} JabRefInfo 'Ara s$\'executarà l$\'instal·lador del programa $\"JabRef$\".$\r$\n\
				Podeu usar totes les opcions predeterminades de l$\'instal·lador del JabRef.'
${LangFileString} JabRefError 'El programa $\"JabRef$\" no s$\'ha instal·lat correctament!$\r$\n\
				L$\'instal·lador continuarà igualment.$\r$\n\
				Intenteu instal·lar el JabRef altre cop més tard.'

${LangFileString} LatexConfigInfo "La configuració següent del LyX pot trigar una mica."

${LangFileString} MiKTeXPathInfo "Per a que cada usuari pugui personalitzar el MiKTeX segons les seves necessitats\r$\n\
					és necessari que la carpeta on es troba instal·lat el MiKTeK$\r$\n\
					$MiKTeXPath $\r$\n\
					i a les seves subcarpetes tinguin permissos d'escriptura per a tots els usuaris."
${LangFileString} MiKTeXInfo 'La distribució LaTeX $\"MiKTeX$\" conjuntament amb el LyX.$\r$\n\
				És recomenable instal·lar les actualitzacions MiKTeX fent servir el programa $\"MiKTeX Update Wizard$\"$\r$\n\
				abans d$\'executar el LyX per primer cop.$\r$\n\
				Voleu comprobar ara si hi ha actualitzacions del MiKTeX?'

${LangFileString} ModifyingConfigureFailed "No es pot establir 'path_prefix' durant el programa de configuració"
${LangFileString} RunConfigureFailed "No es pot executar el programa de configuració"
${LangFileString} NotAdmin "Necessiteu drets d'administrador per instal·lar el LyX!"
${LangFileString} InstallRunning "L'instal·lador ja s'està executant!"
${LangFileString} AlreadyInstalled "El LyX ${APP_SERIES_KEY2} ja es troba instal·lat!$\r$\n\
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

${LangFileString} FinishPageMessage "Felicitats! Heu instal·lat correctament el LyX.$\r$\n\
					$\r$\n\
					(La primera execució del LyX pot trigar alguns segons.)"
${LangFileString} FinishPageRun "Executa el LyX"

${LangFileString} UnNotInRegistryLabel "No es possible trobar el LyX al registre.$\r$\n\
					No se suprimiran les dreceres de l'escriptori i del menú inici."
${LangFileString} UnInstallRunning "Primer heu de tancar el LyX!"
${LangFileString} UnNotAdminLabel "Necessiteu drets d'administrador per desinstal·lar el LyX!"
${LangFileString} UnReallyRemoveLabel "Esteu segur de voler suprimir completament el LyX i tots els seus components?"
${LangFileString} UnLyXPreferencesTitle 'Preferències d$\'usuari del LyX'

${LangFileString} SecUnMiKTeXDescription "Desintal·la la distribució de LaTeX MiKTeX."
${LangFileString} SecUnJabRefDescription "Desinstal·la el gestor de bibliografia JabRef."
${LangFileString} SecUnPreferencesDescription 'Suprimeix les carptes de configuració del LyX$\r$\n\
						$\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						de tots els usuaris.'
${LangFileString} SecUnProgramFilesDescription "Desinstal·la el LyX i tots els seus components."

