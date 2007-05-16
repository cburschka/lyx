!ifndef _LYX_LANGUAGES_PORTUGUESE_NSH_
!define _LYX_LANGUAGES_PORTUGUESE_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_PORTUGUESE}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "Este assistente de instalação irá guiá-lo através da instalação do LyX.\r\n\
					  \r\n\
					  Precisa de privilégios de administrador para instalar o LyX.\r\n\
					  \r\n\
					  Recomenda-se fechar todas as outras aplicações antes de iniciar o programa de configuração. Isto torna possivel actualizar os ficheiros de sistema relevantes sem ter que reiniciar o computador.\r\n\
					  \r\n\
					  $_CLICK"

LangString FileTypeTitle "${LYX_LANG}" "Documento LyX"

LangString SecInstGSviewTitle "${LYX_LANG}" "GSview"
LangString SecInstJabRefTitle "${LYX_LANG}" "JabRef"
LangString SecAllUsersTitle "${LYX_LANG}" "Instalar para todos os utilizadores?"
LangString SecFileAssocTitle "${LYX_LANG}" "Associação dos ficheiros"
LangString SecDesktopTitle "${LYX_LANG}" "Icone do ambiente de trabalho"

LangString SecCoreDescription "${LYX_LANG}" "Os ficheiros LyX."
LangString SecInstGSviewDescription "${LYX_LANG}" "Programa para ver documentos em Postscript e PDF."
LangString SecInstJabRefDescription "${LYX_LANG}" "Gestor de referências bibliográficas e editor de ficheiros BibTeX."
LangString SecAllUsersDescription "${LYX_LANG}" "Instalar o LyX para todos os utilizadores ou apenas para o presente utilizador."
LangString SecFileAssocDescription "${LYX_LANG}" "Os ficheiros com a extensão .lyx irão abrir automaticamente no LyX."
LangString SecDesktopDescription "${LYX_LANG}" "Um icone do LyX no ambiente de trabalho."

LangString LangSelectHeader "${LYX_LANG}" "Selecção da linguagem dos menus no LyX"
LangString AvailableLang "${LYX_LANG}" " Línguas disponíveis "

LangString MissProgHeader "${LYX_LANG}" "Verificação para os programas exigidos"
LangString MissProgCap "${LYX_LANG}" "O(s) seguinte(s) programa(s) exigidos serão instalados adicionalmente"
LangString MissProgLatex "${LYX_LANG}" "MiKTeX, uma distribuição de LaTeX"
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript, um interpretador para PostScript e PDF"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick, um conversor de imagens"
LangString MissProgAspell "${LYX_LANG}" "Aspell, um corrector ortográfico"
LangString MissProgMessage "${LYX_LANG}" 'Não será instalado nenhum programa adicional.'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'Distribuição de LaTeX'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'Escolher a distribuição de LaTeX que o LyX deverá usar.'
LangString EnterLaTeXFolder "${LYX_LANG}" 'Opcionalmente pode especificar o caminho do ficheiro "latex.exe" e posteriormente \
					   escolher a distribuição de LaTeX que vai usar o LyX.\r\n\
					   Sem LaTeX o LyX não pode (pré-)imprimir os documentos!\r\n\
					   \r\n\
					   O instalador detectou a distribuição de LaTeX "$LaTeXName"\
					   no seu sistema, no caminho que se mostra abaixo.'
LangString EnterLaTeXFolderNone "${LYX_LANG}" 'Especifique abaixo o caminho ao ficheiro "latex.exe". Posteriormente escolha \
					       a distribuição de LaTeX que deve usar o LyX.\r\n\
					       Sem LaTeX o LyX não pode (pré-)imprimir os documentos!\r\n\
					       \r\n\
					       O instalador não pudo achar nenhuma distribuição de LaTeX no seu sistema.'
LangString PathName "${LYX_LANG}" 'Caminho ao ficheiro "latex.exe"'
LangString DontUseLaTeX "${LYX_LANG}" "Não usar LaTeX"
LangString InvalidLaTeXFolder "${LYX_LANG}" 'O ficheiro "latex.exe" não está no caminho especificado.'

LangString LatexInfo "${LYX_LANG}" 'Agora lançar-se-á o instalador da distribuição de LaTeX "MiKTeX".$\r$\n\
			            Para instalar o programa carregue no botão "Próximo" na janela de instalação até a instalação começar.$\r$\n\
				    $\r$\n\
				    !!! Por favor use todas as opções por defeito do instalador do MiKTeX !!!'
LangString LatexError1 "${LYX_LANG}" 'Não foi encontrada nenhuma distribuição de LaTeX!$\r$\n\
                      		      O LyX não pode ser usado sem uma distribuição de LaTeX como o "MiKTeX"!$\r$\n\
				      A instalação será por isso abortada.'

LangString GSviewInfo "${LYX_LANG}" 'Agora lançar-se-á o instalador do programa "GSview".$\r$\n\
			             Para instalar carregue no botão "Setup" na primeira janela de instalação$\r$\n\
				     escolha a língua e depois carregue no botão "Próximo" na janela seguinte do instalador.$\r$\n\
				     Pode usar todas as opções por defeito no instalador do GSview.'
LangString GSviewError "${LYX_LANG}" 'Não se conseguiu instalar o programa "GSview"!$\r$\n\
		                      A instalação irá continuar na mesma.$\r$\n\
				      Tente instalar o GSview outra vez mais tarde.'

LangString JabRefInfo "${LYX_LANG}" 'Agora lançar-se-á o instalador do programa "JabRef".$\r$\n\
				     Pode usar todas as opções por defeito no instalador do JabRef.'
