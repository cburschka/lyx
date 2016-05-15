/*
LyX Installer Language File
Language: Spanish
Author: Ignacio Garc�a
*/

!insertmacro LANGFILE_EXT "Spanish"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Instalado para el actual usuario)"

${LangFileString} TEXT_WELCOME "Este programa instalar� LyX en su ordenador.$\r$\n\
				$\r$\n\
				$_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Actualizando la base de datos de nombre de archivo MiKTeX..."
${LangFileString} TEXT_CONFIGURE_LYX "Configurando LyX ($LaTeXInstalled podr�a descargar paquetes faltantes, lo que puede tardar un tiempo) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compilando guiones Python..."

${LangFileString} TEXT_FINISH_DESKTOP "Crear acceso directo en el escritorio"
${LangFileString} TEXT_FINISH_WEBSITE "Visite lyx.org para �ltimas noticias, ayuda y consejos"

${LangFileString} FileTypeTitle "Documento LyX"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Instalar para todos los usuarios"
${LangFileString} SecFileAssocTitle "Asociar ficheros"
${LangFileString} SecDesktopTitle "Icono de escritorio"

${LangFileString} SecCoreDescription "Los ficheros de LyX."
${LangFileString} SecInstJabRefDescription "Administrador de referencias bibliograficas e editor de ficheros BibTeX."
${LangFileString} SecAllUsersDescription "Instalar LyX para todos los usuarios o s�lo para el usuario actual."
${LangFileString} SecFileAssocDescription "Asociar la extensi�n .lyx con LyX."
${LangFileString} SecDesktopDescription "Crear un icono de LyX en el escritorio."
${LangFileString} SecDictionariesDescription "Diccionarios de revisi�n ortogr�fica que se pueden descargar e instalar."
${LangFileString} SecThesaurusDescription "Diccionarios de sin�nimos que se pueden descargar e instalar."

${LangFileString} EnterLaTeXHeader1 'Distribuci�n LaTeX'
${LangFileString} EnterLaTeXHeader2 'Elija la distribuci�n de LaTeX que deber�a emplear LyX.'
${LangFileString} EnterLaTeXFolder 'Opcionalmente puede especificar el camino del fichero $\"latex.exe$\" y posteriormente \
					elegir la distribuci�n de LaTeX que va usar LyX.\r\n\
					�Sin LaTeX LyX no puede (pre)imprimir documentos!\r\n\
					\r\n\
					El instalador ha detectado la distribuci�n de LaTeX $\"$LaTeXName$\" \
					en su sistema, en el camino que se muestra abajo.'
${LangFileString} EnterLaTeXFolderNone 'Especifique abajo el camino al fichero $\"latex.exe$\". Posteriormente elija \
					la distribuci�n de LaTeX que debe usar LyX.\r\n\
					�Sin LaTeX LyX no puede (pre)imprimir documentos!\r\n\
					\r\n\
					El instalador non pudo encontrar ninguna distribuci�n LaTeX en su sistema.'
${LangFileString} PathName 'Camino al fichero $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "No usar LaTeX"
${LangFileString} InvalidLaTeXFolder 'Imposible encontrar $\"latex.exe$\".'

${LangFileString} LatexInfo 'Ahora se lanzar� el instalador de $\"MiKTeX$\", la distribuci�n de LaTeX.$\r$\n\
				Para instalar el programa presione el bot�n $\"Next$\" en la  ventana del instalador hasta que la aplicaci�n arranque.$\r$\n\
				$\r$\n\
				��� Por favor use todas las opciones por defecto del instalador de MiKTeX !!!'
${LangFileString} LatexError1 '�No se ha encontrado ninguna distribuci�n de LaTeX!$\r$\n\
				�LyX no funciona sin una distribuci�n de LaTeX como $\"MiKTeX$\"!$\r$\n\
				Por lo tanto la instalaci�n se abortar�.'

${LangFileString} HunspellFailed 'La descarga del diccionario para el idioma $\"$R3$\" ha fallado.'
${LangFileString} ThesaurusFailed 'La descarga del diccionario de sin�nimos para el idioma $\"$R3$\" ha fallado.'

${LangFileString} JabRefInfo 'Ahora se lanzar� el instalador del programa $\"JabRef$\".$\r$\n\
				Puede utilizar todas las opciones por defecto del instaldor de JabRef.'
