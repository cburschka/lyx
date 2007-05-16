!ifndef _LYX_LANGUAGES_GALICIAN_NSH_
!define _LYX_LANGUAGES_GALICIAN_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_GALICIAN}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "Este asistente vai-no guiar na instalación do LyX no seu computador.\r\n\
					  \r\n\
					  Para poder instalar o LyX precisa de priviléxios de administrador.\r\n\
					  \r\n\
					  Recomenda-se fechar todas as outras aplicacións antes de iniciar a instalación. Isto posibilita actualizar os ficheiros do sistema relevantes sen ter que reiniciar o computador.\r\n\
					  \r\n\
					  $_CLICK"

LangString FileTypeTitle "${LYX_LANG}" "Documento LyX"

LangString SecInstGSviewTitle "${LYX_LANG}" "GSview"
LangString SecInstJabRefTitle "${LYX_LANG}" "JabRef"
LangString SecAllUsersTitle "${LYX_LANG}" "Instalar para todos os usuários?"
LangString SecFileAssocTitle "${LYX_LANG}" "Asociación dos ficheiros"
LangString SecDesktopTitle "${LYX_LANG}" "Icone do ambiente de traballo"

LangString SecCoreDescription "${LYX_LANG}" "Os ficheiros LyX."
LangString SecInstGSviewDescription "${LYX_LANG}" "Programa para visualizar documentos en Postscript e PDF."
LangString SecInstJabRefDescription "${LYX_LANG}" "Xestor de referéncias bibliográficas e editor de ficheiros BibTeX."
LangString SecAllUsersDescription "${LYX_LANG}" "Instalar o LyX monousuário ou multiusuário."
LangString SecFileAssocDescription "${LYX_LANG}" "Asociar a extensión .lyx co LyX."
LangString SecDesktopDescription "${LYX_LANG}" "Cria un icone do LyX no ambiente de traballo."

LangString LangSelectHeader "${LYX_LANG}" "Selección da lingua dos menus no LyX"
LangString AvailableLang "${LYX_LANG}" " Línguas disponíbeis "

LangString MissProgHeader "${LYX_LANG}" "Verificación dos programas necesários"
LangString MissProgCap "${LYX_LANG}" "O(s) seguinte(s) programa(s) necesário(s) van ser instalados adicionalmente"
LangString MissProgLatex "${LYX_LANG}" "MiKTeX, unha distribución de LaTeX"
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript, un interprete de PostScript e PDF"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick, um conversor de imaxes"
LangString MissProgAspell "${LYX_LANG}" "Aspell, un corrector ortográfico"
LangString MissProgMessage "${LYX_LANG}" 'Non é necesário instalar programas adicionais.'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'Distribuición de LaTeX'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'Escolha a distribución de LaTeX que vai usar o LyX.'
LangString EnterLaTeXFolder "${LYX_LANG}" 'Opcionalmente pode especificar o camiño do ficheiro "latex.exe" e posteriormente \
					   escoller a distribuición de LaTeX que vai usar o LyX.\r\n\
					   Sen LaTeX o LyX non pode (pré-)imprimir os documentos!\r\n\
					   \r\n\
					   O instalador detectou a distribución de LaTeX "$LaTeXName"\
					   no seu sistema, no camiño que se mostra abaixo.'
LangString EnterLaTeXFolderNone "${LYX_LANG}" 'Especifique abaixo o camiño ao ficheiro "latex.exe". Posteriormente escolla \
					       a distribución de LaTeX que debe usar o LyX.\r\n\
					       Sem LaTeX o LyX non pode (pré-)imprimir os documentos!\r\n\
					       \r\n\
					       O instalador non pudo achar nengunha distribución de LaTeX no seu sistema.'
LangString PathName "${LYX_LANG}" 'Camiño ao ficheiro "latex.exe"'
LangString DontUseLaTeX "${LYX_LANG}" "Non usar LaTeX"
LangString InvalidLaTeXFolder "${LYX_LANG}" 'O ficheiro "latex.exe" non está no camiño especificado.'

LangString LatexInfo "${LYX_LANG}" 'Agora lanzará-se o instalador da distribución de LaTeX "MiKTeX".$\r$\n\
			            Para instalar o programa prema no botón "Próximo" na xanela de instalación até a instalación comezar.$\r$\n\
				    $\r$\n\
				    !!! Por favor use todas as opcións por defeito do instalador do MiKTeX !!!'
LangString LatexError1 "${LYX_LANG}" 'Non se achou nengunha distribución de LaTeX!$\r$\n\
                      		      Non se pode usar o LyX sen unha distribución de LaTeX como a "MiKTeX"!$\r$\n\
				      Por iso a instalación vai ser abortada.'

LangString GSviewInfo "${LYX_LANG}" 'Agora lanzará-se o instalador do programa "GSview".$\r$\n\
			             Para instalar prema no botón "Setup" na primeira xanela de instalación$\r$\n\
				     escolla a língua e despois prema no botón "Próximo" na seguinte xanela do instalador.$\r$\n\
				     Pode usar todas as opcións por defeito no instalador do GSview.'
LangString GSviewError "${LYX_LANG}" 'Non se deu instalado o programa "GSview"!$\r$\n\
		                      De todos os xeitos a instalación do LyX continua.$\r$\n\
				      Tente instalar máis adiante o GSview.'

LangString JabRefInfo "${LYX_LANG}" 'Agora lanzará-se o instalador do programa "JabRef".$\r$\n\
				     Pode usar todas as opcións por defeito no instalador do JabRef.'