LangString JabRefError "${LYX_LANG}" 'Não se conseguiu instalar o programa "JabRef"!$\r$\n\
		                      A instalação irá continuar na mesma.$\r$\n\
				      Tente instalar o JabRef outra vez mais tarde.'

LangString LatexConfigInfo "${LYX_LANG}" "A configuração seguinte do LyX irá demorar um bocado."

LangString AspellInfo "${LYX_LANG}" 'Agora se vão descarregar e instalar dicionários do corrector ortográfico "Aspell".$\r$\n\
				     Cada dicionário tem uma licença diferente, que se mostrará antes da instalação.'
LangString AspellDownloadFailed "${LYX_LANG}" "Não foi possível descarregar nenhum dicionário do Aspell!"
LangString AspellInstallFailed "${LYX_LANG}" "Não foi possível instalar nenhum dicionário do Aspell!"
LangString AspellPartAnd "${LYX_LANG}" " e "
LangString AspellPartSep "${LYX_LANG}" ", "
LangString AspellAll "${LYX_LANG}" 'Já há instalado um dicionário desta língua'
LangString AspellNone "${LYX_LANG}" 'Não se instalou nenhum dicionário do "Aspell".$\r$\n\
				     Os dicionários podem descarregar-se de$\r$\n\
				     ${AspellLocation}$\r$\n\
				     Deseja descarregar dicionários agora?'
LangString AspellPartStart "${LYX_LANG}" "Instalou-se com sucesso "
LangString AspellPart1 "${LYX_LANG}" "um dicionário de inglês"
LangString AspellPart2 "${LYX_LANG}" "um dicionário de $LangName"
LangString AspellPart3 "${LYX_LANG}" "um$\r$\n\
				      dicionário de $LangNameSys"
LangString AspellPart4 "${LYX_LANG}" ' para o corrector ortográfico "Aspell".$\r$\n\
				      É posível descarregar mais dicionários de$\r$\n\
				      ${AspellLocation}$\r$\n\
				      Deseja descarregar outros dicionários agora?'

LangString MiKTeXPathInfo "${LYX_LANG}" "Para que cada utilizador possa customizar posteriormente o MiKTeX segundo as suas$\r$\n\
					 necesidades, é preciso a pasta onde foi instalado o MiKTeX$\r$\n\
					 $MiKTeXPath $\r$\n\
					 e as suas subpastas terem permissão de escritura para todos os utilizadores."
LangString MiKTeXInfo "${LYX_LANG}" 'Junto com o LyX vai-se utilizar a distribuição de LaTeX "MiKTeX".$\r$\n\
				     Antes de usar o LyX pela primeira vez, recomenda-se instalar as actualizações disponíveis$\r$\n\
				     do MiKTeX com o instalador "MiKTeX Update Wizard"$\r$\n\
				     Deseja comprovar agora se há actualizações do MiKTeX?'

LangString UpdateNotAllowed "${LYX_LANG}" "This update package can only update ${PRODUCT_VERSION_OLD}!"
LangString ModifyingConfigureFailed "${LYX_LANG}" "Não conseguimos establecer o 'path_prefix' no script de configuração"
LangString CreateCmdFilesFailed "${LYX_LANG}" "Não foi possível criar o lyx.bat"
LangString RunConfigureFailed "${LYX_LANG}" "Não foi possível executar o script de configuração"
LangString NotAdmin "${LYX_LANG}" "Precisa de privilégios de administrador para instalar o LyX!"
LangString InstallRunning "${LYX_LANG}" "O instalador já está a correr!"
LangString StillInstalled "${LYX_LANG}" "O LyX já está instalado! Desinstale o LyX primeiro."

LangString FinishPageMessage "${LYX_LANG}" "Parabéns! O LyX foi instalado com sucesso.\r\n\
					    \r\n\
					    (O primeiro início do LyX pode levar alguns segundos.)"
LangString FinishPageRun "${LYX_LANG}" "Lançar o LyX"

LangString UnNotInRegistryLabel "${LYX_LANG}" "Incapaz de encontrar o LyX no registry.$\r$\n\
					       Os atalhos para o ambiente de trabalho no menu Start não serão removidos."
LangString UnInstallRunning "${LYX_LANG}" "Deve fechar o LyX em primeiro lugar!"
LangString UnNotAdminLabel "${LYX_LANG}" "Precisa de privilégios de administrador para desinstalar o LyX!"
LangString UnReallyRemoveLabel "${LYX_LANG}" "Tem a certeza que quer remover completamente o LyX e todas as suas componentes?"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'Preferências de utilizador do LyX'
LangString UnGSview "${LYX_LANG}" 'Por favor carregue no botão "Desinstalar" na próxima janela para desinstalar$\r$\n\
				   o visualisador de Postscript e PDF "GSview".'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "LyX foi removido com sucesso do seu computador."

LangString SecUnAspellDescription "${LYX_LANG}" "Desinstala o corrector ortográfico Aspell e todos os seus dicionários."
LangString SecUnMiKTeXDescription "${LYX_LANG}" "Desinstala a distribuição de LaTeX MiKTeX."
LangString SecUnJabRefDescription "${LYX_LANG}" "Desinstala o gestor de referências bibliográficas JabRef."
LangString SecUnPreferencesDescription "${LYX_LANG}" 'Apaga as pastas de configuração do  LyX$\r$\n\
					   "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
					   de todos os utilizadores.'
LangString SecUnProgramFilesDescription "${LYX_LANG}" "Desinstala LyX e todas as suas componentes."

!undef LYX_LANG

!endif # _LYX_LANGUAGES_PORTUGUESE_NSH_
