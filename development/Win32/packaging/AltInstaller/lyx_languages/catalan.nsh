!ifndef _LYX_LANGUAGES_CATALAN_NSH_
!define _LYX_LANGUAGES_CATALAN_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_CATALAN}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "Aquest assistent us guiarà en la instal·lació del LyX.\r\n\
					  \r\n\
					  Necessiteu drets d'administrador per instal·lar el LyX.\r\n\
					  \r\n\
					  Abans de començar la instal·lació, és recomenable tancar totes les altres aplicacions. Això permet actualitzar alguns fitxerr del sistema sense haver de reiniciar l'ordinador.\r\n\
					  \r\n\
					  $_CLICK"

LangString FileTypeTitle "${LYX_LANG}" "Document LyX"

LangString SecInstGSviewTitle "${LYX_LANG}" "GSview"
LangString SecInstJabRefTitle "${LYX_LANG}" "JabRef"
LangString SecAllUsersTitle "${LYX_LANG}" "Voleu instal·lar-ho per a tots els usuaris?"
LangString SecFileAssocTitle "${LYX_LANG}" "Associació de fitxers"
LangString SecDesktopTitle "${LYX_LANG}" "Icona a l'escriptori"

LangString SecCoreDescription "${LYX_LANG}" "Els fitxers del LyX."
LangString SecInstGSviewDescription "${LYX_LANG}" "Programa per visualitzar documents en format Postscript i PDF."
LangString SecInstJabRefDescription "${LYX_LANG}" "Gestor i editor de referències bibliogràfiques per a fitxers BibTeX."
LangString SecAllUsersDescription "${LYX_LANG}" "Instal·la el LyX per a tots els usuaris o només per a l'usuari actual."
LangString SecFileAssocDescription "${LYX_LANG}" "Els fitxers amb extensió .lyx s'obriran automàticament amb el LyX."
LangString SecDesktopDescription "${LYX_LANG}" "Una icona del LyX a l'escriptori."

LangString LangSelectHeader "${LYX_LANG}" "Selecció de la llengua de menús del LyX"
LangString AvailableLang "${LYX_LANG}" " Llengües disponibles "

LangString MissProgHeader "${LYX_LANG}" "Verificació dels programes requerits"
LangString MissProgCap "${LYX_LANG}" "Addicionalment, s'instal·laran els programes següents"
LangString MissProgLatex "${LYX_LANG}" "MiKTeX, una distribució de LaTeX"
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript, un intèrpret per a documents PostScript i PDF"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick, un conversor d'imatges"
LangString MissProgAspell "${LYX_LANG}" "Aspell, un corrector ortogràfic"
LangString MissProgMessage "${LYX_LANG}" 'No és necessari instal·lar cap programa addicional.'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'Distribució LaTeX'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'Establiu la distribució LaTeX que ha de fer servir el LyX.'
LangString EnterLaTeXFolder "${LYX_LANG}" 'Opcionalment podeu establir aquí el camí al fitxer "latex.exe" i posteriorment establir la \
					   distribució LaTeX que el LyX ha de fer servir.\r\n\
					   Si no useu LaTeX, el LyX no podrà generar documents!\r\n\
					   \r\n\
					   L$\'instal·lador ha detectat la distribució LaTeX \
					   "$LaTeXName" al sistema, al camí que es mostra avall.'
LangString EnterLaTeXFolderNone "${LYX_LANG}" 'Indiqueu el camí al fitxer "latex.exe". Posteriorment establiu \
					       quina distribució LaTeX ha de fer servir el LyX.\r\n\
					       Si no useu LaTeX, el LyX no podrà generar documents!\r\n\
					       \r\n\
					       L$\'instal·lador no ha trobat cap distribució LaTeX al sistema.'
LangString PathName "${LYX_LANG}" 'Camí al fitxer "latex.exe"'
LangString DontUseLaTeX "${LYX_LANG}" "No usis LaTeX"
LangString InvalidLaTeXFolder "${LYX_LANG}" 'El fitxer "latex.exe" no es troba al camí indicat.'

