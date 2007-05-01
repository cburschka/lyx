!ifndef _LYX_LANGUAGES_SWEDISH_NSH_
!define _LYX_LANGUAGES_SWEDISH_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_SWEDISH}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "Denna guide tar dig igenom installationen av LyX.\r\n\
					  \r\n\
					  Du behöver administratörsrättigheter för att installera LyX.\r\n\
					  \r\n\
					  Det rekommenderas att du avslutar alla andra program innan du fortsätter installationen. Detta tillåter att installationen uppdaterar nödvändiga systemfiler utan att behöva starta om din dator.\r\n\
					  \r\n\
					  $_CLICK"

LangString FileTypeTitle "${LYX_LANG}" "LyX-dokument"

LangString SecInstGSviewTitle "${LYX_LANG}" "GSview"
LangString SecInstJabRefTitle "${LYX_LANG}" "JabRef"
LangString SecAllUsersTitle "${LYX_LANG}" "Installera för alla användare?"
LangString SecFileAssocTitle "${LYX_LANG}" "Filassociationer"
LangString SecDesktopTitle "${LYX_LANG}" "Skrivbordsikon"

LangString SecCoreDescription "${LYX_LANG}" "LyX-filer."
LangString SecInstGSviewDescription "${LYX_LANG}" "Program för att visa Postscript- och PDF-dokument."
LangString SecInstJabRefDescription "${LYX_LANG}" "Program för att hantera bibliografiska referenser och redigera BibTeX-filer."
LangString SecAllUsersDescription "${LYX_LANG}" "Installera LyX för alla användare, eller enbart för den aktuella användare."
LangString SecFileAssocDescription "${LYX_LANG}" "Skapa en association mellan programmet och filtilläget .lyx."
LangString SecDesktopDescription "${LYX_LANG}" "En LyXikon på skrivbordet."

LangString LangSelectHeader "${LYX_LANG}" "Val av språk i LyXs menyer"
LangString AvailableLang "${LYX_LANG}" " Tillgängliga språk "

LangString MissProgHeader "${LYX_LANG}" "Leta efter nödvändiga program"
LangString MissProgCap "${LYX_LANG}" "Följande nödvändiga program kommer dessutom att installeras"
LangString MissProgLatex "${LYX_LANG}" "MiKTeX, en LaTeX-distribution"
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript, ett konverteringsprogram mm för PostScript och PDF"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick, ett konverteringsprogram för bilder"
LangString MissProgAspell "${LYX_LANG}" "Aspell, ett rättstavningsprogram"
LangString MissProgMessage "${LYX_LANG}" 'Inga ytterligare program behöver installeras.'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'LaTeX-distribution'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'Ställ in LaTeX-distributionen som LyX skall använda.'
LangString EnterLaTeXFolder "${LYX_LANG}" 'Alternativt kan du här ställa in sökvägen till filen "latex.exe" och därmed manuellt bestämma vilken \
					   LaTeX-distribution som skall användas av LyX.\r\n\
					   Utan LaTeX kan LyX bara redigera LyX-filer, inte producera t.ex. PDF-dokument!\r\n\
					   \r\n\
					   Installationsprogrammet har upptäckt LaTeX-distributionen \
					   "$LaTeXName" i ditt system. Nedan visas dess sökväg.'
LangString EnterLaTeXFolderNone "${LYX_LANG}" 'Sätt nedan sökvägen till filen "latex.exe". Därmed väljer du vilken \
					       LaTeX-distribution som skall användas av LyX.\r\n\
					       Utan LaTeX kan LyX bara redigera LyX-filer, inte producera t.ex. PDF-dokument!\r\n\
					       \r\n\
					       Installationsprogrammetn kunde inte hitta LaTeX-distributionen i ditt system.'
LangString PathName "${LYX_LANG}" 'Sökväg till filen "latex.exe"'
LangString DontUseLaTeX "${LYX_LANG}" "Använd inte LaTeX"
LangString InvalidLaTeXFolder "${LYX_LANG}" 'Kan inte hitta "latex.exe"'

LangString LatexInfo "${LYX_LANG}" 'Nu kommer installationsprogrammet för LaTeX-distribution "MiKTeX" att köras.$\r$\n\
			            För att installera programmet, klicka på knappen "Next" i installationsfönstret tills installation börjar.$\r$\n\
				    $\r$\n\
				    !!! Var snäll och använd standardinställningarna i installationsprogrammet för MiKTeX !!!'
LangString LatexError1 "${LYX_LANG}" 'Kan inte hitta någon LaTeX-distribution!$\r$\n\
                      		      LyX kan inte användas utan en LaTeX-distribution som tex "MiKTeX"!$\r$\n\
				      Installation avbryts därför nu.'
				    
LangString GSviewInfo "${LYX_LANG}" 'Nu kommer installationsprogrammet för "GSview" att köras.$\r$\n\
			             För att installera programmet, klicka på knappen "Setup" i den första dialogrutan i installationsprogrammet\r$\n\
				     välj ett språk och klicka sedan på knappen "Next" i det nästkommande installationsfönstret.$\r$\n\
				     Du kan använda alla standardalternativ i installationsprogrammet för GSview.'
LangString GSviewError "${LYX_LANG}" 'Programmet "GSview" kunde inte installeras ordentligt!$\r$\n\
		                      Installationen av LyX kommer att fortsätta ändå.$\r$\n\
				      Försök installera GSview igen senare.'

LangString JabRefInfo "${LYX_LANG}" 'Nu kommer installationsprogrammet för "JabRef" att köras.$\r$\n\
				     Du kan använda alla standardalternativ i installationsprogrammet för JabRef.'
