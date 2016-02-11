/*
LyX Installer Language File
Language: German
Author: Uwe Stöhr
*/

!insertmacro LANGFILE_EXT "German"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installiert für den aktuellen Benutzer)"

${LangFileString} TEXT_WELCOME "Dieser Assistent wird Sie durch die Installation von $(^NameDA) begleiten.$\r$\n\
				$\r$\n\
				$_CLICK"

#${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Aktualisiere MiKTeXs Dateinamen Datenbank..."
${LangFileString} TEXT_CONFIGURE_LYX "Konfiguriere LyX ($LaTeXInstalled kann fehlende Pakte herunterladen, was etwas Zeit braucht) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Kompiliere Python Skripte..."

${LangFileString} TEXT_FINISH_DESKTOP "Ein Symbol auf der Arbeitsoberfläche erzeugen"
${LangFileString} TEXT_FINISH_WEBSITE "Besuchen Sie lyx.org für aktuelle Neuigkeiten"

#${LangFileString} FileTypeTitle "LyX-Dokument"

${LangFileString} SecInstJabRefTitle "JabRef"
#${LangFileString} SecAllUsersTitle "Für alle Nutzer installieren?"
${LangFileString} SecFileAssocTitle "Dateizuordnungen"
${LangFileString} SecDesktopTitle "Desktopsymbol"

${LangFileString} SecCoreDescription "Das Programm LyX."
${LangFileString} SecInstJabRefDescription "Manager für bibliografische Referenzen und Editor für BibTeX-Dateien."
#${LangFileString} SecAllUsersDescription "LyX für alle Nutzer oder nur für den aktuellen Nutzer installieren."
${LangFileString} SecFileAssocDescription "Vernüpfung zwischen LyX und der .lyx Dateiendung."
${LangFileString} SecDesktopDescription "Verknüpfung zu LyX auf dem Desktop."
${LangFileString} SecDictionariesDescription "Rechtschreibprüfung- Wörterbucher die heruntergeladen und installiert werden können."
${LangFileString} SecThesaurusDescription "Thesaurus- Wörterbucher die heruntergeladen und installiert werden können."

${LangFileString} EnterLaTeXHeader1 'LaTeX-Distribution'
${LangFileString} EnterLaTeXHeader2 'Legen Sie die LaTeX-Distribution fest die LyX verwenden soll.'
${LangFileString} EnterLaTeXFolder 'Optional können Sie hier den Pfad zur Datei $\"latex.exe$\" angeben und damit die \
					LaTeX-Distribution festlegen, die LyX verwenden soll.\r\n\
					Wenn Sie kein LaTeX verwenden, kann LyX keine Dokumente ausgeben!\r\n\
					\r\n\
					Der Installer hat auf Ihrem System die LaTeX-Distribution\r\n\
					$\"$LaTeXName$\" erkannt. Unten angegeben ist ihr Pfad.'
${LangFileString} EnterLaTeXFolderNone 'Geben Sie unten den Pfad zur Datei $\"latex.exe$\" an. Damit legen Sie fest welche \
					LaTeX-Distribution LyX verwenden soll.\r\n\
					Wenn Sie kein LaTeX verwenden, kann LyX keine Dokumente ausgeben!\r\n\
					\r\n\
					Der Installer konnte auf Ihrem System keine LaTeX-Distribution finden.'
${LangFileString} PathName 'Pfad zur Datei $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Kein LaTeX benutzen"
${LangFileString} InvalidLaTeXFolder 'Kann die Datei $\"latex.exe$\" nicht finden.'

${LangFileString} LatexInfo 'Als Nächstes wird der Installer der LaTeX-Distribution $\"MiKTeX$\" gestartet.$\r$\n\
				Um das Programm zu installieren, drücken Sie den $\"Weiter$\"-Knopf in den Installerfenstern bis die Installation beginnt.$\r$\n\
				$\r$\n\
				!!! Bitte verwenden Sie alle voreingestellten Optionen des MiKTeX-Installers !!!'
${LangFileString} LatexError1 'Es konnte keine LaTeX-Distribution gefunden werden!$\r$\n\
				LyX kann ohne eine LaTeX-Distribution wie z.B. $\"MiKTeX$\" nicht benutzt werden!$\r$\n\
				Die Installation wird daher abgebrochen.'

${LangFileString} HunspellFailed 'Herunterladen des Wörterbuchs für Sprache $\"$R3$\" fehlgeschlagen.'
${LangFileString} ThesaurusFailed 'Herunterladen des Thesaurus für Sprache $\"$R3$\" fehlgeschlagen.'

