!insertmacro LANGFILE_EXT "Turkish"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installed for Current User)"

${LangFileString} TEXT_WELCOME "Bu sihirbaz size LyX programını kuracak.$\r$\n\
				$\r$\n\
				$_CLICK"

#${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Updating MiKTeX filename database..."
${LangFileString} TEXT_CONFIGURE_LYX "Configuring LyX ($LaTeXInstalled may download missing packages, this can take some time) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compiling Python scripts..."

${LangFileString} TEXT_FINISH_DESKTOP "Create desktop shortcut"
${LangFileString} TEXT_FINISH_WEBSITE "Visit lyx.org for the latest news, support and tips"

#${LangFileString} FileTypeTitle "LyX-Document"

#${LangFileString} SecInstJabRefTitle "JabRef"
#${LangFileString} SecAllUsersTitle "Tüm kullanıcılar için kur?"
${LangFileString} SecFileAssocTitle "Dosya eşleşmeleri"
${LangFileString} SecDesktopTitle "Masaüstü ikonu"

${LangFileString} SecCoreDescription "LyX dosyaları."
#${LangFileString} SecInstJabRefDescription "Bibliography reference manager and editor for BibTeX files."
#${LangFileString} SecAllUsersDescription "LyX tüm kullanıcılar için mi yoksa yalnızca bu kullanıcıya mı kurulacak."
${LangFileString} SecFileAssocDescription "Uzantısı .lyx olan dosyalar otomatik olarak LyX ile açılsın."
${LangFileString} SecDesktopDescription "Masaüstüne bir LyX ikonu koy."
${LangFileString} SecDictionaries "Sözlükleri"
${LangFileString} SecDictionariesDescription "Spell-checker dictionaries that can be downloaded and installed."
${LangFileString} SecThesaurus "Tesarus"
${LangFileString} SecThesaurusDescription "Thesaurus dictionaries that can be downloaded and installed."

${LangFileString} EnterLaTeXHeader1 'LaTeX-dağıtımı'
${LangFileString} EnterLaTeXHeader2 'LyX in kullanacağı LaTeX dağıtımını seçin.'
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
${LangFileString} InvalidLaTeXFolder '$\"latex.exe$\" dosyası belirttiğiniz dizinde bulunamadı.'

${LangFileString} LatexInfo 'Şimdi LaTeX dağıtımı $\"MiKTeX$\" kurulumu başlatılacak.$\r$\n\
				Programı kurmak için kurulum başlayana kadar $\"Devam$\" tuşuyla ilerleyin.$\r$\n\
				$\r$\n\
				!!! Lütfen MiKTeX kurucusunun öntanımlı ayarlarını değiştirmeyin !!!'
${LangFileString} LatexError1 'Hiç bir LaTeX dağıtımı bulunamadı!$\r$\n\
				LyX $\"MiKTeX$\" gibi bir LaTeX dağıtımı olmadan kullanılamaz!$\r$\n\
				Bu nedenle kurulum iptal edilecek.'

${LangFileString} HunspellFailed 'Download of dictionary for language $\"$R3$\" failed.'
${LangFileString} ThesaurusFailed 'Download of thesaurus for language $\"$R3$\" failed.'

#${LangFileString} JabRefInfo 'Now the installer of the program $\"JabRef$\" will be launched.$\r$\n\
#				You can use all default options of the JabRef-installer.'
#${LangFileString} JabRefError 'The program $\"JabRef$\" could not successfully be installed!$\r$\n\
#				The installer will continue anyway.$\r$\n\
#				Try to install JabRef again later.'

#${LangFileString} LatexConfigInfo "Sıradaki LyX yapılandırması biraz zaman alacak."

