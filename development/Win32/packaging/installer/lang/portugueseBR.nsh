/*
LyX Installer Language File
Language: Brazilian Portuguese
Author: Georger Ara�jo
*/

!insertmacro LANGFILE_EXT "PortugueseBR"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Instalado para o Usu�rio Atual)"

${LangFileString} TEXT_WELCOME "Este assistente guiar� voc� durante a instala��o do $(^NameDA), $\r$\n\
				$\r$\n\
				$_CLICK"

#${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Atualizando a base de dados de nomes de arquivo do MiKTeX..."
${LangFileString} TEXT_CONFIGURE_LYX "Configurando o LyX (talvez o $LaTeXInstalled precise baixar pacotes, isto pode demorar algum tempo) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compilando scripts Python..."

${LangFileString} TEXT_FINISH_DESKTOP "Criar atalho na �rea de trabalho"
${LangFileString} TEXT_FINISH_WEBSITE "Visite lyx.org para ver as �ltimas novidades do LyX!"

#${LangFileString} FileTypeTitle "Documento-LyX"

${LangFileString} SecInstJabRefTitle "JabRef"
#${LangFileString} SecAllUsersTitle "Instalar para todos os usu�rios?"
${LangFileString} SecFileAssocTitle "Associa��es de arquivos"
${LangFileString} SecDesktopTitle "�cone de �rea de trabalho"

${LangFileString} SecCoreDescription "Os arquivos do LyX."
${LangFileString} SecInstJabRefDescription "Gerenciador de refer�ncias bibliogr�ficas e editor para arquivos BibTeX."
#${LangFileString} SecAllUsersDescription "Instalar o LyX para todos os usu�rios ou apenas para o usu�rio atual."
${LangFileString} SecFileAssocDescription "Arquivos com a extens�o .lyx ser�o abertos automaticamente no LyX."
${LangFileString} SecDesktopDescription "Um �cone do LyX na �rea de trabalho."
${LangFileString} SecDictionariesDescription "Dicion�rios ortogr�ficos que podem ser baixados e instalados."
${LangFileString} SecThesaurusDescription "Dicion�rios de sin�nimos que podem ser baixados e instalados."

${LangFileString} EnterLaTeXHeader1 'Distribui��o LaTeX'
${LangFileString} EnterLaTeXHeader2 'Escolha que distribui��o LaTeX o LyX dever� usar.'
${LangFileString} EnterLaTeXFolder 'Voc� pode tamb�m optar por configurar aqui o caminho para o arquivo $\"latex.exe$\" e assim escolher \
					a distribui��o LaTeX que o LyX dever� usar.\r\n\
					Sem uma distribui��o LaTeX, o LyX n�o poder� gerar documentos de sa�da!\r\n\
					\r\n\
					O instalador detectou a distribui��o LaTeX\
					$\"$LaTeXName$\" no seu sistema, cujo caminho � exibido abaixo.'
${LangFileString} EnterLaTeXFolderNone 'Configure abaixo o caminho para o arquivo $\"latex.exe$\". Ao faz�-lo, \
					voc� escolhe a distribui��o LaTeX que o LyX dever� usar.\r\n\
					Sem uma distribui��o LaTeX, o LyX n�o poder� gerar documentos de sa�da!\r\n\
					\r\n\
					O instalador n�o conseguiu encontrar uma distribui��o LaTeX no seu sistema.'
${LangFileString} PathName 'Caminho para o arquivo $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "N�o usar LaTeX"
${LangFileString} InvalidLaTeXFolder 'O arquivo $\"latex.exe$\" n�o existe no caminho especificado.'

${LangFileString} LatexInfo 'Agora o instalador da distribui��o LaTeX $\"MiKTeX$\" ser� executado.$\r$\n\
				Para instalar o programa, clique no bot�o $\"Next$\"- nas janelas do instalador at� a instala��o come�ar.$\r$\n\
				$\r$\n\
				!!! Por favor deixe todas as op��es do instalador do MiKTeX em seus valores predefinidos !!!'
${LangFileString} LatexError1 'Nenhuma distribui��o LaTeX foi encontrada!$\r$\n\
				Sem uma distribui��o LaTeX como o $\"MiKTeX$\", o LyX n�o pode ser usado!$\r$\n\
				Por isso, a instala��o ser� abortada.'

${LangFileString} HunspellFailed 'Ocorreu uma falha ao baixar o dicion�rio ortogr�fico do idioma $\"$R3$\".'
${LangFileString} ThesaurusFailed 'Ocorreu uma falha ao baixar o dicion�rio de sin�nimos do idioma $\"$R3$\".'