LangString JabRefError "${LYX_LANG}" 'Non se deu instalado o programa "JabRef"!$\r$\n\
		                      De todos os xeitos a instalación do LyX continua.$\r$\n\
				      Tente instalar mais adiante o JabRef.'

LangString LatexConfigInfo "${LYX_LANG}" "A configuración seguinte do LyX irá demorar un pouco."

LangString AspellInfo "${LYX_LANG}" 'Agora se van descarregar e instalar dicionários do corrector ortográfico "Aspell".$\r$\n\
				     Cada dicionário ten unha licenza diferente, que se mostrará antes da instalación.'
LangString AspellDownloadFailed "${LYX_LANG}" "Non se deu descarregadao nengun dicionário do Aspell!"
LangString AspellInstallFailed "${LYX_LANG}" "Non se deu instalado nengun dicionário do Aspell!"
LangString AspellPartAnd "${LYX_LANG}" " e "
LangString AspellPartSep "${LYX_LANG}" ", "
LangString AspellAll "${LYX_LANG}" 'Xa ha instalado un dicionário desta língua'
LangString AspellNone "${LYX_LANG}" 'Non se instalou nengun dicionário do "Aspell".$\r$\n\
				     Os dicionários poden descarregar-se de$\r$\n\
				     ${AspellLocation}$\r$\n\
				     Desexa descarregar dicionários agora?'
LangString AspellPartStart "${LYX_LANG}" "Instalou-se con suceso "
LangString AspellPart1 "${LYX_LANG}" "un dicionário de inglés"
LangString AspellPart2 "${LYX_LANG}" "un dicionário de $LangName"
LangString AspellPart3 "${LYX_LANG}" "un$\r$\n\
				      dicionário de $LangNameSys"
LangString AspellPart4 "${LYX_LANG}" ' para o corrector ortográfico "Aspell".$\r$\n\
				      É posíbel descarregar máis dicionários de$\r$\n\
				      ${AspellLocation}$\r$\n\
				      Desexa descarregar outros dicionários agora?'

LangString MiKTeXPathInfo "${LYX_LANG}" "Para que cada usuário poda personalizar posteriormente o MiKTeX segundo as suas$\r$\n\
					 necesidades, cumpre a pasta onde foi instalado o MiKTeX$\r$\n\
					 $MiKTeXPath $\r$\n\
					 e as suas subpastas teren permiso de escritura para todos os usuários."
LangString MiKTeXInfo "${LYX_LANG}" 'Xunto co LyX vai-se empregar a distribución de LaTeX "MiKTeX".$\r$\n\
				     Antes de usar o LyX pola primeira vez, recomenda-se instalar as actualizacións disponíbeis$\r$\n\
				     do MiKTeX co instalador "MiKTeX Update Wizard"$\r$\n\
				     Desexa comprovar agora se ha actualizacións do MiKTeX?'

LangString UpdateNotAllowed "${LYX_LANG}" "This update package can only update ${PRODUCT_VERSION_OLD}!"
LangString ModifyingConfigureFailed "${LYX_LANG}" "Non se puido engadir o 'path_prefix' no script de configuración"
LangString CreateCmdFilesFailed "${LYX_LANG}" "Non se deu criado o lyx.bat"
LangString RunConfigureFailed "${LYX_LANG}" "Non se deu executado o script de configuración"
LangString NotAdmin "${LYX_LANG}" "Precisa de priviléxios de administrador para instalar o LyX!"
LangString InstallRunning "${LYX_LANG}" "O instalador xa está a correr!"
LangString StillInstalled "${LYX_LANG}" "O LyX xa está instalado! Desinstale o LyX primeiro."

LangString FinishPageMessage "${LYX_LANG}" "Parabéns! O LyX foi instalado con suceso.\r\n\
					    \r\n\
					    (O primeiro início do LyX pode levar alguns segundos.)"
LangString FinishPageRun "${LYX_LANG}" "Lanzar o LyX"

LangString UnNotInRegistryLabel "${LYX_LANG}" "Non se da achado o LyX no registo.$\r$\n\
					       Non se eliminarán os atallos para o ambiente de traballo e no menu de Início."
LangString UnInstallRunning "${LYX_LANG}" "Debe fechar o LyX en primeiro lugar!"
LangString UnNotAdminLabel "${LYX_LANG}" "Precisa de priviléxios de administrador para desinstalar o LyX!"
LangString UnReallyRemoveLabel "${LYX_LANG}" "Seguro que quer eliminar completamente o LyX e todos os seus componentes?"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'Preferéncias de usuário do LyX'
LangString UnGSview "${LYX_LANG}" 'Por favor prema no botón "Desinstalar" na próxima xanela para desinstalar$\r$\n\
				   o visualizador de Postscript e PDF "GSview".'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "Eliminou-se o LyX do seu computador."

LangString SecUnAspellDescription "${LYX_LANG}" "Desinstala o corrector ortográfico Aspell e todos os seus dicionários."
LangString SecUnMiKTeXDescription "${LYX_LANG}" "Desinstala a distribución de LaTeX MiKTeX."
LangString SecUnJabRefDescription "${LYX_LANG}" "Desinstala o xestor de referéncias bibliográficas JabRef."
LangString SecUnPreferencesDescription "${LYX_LANG}" 'Elimina as pastas de configuración do  LyX$\r$\n\
					   "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
					   de todos os usuários.'
LangString SecUnProgramFilesDescription "${LYX_LANG}" "Desinstala LyX e todos os seus componentes."

!undef LYX_LANG

!endif # _LYX_LANGUAGES_GALICIAN_NSH_
