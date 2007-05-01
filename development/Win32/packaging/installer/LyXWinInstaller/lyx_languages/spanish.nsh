!ifndef _LYX_LANGUAGES_SPANISH_NSH_
!define _LYX_LANGUAGES_SPANISH_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_SPANISH}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "Este programa instalará LyX en su ordenador.\r\n\
					  \r\n\
					  Usted necesita privilegios de administrador para instalar LyX.\r\n\
					  \r\n\
					  Se recomienda que cierre todas las demás aplicaciones antes de iniciar la instalación. Esto hará posible actualizar archivos relacionados con el sistema sin tener que reiniciar su ordenador.\r\n\
					  \r\n\
					  $_CLICK"

LangString FileTypeTitle "${LYX_LANG}" "Documento LyX"

LangString SecInstGSviewTitle "${LYX_LANG}" "GSview"
LangString SecInstJabRefTitle "${LYX_LANG}" "JabRef"
LangString SecAllUsersTitle "${LYX_LANG}" "Instalar para todos los usuarios"
LangString SecFileAssocTitle "${LYX_LANG}" "Asociar ficheros"
LangString SecDesktopTitle "${LYX_LANG}" "Icono de escritorio"

LangString SecCoreDescription "${LYX_LANG}" "Los ficheros de LyX."
LangString SecInstGSviewDescription "${LYX_LANG}" "Programa para ver documentos Postscript y PDF."
LangString SecInstJabRefDescription "${LYX_LANG}" "Administrador de referencias bibliograficas e editor de ficheros BibTeX."
LangString SecAllUsersDescription "${LYX_LANG}" "Instalar LyX para todos los usuarios o sólo para el usuario actual."
LangString SecFileAssocDescription "${LYX_LANG}" "Asociar la extensión .lyx con LyX."
LangString SecDesktopDescription "${LYX_LANG}" "Crear un icono de LyX en el escritorio."

LangString LangSelectHeader "${LYX_LANG}" "Selección del idioma del menu de LyX "
LangString AvailableLang "${LYX_LANG}" " Idiomas disponibles "

LangString MissProgHeader "${LYX_LANG}" "Verificación de programas necesarios "
LangString MissProgCap "${LYX_LANG}" "Los siguientes programas necesarios serán también instalados "
LangString MissProgLatex "${LYX_LANG}" "MiKTeX, una distribución de LaTeX "
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript, un interprete para PostScript y PDF"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick, un conversor de imágenes "
LangString MissProgAspell "${LYX_LANG}" "Aspell, un corrector ortográfico"
LangString MissProgMessage "${LYX_LANG}" 'No es necesario instalar programas adicionales.'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'Distribución LaTeX'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'Elija la distribución de LaTeX que debería emplear LyX.'
LangString EnterLaTeXFolder "${LYX_LANG}" 'Opcionalmente puede especificar el camino del fichero "latex.exe" y posteriormente \
					   elegir la distribución de LaTeX que va usar LyX.\r\n\
					   ¡Sin LaTeX LyX no puede (pre)imprimir documentos!\r\n\
					   \r\n\
					   El instalador ha detectado la distribución de LaTeX "$LaTeXName" \
					   en su sistema, en el camino que se muestra abajo.'
LangString EnterLaTeXFolderNone "${LYX_LANG}" 'Especifique abajo el camino al fichero "latex.exe". Posteriormente elija \
					       la distribución de LaTeX que debe usar LyX.\r\n\
					       ¡Sin LaTeX LyX no puede (pre)imprimir documentos!\r\n\
					       \r\n\
					       El instalador non pudo encontrar ninguna distribución LaTeX en su sistema.'
LangString PathName "${LYX_LANG}" 'Camino al fichero "latex.exe"'
LangString DontUseLaTeX "${LYX_LANG}" "No usar LaTeX"
LangString InvalidLaTeXFolder "${LYX_LANG}" 'Imposible encontrar "latex.exe".'

LangString LatexInfo "${LYX_LANG}" 'Ahora se lanzará el instalador de "MiKTeX", la distribución de LaTeX.$\r$\n\
			            Para instalar el programa presione el botón "Next" en la  ventana del instalador hasta que la aplicación arranque.$\r$\n\
				    $\r$\n\
				    ¡¡¡ Por favor use todas las opciones por defecto del instalador de MiKTeX !!!'
LangString LatexError1 "${LYX_LANG}" '¡No se ha encontrado ninguna distribución de LaTeX!$\r$\n\
                      		      ¡LyX no funciona sin una distribución de LaTeX como "MiKTeX"!$\r$\n\
				      Por lo tanto la instalación se abortará.'

LangString GSviewInfo "${LYX_LANG}" 'Ahora se lanzará el instalador del programa "GSview".$\r$\n\
			             Para instalar el programa presione el botón "Setup" en la primera ventana del instalador$\r$\n\
				     elija un idioma y luego presione el botón "Next" en la siguiente ventana del instalador.$\r$\n\
				     Puede utilizar todas las opciones por defecto del instalador de GSview.'
LangString GSviewError "${LYX_LANG}" '¡El programa "GSview" no pudo instalarse con éxito!$\r$\n\
		                      La instalación se continuará de toda forma.$\r$\n\
				      Intente instalar otra vez GSview más tarde.'

LangString JabRefInfo "${LYX_LANG}" 'Ahora se lanzará el instalador del programa "JabRef".$\r$\n\
				     Puede utilizar todas las opciones por defecto del instaldor de JabRef.'
