!insertmacro LANGFILE_EXT "Swedish"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installerad f�r aktuell anv�ndare)"

${LangFileString} TEXT_WELCOME "Denna guide tar dig igenom installationen av $(^NameDA), $\r$\n\
				$\r$\n\
				$_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Uppdaterar MiKTeX filnamnsdatabas..."
${LangFileString} TEXT_CONFIGURE_LYX "Konfigurerar LyX ($LaTeXInstalled kanske laddar ned paket som saknas, detta kan ta lite tid) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Kompilerar Pythonskript..."

${LangFileString} TEXT_FINISH_DESKTOP "Skapa skrivbordsgenv�g"
${LangFileString} TEXT_FINISH_WEBSITE "Bes�k lyx.org f�r de senaste nyheterna, support och tips"

${LangFileString} FileTypeTitle "LyX-dokument"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Installera f�r alla anv�ndare?"
${LangFileString} SecFileAssocTitle "Filassociationer"
${LangFileString} SecDesktopTitle "Skrivbordsikon"

${LangFileString} SecCoreDescription "LyX-filerna."
${LangFileString} SecInstJabRefDescription "Program f�r att hantera bibliografiska referenser och redigera BibTeX-filer."
${LangFileString} SecAllUsersDescription "Installera LyX f�r alla anv�ndare, eller enbart f�r den aktuella anv�ndaren."
${LangFileString} SecFileAssocDescription "Filer med �ndelsen .lyx kommer att automatiskt �ppnas i LyX."
${LangFileString} SecDesktopDescription "En LyX-ikon p� skrivbordet."
${LangFileString} SecDictionariesDescription "Stavningskontrollens ordb�cker som kan laddas ned och installeras."
${LangFileString} SecThesaurusDescription "Synonymordbokens ordb�cker som kan laddas ned och installeras."

${LangFileString} EnterLaTeXHeader1 'LaTeX-distribution'
${LangFileString} EnterLaTeXHeader2 'St�ll in LaTeX-distributionen som LyX skall anv�nda.'
${LangFileString} EnterLaTeXFolder 'Alternativt kan du h�r st�lla in s�kv�gen till filen $\"latex.exe$\" och d�rmed best�mma vilken \
					LaTeX-distribution som skall anv�ndas av LyX.\r\n\
					Om du inte anv�nder LaTeX kan LyX inte mata ut dokument!\r\n\
					\r\n\
					Installationsprogrammet har uppt�ckt LaTeX-distributionen \
					$\"$LaTeXName$\" i ditt system. Nedan visas dess s�kv�g.'
${LangFileString} EnterLaTeXFolderNone 'S�tt nedan s�kv�gen till filen $\"latex.exe$\". D�rmed v�ljer du vilken \
					LaTeX-distribution som skall anv�ndas av LyX.\r\n\
					Om du inte anv�nder LaTeX kan LyX inte mata ut dokument!\r\n\
					\r\n\
					Installationsprogrammet kunde inte hitta LaTeX-distributionen i ditt system.'
${LangFileString} PathName 'S�kv�g till filen $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Anv�nd inte LaTeX"
${LangFileString} InvalidLaTeXFolder 'Filen $\"latex.exe$\" finns inte i den angivna s�kv�gen.'

${LangFileString} LatexInfo 'Nu kommer installationsprogrammet f�r LaTeX-distribution $\"MiKTeX$\" att k�ras.$\r$\n\
				F�r att installera programmet, klicka p� knappen $\"Next$\" i installationsprogrammets f�nster tills installation b�rjar.$\r$\n\
				$\r$\n\
				!!! V�nligen anv�nd standardinst�llningarna i installationsprogrammet f�r MiKTeX !!!'
${LangFileString} LatexError1 'Ingen LaTeX-distribution kunde hittas!$\r$\n\
				LyX kan inte anv�ndas utan en LaTeX-distribution som $\"MiKTeX$\"!$\r$\n\
				Installationen kommer d�rf�r att avbrytas.'

${LangFileString} HunspellFailed 'Nedladdning av ordbok f�r spr�k $\"$R3$\" misslyckades.'
${LangFileString} ThesaurusFailed 'Nedladdning av synonymordbok f�r spr�k $\"$R3$\" misslyckades.'

${LangFileString} JabRefInfo 'Nu kommer installationsprogrammet f�r $\"JabRef$\" att k�ras.$\r$\n\
				Du kan anv�nda alla standardinst�llningar i installationsprogrammet f�r JabRef.'
