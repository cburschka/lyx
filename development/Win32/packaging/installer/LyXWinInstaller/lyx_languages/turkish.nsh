!ifndef _LYX_LANGUAGES_TURKISH_NSH_
!define _LYX_LANGUAGES_TURKISH_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_TURKISH}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "Bu sihirbaz size LyX programýný kuracak.\r\n\
					  \r\n\
					  LyX kurabilmek için yönetici yetkilerine ihtiyacýnýz var.\r\n\
					  \r\n\
					  Kuruluma baþlamadan önce diðer programlarý kapatmanýz tavsiye edilir. Böylece bilgisayarýnýzý yeniden baþlatmanýza gerek kalmadan ilgili sistem dosyalarý güncellenebilir.\r\n\
					  \r\n\
					  $_CLICK"

LangString FileTypeTitle "${LYX_LANG}" "LyX-Document"

LangString SecInstGSviewTitle "${LYX_LANG}" "GSview"
LangString SecInstJabRefTitle "${LYX_LANG}" "JabRef"
LangString SecAllUsersTitle "${LYX_LANG}" "Tüm kullanýcýlar için kur?"
LangString SecFileAssocTitle "${LYX_LANG}" "Dosya eþleþmeleri"
LangString SecDesktopTitle "${LYX_LANG}" "Masaüstü ikonu"

LangString SecCoreDescription "${LYX_LANG}" "LyX dosyalarý."
LangString SecInstGSviewDescription "${LYX_LANG}" "Postscript ve PDF gösterici program."
LangString SecInstJabRefDescription "${LYX_LANG}" "Bibliography reference manager and editor for BibTeX files."
LangString SecAllUsersDescription "${LYX_LANG}" "LyX tüm kullanýcýlar için mi yoksa yalnýzca bu kullanýcýya mý kurulacak."
LangString SecFileAssocDescription "${LYX_LANG}" "Uzantýsý .lyx olan dosyalar otomatik olarak LyX ile açýlsýn."
LangString SecDesktopDescription "${LYX_LANG}" "Masaüstüne bir LyX ikonu koy."

LangString LangSelectHeader "${LYX_LANG}" "LyX arabirim dili seçimi"
LangString AvailableLang "${LYX_LANG}" " Mevcut Diller "

LangString MissProgHeader "${LYX_LANG}" "Gerekli programlarýn kontrolü"
LangString MissProgCap "${LYX_LANG}" "Aþaðýdaki gerekli programlar da otomatik olarak kurulacak"
LangString MissProgLatex "${LYX_LANG}" "MiKTeX, bir LaTeX daðýtýmý"
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript, bir PostScript ve PDF yorumlayýcý"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick, bir resim dönüþtürücü"
LangString MissProgAspell "${LYX_LANG}" "Aspell, bir yazým denetleyici"
LangString MissProgMessage "${LYX_LANG}" 'Ek bir program kurmaya gerek bulunmuyor.'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'LaTeX-daðýtýmý'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'LyX in kullanacaðý LaTeX daðýtýmýný seçin.'
LangString EnterLaTeXFolder "${LYX_LANG}" 'You can optionally set here the path to the file "latex.exe" and therewith set the \
					   LaTeX-distribution that should be used by LyX.\r\n\
					   If you don$\'t use LaTeX, LyX cannot output documents!\r\n\
					   \r\n\
					   The installer has detected the LaTeX-distribution \
					   "$LaTeXName" on your system. Displayed below is its path.'
LangString EnterLaTeXFolderNone "${LYX_LANG}" 'Set below the path to the file "latex.exe". Therewith you set which \
					       LaTeX-distribution should be used by LyX.\r\n\
					       If you don$\'t use LaTeX, LyX cannot output documents!\r\n\
					       \r\n\
					       The installer couldn$\'t find a LaTeX-distribution on your system.'
LangString PathName "${LYX_LANG}" 'Path to the file "latex.exe"'
LangString DontUseLaTeX "${LYX_LANG}" "LaTeX kullanma"
LangString InvalidLaTeXFolder "${LYX_LANG}" '"latex.exe" dosyasý belirttiðiniz dizinde bulunamadý.'

LangString LatexInfo "${LYX_LANG}" 'Þimdi LaTeX daðýtýmý "MiKTeX" kurulumu baþlatýlacak.$\r$\n\
			            Programý kurmak için kurulum baþlayana kadar "Devam" tuþuyla ilerleyin.$\r$\n\
				    $\r$\n\
				    !!! Lütfen MiKTeX kurucusunun öntanýmlý ayarlarýný deðiþtirmeyin !!!'
LangString LatexError1 "${LYX_LANG}" 'Hiç bir LaTeX daðýtýmý bulunamadý!$\r$\n\
                      		      LyX "MiKTeX" gibi bir LaTeX daðýtýmý olmadan kullanýlamaz!$\r$\n\
				      Bu nedenle kurulum iptal edilecek.'
			
LangString GSviewInfo "${LYX_LANG}" 'Þimdi "GSview" kurulum programý baþlatýlacak.$\r$\n\
			             Programý kurmak için ile kurulum penceresinde "Setup" tuþuna basýp$\r$\n\
				     bir dil seçin, ve sonraki pencerelerde "Devam" tuþuyla ilerleyin.$\r$\n\
				     GSview kurulumunun tüm öntanýmlý seçeneklerini kullanabilirsiniz.'
LangString GSviewError "${LYX_LANG}" '"GSview" kurulumu baþarýsýz oldu!$\r$\n\
		                      Kurulum gene de devam edecek.$\r$\n\
				      Daha sonra GSview kurmayý tekrar deneyin.'