LangString LatexInfo "${LYX_LANG}" 'Ara s$\'executarà l$\'instal·lador de la distribució LaTeX "MiKTeX".$\r$\n\
			            Per instal·lar el programa pitgeu el botó "Següent" a les finestres de l$\'instal·lador fins que la instal·lació comenci.$\r$\n\
				    $\r$\n\
				    Si us plau, useu les opcions predeterminades de l$\'instal·lador MiKTeX !!!'
LangString LatexError1 "${LYX_LANG}" 'No s$\'ha trobat cap distribució LaTeX!$\r$\n\
                      		      No podeu usar el LyX sense cap distribució LaTeX, com ara "MiKTeX"!$\r$\n\
				      Es cancel·larà la instal·lació.'
			    
LangString GSviewInfo "${LYX_LANG}" 'Ara s$\'executarà l$\'instal·lador del programa "GSview".$\r$\n\
			             Per instal·lar el programa pitgeu el botó "Setup" a la primera finestra de l$\'instal·lador,$\r$\n\
				     seleccioneu la llengua i pitgeu el botó "Següent" a la finstres següent de l$\'instal·lador.$\r$\n\
				     Podeu usar totes les opcions predeterminades de l$\'instal·lador del GSview.'
LangString GSviewError "${LYX_LANG}" 'El programa "GSview" no s$\'ha instal·lat correctament!$\r$\n\
		                      L$\'instal·lador continuarà igualment.$\r$\n\
				      Intenteu instal·lar el GSview altre cop més tard.'
				      
LangString JabRefInfo "${LYX_LANG}" 'Ara s$\'executarà l$\'instal·lador del programa "JabRef".$\r$\n\
				     Podeu usar totes les opcions predeterminades de l$\'instal·lador del JabRef.'
LangString JabRefError "${LYX_LANG}" 'El programa "JabRef" no s$\'ha instal·lat correctament!$\r$\n\
		                      L$\'instal·lador continuarà igualment.$\r$\n\
				      Intenteu instal·lar el JabRef altre cop més tard.'
				      
LangString LatexConfigInfo "${LYX_LANG}" "La configuració següent del LyX pot trigar una mica."

LangString AspellInfo "${LYX_LANG}" 'Ara es descarregaran i instal·laran els diccionaris pel corrector ortogràfic "Aspell".$\r$\n\
				     Cada diccionari té una llicència diferent que es mostrarà abans de la instal·lació.'
LangString AspellDownloadFailed "${LYX_LANG}" "No s$\'ha pogut descarregar cap diccionari de l$\'Aspell!"
LangString AspellInstallFailed "${LYX_LANG}" "No s$\'ha pogut instal·lar cap diccionari de l$\'Aspell!"
LangString AspellPartAnd "${LYX_LANG}" " i "
LangString AspellPartSep "${LYX_LANG}" ", "
LangString AspellAll "${LYX_LANG}" 'Ja es troba instal·lat un diccionari per a la llengua'
LangString AspellNone "${LYX_LANG}" 'No s$\'ha instal·lat el diccionari de l$\'Aspell.$\r$\n\
				     Podeu descarregar els diccionaris des de$\r$\n\
				     ${AspellLocation}$\r$\n\
				     Voleu descarregar els diccionaris ara?'
LangString AspellPartStart "${LYX_LANG}" "S'ha instal·lat correctament "
LangString AspellPart1 "${LYX_LANG}" "un diccionari per a la llengua anglesa"
LangString AspellPart2 "${LYX_LANG}" "un diccionari per a la llengua $LangName"
LangString AspellPart3 "${LYX_LANG}" "un$\r$\n\
				      diccionari per a la llengua $LangNameSys"