${LangFileString} JabRefInfo 'Agora o instalador do programa $\"JabRef$\" ser� executado.$\r$\n\
				Voc� pode deixar todas as op��es do instalador do JabRef em seus valores predefinidos.'
${LangFileString} JabRefError 'A instala��o do $\"JabRef$\" n�o foi conclu�da com sucesso!$\r$\n\
				Mesmo assim, o instalador seguir� em frente.$\r$\n\
				Tente instalar o JabRef novamente depois.'

#${LangFileString} LatexConfigInfo "A configura��o do LyX que ser� feita a seguir vai demorar bastante."

#${LangFileString} MiKTeXPathInfo "Para que todos os usu�rios possam configurar o MiKTeX de acordo com suas necessidades$\r$\n\
#					no futuro, � necess�rio conceder para todos os usu�rios a permiss�o de modifica��o na pasta $MiKTeXPath $\r$\n\
#					onde est� instalado o MiKTeX$\r$\n\
#					(e tamb�m nas subpastas)."
${LangFileString} MiKTeXInfo 'A distribui��o LaTeX $\"MiKTeX$\" ser� usada pelo LyX.$\r$\n\
				� recomendado atualizar o MiKTeX por meio do programa $\"MiKTeX Update Wizard$\"$\r$\n\
				antes de executar o LyX pela primeira vez.$\r$\n\
				Deseja verificar agora se h� atualiza��es para o MiKTeX?'

${LangFileString} ModifyingConfigureFailed "N�o foi poss�vel definir 'path_prefix' no script de configura��o"
#${LangFileString} RunConfigureFailed "N�o foi poss�vel executar o script de configura��o"
${LangFileString} InstallRunning "O instalador j� est� em execu��o!"
${LangFileString} AlreadyInstalled "O LyX ${APP_SERIES_KEY2} j� est� instalado!$\r$\n\
				N�o � recomendado instalar sobre uma instala��o existente se a vers�o j� instalada$\r$\n\
				for uma vers�o de teste ou se houver algum problema com a instala��o existente do LyX.$\r$\n\
				Nesses casos � melhor reinstalar o LyX.$\r$\n\
				Deseja instalar sobre a vers�o existente mesmo assim?"
${LangFileString} NewerInstalled "A vers�o que voc� est� tentando instalar � mais antiga que aquela que j� est� instalada.$\r$\n\
				  Se isso for realmente o que deseja, primeiro desinstale o LyX $OldVersionNumber."

${LangFileString} MultipleIndexesNotAvailable "O suporte a documentos com m�ltiplos �ndices n�o estar� dispon�vel porque$\r$\n\
						o MiKTeX foi instalado com privil�gios de administrador mas voc� est� instalando o LyX sem esses privil�gios."
${LangFileString} MetafileNotAvailable "O suporte do LyX para imagens nos formatos EMF e WMF n�o estar�$\r$\n\
					dispon�vel porque requer a instala��o de uma impressora virtual no$\r$\n\
					Windows, o que somente � poss�vel com privil�gios de administrador."

#${LangFileString} FinishPageMessage "Parab�ns! O LyX foi instalado com sucesso.$\r$\n\
#					$\r$\n\
#					(A primeira execu��o do LyX pode demorar alguns segundos.)"
${LangFileString} FinishPageRun "Executar o LyX"

${LangFileString} UnNotInRegistryLabel "N�o foi poss�vel encontrar o LyX no Registro.$\r$\n\
					Os atalhos na �rea de trabalho e no Menu Iniciar n�o ser�o removidos."
${LangFileString} UnInstallRunning "� necess�rio fechar o LyX primeiro!"
${LangFileString} UnNotAdminLabel "Para desinstalar o LyX � necess�rio ter privil�gios de administrador!"
${LangFileString} UnReallyRemoveLabel "Tem certeza que deseja remover completamente o LyX e todos os seus componentes?"
${LangFileString} UnLyXPreferencesTitle 'Prefer�ncias de usu�rio do LyX'

${LangFileString} SecUnMiKTeXDescription "Desinstala a distribui��o LaTeX MiKTeX."
${LangFileString} SecUnJabRefDescription "Desinstala o gerenciador de refer�ncias bibliogr�ficas JabRef."
${LangFileString} SecUnPreferencesDescription 'Exclui a configura��o do LyX$\r$\n\
						(pasta $\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						para voc� ou para todos os usu�rios (se voc� for um administrador)).'
${LangFileString} SecUnProgramFilesDescription "Desinstalar o LyX e todos os seus componentes."

