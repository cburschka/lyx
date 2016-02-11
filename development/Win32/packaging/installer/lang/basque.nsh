/*
LyX Installer Language File
Language: Basque
Author: Iñaki Larrañaga Murgoitio
*/

!insertmacro LANGFILE_EXT "Basque"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Instalatu uneko erabiltzailearentzat)"

${LangFileString} TEXT_WELCOME "Morroi honek $(^NameDA) aplikazioaren instalazio urratsetan zehar lagunduko dizu, $\r$\n\
				$\r$\n\
				$_CLICK"

#${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "MiKTeX fitxategiaren datu-basea eguneratzen..."
${LangFileString} TEXT_CONFIGURE_LYX "LyX konfiguratzen ($LaTeXInstalled-ek falta diren paketeak deskarga ditzake, denbora piskat behar lezake) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Python script-ak konpilatzen..."

${LangFileString} TEXT_FINISH_DESKTOP "Sortu mahaigaineko lasterbidea"
${LangFileString} TEXT_FINISH_WEBSITE "Bisitatu lyx.org azken berriak, aholkuak eta laguntza lortzeko"

#${LangFileString} FileTypeTitle "LyX-dokumentua"

${LangFileString} SecInstJabRefTitle "JabRef"
#${LangFileString} SecAllUsersTitle "Instalatu erabiltzaile guztientzako?"
${LangFileString} SecFileAssocTitle "Fitxategiaren esleipenak"
${LangFileString} SecDesktopTitle "Mahaigaineko ikonoa"

${LangFileString} SecCoreDescription "LyX fitxategiak."
${LangFileString} SecInstJabRefDescription "Bibliografiako erreferentzien kudeatzailea eta BibTeX fitxategien editorea."
#${LangFileString} SecAllUsersDescription "Instalatu LyX erabiltzaile guztientzako, edo soilik uneko erabiltzailearentzako."
${LangFileString} SecFileAssocDescription ".lyx luzapeneko fitxategiak LyX-ekin irekiko dira automatikoki."
${LangFileString} SecDesktopDescription "LyX ikonoa mahaigainean."
${LangFileString} SecDictionariesDescription "Zuzentzaile ortografikoen hiztegiak deskarga eta instala daitezke."
${LangFileString} SecThesaurusDescription "Thesaurus hiztegiak deskarga eta instala daitezke."

${LangFileString} EnterLaTeXHeader1 'LaTeX-banaketa'
${LangFileString} EnterLaTeXHeader2 'Ezarri LaTeX banaketa LyX-ek erabiltzeko.'
${LangFileString} EnterLaTeXFolder 'Aukeran hemen $\"latex.exe$\" fitxategiaren bide-izena ezar dezakezu, horrekin \
					LyX-ekin erabiltzea nahi duzun LaTeX-en banaketan zehaztuko baituzu.\r\n\
					Ez baduzu LaTeX erabiltzen, LyX-ek ezin izango du irteerako dokumenturik sortu!\r\n\
					\r\n\
					Instalatzaileak LaTeX-en $\"$LaTeXName$\" banaketa aurkitu du sisteman. \
					Azpian dagokion bide-izena erakusten da.'
${LangFileString} EnterLaTeXFolderNone 'Azpian ezarri $\"latex.exe$\" fitxategiaren bide-izena. horrekin \
					LyX-ekin erabiltzea nahi duzun LaTeX-en banaketan zehaztuko baituzu.\r\n\
					Ez baduzu LaTeX erabiltzen, LyX-ek ezin izango du irteerako dokumenturik sortu!\r\n\
					\r\n\
					Instalatzaileak ezin izan du LaTeX banaketarik aurkitu sisteman.'
${LangFileString} PathName '$\"latex.exe$\" fitxategiaren bide-izena'
${LangFileString} DontUseLaTeX "Ez erabili LaTeX"
${LangFileString} InvalidLaTeXFolder '$\"latex.exe$\" fitxategia ez dago zehaztutako bide-izenean.'

${LangFileString} LatexInfo 'Orain $\"MiKTeX$\" LaTeX banaketaren instalatzailea abiaraziko da.$\r$\n\
				Programa instalatzeko sakatu $\"Hurrengoa$\" botoia  instalatzailearen leihoan instalazioa hasi arte.$\r$\n\
				$\r$\n\
				Mesedez, erabili MiKTeX-en instalatzailearen aukera lehenetsi guztiak!!!'
${LangFileString} LatexError1 'Ezin izan da LaTeX banaketarik aurkitu!$\r$\n\
				Ezin  da LyX erabili $\"MiKTeX$\" bezalako LaTeX banaketarik gabe!$\r$\n\
				Instalazioa bertan behera utziko da.'

