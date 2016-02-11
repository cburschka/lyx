!insertmacro LANGFILE_EXT "Portuguese"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installed for Current User)"

${LangFileString} TEXT_WELCOME "Este assistente de instalação irá guiá-lo através da instalação do LyX.$\r$\n\
				$\r$\n\
				$_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Updating MiKTeX filename database..."
${LangFileString} TEXT_CONFIGURE_LYX "Configuring LyX ($LaTeXInstalled may download missing packages, this can take some time) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compiling Python scripts..."

${LangFileString} TEXT_FINISH_DESKTOP "Create desktop shortcut"
${LangFileString} TEXT_FINISH_WEBSITE "Visit lyx.org for the latest news, support and tips"

${LangFileString} FileTypeTitle "Documento LyX"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Instalar para todos os utilizadores?"
${LangFileString} SecFileAssocTitle "Associação dos ficheiros"
${LangFileString} SecDesktopTitle "Icone do ambiente de trabalho"

${LangFileString} SecCoreDescription "Os ficheiros LyX."
${LangFileString} SecInstJabRefDescription "Gestor de referências bibliográficas e editor de ficheiros BibTeX."
${LangFileString} SecAllUsersDescription "Instalar o LyX para todos os utilizadores ou apenas para o presente utilizador."
${LangFileString} SecFileAssocDescription "Os ficheiros com a extensão .lyx irão abrir automaticamente no LyX."
${LangFileString} SecDesktopDescription "Um icone do LyX no ambiente de trabalho."
${LangFileString} SecDictionariesDescription "Spell-checker dictionaries that can be downloaded and installed."
${LangFileString} SecThesaurusDescription "Thesaurus dictionaries that can be downloaded and installed."

${LangFileString} EnterLaTeXHeader1 'Distribuição de LaTeX'
${LangFileString} EnterLaTeXHeader2 'Escolher a distribuição de LaTeX que o LyX deverá usar.'
${LangFileString} EnterLaTeXFolder 'Opcionalmente pode especificar o caminho do ficheiro $\"latex.exe$\" e posteriormente \
					escolher a distribuição de LaTeX que vai usar o LyX.\r\n\
					Sem LaTeX o LyX não pode (pré-)imprimir os documentos!\r\n\
					\r\n\
					O instalador detectou a distribuição de LaTeX $\"$LaTeXName$\"\
					no seu sistema, no caminho que se mostra abaixo.'
${LangFileString} EnterLaTeXFolderNone 'Especifique abaixo o caminho ao ficheiro $\"latex.exe$\". Posteriormente escolha \
					a distribuição de LaTeX que deve usar o LyX.\r\n\
					Sem LaTeX o LyX não pode (pré-)imprimir os documentos!\r\n\
					\r\n\
					O instalador não pudo achar nenhuma distribuição de LaTeX no seu sistema.'
${LangFileString} PathName 'Caminho ao ficheiro $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Não usar LaTeX"
${LangFileString} InvalidLaTeXFolder 'O ficheiro $\"latex.exe$\" não está no caminho especificado.'

${LangFileString} LatexInfo 'Agora lançar-se-á o instalador da distribuição de LaTeX $\"MiKTeX$\".$\r$\n\
				Para instalar o programa carregue no botão $\"Próximo$\" na janela de instalação até a instalação começar.$\r$\n\
				$\r$\n\
				!!! Por favor use todas as opções por defeito do instalador do MiKTeX !!!'
${LangFileString} LatexError1 'Não foi encontrada nenhuma distribuição de LaTeX!$\r$\n\
				O LyX não pode ser usado sem uma distribuição de LaTeX como o $\"MiKTeX$\"!$\r$\n\
				A instalação será por isso abortada.'

${LangFileString} HunspellFailed 'Download of dictionary for language $\"$R3$\" failed.'
${LangFileString} ThesaurusFailed 'Download of thesaurus for language $\"$R3$\" failed.'

${LangFileString} JabRefInfo 'Agora lançar-se-á o instalador do programa $\"JabRef$\".$\r$\n\
				Pode usar todas as opções por defeito no instalador do JabRef.'
${LangFileString} JabRefError 'Não se conseguiu instalar o programa $\"JabRef$\"!$\r$\n\
				A instalação irá continuar na mesma.$\r$\n\
				Tente instalar o JabRef outra vez mais tarde.'

${LangFileString} LatexConfigInfo "A configuração seguinte do LyX irá demorar um bocado."

${LangFileString} MiKTeXPathInfo "Para que cada utilizador possa customizar posteriormente o MiKTeX segundo as suas$\r$\n\
					necesidades, é preciso a pasta onde foi instalado o MiKTeX$\r$\n\
					$MiKTeXPath $\r$\n\
					e as suas subpastas terem permissão de escritura para todos os utilizadores."
${LangFileString} MiKTeXInfo 'Junto com o LyX vai-se utilizar a distribuição de LaTeX $\"MiKTeX$\".$\r$\n\
				Antes de usar o LyX pela primeira vez, recomenda-se instalar as actualizações disponíveis$\r$\n\
				do MiKTeX com o instalador $\"MiKTeX Update Wizard$\"$\r$\n\
				Deseja comprovar agora se há actualizações do MiKTeX?'

${LangFileString} ModifyingConfigureFailed "Não conseguimos establecer o 'path_prefix' no script de configuração"
${LangFileString} RunConfigureFailed "Não foi possível executar o script de configuração"
${LangFileString} NotAdmin "Precisa de privilégios de administrador para instalar o LyX!"
${LangFileString} InstallRunning "O instalador já está a correr!"
${LangFileString} AlreadyInstalled "O LyX ${APP_SERIES_KEY2} já está instalado!$\r$\n\
				Installing over existing installations is not recommended if the installed version$\r$\n\
				is a test release or if you have problems with your existing LyX installation.$\r$\n\
				In these cases better reinstall LyX.$\r$\n\
				Dou you nevertheles want to install LyX over the existing version?"
${LangFileString} NewerInstalled "You are trying to install an older version of LyX than what you have installed.$\r$\n\
				  If you really want this, you must uninstall the existing LyX $OldVersionNumber before."

${LangFileString} MultipleIndexesNotAvailable "The support for using several indexes in a document will not be available because$\r$\n\
						MiKTeX was installed with administrator privileges but you are installing LyX without them."

${LangFileString} FinishPageMessage "Parabéns! O LyX foi instalado com sucesso.$\r$\n\
					$\r$\n\
					(O primeiro início do LyX pode levar alguns segundos.)"
${LangFileString} FinishPageRun "Lançar o LyX"

${LangFileString} UnNotInRegistryLabel "Incapaz de encontrar o LyX no registry.$\r$\n\
					Os atalhos para o ambiente de trabalho no menu Start não serão removidos."
${LangFileString} UnInstallRunning "Deve fechar o LyX em primeiro lugar!"
${LangFileString} UnNotAdminLabel "Precisa de privilégios de administrador para desinstalar o LyX!"
${LangFileString} UnReallyRemoveLabel "Tem a certeza que quer remover completamente o LyX e todas as suas componentes?"
${LangFileString} UnLyXPreferencesTitle 'Preferências de utilizador do LyX'

${LangFileString} SecUnMiKTeXDescription "Desinstala a distribuição de LaTeX MiKTeX."
${LangFileString} SecUnJabRefDescription "Desinstala o gestor de referências bibliográficas JabRef."
${LangFileString} SecUnPreferencesDescription 'Apaga as pastas de configuração do  LyX$\r$\n\
						$\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						de todos os utilizadores.'
${LangFileString} SecUnProgramFilesDescription "Desinstala LyX e todas as suas componentes."

