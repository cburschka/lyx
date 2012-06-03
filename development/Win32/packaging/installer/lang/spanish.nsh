!insertmacro LANGFILE_EXT "Spanish"

${LangFileString} WelcomePageText "Este programa instalará LyX en su ordenador.$\r$\n\
					  $\r$\n\
					  Usted necesita privilegios de administrador para instalar LyX.$\r$\n\
					  $\r$\n\
					  Se recomienda que cierre todas las demás aplicaciones antes de iniciar la instalación. Esto hará posible actualizar archivos relacionados con el sistema sin tener que reiniciar su ordenador.$\r$\n\
					  $\r$\n\
					  $_CLICK"

${LangFileString} FileTypeTitle "Documento LyX"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Instalar para todos los usuarios"
${LangFileString} SecFileAssocTitle "Asociar ficheros"
${LangFileString} SecDesktopTitle "Icono de escritorio"

${LangFileString} SecCoreDescription "Los ficheros de LyX."
${LangFileString} SecInstJabRefDescription "Administrador de referencias bibliograficas e editor de ficheros BibTeX."
${LangFileString} SecAllUsersDescription "Instalar LyX para todos los usuarios o sólo para el usuario actual."
${LangFileString} SecFileAssocDescription "Asociar la extensión .lyx con LyX."
${LangFileString} SecDesktopDescription "Crear un icono de LyX en el escritorio."

${LangFileString} LangSelectHeader "Selección del idioma del menu de LyX "
${LangFileString} AvailableLang " Idiomas disponibles "

${LangFileString} EnterLaTeXHeader1 'Distribución LaTeX'
${LangFileString} EnterLaTeXHeader2 'Elija la distribución de LaTeX que debería emplear LyX.'
${LangFileString} EnterLaTeXFolder 'Opcionalmente puede especificar el camino del fichero $\"latex.exe$\" y posteriormente \
					   elegir la distribución de LaTeX que va usar LyX.$\r$\n\
					   ¡Sin LaTeX LyX no puede (pre)imprimir documentos!$\r$\n\
					   $\r$\n\
					   El instalador ha detectado la distribución de LaTeX $\"$LaTeXName$\" \
					   en su sistema, en el camino que se muestra abajo.'
${LangFileString} EnterLaTeXFolderNone 'Especifique abajo el camino al fichero $\"latex.exe$\". Posteriormente elija \
					       la distribución de LaTeX que debe usar LyX.$\r$\n\
					       ¡Sin LaTeX LyX no puede (pre)imprimir documentos!$\r$\n\
					       $\r$\n\
					       El instalador non pudo encontrar ninguna distribución LaTeX en su sistema.'
${LangFileString} PathName 'Camino al fichero $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "No usar LaTeX"
${LangFileString} InvalidLaTeXFolder 'Imposible encontrar $\"latex.exe$\".'

${LangFileString} LatexInfo 'Ahora se lanzará el instalador de $\"MiKTeX$\", la distribución de LaTeX.$\r$\n\
			            Para instalar el programa presione el botón $\"Next$\" en la  ventana del instalador hasta que la aplicación arranque.$\r$\n\
				    $\r$\n\
				    ¡¡¡ Por favor use todas las opciones por defecto del instalador de MiKTeX !!!'
${LangFileString} LatexError1 '¡No se ha encontrado ninguna distribución de LaTeX!$\r$\n\
                      		      ¡LyX no funciona sin una distribución de LaTeX como $\"MiKTeX$\"!$\r$\n\
				      Por lo tanto la instalación se abortará.'

${LangFileString} HunspellFailed 'Download of dictionary for language $\"$R3$\" failed.'
${LangFileString} ThesaurusFailed 'Download of thesaurus for language $\"$R3$\" failed.'

${LangFileString} JabRefInfo 'Ahora se lanzará el instalador del programa $\"JabRef$\".$\r$\n\
				     Puede utilizar todas las opciones por defecto del instaldor de JabRef.'
${LangFileString} JabRefError '¡El programa $\"JabRef$\" no pudo instalarse con éxito!$\r$\n\
		                      La instalación se continuará de toda forma.$\r$\n\
				      Intente instalar otra vez JabRef más tarde.'

${LangFileString} LatexConfigInfo "La siguiente configuración de LyX va a tardar un poco."

${LangFileString} MiKTeXPathInfo "Para que cada usuario pueda personalizar posteriormente MiKTeX acorde sus$\r$\n\
					 necesidades, es necesario que la carpeta donde fue instalado MiKTeX's $\r$\n\
					 $MiKTeXPath $\r$\n\
					 y sus subcarpetas tengan permiso de escritura para todos los usuarios."
${LangFileString} MiKTeXInfo 'Junto con LyX se va usar la distribución de LaTeX $\"MiKTeX$\".$\r$\n\
				     Antes de usar LyX por primera vez, se recomienda instalar las actualizaciones disponibles$\r$\n\
				     de MiKTeX con el instalador $\"MiKTeX Update Wizard$\"$\r$\n\
				     ¿Desea comprobar ahora si hay actualizaciones de MiKTeX?'

${LangFileString} UpdateNotAllowed "This update package can only update ${PRODUCT_VERSION_OLD}!"
${LangFileString} ModifyingConfigureFailed "Error al intentar añadir 'path_prefix' durante la ejecución del programa de configuración"
${LangFileString} CreateCmdFilesFailed "No se pudo crear lyx.bat"
${LangFileString} RunConfigureFailed "Error al intentar ejecutar el programa de configuración"
${LangFileString} NotAdmin "Necesita privilegios de administrador para instalar LyX!"
${LangFileString} InstallRunning "El instalador ya esta siendo ejecutado!"
${LangFileString} StillInstalled "¡LyX ya esta instalado! Antes desinstale LyX."

${LangFileString} FinishPageMessage "¡Enhorabuena! LyX ha sido instalado con éxito.$\r$\n\
					    $\r$\n\
					    (El primer arranque de LyX puede tardar algunos segundos.)"
${LangFileString} FinishPageRun "Ejecutar LyX"

${LangFileString} UnNotInRegistryLabel "Imposible encontrar LyX en el registro.$\r$\n\
					       Los accesos rápidos del escritorio y del Menú de Inicio no serán eliminados."
${LangFileString} UnInstallRunning "Antes cierre LyX!"
${LangFileString} UnNotAdminLabel "Necesita privilegios de administrador para desinstalar LyX!"
${LangFileString} UnReallyRemoveLabel "¿Está seguro de que desea eliminar completamente LyX y todos sus componentes?"
${LangFileString} UnLyXPreferencesTitle 'Preferencias de usuario de LyX'

${LangFileString} SecUnMiKTeXDescription "Desinstala a distribución de LaTeX MiKTeX."
${LangFileString} SecUnJabRefDescription "Desinstala el administrador de referencias bibliograficas JabRef."
${LangFileString} SecUnPreferencesDescription 'Elimina las carpetas de configuración de LyX$\r$\n\
					   $\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
					   de todos los usuarios.'
${LangFileString} SecUnProgramFilesDescription "Desinstala LyX y todos sus componentes."