${LangFileString} JabRefError 'Programmet $\"JabRef$\" kunde inte installeras ordentligt!$\r$\n\
				Installationsprogrammet kommer att forts�tta �nd�.$\r$\n\
				F�rs�k installera JabRef igen senare.'

${LangFileString} LatexConfigInfo "F�ljande konfigurering av LyX kommer att ta en stund."

${LangFileString} MiKTeXPathInfo "F�r att varje anv�ndare senare skall kunna anpassa MiKTeX f�r sina behov$\r$\n\
					�r det n�dv�ndigt att ge skrivr�ttigheter till alla anv�ndare f�r MiKTeX installationskatalog$\r$\n\
					$MiKTeXPath $\r$\n\
					och dess underkataloger."
${LangFileString} MiKTeXInfo 'LaTeX-distributionen $\"MiKTeX$\" kommer att anv�ndas tillsammans med LyX.$\r$\n\
				Det rekommenderas att installera tillg�ngliga uppdateringar f�r MiKTeX med hj�lp av programmet $\"MiKTeX Update Wizard$\".$\r$\n\
				innan du anv�nder LyX f�r f�rsta g�ngen.$\r$\n\
				Vill du kontrollera om det finns uppdateringar f�r MiKTeX nu?'

${LangFileString} ModifyingConfigureFailed "Kunde inte s�tta  'path_prefix' i konfigurationsskriptet"
${LangFileString} RunConfigureFailed "Kunde inte k�ra konfigurationsskriptet"
${LangFileString} NotAdmin "Du m�ste ha administrat�rsr�ttigheter f�r att installera LyX!"
${LangFileString} InstallRunning "Installationsprogrammet k�rs redan!"
${LangFileString} AlreadyInstalled "LyX ${APP_SERIES_KEY2} �r redan installerad!$\r$\n\
				Installing over existing installations is not recommended if the installed version$\r$\n\
				is a test release or if you have problems with your existing LyX installation.$\r$\n\
				In these cases better reinstall LyX.$\r$\n\
				Dou you nevertheles want to install LyX over the existing version?"
${LangFileString} NewerInstalled "Du f�rs�ker att installera en �ldre version av LyX �n vad du har installerad.$\r$\n\
				  Om du verkligen vill detta m�ste du avinstallera den befintliga LyX $OldVersionNumber innan."

${LangFileString} MultipleIndexesNotAvailable "St�det f�r att anv�nda flera index i ett dokument kommer inte vara tillg�ngligt eftersom$\r$\n\
						MiKTeX installerades med administrat�rr�ttigheter men du installerar LyX utan dem."
${LangFileString} MetafileNotAvailable "LyX-st�det f�r bilder i formatet EMF eller WMF kommer inte vara$\r$\n\
					tillg�ngligt eftersom det kr�ver att installera en mjukvaruskrivare till$\r$\n\
					Windows vilket endast �r m�jligt med administrat�rsbeh�righet."

${LangFileString} FinishPageMessage "Gratulerar! LyX har installerats framg�ngsrikt.$\r$\n\
					$\r$\n\
					(Den f�rsta starten av LyX kan ta n�gra sekunder.)"
${LangFileString} FinishPageRun "K�r LyX"

${LangFileString} UnNotInRegistryLabel "Kan inte hitta LyX i registret.$\r$\n\
					Genv�gar p� skrivbordet och i startmenyn kommer inte att tas bort."
${LangFileString} UnInstallRunning "Du m�ste st�nga LyX f�rst!"
${LangFileString} UnNotAdminLabel "Du m�ste ha administrat�rsbeh�righet f�r att avinstallera LyX!"
${LangFileString} UnReallyRemoveLabel "�r du s�ker p� att du verkligen vill fullst�ndigt ta bort LyX och alla dess komponenter?"
${LangFileString} UnLyXPreferencesTitle 'LyX anv�ndarinst�llningar'

${LangFileString} SecUnMiKTeXDescription "Avinstallerar LaTeX-distributionen MiKTeX."
${LangFileString} SecUnJabRefDescription "Avinstallerar bibliografihanteraren JabRef."
${LangFileString} SecUnPreferencesDescription 'Raderar LyX konfiguration$\r$\n\
						(katalog $\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						f�r dig eller f�r alla anv�ndare (om du �r admin).'
${LangFileString} SecUnProgramFilesDescription "Avinstallera LyX och alla dess komponenter."