${LangFileString} JabRefInfo 'Als Nächstes wird der Installer des Programms $\"JabRef$\" gestartet.$\r$\n\
				Sie können alle voreingestellten Optionen des JabRef-Installers verwenden.'
${LangFileString} JabRefError 'Das Programm $\"JabRef$\" konnte nicht erfolgreich installiert werden!$\r$\n\
				Der Installer wird trotzdem fortgesetzt.$\r$\n\
				Versuchen Sie JabRef später noch einmal zu installieren.'

#${LangFileString} LatexConfigInfo "Die folgende Konfiguration von LyX wird eine Weile dauern."

#${LangFileString} MiKTeXPathInfo "Damit jeder Benutzer später MiKTeX für seine Bedürfnisse anpassen kann,$\r$\n\
#					ist es erforderlich für MiKTeXs Installationsordner$\r$\n\
#					$MiKTeXPath $\r$\n\
#					und seine Unterordner Schreibrechte für alle Benutzer zu setzen."
${LangFileString} MiKTeXInfo 'Die LaTeX-Distribution $\"MiKTeX$\" wird zusammen mit LyX verwendet werden.$\r$\n\
				Es wird empfohlen dass Sie verfügbare MiKTeX-Updates mit Hilfe des Programms $\"MiKTeX Update Wizard$\"$\r$\n\
				installieren, bevor sie LyX das erste Mal benutzen.$\r$\n\
				Möchten Sie jetzt nach Updates für MiKTeX suchen?'

${LangFileString} ModifyingConfigureFailed 'Der $\"PATH_präfix$\" (Liste mit Programmpfaden) konnte nicht im Konfigurationsskript gesetzt werden.'
#${LangFileString} RunConfigureFailed "Konnte das Konfigurationsskript nicht ausführen."
${LangFileString} NotAdmin "Sie benötigen Administratorrechte um LyX zu installieren!"
${LangFileString} InstallRunning "Der Installer läuft bereits!"
${LangFileString} AlreadyInstalled "LyX ${APP_SERIES_KEY2} ist bereits installiert!$\r$\n\
				Das Installieren über bestehende Installationen ist nicht empfohlen, wenn die installierte Version$\r$\n\
				eine Testversion ist oder wenn es Probleme mit der bestehenden LyX-Installation gibt.$\r$\n\
				Besser Sie deinstallieren in diesen Fällen LyX zuerst.$\r$\n\
				Wollen Sie LyX dennoch über die bestehende Version installieren?"
${LangFileString} NewerInstalled "Sie versuchen eine Vesion von LyX zu installieren, die älter als die derzeit installierte ist.$\r$\n\
				  Wenn Sie das wirklich wollen, müssen Sie erst das existierende LyX $OldVersionNumber deinstallieren."

${LangFileString} MultipleIndexesNotAvailable "Die Unterstützung für mehrere Indexe in einem Dokument wird nicht verfügbar sein, denn$\r$\n\
						MiKTeX wurde mit Administratorrechten installiert aber Sie installieren LyX ohne diese."

#${LangFileString} FinishPageMessage "Glückwunsch! LyX wurde erfolgreich installiert.$\r$\n\
#					$\r$\n\
#					(Der erste Start von LyX kann etwas länger dauern.)"
${LangFileString} FinishPageRun "LyX starten"

${LangFileString} UnNotInRegistryLabel "Kann LyX nicht in der Registry finden.$\r$\n\
					Desktopsymbole und Einträge im Startmenü können nicht entfernt werden."
${LangFileString} UnInstallRunning "Sie müssen LyX zuerst beenden!"
${LangFileString} UnNotAdminLabel "Sie benötigen Administratorrechte um LyX zu deinstallieren!"
${LangFileString} UnReallyRemoveLabel "Sind Sie sicher, dass sie LyX und all seine Komponenten deinstallieren möchten?"
${LangFileString} UnLyXPreferencesTitle 'LyXs Benutzereinstellungen'

${LangFileString} SecUnMiKTeXDescription "Deinstalliert die LaTeX-Distribution MiKTeX."
${LangFileString} SecUnJabRefDescription "Deinstalliert den Bibliografiemanager JabRef."
${LangFileString} SecUnPreferencesDescription 'Löscht LyXs Benutzereinstellungen$\r$\n\
						(Ordner $\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\")$\r$\n\
						für Sie oder für alle Benutzer (wenn Sie Admin sind).'
${LangFileString} SecUnProgramFilesDescription "Deinstalliert LyX und all seine Komponenten."