LangString JabRefError "${LYX_LANG}" 'Programmet "JabRef" kunde inte installeras ordentligt!$\r$\n\
		                      Installationen av LyX kommer att fortsätta ändå.$\r$\n\
				      Försök installera GSview igen senare.'
				      
LangString LatexConfigInfo "${LYX_LANG}" "Följande konfigurering av LyX kommer att ta ett tag."

LangString AspellInfo "${LYX_LANG}" 'Nu kommer ordlistor för stavningsprogrammet "Aspell" att laddas ner och installeras.$\r$\n\
				     Varje ordlista har en egen licens vilken kommer att visas före installationen.'
LangString AspellDownloadFailed "${LYX_LANG}" "Ingen ordlista för stavningsprogrammet Aspell kunde laddas ner!"
LangString AspellInstallFailed "${LYX_LANG}" "Ingen ordlista för stavningsprogrammet Aspell kunde installeras!"
LangString AspellPartAnd "${LYX_LANG}" " and "
LangString AspellPartSep "${LYX_LANG}" ", "
LangString AspellAll "${LYX_LANG}" 'Det finns redan en installerad ordlista för språket'
LangString AspellNone "${LYX_LANG}" 'Ingen ordlista för stavningsprogrammet "Aspell" har blivit installerad.$\r$\n\
				     Ordlistor kan laddas ner från\r$\n\
				     ${AspellLocation}$\r$\n\
				     Vill du ladda ner ordlistor nu?'
LangString AspellPartStart "${LYX_LANG}" "Det installerades framgångsrikt "
LangString AspellPart1 "${LYX_LANG}" "en engelsk ordlista"
LangString AspellPart2 "${LYX_LANG}" "en ordlista för språket $LangName"
LangString AspellPart3 "${LYX_LANG}" "en$\r$\n\
				      ordlista för språket $LangNameSys"
LangString AspellPart4 "${LYX_LANG}" ' för stavningsprogrammet "Aspell".$\r$\n\
				      Ytterligare ordlistor kan laddas ner från\r$\n\
				      ${AspellLocation}$\r$\n\
				      Vill du ladda ner ytterligare ordlistor nu?'

LangString MiKTeXPathInfo "${LYX_LANG}" "För att varje användare senare skall kunna anpassa MiKTeX för sina behov$\r$\n\
					 är det nödvändigt att ge skrivrättigheter till alla användare för MiKTeX's installationskatalog$\r$\n\
					 $MiKTeXPath $\r$\n\
					 och dess underkataloger."
LangString MiKTeXInfo "${LYX_LANG}" 'LaTeX-distributionen "MiKTeX" kommer att användas tillsammans med LyX.$\r$\n\
				     Det rekommenderas att installera tillgängliga uppdateringar av MiKTeX med hjälp av programmet "MiKTeX Update Wizard".$\r$\n\
				     innan du använder LyX för första gången.$\r$\n\
				     Vill du kontrollera om det finns uppdateringar tillgängliga för MiKTeX nu?'

LangString UpdateNotAllowed "${LYX_LANG}" "This update package can only update ${PRODUCT_VERSION_OLD}!"
LangString ModifyingConfigureFailed "${LYX_LANG}" "Misslyckades med att sätta  'path_prefix' i konfigurationsskriptet"
LangString CreateCmdFilesFailed "${LYX_LANG}" "Kunde inte skapa filen (kommandoskriptet) lyx.bat"
LangString RunConfigureFailed "${LYX_LANG}" "Misslyckades med att köra konfigurationsskriptet"
LangString NotAdmin "${LYX_LANG}" "Du måste ha administratörsrättigheter för att installera LyX!"
LangString InstallRunning "${LYX_LANG}" "Installationsprogrammet körs readan!"
LangString StillInstalled "${LYX_LANG}" "LyX är redan installerat! Avinstallera LyX först."

LangString FinishPageMessage "${LYX_LANG}" "Gratulerar! LyX har installerats framgångsrikt.\r\n\
					    \r\n\
					    (Första gången LyX startas tar det litet längre tid då programmet konfigurerar sig själv automatiskt.)"
LangString FinishPageRun "${LYX_LANG}" "Kör LyX"

LangString UnNotInRegistryLabel "${LYX_LANG}" "Kan inte hitta LyX i registret\r$\n\
					       Genvägar på skrivbordet och i startmeny kommer inte att tas bort."
LangString UnInstallRunning "${LYX_LANG}" "Du måste stänga LyX först!"
LangString UnNotAdminLabel "${LYX_LANG}" "Du måste ha administratörsrättigheter för att avinstallera LyX!"
LangString UnReallyRemoveLabel "${LYX_LANG}" "Är du säker på att du verkligen vill fullständigt avinstallera LyX och alla dess komponenter?"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'LyX$\'s användarinställningar'
LangString UnGSview "${LYX_LANG}" 'Var god tryck på knappen "Avinstallera" i följande fönster för att avinstallera$\r$\n\
				    "GSview" som visar Postscript och PDF.'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "LyX har framgångsrikt avinstallerats från din dator."

LangString SecUnAspellDescription "${LYX_LANG}" "Avinstallerar stavningsprogrammet Aspell och alla dess kataloger."
LangString SecUnMiKTeXDescription "${LYX_LANG}" "Avinstallerar LaTeX-distributionen MiKTeX."
LangString SecUnJabRefDescription "${LYX_LANG}" "Avinstallerar programmet JabRef som används för att hantera bibliografiska referenser."
LangString SecUnPreferencesDescription "${LYX_LANG}" 'Raderar LyX$\'s konfigurationskatalog$\r$\n\
					   "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
					   för alla användare.'
LangString SecUnProgramFilesDescription "${LYX_LANG}" "Avinstallera LyX och alla dess komponenter."

!undef LYX_LANG

!endif # _LYX_LANGUAGES_SWEDISH_NSH_
