!insertmacro LANGFILE_EXT "Swedish"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installerad för aktuell användare)"

${LangFileString} TEXT_WELCOME "Denna guide tar dig igenom installationen av $(^NameDA), $\r$\n\
				$\r$\n\
				$_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Uppdaterar MiKTeX filnamnsdatabas..."
${LangFileString} TEXT_CONFIGURE_LYX "Konfigurerar LyX ($LaTeXInstalled kanske laddar ned paket som saknas, detta kan ta lite tid) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Kompilerar Pythonskript..."

${LangFileString} TEXT_FINISH_DESKTOP "Skapa skrivbordsgenväg"
${LangFileString} TEXT_FINISH_WEBSITE "Besök lyx.org för de senaste nyheterna, support och tips"

${LangFileString} FileTypeTitle "LyX-dokument"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Installera för alla användare?"
${LangFileString} SecFileAssocTitle "Filassociationer"
${LangFileString} SecDesktopTitle "Skrivbordsikon"

${LangFileString} SecCoreDescription "LyX-filerna."
${LangFileString} SecInstJabRefDescription "Program för att hantera bibliografiska referenser och redigera BibTeX-filer."
${LangFileString} SecAllUsersDescription "Installera LyX för alla användare, eller enbart för den aktuella användaren."
${LangFileString} SecFileAssocDescription "Filer med ändelsen .lyx kommer att automatiskt öppnas i LyX."
${LangFileString} SecDesktopDescription "En LyX-ikon på skrivbordet."
${LangFileString} SecDictionariesDescription "Stavningskontrollens ordböcker som kan laddas ned och installeras."
${LangFileString} SecThesaurusDescription "Synonymordbokens ordböcker som kan laddas ned och installeras."

${LangFileString} EnterLaTeXHeader1 'LaTeX-distribution'
${LangFileString} EnterLaTeXHeader2 'Ställ in LaTeX-distributionen som LyX skall använda.'
${LangFileString} EnterLaTeXFolder 'Alternativt kan du här ställa in sökvägen till filen $\"latex.exe$\" och därmed bestämma vilken \
					LaTeX-distribution som skall användas av LyX.\r\n\
					Om du inte använder LaTeX kan LyX inte mata ut dokument!\r\n\
					\r\n\
					Installationsprogrammet har upptäckt LaTeX-distributionen \
					$\"$LaTeXName$\" i ditt system. Nedan visas dess sökväg.'
${LangFileString} EnterLaTeXFolderNone 'Sätt nedan sökvägen till filen $\"latex.exe$\". Därmed väljer du vilken \
					LaTeX-distribution som skall användas av LyX.\r\n\
					Om du inte använder LaTeX kan LyX inte mata ut dokument!\r\n\
					\r\n\
					Installationsprogrammet kunde inte hitta LaTeX-distributionen i ditt system.'
${LangFileString} PathName 'Sökväg till filen $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Använd inte LaTeX"
${LangFileString} InvalidLaTeXFolder 'Filen $\"latex.exe$\" finns inte i den angivna sökvägen.'

${LangFileString} LatexInfo 'Nu kommer installationsprogrammet för LaTeX-distribution $\"MiKTeX$\" att köras.$\r$\n\
				För att installera programmet, klicka på knappen $\"Next$\" i installationsprogrammets fönster tills installation börjar.$\r$\n\
				$\r$\n\
				!!! Vänligen använd standardinställningarna i installationsprogrammet för MiKTeX !!!'
${LangFileString} LatexError1 'Ingen LaTeX-distribution kunde hittas!$\r$\n\
				LyX kan inte användas utan en LaTeX-distribution som $\"MiKTeX$\"!$\r$\n\
				Installationen kommer därför att avbrytas.'

${LangFileString} HunspellFailed 'Nedladdning av ordbok för språk $\"$R3$\" misslyckades.'
${LangFileString} ThesaurusFailed 'Nedladdning av synonymordbok för språk $\"$R3$\" misslyckades.'

${LangFileString} JabRefInfo 'Nu kommer installationsprogrammet för $\"JabRef$\" att köras.$\r$\n\
				Du kan använda alla standardinställningar i installationsprogrammet för JabRef.'
