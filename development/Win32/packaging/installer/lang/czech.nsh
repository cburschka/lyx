!insertmacro LANGFILE_EXT "Czech"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installed for Current User)"

${LangFileString} TEXT_WELCOME "Tento pomocn�k v�s provede instalac� LyXu.$\r$\n\
				$\r$\n\
				$_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Updating MiKTeX filename database..."
${LangFileString} TEXT_CONFIGURE_LYX "Configuring LyX ($LaTeXInstalled may download missing packages, this can take some time) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compiling Python scripts..."

${LangFileString} TEXT_FINISH_DESKTOP "Create desktop shortcut"
${LangFileString} TEXT_FINISH_WEBSITE "Visit lyx.org for the latest news, support and tips"

${LangFileString} FileTypeTitle "LyX-dokument�"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Instalovat pro v�echny u�ivatele?"
${LangFileString} SecFileAssocTitle "Asociovat soubory"
${LangFileString} SecDesktopTitle "Ikonu na plochu"

${LangFileString} SecCoreDescription "Soubory LyXu."
${LangFileString} SecInstJabRefDescription "Spr�vce pro bibliografii a editor soubor� BibTeXu."
${LangFileString} SecAllUsersDescription "Instalovat LyX pro v�echny u�ivatele nebo pouze pro sou�asn�ho u�ivatele."
${LangFileString} SecFileAssocDescription "Soubory s p��ponou .lyx se automaticky otev�ou v LyXu."
${LangFileString} SecDesktopDescription "Ikonu LyXu na plochu."
${LangFileString} SecDictionariesDescription "Spell-checker dictionaries that can be downloaded and installed."
${LangFileString} SecThesaurusDescription "Thesaurus dictionaries that can be downloaded and installed."

${LangFileString} EnterLaTeXHeader1 'Distribuce LaTeXu'
${LangFileString} EnterLaTeXHeader2 'Nastavte distribuci LaTeXu, kterou m� LyX pou��vat.'
${LangFileString} EnterLaTeXFolder 'M��ete zde voliteln� nastavit cestu k souboru $\"latex.exe$\" a t�m ur�it \
					distribuci LaTeXu, kterou bude pou��vat Lyx.\r\n\
					Pokud nepou�ijete LaTeX, LyX nem��e vys�zet v�sledn� dokumenty!\r\n\
					\r\n\
					Instal�tor nalezl na va�em po��ta�i LaTeXovou distribuci \
					$\"$LaTeXName$\". N�e je uvedena jej� cesta.'
${LangFileString} EnterLaTeXFolderNone 'N�e nastavte cestu k souboru $\"latex.exe$\". Ur��te t�m, kter� \
					distribuce LaTeXu bude pou��v�na LyXem.\r\n\
					Pokud nepou�ijete LaTeX, LyX nem��e vys�zet v�sledn� dokumenty!\r\n\
					\r\n\
					Instal�tor nemohl nal�zt distribuci LaTeXu na va�em po��ta�i.'
${LangFileString} PathName 'Cesta k souboru $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Nepou��vat LaTeX"
${LangFileString} InvalidLaTeXFolder 'Soubor $\"latex.exe$\" nen� v zadan� cest�.'

${LangFileString} LatexInfo 'Nyn� bude spu�t�n instal�tor LaTeXov�-distribuce $\"MiKTeX$\".$\r$\n\
				V instal�toru pokra�ujte tla��tkem $\"Next$\" dokud instalace neza�ne.$\r$\n\
				$\r$\n\
				!!! U�ijte v�echny p�edvolby instal�toru MiKTeX !!!'
${LangFileString} LatexError1 '��dn� LaTeXov�-distribuce nebyla nalezena!$\r$\n\
				Nem��ete pou��vat LyX bez distribuce LaTeXu jako je nap�. $\"MiKTeX$\"!$\r$\n\
				Instalace bude proto p�eru�ena.'

${LangFileString} HunspellFailed 'Download of dictionary for language $\"$R3$\" failed.'
${LangFileString} ThesaurusFailed 'Download of thesaurus for language $\"$R3$\" failed.'

${LangFileString} JabRefInfo 'Nyn� bude spu�t�n instal�tor programu $\"JabRef$\".$\r$\n\
				M��ete pou��t v�echny p�edvolby instal�toru JabRef.'
${LangFileString} JabRefError 'Program $\"JabRef$\" nebyl �sp�n� nainstalov�n.$\r$\n\
				Instal�tor bude navzdory tomu pokra�ovat.$\r$\n\
				Pokuste se nainstalovat JabRef pozd�ji.'

${LangFileString} LatexConfigInfo "N�sleduj�c� konfigurov�n� LyXu chv�li potrv�."

${LangFileString} MiKTeXPathInfo "Aby ka�d� u�ivatel mohl pozd�ji p�izp�sobit MiKTeX pro sv� pot�eby$\r$\n\
					je t�eba nastavit pr�va z�pisu pro v�echny u�ivatele do adres��e MiKTeXu$\r$\n\
					$MiKTeXPath $\r$\n\
					a v�ech jeho podadres���."
${LangFileString} MiKTeXInfo 'LaTeXov� distribuce $\"MiKTeX$\" bude pou�ita dohromady s LyXem.$\r$\n\
				Je doporu�eno nainstalovat dostupn� aktualizace MiKTeXu pomoc� programu $\"MiKTeX Update Wizard$\"$\r$\n\
				p�edt�m ne� poprv� spust�te LyX.$\r$\n\
				Chcete zkontrolovat dostupn� aktualizace MiKTeXu nyn�?'
				     
${LangFileString} ModifyingConfigureFailed "Nelze nastavit 'path_prefix' v konfigura�n�m skriptu"
${LangFileString} RunConfigureFailed "Nelze spustit konfigura�n� skript"
${LangFileString} NotAdmin "Pot�ebujete administr�torsk� pr�va pro nainstalov�n� LyXu!"
${LangFileString} InstallRunning "Instal�tor je ji� spu�t�n!"
${LangFileString} AlreadyInstalled "LyX ${APP_SERIES_KEY2} je ji� nainstalov�n!$\r$\n\
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

${LangFileString} FinishPageMessage "Blahop�ejeme! LyX byl �sp�n� nainstalov�n.$\r$\n\
					$\r$\n\
					(Prvn� spu�t�n� LyXu m��e trvat del�� dobu.)"
${LangFileString} FinishPageRun "Spustit LyX"

${LangFileString} UnNotInRegistryLabel "Nelze nal�zt LyX v registrech.$\r$\n\
					Z�stupce na plo�e a ve Start menu nebude smaz�n."
${LangFileString} UnInstallRunning "Nejprve mus�te zav��t LyX!"
${LangFileString} UnNotAdminLabel "Mus�te m�t administr�torsk� pr�va pro odinstalov�n� LyXu!"
${LangFileString} UnReallyRemoveLabel "Chcete opravdu smazat LyX a v�echny jeho komponenty?"
${LangFileString} UnLyXPreferencesTitle 'U�ivatelsk� nastaven� LyXu'

${LangFileString} SecUnMiKTeXDescription "Odinstalovat LaTeXovou-distribuci MiKTeX."
${LangFileString} SecUnJabRefDescription "Odinstalovat mana�er bibliografie JabRef."
${LangFileString} SecUnPreferencesDescription 'Smazat konfigura�n� adres�� LyXu$\r$\n\
						$\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						pro v�echny u�ivatele.'
${LangFileString} SecUnProgramFilesDescription "Odinstalovat LyX a v�echny jeho komponenty."