${LangFileString} HunspellFailed 'Huts egin du  $\"$R3$\" hizkuntzaren hiztegia deskargatzean.'
${LangFileString} ThesaurusFailed 'Huts egin du  $\"$R3$\" hizkuntzaren thesaurusa deskargatzean.'

${LangFileString} JabRefInfo 'Orain $\"JabRef$\" programaren instalatzailea abiaraziko da.$\r$\n\
				JabRef instalatzailearen aukera lehenetsi guztiak erabil ditzakezu.'
${LangFileString} JabRefError '$\"JabRef$\" programa ezin izan da instalatu.$\r$\n\
				Hala ere, instalatzaileak jarraitu egingo du.$\r$\n\
				Saiatu JabRef beranduago instalatzen.'

#${LangFileString} LatexConfigInfo "LyX-en hurrengo konfigurazioak denbora piskat beharko du."

#${LangFileString} MiKTeXPathInfo "Erabiltzaile bakoitzak beranduago MiKTeX bere beharretara egokitzeko aukera $\r$\n\
#					beharrezkoa da MiKTeX-en instalazioaren karpetari (eta bere azpikarpeta guztiei) $\r$\n\ 
#					erabiltzaile guztientzako idazteko baimenak ematea:$\r$\n\
#					$MiKTeXPath $\r$\n\"
${LangFileString} MiKTeXInfo 'LaTeX-en $\"MiKTeX$\" banaketa LyX-ekin batera erabiltzenn da.$\r$\n\
				Gomendagarria da eskuragarri dauden MiKTeX-en eguneraketak instalatzea $\r$\n\
				$\"MiKTeX Update Wizard$\" (MiKTeX-en eguneraketen morroia) programa erabiliz$\r$\n\
				LyX aurreneko aldiz erabili aurretik.$\r$\n\
				MiKTeX-en eguneraketak begiratu orain?'

${LangFileString} ModifyingConfigureFailed "Ezin izan da 'path_prefix' ezarri konfigurazioaren script-ean"
#${LangFileString} RunConfigureFailed "Ezin izan da konfigurazioaren script-a exekutatu"
${LangFileString} InstallRunning "Instalatzailea jadanik exekutatzen ari da."
${LangFileString} AlreadyInstalled "LyX ${APP_SERIES_KEY2} jadanik instalatuta dago!$\r$\n\
				Installing over existing installations is not recommended if the installed version$\r$\n\
				is a test release or if you have problems with your existing LyX installation.$\r$\n\
				In these cases better reinstall LyX.$\r$\n\
				Dou you nevertheles want to install LyX over the existing version?"
${LangFileString} NewerInstalled "Instalatuta dagoen LyX baino bertsio zaharragoa instalatzen saiatzen ari zara.$\r$\n\
				  Hori egitea nahi baduzu, lehenbizi existitzen den LyX $OldVersionNumber desinstalatu beharko duzu."

${LangFileString} MultipleIndexesNotAvailable "Dokumentu batean hainbat indize erabiltzeko euskarria ez dago erabilgarri $\r$\n\
						MiKTeX administratzailearen pribilegioekin instalatu delako baina zuk LyX $\r$\n\
						haiek gabe instalatzen ari zarelako."

#${LangFileString} FinishPageMessage "Zorionak! LyX ongi instalatu da.$\r$\n\
#					$\r$\n\
#					(LyX aurreneko aldiz abiatzean denbora piskat beharko du.)"
${LangFileString} FinishPageRun "Abiarazi LyX"

${LangFileString} UnNotInRegistryLabel "Ezin da LyX aurkitu erregistroan.$\r$\n\
					Mahaigaineko eta Hasiera menuko lasterbideak ez dira kenduko."
${LangFileString} UnInstallRunning "Aurrenik LyX itxi behar duzu."
${LangFileString} UnNotAdminLabel "Administratzailearen baimenak behar dituzu LyX desinstalatzeko."
${LangFileString} UnReallyRemoveLabel "Ziur zaude LyX eta bere osagai guztiak kentzea nahi dituzula??"
${LangFileString} UnLyXPreferencesTitle 'LyX-eko erabiltzailearen hobespenak'

${LangFileString} SecUnMiKTeXDescription "LaTeX-en MiKTeX banaketa desinstalatzen du."
${LangFileString} SecUnJabRefDescription "Bibliografiaren JabRef kudeatzailea desinstalatzen du."
${LangFileString} SecUnPreferencesDescription 'LyX-en konfigurazioa ezabatzen du$\r$\n\
						($\"$AppPre\erabiltzailea\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						zuretzako edo erabiltzaile guztientzako (administratzailea bazara).'
${LangFileString} SecUnProgramFilesDescription "Desinstalatu LyX eta bere osagai guztiak."