LangString JabRefInfo "${LYX_LANG}" 'Now the installer of the program "JabRef" will be launched.$\r$\n\
				     You can use all default options of the JabRef-installer.'
LangString JabRefError "${LYX_LANG}" 'The program "JabRef" could not successfully be installed!$\r$\n\
		                      The installer will continue anyway.$\r$\n\
				      Try to install JabRef again later.'

LangString LatexConfigInfo "${LYX_LANG}" "Sýradaki LyX yapýlandýrmasý biraz zaman alacak."

LangString AspellInfo "${LYX_LANG}" 'Now dictionaries for the spellchecker "Aspell" will be downloaded and installed.$\r$\n\
				     Every dictionary has a different license that will be displayed before the installation.'
LangString AspellDownloadFailed "${LYX_LANG}" "No Aspell spellchecker dictionary could be downloaded!"
LangString AspellInstallFailed "${LYX_LANG}" "No Aspell spellchecker dictionary could be installed!"
LangString AspellPartAnd "${LYX_LANG}" " and "
LangString AspellPartSep "${LYX_LANG}" ", "
LangString AspellAll "${LYX_LANG}" 'There is already installed a dictionary of the language'
LangString AspellNone "${LYX_LANG}" 'No dictionary for the spellchecker "Aspell" has been installed.$\r$\n\
				     Dictionaries can be downloaded from$\r$\n\
				     ${AspellLocation}$\r$\n\
				     Do you want to download dictionaries now?'
LangString AspellPartStart "${LYX_LANG}" "There was successfully installed "
LangString AspellPart1 "${LYX_LANG}" "an english dictionary"
LangString AspellPart2 "${LYX_LANG}" "a dictionary of the language $LangName"
LangString AspellPart3 "${LYX_LANG}" "a$\r$\n\
				      dictionary of the language $LangNameSys"
LangString AspellPart4 "${LYX_LANG}" ' for the spellchecker "Aspell".$\r$\n\
				      More dictionaries can be downloaded from$\r$\n\
				      ${AspellLocation}$\r$\n\
				      Do you want to download other dictionaries now?'

LangString MiKTeXPathInfo "${LYX_LANG}" "In order that every user is later able to customize MiKTeX for his needs$\r$\n\
					 it is necessary to set write permissions for all users to MiKTeX's installation folder$\r$\n\
					 $MiKTeXPath $\r$\n\
					 and its subfolders."
LangString MiKTeXInfo "${LYX_LANG}" 'The LaTeX-distribution "MiKTeX" will be used together with LyX.$\r$\n\
				     It is recommended to install available MiKTeX-updates using the program "MiKTeX Update Wizard"$\r$\n\
				     before you use LyX for the first time.$\r$\n\
				     Would you now check for MiKTeX updates?'

LangString UpdateNotAllowed "${LYX_LANG}" "This update package can only update ${PRODUCT_VERSION_OLD}!"
LangString ModifyingConfigureFailed "${LYX_LANG}" "Yapýlandýrma programýnda 'path_prefix' ayarlanamadý"
LangString CreateCmdFilesFailed "${LYX_LANG}" "lyx.bat oluþturulamadý"
LangString RunConfigureFailed "${LYX_LANG}" "Yapýlandýrma programý çalýþtýrýlamadý"
LangString NotAdmin "${LYX_LANG}" "LyX kurabilmek için yönetici yetkileri gerekiyor!"
LangString InstallRunning "${LYX_LANG}" "Kurulum programý zaten çalýþýyor!"
LangString StillInstalled "${LYX_LANG}" "LyX kurulu zaten! Önce onu kaldýrýn."

LangString FinishPageMessage "${LYX_LANG}" "Tebrikler! LyX baþarýyla kuruldu.\r\n\
					    \r\n\
					    (LyX in ilk açýlýþý birkaç saniye alabilir.)"
LangString FinishPageRun "${LYX_LANG}" "LyX Baþlat"

LangString UnNotInRegistryLabel "${LYX_LANG}" "Sistem kütüðünde LyX bulunamadý.$\r$\n\
					       Baþlat menüsü ve masaüstünüzdeki kýsayollar silinemeyecek."
LangString UnInstallRunning "${LYX_LANG}" "Önce LyX i kapatmalýsýnýz!"
LangString UnNotAdminLabel "${LYX_LANG}" "LyX kaldýrabilmek için yönetici yetkileri gerekiyor!"
LangString UnReallyRemoveLabel "${LYX_LANG}" "LyX ve tüm bileþenlerini kaldýrmak istediðinize emin misiniz?"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'LyX$\'s user preferences'
LangString UnGSview "${LYX_LANG}" 'Lütfen bir sonraki pencerede "Kaldýr" düðmesine basarak$\r$\n\
				   "GSview" postscript ve pdf göstericisini kaldýrýn.'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "LyX bilgisayarýnýzdan baþarýyla kaldýrýldý."

LangString SecUnAspellDescription "${LYX_LANG}" "Uninstalls the spellchecker Aspell and all of its dictionaries."
LangString SecUnMiKTeXDescription "${LYX_LANG}" "Uninstalls the LaTeX-distribution MiKTeX."
LangString SecUnJabRefDescription "${LYX_LANG}" "Uninstalls the bibliography manager JabRef."
LangString SecUnPreferencesDescription "${LYX_LANG}" 'Deletes LyX$\'s configuration folder$\r$\n\
					   "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
					   for all users.'
LangString SecUnProgramFilesDescription "${LYX_LANG}" "Uninstall LyX and all of its components."

!undef LYX_LANG

!endif # _LYX_LANGUAGES_TURKISH_NSH_