${LangFileString} JabRefError '�El programa $\"JabRef$\" no pudo instalarse con �xito!$\r$\n\
				La instalaci�n se continuar� de toda forma.$\r$\n\
				Intente instalar otra vez JabRef m�s tarde.'

${LangFileString} LatexConfigInfo "La siguiente configuraci�n de LyX va a tardar un poco."

${LangFileString} MiKTeXPathInfo "Para que cada usuario pueda personalizar posteriormente MiKTeX acorde sus$\r$\n\
					necesidades, es necesario que la carpeta donde fue instalado MiKTeX's $\r$\n\
					$MiKTeXPath $\r$\n\
					y sus subcarpetas tengan permiso de escritura para todos los usuarios."
${LangFileString} MiKTeXInfo 'Junto con LyX se va usar la distribuci�n de LaTeX $\"MiKTeX$\".$\r$\n\
				Antes de usar LyX por primera vez, se recomienda instalar las actualizaciones disponibles$\r$\n\
				de MiKTeX con el instalador $\"MiKTeX Update Wizard$\"$\r$\n\
				�Desea comprobar ahora si hay actualizaciones de MiKTeX?'

${LangFileString} ModifyingConfigureFailed "Error al intentar a�adir 'path_prefix' durante la ejecuci�n del programa de configuraci�n"
${LangFileString} RunConfigureFailed "Error al intentar ejecutar el programa de configuraci�n"
${LangFileString} NotAdmin "Necesita privilegios de administrador para instalar LyX!"
${LangFileString} InstallRunning "El instalador ya esta siendo ejecutado!"
${LangFileString} AlreadyInstalled "�LyX ${APP_SERIES_KEY2} ya esta instalado!$\r$\n\
				Installing over existing installations is not recommended if the installed version$\r$\n\
				is a test release or if you have problems with your existing LyX installation.$\r$\n\
				In these cases better reinstall LyX.$\r$\n\
				Dou you nevertheles want to install LyX over the existing version?"
${LangFileString} NewerInstalled "Est� tratando de instalar una versi�n de LyX m�s antigua que la que tiene instalada.$\r$\n\
				  Si realmente lo desea, debe desinstalar antes la versi�n de LyX instalada $OldVersionNumber."

${LangFileString} MultipleIndexesNotAvailable "El soporte para el uso de varios �ndices en un documento no estar� disponible porque$\r$\n\
						MiKTeX se instal� con privilegios de administrador pero LyX se est� instalando ahora sin ellos."
${LangFileString} MetafileNotAvailable "El soporte de LyX para im�genes en formato EMF o WMF no estar�$\r$\n\
					disponible, ya que requiere la instalaci�n de un software de impresora$\r$\n\ 						para Windows que s�lo es posible con privilegios de administrador."

${LangFileString} FinishPageMessage "�Enhorabuena! LyX ha sido instalado con �xito.$\r$\n\
					$\r$\n\
					(El primer arranque de LyX puede tardar algunos segundos.)"
${LangFileString} FinishPageRun "Ejecutar LyX"

${LangFileString} UnNotInRegistryLabel "Imposible encontrar LyX en el registro.$\r$\n\
					Los accesos r�pidos del escritorio y del Men� de Inicio no ser�n eliminados."
${LangFileString} UnInstallRunning "Antes cierre LyX!"
${LangFileString} UnNotAdminLabel "Necesita privilegios de administrador para desinstalar LyX!"
${LangFileString} UnReallyRemoveLabel "�Est� seguro de que desea eliminar completamente LyX y todos sus componentes?"
${LangFileString} UnLyXPreferencesTitle 'Preferencias de usuario de LyX'

${LangFileString} SecUnMiKTeXDescription "Desinstala a distribuci�n de LaTeX MiKTeX."
${LangFileString} SecUnJabRefDescription "Desinstala el administrador de referencias bibliograficas JabRef."
${LangFileString} SecUnPreferencesDescription 'Elimina las carpetas de configuraci�n de LyX$\r$\n\
						$\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						de todos los usuarios.'
${LangFileString} SecUnProgramFilesDescription "Desinstala LyX y todos sus componentes."