${LangFileString} JabRefError 'Programmet $\"JabRef$\" kunde inte installeras ordentligt!$\r$\n\
				Installationsprogrammet kommer att fortsätta ändå.$\r$\n\
				Försök installera JabRef igen senare.'

${LangFileString} LatexConfigInfo "Följande konfigurering av LyX kommer att ta en stund."

${LangFileString} MiKTeXPathInfo "För att varje användare senare skall kunna anpassa MiKTeX för sina behov$\r$\n\
					är det nödvändigt att ge skrivrättigheter till alla användare för MiKTeX installationskatalog$\r$\n\
					$MiKTeXPath $\r$\n\
					och dess underkataloger."
${LangFileString} MiKTeXInfo 'LaTeX-distributionen $\"MiKTeX$\" kommer att användas tillsammans med LyX.$\r$\n\
				Det rekommenderas att installera tillgängliga uppdateringar för MiKTeX med hjälp av programmet $\"MiKTeX Update Wizard$\".$\r$\n\
				innan du använder LyX för första gången.$\r$\n\
				Vill du kontrollera om det finns uppdateringar för MiKTeX nu?'

${LangFileString} ModifyingConfigureFailed "Kunde inte sätta  'path_prefix' i konfigurationsskriptet"
${LangFileString} RunConfigureFailed "Kunde inte köra konfigurationsskriptet"
${LangFileString} NotAdmin "Du måste ha administratörsrättigheter för att installera LyX!"
${LangFileString} InstallRunning "Installationsprogrammet körs redan!"
${LangFileString} AlreadyInstalled "LyX ${APP_SERIES_KEY2} är redan installerad!$\r$\n\
				Installing over existing installations is not recommended if the installed version$\r$\n\
				is a test release or if you have problems with your existing LyX installation.$\r$\n\
				In these cases better reinstall LyX.$\r$\n\
				Dou you nevertheles want to install LyX over the existing version?"
${LangFileString} NewerInstalled "Du försöker att installera en äldre version av LyX än vad du har installerad.$\r$\n\
				  Om du verkligen vill detta måste du avinstallera den befintliga LyX $OldVersionNumber innan."

${LangFileString} MultipleIndexesNotAvailable "Stödet för att använda flera index i ett dokument kommer inte vara tillgängligt eftersom$\r$\n\
						MiKTeX installerades med administratörrättigheter men du installerar LyX utan dem."
${LangFileString} MetafileNotAvailable "LyX-stödet för bilder i formatet EMF eller WMF kommer inte vara$\r$\n\
					tillgängligt eftersom det kräver att installera en mjukvaruskrivare till$\r$\n\
					Windows vilket endast är möjligt med administratörsbehörighet."

${LangFileString} FinishPageMessage "Gratulerar! LyX har installerats framgångsrikt.$\r$\n\
					$\r$\n\
					(Den första starten av LyX kan ta några sekunder.)"
${LangFileString} FinishPageRun "Kör LyX"

${LangFileString} UnNotInRegistryLabel "Kan inte hitta LyX i registret.$\r$\n\
					Genvägar på skrivbordet och i startmenyn kommer inte att tas bort."
${LangFileString} UnInstallRunning "Du måste stänga LyX först!"
${LangFileString} UnNotAdminLabel "Du måste ha administratörsbehörighet för att avinstallera LyX!"
${LangFileString} UnReallyRemoveLabel "Är du säker på att du verkligen vill fullständigt ta bort LyX och alla dess komponenter?"
${LangFileString} UnLyXPreferencesTitle 'LyX användarinställningar'

${LangFileString} SecUnMiKTeXDescription "Avinstallerar LaTeX-distributionen MiKTeX."
${LangFileString} SecUnJabRefDescription "Avinstallerar bibliografihanteraren JabRef."
${LangFileString} SecUnPreferencesDescription 'Raderar LyX konfiguration$\r$\n\
						(katalog $\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						för dig eller för alla användare (om du är admin).'
${LangFileString} SecUnProgramFilesDescription "Avinstallera LyX och alla dess komponenter."