#${LangFileString} MiKTeXPathInfo "In order that every user is later able to customize MiKTeX for his needs$\r$\n\
#					it is necessary to set write permissions for all users to MiKTeX's installation folder$\r$\n\
#					$MiKTeXPath $\r$\n\
#					and its subfolders."
${LangFileString} MiKTeXInfo 'The LaTeX-distribution $\"MiKTeX$\" will be used together with LyX.$\r$\n\
				It is recommended to install available MiKTeX-updates using the program $\"MiKTeX Update Wizard$\"$\r$\n\
				before you use LyX for the first time.$\r$\n\
				Would you now check for MiKTeX updates?'

${LangFileString} ModifyingConfigureFailed "Yapılandırma programında 'path_prefix' ayarlanamadı"
#${LangFileString} RunConfigureFailed "Yapılandırma programı çalıştırılamadı"
${LangFileString} NotAdmin "LyX kurabilmek için yönetici yetkileri gerekiyor!"
${LangFileString} InstallRunning "Kurulum programı zaten çalışıyor!"
${LangFileString} AlreadyInstalled "LyX ${APP_SERIES_KEY2} kurulu zaten!$\r$\n\
				Installing over existing installations is not recommended if the installed version$\r$\n\
				is a test release or if you have problems with your existing LyX installation.$\r$\n\
				In these cases better reinstall LyX.$\r$\n\
				Dou you nevertheles want to install LyX over the existing version?"
${LangFileString} NewerInstalled "You are trying to install an older version of LyX than what you have installed.$\r$\n\
				  If you really want this, you must uninstall the existing LyX $OldVersionNumber before."

${LangFileString} MultipleIndexesNotAvailable "The support for using several indexes in a document will not be available because$\r$\n\
						MiKTeX was installed with administrator privileges but you are installing LyX without them."

#${LangFileString} FinishPageMessage "Tebrikler! LyX başarıyla kuruldu.$\r$\n\
#					$\r$\n\
#					(LyX in ilk açılışı birkaç saniye alabilir.)"
${LangFileString} FinishPageRun "LyX Başlat"

${LangFileString} UnNotInRegistryLabel "Sistem kütüğünde LyX bulunamadı.$\r$\n\
					Başlat menüsü ve masaüstünüzdeki kısayollar silinemeyecek."
${LangFileString} UnInstallRunning "Önce LyX i kapatmalısınız!"
${LangFileString} UnNotAdminLabel "LyX kaldırabilmek için yönetici yetkileri gerekiyor!"
${LangFileString} UnReallyRemoveLabel "LyX ve tüm bileşenlerini kaldırmak istediğinize emin misiniz?"
${LangFileString} UnLyXPreferencesTitle 'LyX$\'s user preferences'

${LangFileString} SecUnMiKTeXDescription "Uninstalls the LaTeX-distribution MiKTeX."
#${LangFileString} SecUnJabRefDescription "Uninstalls the bibliography manager JabRef."
${LangFileString} SecUnPreferencesDescription 'Deletes LyX$\'s configuration folder$\r$\n\
						$\"$AppPre\username\$\r$\n\
						$AppSuff\$\r$\n\
						${APP_DIR_USERDATA}$\")$\r$\n\
						for all users.'
${LangFileString} SecUnProgramFilesDescription "Uninstall LyX and all of its components."

# MikTeX Update Stuff
${LangFileString} MiKTeXUpdateMsg "MiKTeX'teki değişikliklerden dolayı, LyX sürümlerinin 2.2.3'den sonraki sürümleri, MiKTeX'i kullanılamaz hale getiren kısmi bir güncelleştirmeye neden olabilir.$\r$\n\
$\r$\n\
Henüz yapmadıysanız, yeni bir LyX sürümü yüklemeden önce MiKTeX'i en son sürüme güncellemenizi öneririz.$\r$\n\
$\r$\n\
Artık yükleme ile devam edebilir veya iptal edebilirsiniz, böylece MiKTeX'i güncelleyebilirsiniz.$\r$\n\
$\r$\n\
(Otomatik çeviri için özür dilerim.)"

${LangFileString} MiKTeXUpdateHeader "UYARI: MiKTeX Değişiklikleri"
