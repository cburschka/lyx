!insertmacro LANGFILE_EXT "Swedish"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installed for Current User)"

${LangFileString} TEXT_WELCOME "Denna guide tar dig igenom installationen av LyX.$\r$\n\
					  $\r$\n\
					  $_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Updating MiKTeX filename database..."
${LangFileString} TEXT_CONFIGURE_LYX "Configuring LyX (MiKTeX may download missing packages, this can take some time) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compiling Python scripts..."

${LangFileString} TEXT_FINISH_DESKTOP "Create desktop shortcut"
${LangFileString} TEXT_FINISH_WEBSITE "Visit lyx.org for the latest news, support and tips"

${LangFileString} FileTypeTitle "LyX-dokument"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Installera för alla användare?"
${LangFileString} SecFileAssocTitle "Filassociationer"
${LangFileString} SecDesktopTitle "Skrivbordsikon"

${LangFileString} SecCoreDescription "LyX-filer."
${LangFileString} SecInstJabRefDescription "Program för att hantera bibliografiska referenser och redigera BibTeX-filer."
${LangFileString} SecAllUsersDescription "Installera LyX för alla användare, eller enbart för den aktuella användare."
${LangFileString} SecFileAssocDescription "Skapa en association mellan programmet och filtilläget .lyx."
${LangFileString} SecDesktopDescription "En LyXikon på skrivbordet."

${LangFileString} EnterLaTeXHeader1 'LaTeX-distribution'
${LangFileString} EnterLaTeXHeader2 'Ställ in LaTeX-distributionen som LyX skall använda.'
${LangFileString} EnterLaTeXFolder 'Alternativt kan du här ställa in sökvägen till filen $\"latex.exe$\" och därmed manuellt bestämma vilken \
					   LaTeX-distribution som skall användas av LyX.$\r$\n\
					   Utan LaTeX kan LyX bara redigera LyX-filer, inte producera t.ex. PDF-dokument!$\r$\n\
					   $\r$\n\
					   Installationsprogrammet har upptäckt LaTeX-distributionen \
					   $\"$LaTeXName$\" i ditt system. Nedan visas dess sökväg.'
${LangFileString} EnterLaTeXFolderNone 'Sätt nedan sökvägen till filen $\"latex.exe$\". Därmed väljer du vilken \
					       LaTeX-distribution som skall användas av LyX.$\r$\n\
					       Utan LaTeX kan LyX bara redigera LyX-filer, inte producera t.ex. PDF-dokument!$\r$\n\
					       $\r$\n\
					       Installationsprogrammetn kunde inte hitta LaTeX-distributionen i ditt system.'
${LangFileString} PathName 'Sökväg till filen $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Använd inte LaTeX"
${LangFileString} InvalidLaTeXFolder 'Kan inte hitta $\"latex.exe$\"'

${LangFileString} LatexInfo 'Nu kommer installationsprogrammet för LaTeX-distribution $\"MiKTeX$\" att köras.$\r$\n\
			            För att installera programmet, klicka på knappen $\"Next$\" i installationsfönstret tills installation börjar.$\r$\n\
				    $\r$\n\
				    !!! Var snäll och använd standardinställningarna i installationsprogrammet för MiKTeX !!!'
${LangFileString} LatexError1 'Kan inte hitta någon LaTeX-distribution!$\r$\n\
                      		      LyX kan inte användas utan en LaTeX-distribution som tex $\"MiKTeX$\"!$\r$\n\
				      Installation avbryts därför nu.'

${LangFileString} HunspellFailed 'Download of dictionary for language $\"$R3$\" failed.'
${LangFileString} ThesaurusFailed 'Download of thesaurus for language $\"$R3$\" failed.'

${LangFileString} JabRefInfo 'Nu kommer installationsprogrammet för $\"JabRef$\" att köras.$\r$\n\
				     Du kan använda alla standardalternativ i installationsprogrammet för JabRef.'
${LangFileString} JabRefError 'Programmet $\"JabRef$\" kunde inte installeras ordentligt!$\r$\n\
		                      Installationen av LyX kommer att fortsätta ändå.$\r$\n\
				      Försök installera GSview igen senare.'

${LangFileString} LatexConfigInfo "Följande konfigurering av LyX kommer att ta ett tag."

${LangFileString} MiKTeXPathInfo "För att varje användare senare skall kunna anpassa MiKTeX för sina behov$\r$\n\
					 är det nödvändigt att ge skrivrättigheter till alla användare för MiKTeX's installationskatalog$\r$\n\
					 $MiKTeXPath $\r$\n\
					 och dess underkataloger."
${LangFileString} MiKTeXInfo 'LaTeX-distributionen $\"MiKTeX$\" kommer att användas tillsammans med LyX.$\r$\n\
				     Det rekommenderas att installera tillgängliga uppdateringar av MiKTeX med hjälp av programmet $\"MiKTeX Update Wizard$\".$\r$\n\
				     innan du använder LyX för första gången.$\r$\n\
				     Vill du kontrollera om det finns uppdateringar tillgängliga för MiKTeX nu?'

${LangFileString} ModifyingConfigureFailed "Misslyckades med att sätta  'path_prefix' i konfigurationsskriptet"
${LangFileString} RunConfigureFailed "Misslyckades med att köra konfigurationsskriptet"
${LangFileString} NotAdmin "Du måste ha administratörsrättigheter för att installera LyX!"
${LangFileString} InstallRunning "Installationsprogrammet körs readan!"
${LangFileString} StillInstalled "LyX är redan installerat! Avinstallera LyX först."

${LangFileString} MultipleIndexesNotAvailable "The support for using several indexes in a document will not be available because$\r$\n\
					       MiKTeX was installed with administrator privileges but you are installing LyX without them."
${LangFileString} MetafileNotAvailable "The LyX support for images in the format EMF or WMF will not be$\r$\n\
					available because that requires to install a software printer to$\r$\n\
					Windows which is only possible with administrator privileges."

${LangFileString} FinishPageMessage "Gratulerar! LyX har installerats framgångsrikt.$\r$\n\
					    $\r$\n\
					    (Första gången LyX startas tar det litet längre tid då programmet konfigurerar sig själv automatiskt.)"
${LangFileString} FinishPageRun "Kör LyX"

${LangFileString} UnNotInRegistryLabel "Kan inte hitta LyX i registret\r$\n\
					       Genvägar på skrivbordet och i startmeny kommer inte att tas bort."
${LangFileString} UnInstallRunning "Du måste stänga LyX först!"
${LangFileString} UnNotAdminLabel "Du måste ha administratörsrättigheter för att avinstallera LyX!"
${LangFileString} UnReallyRemoveLabel "Är du säker på att du verkligen vill fullständigt avinstallera LyX och alla dess komponenter?"
${LangFileString} UnLyXPreferencesTitle 'LyX$\'s användarinställningar'

${LangFileString} SecUnMiKTeXDescription "Avinstallerar LaTeX-distributionen MiKTeX."
${LangFileString} SecUnJabRefDescription "Avinstallerar programmet JabRef som används för att hantera bibliografiska referenser."
${LangFileString} SecUnPreferencesDescription 'Raderar LyX$\'s konfigurationskatalog$\r$\n\
					   $\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
					   för alla användare.'
${LangFileString} SecUnProgramFilesDescription "Avinstallera LyX och alla dess komponenter."

