!insertmacro LANGFILE_EXT "Turkish"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installed for Current User)"

${LangFileString} TEXT_WELCOME "Bu sihirbaz size LyX program�n� kuracak.$\r$\n\
				$\r$\n\
				$_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Updating MiKTeX filename database..."
${LangFileString} TEXT_CONFIGURE_LYX "Configuring LyX ($LaTeXInstalled may download missing packages, this can take some time) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compiling Python scripts..."

${LangFileString} TEXT_FINISH_DESKTOP "Create desktop shortcut"
${LangFileString} TEXT_FINISH_WEBSITE "Visit lyx.org for the latest news, support and tips"

${LangFileString} FileTypeTitle "LyX-Document"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "T�m kullan�c�lar i�in kur?"
${LangFileString} SecFileAssocTitle "Dosya e�le�meleri"
${LangFileString} SecDesktopTitle "Masa�st� ikonu"

${LangFileString} SecCoreDescription "LyX dosyalar�."
${LangFileString} SecInstJabRefDescription "Bibliography reference manager and editor for BibTeX files."
${LangFileString} SecAllUsersDescription "LyX t�m kullan�c�lar i�in mi yoksa yaln�zca bu kullan�c�ya m� kurulacak."
${LangFileString} SecFileAssocDescription "Uzant�s� .lyx olan dosyalar otomatik olarak LyX ile a��ls�n."
${LangFileString} SecDesktopDescription "Masa�st�ne bir LyX ikonu koy."
${LangFileString} SecDictionariesDescription "Spell-checker dictionaries that can be downloaded and installed."
${LangFileString} SecThesaurusDescription "Thesaurus dictionaries that can be downloaded and installed."

${LangFileString} EnterLaTeXHeader1 'LaTeX-da��t�m�'
${LangFileString} EnterLaTeXHeader2 'LyX in kullanaca�� LaTeX da��t�m�n� se�in.'
${LangFileString} EnterLaTeXFolder 'You can optionally set here the path to the file $\"latex.exe$\" and therewith set the \
					LaTeX-distribution that should be used by LyX.\r\n\
					If you don$\'t use LaTeX, LyX cannot output documents!\r\n\
					\r\n\
					The installer has detected the LaTeX-distribution \
					$\"$LaTeXName$\" on your system. Displayed below is its path.'
${LangFileString} EnterLaTeXFolderNone 'Set below the path to the file $\"latex.exe$\". Therewith you set which \
					LaTeX-distribution should be used by LyX.\r\n\
					If you don$\'t use LaTeX, LyX cannot output documents!\r\n\
					\r\n\
					The installer couldn$\'t find a LaTeX-distribution on your system.'
${LangFileString} PathName 'Path to the file $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "LaTeX kullanma"
${LangFileString} InvalidLaTeXFolder '$\"latex.exe$\" dosyas� belirtti�iniz dizinde bulunamad�.'

${LangFileString} LatexInfo '�imdi LaTeX da��t�m� $\"MiKTeX$\" kurulumu ba�lat�lacak.$\r$\n\
				Program� kurmak i�in kurulum ba�layana kadar $\"Devam$\" tu�uyla ilerleyin.$\r$\n\
				$\r$\n\
				!!! L�tfen MiKTeX kurucusunun �ntan�ml� ayarlar�n� de�i�tirmeyin !!!'
${LangFileString} LatexError1 'Hi� bir LaTeX da��t�m� bulunamad�!$\r$\n\
				LyX $\"MiKTeX$\" gibi bir LaTeX da��t�m� olmadan kullan�lamaz!$\r$\n\
				Bu nedenle kurulum iptal edilecek.'

${LangFileString} HunspellFailed 'Download of dictionary for language $\"$R3$\" failed.'
${LangFileString} ThesaurusFailed 'Download of thesaurus for language $\"$R3$\" failed.'

${LangFileString} JabRefInfo 'Now the installer of the program $\"JabRef$\" will be launched.$\r$\n\
				You can use all default options of the JabRef-installer.'
${LangFileString} JabRefError 'The program $\"JabRef$\" could not successfully be installed!$\r$\n\
				The installer will continue anyway.$\r$\n\
				Try to install JabRef again later.'

${LangFileString} LatexConfigInfo "S�radaki LyX yap�land�rmas� biraz zaman alacak."

${LangFileString} MiKTeXPathInfo "In order that every user is later able to customize MiKTeX for his needs$\r$\n\
					it is necessary to set write permissions for all users to MiKTeX's installation folder$\r$\n\
					$MiKTeXPath $\r$\n\
					and its subfolders."
${LangFileString} MiKTeXInfo 'The LaTeX-distribution $\"MiKTeX$\" will be used together with LyX.$\r$\n\
				It is recommended to install available MiKTeX-updates using the program $\"MiKTeX Update Wizard$\"$\r$\n\
				before you use LyX for the first time.$\r$\n\
				Would you now check for MiKTeX updates?'

${LangFileString} ModifyingConfigureFailed "Yap�land�rma program�nda 'path_prefix' ayarlanamad�"
${LangFileString} RunConfigureFailed "Yap�land�rma program� �al��t�r�lamad�"
${LangFileString} NotAdmin "LyX kurabilmek i�in y�netici yetkileri gerekiyor!"
${LangFileString} InstallRunning "Kurulum program� zaten �al���yor!"
${LangFileString} AlreadyInstalled "LyX ${APP_SERIES_KEY2} kurulu zaten!$\r$\n\
				Installing over existing installations is not recommended if the installed version$\r$\n\
				is a test release or if you have problems with your existing LyX installation.$\r$\n\
				In these cases better reinstall LyX.$\r$\n\
				Dou you nevertheles want to install LyX over the existing version?"
${LangFileString} NewerInstalled "You are trying to install an older version of LyX than what you have installed.$\r$\n\
				  If you really want this, you must uninstall the existing LyX $OldVersionNumber before."

${LangFileString} MultipleIndexesNotAvailable "The support for using several indexes in a document will not be available because$\r$\n\
						MiKTeX was installed with administrator privileges but you are installing LyX without them."
${LangFileString} MetafileNotAvailable "The LyX support for images in the format EMF or WMF will not be$\r$\n\
					available because that requires to install a software printer to$\r$\n\
					Windows which is only possible with administrator privileges."

${LangFileString} FinishPageMessage "Tebrikler! LyX ba�ar�yla kuruldu.$\r$\n\
					$\r$\n\
					(LyX in ilk a��l��� birka� saniye alabilir.)"
${LangFileString} FinishPageRun "LyX Ba�lat"

${LangFileString} UnNotInRegistryLabel "Sistem k�t���nde LyX bulunamad�.$\r$\n\
					Ba�lat men�s� ve masa�st�n�zdeki k�sayollar silinemeyecek."
${LangFileString} UnInstallRunning "�nce LyX i kapatmal�s�n�z!"
${LangFileString} UnNotAdminLabel "LyX kald�rabilmek i�in y�netici yetkileri gerekiyor!"
${LangFileString} UnReallyRemoveLabel "LyX ve t�m bile�enlerini kald�rmak istedi�inize emin misiniz?"
${LangFileString} UnLyXPreferencesTitle 'LyX$\'s user preferences'

${LangFileString} SecUnMiKTeXDescription "Uninstalls the LaTeX-distribution MiKTeX."
${LangFileString} SecUnJabRefDescription "Uninstalls the bibliography manager JabRef."
${LangFileString} SecUnPreferencesDescription 'Deletes LyX$\'s configuration folder$\r$\n\
						$\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						for all users.'
${LangFileString} SecUnProgramFilesDescription "Uninstall LyX and all of its components."

