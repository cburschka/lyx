/*
LyX Installer Language File
Language: Brazilian Portuguese
Author: Georger Araújo
*/

!insertmacro LANGFILE_EXT "Brazilian Portuguese"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Instalado para o Usuário Atual)"

${LangFileString} TEXT_WELCOME "Este assistente guiará você durante a instalação do $(^NameDA), $\r$\n\
				$\r$\n\
				$_CLICK"

#${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Atualizando a base de dados de nomes de arquivo do MiKTeX..."
${LangFileString} TEXT_CONFIGURE_LYX "Configurando o LyX (talvez o $LaTeXInstalled precise baixar pacotes, isto pode demorar algum tempo) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compilando scripts Python..."

${LangFileString} TEXT_FINISH_DESKTOP "Criar atalho na área de trabalho"
${LangFileString} TEXT_FINISH_WEBSITE "Visite lyx.org para ver as últimas novidades do LyX!"

#${LangFileString} FileTypeTitle "Documento-LyX"

${LangFileString} SecInstJabRefTitle "JabRef"
#${LangFileString} SecAllUsersTitle "Instalar para todos os usuários?"
${LangFileString} SecFileAssocTitle "Associações de arquivos"
${LangFileString} SecDesktopTitle "Ícone de área de trabalho"

${LangFileString} SecCoreDescription "Os arquivos do LyX."
${LangFileString} SecInstJabRefDescription "Gerenciador de referências bibliográficas e editor para arquivos BibTeX."
#${LangFileString} SecAllUsersDescription "Instalar o LyX para todos os usuários ou apenas para o usuário atual."
${LangFileString} SecFileAssocDescription "Arquivos com a extensão .lyx serão abertos automaticamente no LyX."
${LangFileString} SecDesktopDescription "Um ícone do LyX na área de trabalho."
${LangFileString} SecDictionariesDescription "Dicionários ortográficos que podem ser baixados e instalados."
${LangFileString} SecThesaurusDescription "Dicionários de sinônimos que podem ser baixados e instalados."

${LangFileString} EnterLaTeXHeader1 'Distribuição LaTeX'
${LangFileString} EnterLaTeXHeader2 'Escolha que distribuição LaTeX o LyX deverá usar.'
${LangFileString} EnterLaTeXFolder 'Você pode também optar por configurar aqui o caminho para o arquivo $\"latex.exe$\" e assim escolher a\
					distribuição LaTeX que o LyX deverá usar.\r\n\
					Sem uma distribuição LaTeX, o LyX não poderá gerar documentos de saída!\r\n\
					\r\n\
					O instalador detectou a distribuição LaTeX\
					$\"$LaTeXName$\" no seu sistema, cujo caminho é exibido abaixo.'
${LangFileString} EnterLaTeXFolderNone 'Configure abaixo o caminho para o arquivo $\"latex.exe$\". Ao fazê-lo você escolhe\
					a distribuição LaTeX que o LyX deverá usar.\r\n\
					Sem uma distribuição LaTeX, o LyX não poderá gerar documentos de saída!\r\n\
					\r\n\
					O instalador não conseguiu encontrar uma distribuição LaTeX no seu sistema.'
${LangFileString} PathName 'Caminho para o arquivo $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Não usar LaTeX"
${LangFileString} InvalidLaTeXFolder 'O arquivo $\"latex.exe$\" não existe no caminho especificado.'

${LangFileString} LatexInfo 'Agora o instalador da distribuição LaTeX $\"MiKTeX$\" será executado.$\r$\n\
				Para instalar o programa, clique no botão $\"Next$\"- nas janelas do instalador até a instalação começar.$\r$\n\
				$\r$\n\
				!!! Por favor deixe todas as opções do instalador do MiKTeX em seus valores predefinidos !!!'
${LangFileString} LatexError1 'Nenhuma distribuição LaTeX foi encontrada!$\r$\n\
				Sem uma distribuição LaTeX como o $\"MiKTeX$\", o LyX não pode ser usado!$\r$\n\
				Por isso, a instalação será abortada.'

${LangFileString} HunspellFailed 'Ocorreu uma falha ao baixar o dicionário ortográfico do idioma $\"$R3$\".'
${LangFileString} ThesaurusFailed 'Ocorreu uma falha ao baixar o dicionário de sinônimos do idioma $\"$R3$\".'