LangString JabRefError "${LYX_LANG}" '¡El programa "JabRef" no pudo instalarse con éxito!$\r$\n\
		                      La instalación se continuará de toda forma.$\r$\n\
				      Intente instalar otra vez JabRef más tarde.'

LangString LatexConfigInfo "${LYX_LANG}" "La siguiente configuración de LyX va a tardar un poco."

LangString AspellInfo "${LYX_LANG}" 'Ahora se van a descargar e instalar diccionarios del corrector ortográfico "Aspell".$\r$\n\
				     Cada diccionario tiene una licencia diferente, que se mostrará antes da instalación.'
LangString AspellDownloadFailed "${LYX_LANG}" "¡No se pudo descargar ningún diccionario para Aspell!"
LangString AspellInstallFailed "${LYX_LANG}" "¡No se pudo instalar ningún diccionario para Aspell!"
LangString AspellPartAnd "${LYX_LANG}" " y "
LangString AspellPartSep "${LYX_LANG}" ", "
LangString AspellAll "${LYX_LANG}" 'Ya hay instalado un diccionario de esta lengua'
LangString AspellNone "${LYX_LANG}" 'No se ha instalado ningún diccionario para "Aspell".$\r$\n\
				     Los diccionarios puede descargarse de$\r$\n\
				     ${AspellLocation}$\r$\n\
				     ¿Desea descargar diccionarios ahora?'
LangString AspellPartStart "${LYX_LANG}" "Se ha instalado con éxito "
LangString AspellPart1 "${LYX_LANG}" "un diccionario de inglés"
LangString AspellPart2 "${LYX_LANG}" "un diccionario de $LangName"
LangString AspellPart3 "${LYX_LANG}" "un$\r$\n\
				      diccionario de $LangNameSys"
LangString AspellPart4 "${LYX_LANG}" ' para el corrector ortográfico "Aspell".$\r$\n\
				      Es posible descargar mas diccionarios de$\r$\n\
				      ${AspellLocation}$\r$\n\
				      ¿Desea descargar otros diccionarios ahora?'

LangString MiKTeXPathInfo "${LYX_LANG}" "Para que cada usuario pueda personalizar posteriormente MiKTeX acorde sus$\r$\n\
					 necesidades, es necesario que la carpeta donde fue instalado MiKTeX's $\r$\n\
					 $MiKTeXPath $\r$\n\
					 y sus subcarpetas tengan permiso de escritura para todos los usuarios."
LangString MiKTeXInfo "${LYX_LANG}" 'Junto con LyX se va usar la distribución de LaTeX "MiKTeX".$\r$\n\
				     Antes de usar LyX por primera vez, se recomienda instalar las actualizaciones disponibles$\r$\n\
				     de MiKTeX con el instalador "MiKTeX Update Wizard"$\r$\n\
				     ¿Desea comprobar ahora si hay actualizaciones de MiKTeX?'

LangString UpdateNotAllowed "${LYX_LANG}" "This update package can only update ${PRODUCT_VERSION_OLD}!"
LangString ModifyingConfigureFailed "${LYX_LANG}" "Error al intentar añadir 'path_prefix' durante la ejecución del programa de configuración"
LangString CreateCmdFilesFailed "${LYX_LANG}" "No se pudo crear lyx.bat"
LangString RunConfigureFailed "${LYX_LANG}" "Error al intentar ejecutar el programa de configuración"
LangString NotAdmin "${LYX_LANG}" "Necesita privilegios de administrador para instalar LyX!"
LangString InstallRunning "${LYX_LANG}" "El instalador ya esta siendo ejecutado!"
LangString StillInstalled "${LYX_LANG}" "¡LyX ya esta instalado! Antes desinstale LyX."

LangString FinishPageMessage "${LYX_LANG}" "¡Enhorabuena! LyX ha sido instalado con éxito.\r\n\
					    \r\n\
					    (El primer arranque de LyX puede tardar algunos segundos.)"
LangString FinishPageRun "${LYX_LANG}" "Ejecutar LyX"

LangString UnNotInRegistryLabel "${LYX_LANG}" "Imposible encontrar LyX en el registro.$\r$\n\
					       Los accesos rápidos del escritorio y del Menú de Inicio no serán eliminados."
LangString UnInstallRunning "${LYX_LANG}" "Antes cierre LyX!"
LangString UnNotAdminLabel "${LYX_LANG}" "Necesita privilegios de administrador para desinstalar LyX!"
LangString UnReallyRemoveLabel "${LYX_LANG}" "¿Está seguro de que desea eliminar completamente LyX y todos sus componentes?"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'Preferencias de usuario de LyX'
LangString UnGSview "${LYX_LANG}" 'Por favor, haga clic sobre el botón "Uninstall" en la siguiente ventana para de-instalar$\r$\n\
				   el programa "GSview" (Postscript y PDF-viewer).'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "LyX fue eliminado con éxito de su ordenador."

LangString SecUnAspellDescription "${LYX_LANG}" "Desinstala el corrector ortográfico Aspell y todos sus diccionarios."
LangString SecUnMiKTeXDescription "${LYX_LANG}" "Desinstala a distribución de LaTeX MiKTeX."
LangString SecUnJabRefDescription "${LYX_LANG}" "Desinstala el administrador de referencias bibliograficas JabRef."
LangString SecUnPreferencesDescription "${LYX_LANG}" 'Elimina las carpetas de configuración de LyX$\r$\n\
					   "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
					   de todos los usuarios.'
LangString SecUnProgramFilesDescription "${LYX_LANG}" "Desinstala LyX y todos sus componentes."

!undef LYX_LANG

!endif # _LYX_LANGUAGES_SPANISH_NSH_