LangString AspellPart4 "${LYX_LANG}" ' pel corrector ortogràfic "Aspell".$\r$\n\
				      Podeu descarregar més diccionaris des de$\r$\n\
				      ${AspellLocation}$\r$\n\
				      Voleu descarregar altres diccionaris ara?'

LangString MiKTeXPathInfo "${LYX_LANG}" "Per a que cada usuari pugui personalitzar el MiKTeX segons les seves necessitats\r$\n\
					 és necessari que la carpeta on es troba instal·lat el MiKTeK$\r$\n\
					 $MiKTeXPath $\r$\n\
					 i a les seves subcarpetes tinguin permissos d'escriptura per a tots els usuaris."
LangString MiKTeXInfo "${LYX_LANG}" 'La distribució LaTeX "MiKTeX" conjuntament amb el LyX.$\r$\n\
				     És recomenable instal·lar les actualitzacions MiKTeX fent servir el programa "MiKTeX Update Wizard"$\r$\n\
				     abans d$\'executar el LyX per primer cop.$\r$\n\
				     Voleu comprobar ara si hi ha actualitzacions del MiKTeX?'

LangString UpdateNotAllowed "${LYX_LANG}" "Aquest paquet d'actualització només actualitza ${PRODUCT_VERSION_OLD}!"
LangString ModifyingConfigureFailed "${LYX_LANG}" "No es pot establir 'path_prefix' durant el programa de configuració"
LangString CreateCmdFilesFailed "${LYX_LANG}" "No es pot crear el fitxer lyx.bat"
LangString RunConfigureFailed "${LYX_LANG}" "No es pot executar el programa de configuració"
LangString NotAdmin "${LYX_LANG}" "Necessiteu drets d'administrador per instal·lar el LyX!"
LangString InstallRunning "${LYX_LANG}" "L'instal·lador ja s'està executant!"
LangString StillInstalled "${LYX_LANG}" "El LyX ja es troba instal·lat! Desinstal·leu-lo primer."

LangString FinishPageMessage "${LYX_LANG}" "Felicitats! Heu instal·lat correctament el LyX.\r\n\
					    \r\n\
					    (La primera execució del LyX pot trigar alguns segons.)"
LangString FinishPageRun "${LYX_LANG}" "Executa el LyX"

LangString UnNotInRegistryLabel "${LYX_LANG}" "No es possible trobar el LyX al registre.$\r$\n\
					       No se suprimiran les dreceres de l'escriptori i del menú inici."
LangString UnInstallRunning "${LYX_LANG}" "Primer heu de tancar el LyX!"
LangString UnNotAdminLabel "${LYX_LANG}" "Necessiteu drets d'administrador per desinstal·lar el LyX!"
LangString UnReallyRemoveLabel "${LYX_LANG}" "Esteu segur de voler suprimir completament el LyX i tots els seus components?"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'Preferències d$\'usuari del LyX'
LangString UnGSview "${LYX_LANG}" 'Pitgeu el botó "Uninstall" a la finestra següent per desinstal·lar\r$\n\
				   el visor de fitxers Postscript i PDF "GSview".'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "El LyX s'ha desinstal·lat correctament de l'ordinador."

LangString SecUnAspellDescription "${LYX_LANG}" 'Desinstal·la el corrector ortogràfic Aspell i tots els seus diccionaris.'
LangString SecUnMiKTeXDescription "${LYX_LANG}" "Desintal·la la distribució de LaTeX MiKTeX."
LangString SecUnJabRefDescription "${LYX_LANG}" "Desinstal·la el gestor de bibliografia JabRef."
LangString SecUnPreferencesDescription "${LYX_LANG}" 'Suprimeix les carptes de configuració del LyX$\r$\n\
					   "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
					   de tots els usuaris.'
LangString SecUnProgramFilesDescription "${LYX_LANG}" "Desinstal·la el LyX i tots els seus components."

!undef LYX_LANG

!endif # _LYX_LANGUAGES_CATALAN_NSH_