${LangFileString} JabRefInfo 'Agora o instalador do programa $\"JabRef$\" será executado.$\r$\n\
				Você pode deixar todas as opções do instalador do JabRef em seus valores predefinidos.'
${LangFileString} JabRefError 'A instalação do $\"JabRef$\" não foi concluída com sucesso!$\r$\n\
				Mesmo assim, o instalador seguirá em frente.$\r$\n\
				Tente instalar o JabRef novamente depois.'

#${LangFileString} LatexConfigInfo "A configuração do LyX que será feita a seguir vai demorar bastante."

#${LangFileString} MiKTeXPathInfo "Para que todos os usuários possam configurar o MiKTeX de acordo com suas necessidades$\r$\n\
#					no futuro, é necessário conceder para todos os usuários a permissão de modificação na pasta $MiKTeXPath $\r$\n\
#					onde está instalado o MiKTeX$\r$\n\
#					(e também nas subpastas)."
${LangFileString} MiKTeXInfo 'A distribuição LaTeX $\"MiKTeX$\" será usada pelo LyX.$\r$\n\
				É recomendado atualizar o MiKTeX por meio do programa $\"MiKTeX Update Wizard$\"$\r$\n\
				antes de executar o LyX pela primeira vez.$\r$\n\
				Deseja verificar agora se há atualizações para o MiKTeX?'

${LangFileString} ModifyingConfigureFailed "Não foi possível definir 'path_prefix' no script de configuração"
#${LangFileString} RunConfigureFailed "Não foi possível executar o script de configuração"
${LangFileString} InstallRunning "O instalador já está em execução!"
${LangFileString} AlreadyInstalled "O LyX ${APP_SERIES_KEY2} já está instalado!$\r$\n\
				Não é recomendado instalar sobre uma instalação existente se estiver instalando$\r$\n\
				uma versão de teste ou se houver algum problema com a instalação existente do LyX.$\r$\n\
				Nesses casos é melhor reinstalar o LyX.$\r$\n\
				Deseja instalar sobre a versão existente mesmo assim?"
${LangFileString} NewerInstalled "A versão que você está tentando instalar é mais antiga que aquela que já está instalada.$\r$\n\
				  Se isso for realmente o que deseja, primeiro desinstale o LyX $OldVersionNumber."

${LangFileString} MultipleIndexesNotAvailable "O suporte a documentos com múltiplos índices não estará disponível porque$\r$\n\
						o MiKTeX foi instalado com privilégios de administrador mas você está instalando o LyX sem esses privilégios."
${LangFileString} MetafileNotAvailable "O suporte do LyX para imagens nos formatos EMF e WMF não estará$\r$\n\
					disponível porque requer a instalação de uma impressora virtual no$\r$\n\
					Windows, o que somente é possível com privilégios de administrador."

#${LangFileString} FinishPageMessage "Parabéns! O LyX foi instalado com sucesso.$\r$\n\
#					$\r$\n\
#					(A primeira execução do LyX pode demorar alguns segundos.)"
${LangFileString} FinishPageRun "Executar o LyX"

${LangFileString} UnNotInRegistryLabel "Não foi possível encontrar o LyX no Registro.$\r$\n\
					Os atalhos na área de trabalho e no Menu Iniciar não serão removidos."
${LangFileString} UnInstallRunning "É necessário fechar o LyX primeiro!"
${LangFileString} UnNotAdminLabel "Para desinstalar o LyX é necessário ter privilégios de administrador!"
${LangFileString} UnReallyRemoveLabel "Tem certeza que deseja remover completamente o LyX e todos os seus componentes?"
${LangFileString} UnLyXPreferencesTitle 'Preferências de usuário do LyX'

${LangFileString} SecUnMiKTeXDescription "Desinstala a distribuição LaTeX MiKTeX."
${LangFileString} SecUnJabRefDescription "Desinstala o gerenciador de referências bibliográficas JabRef."
${LangFileString} SecUnPreferencesDescription 'Exclui a configuração do LyX$\r$\n\
						(pasta $\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						para você ou para todos os usuárois (se você for um administrador)).'
${LangFileString} SecUnProgramFilesDescription "Desinstalar o LyX e todos os seus componentes."

