!insertmacro LANGFILE_EXT "Turkish"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installed for Current User)"

${LangFileString} TEXT_WELCOME "Bu sihirbaz size LyX programýný kuracak.$\r$\n\
				$\r$\n\
				$_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Updating MiKTeX filename database..."
${LangFileString} TEXT_CONFIGURE_LYX "Configuring LyX ($LaTeXInstalled may download missing packages, this can take some time) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compiling Python scripts..."

${LangFileString} TEXT_FINISH_DESKTOP "Create desktop shortcut"
${LangFileString} TEXT_FINISH_WEBSITE "Visit lyx.org for the latest news, support and tips"

${LangFileString} FileTypeTitle "LyX-Document"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Tüm kullanýcýlar için kur?"
${LangFileString} SecFileAssocTitle "Dosya eþleþmeleri"
${LangFileString} SecDesktopTitle "Masaüstü ikonu"

${LangFileString} SecCoreDescription "LyX dosyalarý."
${LangFileString} SecInstJabRefDescription "Bibliography reference manager and editor for BibTeX files."
${LangFileString} SecAllUsersDescription "LyX tüm kullanýcýlar için mi yoksa yalnýzca bu kullanýcýya mý kurulacak."
${LangFileString} SecFileAssocDescription "Uzantýsý .lyx olan dosyalar otomatik olarak LyX ile açýlsýn."
${LangFileString} SecDesktopDescription "Masaüstüne bir LyX ikonu koy."
${LangFileString} SecDictionariesDescription "Spell-checker dictionaries that can be downloaded and installed."
${LangFileString} SecThesaurusDescription "Thesaurus dictionaries that can be downloaded and installed."

${LangFileString} EnterLaTeXHeader1 'LaTeX-daðýtýmý'
${LangFileString} EnterLaTeXHeader2 'LyX in kullanacaðý LaTeX daðýtýmýný seçin.'
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
${LangFileString} InvalidLaTeXFolder '$\"latex.exe$\" dosyasý belirttiðiniz dizinde bulunamadý.'

${LangFileString} LatexInfo 'Þimdi LaTeX daðýtýmý $\"MiKTeX$\" kurulumu baþlatýlacak.$\r$\n\
				Programý kurmak için kurulum baþlayana kadar $\"Devam$\" tuþuyla ilerleyin.$\r$\n\
				$\r$\n\
				!!! Lütfen MiKTeX kurucusunun öntanýmlý ayarlarýný deðiþtirmeyin !!!'
${LangFileString} LatexError1 'Hiç bir LaTeX daðýtýmý bulunamadý!$\r$\n\
				LyX $\"MiKTeX$\" gibi bir LaTeX daðýtýmý olmadan kullanýlamaz!$\r$\n\
				Bu nedenle kurulum iptal edilecek.'

${LangFileString} HunspellFailed 'Download of dictionary for language $\"$R3$\" failed.'
${LangFileString} ThesaurusFailed 'Download of thesaurus for language $\"$R3$\" failed.'

${LangFileString} JabRefInfo 'Now the installer of the program $\"JabRef$\" will be launched.$\r$\n\
				You can use all default options of the JabRef-installer.'
${LangFileString} JabRefError 'The program $\"JabRef$\" could not successfully be installed!$\r$\n\
				The installer will continue anyway.$\r$\n\
				Try to install JabRef again later.'

${LangFileString} LatexConfigInfo "Sýradaki LyX yapýlandýrmasý biraz zaman alacak."

${LangFileString} MiKTeXPathInfo "In order that every user is later able to customize MiKTeX for his needs$\r$\n\
					it is necessary to set write permissions for all users to MiKTeX's installation folder$\r$\n\
					$MiKTeXPath $\r$\n\
					and its subfolders."
${LangFileString} MiKTeXInfo 'The LaTeX-distribution $\"MiKTeX$\" will be used together with LyX.$\r$\n\
				It is recommended to install available MiKTeX-updates using the program $\"MiKTeX Update Wizard$\"$\r$\n\
				before you use LyX for the first time.$\r$\n\
				Would you now check for MiKTeX updates?'

${LangFileString} ModifyingConfigureFailed "Yapýlandýrma programýnda 'path_prefix' ayarlanamadý"
${LangFileString} RunConfigureFailed "Yapýlandýrma programý çalýþtýrýlamadý"
${LangFileString} NotAdmin "LyX kurabilmek için yönetici yetkileri gerekiyor!"
${LangFileString} InstallRunning "Kurulum programý zaten çalýþýyor!"
${LangFileString} AlreadyInstalled "LyX ${APP_SERIES_KEY2} kurulu zaten!$\r$\n\
				Installing over existing installations is not recommended if the installed version$\r$\n\
				is a test release or if you have problems with your existing LyX installation.$\r$\n\
				In these cases better reinstall LyX.$\r$\n\
				Dou you nevertheles want to install LyX over the existing version?"
${LangFileString} NewerInstalled "You are trying to install an older version of LyX than what you have installed.$\r$\n\
				  If you really want this, you must uninstall the existing LyX $OldVersionNumber before."

${LangFileString} MultipleIndexesNotAvailable "The support for using several indexes in a document will not be available because$\r$\n\
						MiKTeX was installed with administrator privileges but you are installing LyX without them."

${LangFileString} FinishPageMessage "Tebrikler! LyX baþarýyla kuruldu.$\r$\n\
					$\r$\n\
					(LyX in ilk açýlýþý birkaç saniye alabilir.)"
${LangFileString} FinishPageRun "LyX Baþlat"

${LangFileString} UnNotInRegistryLabel "Sistem kütüðünde LyX bulunamadý.$\r$\n\
					Baþlat menüsü ve masaüstünüzdeki kýsayollar silinemeyecek."
${LangFileString} UnInstallRunning "Önce LyX i kapatmalýsýnýz!"
${LangFileString} UnNotAdminLabel "LyX kaldýrabilmek için yönetici yetkileri gerekiyor!"
${LangFileString} UnReallyRemoveLabel "LyX ve tüm bileþenlerini kaldýrmak istediðinize emin misiniz?"
${LangFileString} UnLyXPreferencesTitle 'LyX$\'s user preferences'

${LangFileString} SecUnMiKTeXDescription "Uninstalls the LaTeX-distribution MiKTeX."
${LangFileString} SecUnJabRefDescription "Uninstalls the bibliography manager JabRef."
${LangFileString} SecUnPreferencesDescription 'Deletes LyX$\'s configuration folder$\r$\n\
						$\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						for all users.'
${LangFileString} SecUnProgramFilesDescription "Uninstall LyX and all of its components."

