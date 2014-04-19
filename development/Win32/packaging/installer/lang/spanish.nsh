/*
LyX Installer Language File
Language: Spanish
Author: Ignacio García
*/

!insertmacro LANGFILE_EXT "Spanish"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Instalado para el actual usuario)"

${LangFileString} TEXT_WELCOME "Este programa instalará LyX en su ordenador.$\r$\n\
				$\r$\n\
				$_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Actualizando la base de datos de nombre de archivo MiKTeX..."
${LangFileString} TEXT_CONFIGURE_LYX "Configurando LyX ($LaTeXInstalled podría descargar paquetes faltantes, lo que puede tardar un tiempo) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compilando guiones Python..."

${LangFileString} TEXT_FINISH_DESKTOP "Crear acceso directo en el escritorio"
${LangFileString} TEXT_FINISH_WEBSITE "Visite lyx.org para últimas noticias, ayuda y consejos"

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
${LangFileString} SecDictionariesDescription "Diccionarios de revisión ortográfica que se pueden descargar e instalar."
${LangFileString} SecThesaurusDescription "Diccionarios de sinónimos que se pueden descargar e instalar."

${LangFileString} EnterLaTeXHeader1 'Distribución LaTeX'
${LangFileString} EnterLaTeXHeader2 'Elija la distribución de LaTeX que debería emplear LyX.'
${LangFileString} EnterLaTeXFolder 'Opcionalmente puede especificar el camino del fichero $\"latex.exe$\" y posteriormente \
					elegir la distribución de LaTeX que va usar LyX.\r\n\
					¡Sin LaTeX LyX no puede (pre)imprimir documentos!\r\n\
					\r\n\
					El instalador ha detectado la distribución de LaTeX $\"$LaTeXName$\" \
					en su sistema, en el camino que se muestra abajo.'
${LangFileString} EnterLaTeXFolderNone 'Especifique abajo el camino al fichero $\"latex.exe$\". Posteriormente elija \
					la distribución de LaTeX que debe usar LyX.\r\n\
					¡Sin LaTeX LyX no puede (pre)imprimir documentos!\r\n\
					\r\n\
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

${LangFileString} HunspellFailed 'La descarga del diccionario para el idioma $\"$R3$\" ha fallado.'
${LangFileString} ThesaurusFailed 'La descarga del diccionario de sinónimos para el idioma $\"$R3$\" ha fallado.'

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

${LangFileString} ModifyingConfigureFailed "Error al intentar añadir 'path_prefix' durante la ejecución del programa de configuración"
${LangFileString} RunConfigureFailed "Error al intentar ejecutar el programa de configuración"
${LangFileString} NotAdmin "Necesita privilegios de administrador para instalar LyX!"
${LangFileString} InstallRunning "El instalador ya esta siendo ejecutado!"
${LangFileString} AlreadyInstalled "¡LyX ${APP_SERIES_KEY2} ya esta instalado!$\r$\n\
				Installing over existing installations is not recommended if the installed version$\r$\n\
				is a test release or if you have problems with your existing LyX installation.$\r$\n\
				In these cases better reinstall LyX.$\r$\n\
				Dou you nevertheles want to install LyX over the existing version?"
${LangFileString} NewerInstalled "Está tratando de instalar una versión de LyX más antigua que la que tiene instalada.$\r$\n\
				  Si realmente lo desea, debe desinstalar antes la versión de LyX instalada $OldVersionNumber."

${LangFileString} MultipleIndexesNotAvailable "El soporte para el uso de varios índices en un documento no estará disponible porque$\r$\n\
						MiKTeX se instaló con privilegios de administrador pero LyX se está instalando ahora sin ellos."
${LangFileString} MetafileNotAvailable "El soporte de LyX para imágenes en formato EMF o WMF no estará$\r$\n\
					disponible, ya que requiere la instalación de un software de impresora$\r$\n\ 						para Windows que sólo es posible con privilegios de administrador."

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

