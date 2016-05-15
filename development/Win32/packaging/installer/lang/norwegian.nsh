/*
LyX Installer Language File
Language: Norwegian
Author: Ingar Pareliussen
*/

!insertmacro LANGFILE_EXT "Norwegian"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installer for denne brukeren)"

${LangFileString} TEXT_WELCOME "Denne veiviseren installerer LyX på datamaskinen din.$\r$\n\
				$\r$\n\
				$_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Oppdaterer MiKTeX filenavn database..."
${LangFileString} TEXT_CONFIGURE_LYX "Konfigurerer LyX ($LaTeXInstalled kan laste ned manglende pakker, så dette kan ta litt tid) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Kompilerer Python script..."

${LangFileString} TEXT_FINISH_DESKTOP "Lager snarveg på skrivebordet"
${LangFileString} TEXT_FINISH_WEBSITE "Besøk lyx.org for de seneste nyhetene, hjelp og støtte"

${LangFileString} FileTypeTitle "LyX-dokument"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Installer for alle brukere?"
${LangFileString} SecFileAssocTitle "Fil-assosiasjoner"
${LangFileString} SecDesktopTitle "Skrivebordsikon"

${LangFileString} SecCoreDescription "LyX-filene."
${LangFileString} SecInstJabRefDescription "Bibliografi og referanse program for å kunne editere BibTeX filer."
${LangFileString} SecAllUsersDescription "Installer LyX for alle brukere, eller kun for denne brukeren."
${LangFileString} SecFileAssocDescription "Filer med endelsen .lyx åpnes automatisk i LyX."
${LangFileString} SecDesktopDescription "Et LyX-ikon på skrivebordet."
${LangFileString} SecDictionariesDescription "Ordbøker til rettskrivningsprogram som kan lastes ned og installeres."
${LangFileString} SecThesaurusDescription "Synonymordbøker som kan lastes ned og installeres."

${LangFileString} EnterLaTeXHeader1 'LaTeX-distribusjon'
${LangFileString} EnterLaTeXHeader2 'Velg LaTeX-distribusjonen LyX skal bruke.'
${LangFileString} EnterLaTeXFolder 'Dersom du vil kan du sette stien til fila $\"latex.exe$\" og slik sette\
					LaTeX-distribusjonen som LyX bruker.\r\n\
					Om du ikke bruker LaTeX får du ikke vist dokument!\r\n\
					\r\n\
					Installasjonsprosessen har funne  LaTeX-distribusjonen \
					$\"$LaTeXName$\" i systemet ditt. Stien står nedenfor.'
${LangFileString} EnterLaTeXFolderNone 'Dersom du vil kan du sette stien til fila $\"latex.exe$\" og slik sette\
					LaTeX-distribusjonen som LyX bruker.\r\n\
					Om du ikke bruker LaTeX får du ikke vist dokument!\r\n\
					\r\n\
					Installasjonsprosessen har ikke funne noen LaTeX-distribusjon.'
${LangFileString} PathName 'Stien til filen $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Ikke bruk LaTeX"
${LangFileString} InvalidLaTeXFolder 'Filen $\"latex.exe$\" fins ikke i den oppgitte mappa.'

${LangFileString} LatexInfo 'Nå starter installasjon av LaTeX-distribusjonen $\"MiKTeX$\"$\r$\n\
				For å installere programmet, klikk $\"Neste$\"-knappen i installasjonsveiviseren til installasjonen begynner.$\r$\n\
				$\r$\n\
				!!! Vennligst bruk standardopsjonene for MiKTeX-installasjonen !!!'
${LangFileString} LatexError1 'Fant ikke noen LaTeX-distribusjon!$\r$\n\
				LyX kan ikke brukes uten en LaTeX-distribusjon slik som $\"MiKTeX$\"!$\r$\n\
				Derfor avbrytes installasjonen.'

${LangFileString} HunspellFailed 'Nedlastingen av ordliste for språket $\"$R3$\" feilet.'
${LangFileString} ThesaurusFailed 'Nedlastingen av synonymordliste for språket $\"$R3$\" feilet.'

${LangFileString} JabRefInfo 'Nå vil installasjonen av programmet $\"JabRef$\" starte.$\r$\n\
				Du kan bruke alle de vanlige opsjonene i JabRef installasjonen.'
${LangFileString} JabRefError 'Installasjonen av $\"JabRef$\" feilet!$\r$\n\
				Installasjonen vil fortsette uansett.$\r$\n\
				Prøv å installere JabRef ved en senere anledning.'

${LangFileString} LatexConfigInfo "Konfigurasjon av LyX vil ta en stund."

${LangFileString} MiKTeXPathInfo "For at alle brukerne skal kunne endre innstillingene til MiKTeX til sitt bruk $\r$\n\
					må alle brukere ha skrivetilgang til MiKTeXs installasjon mapper$\r$\n\
					$MiKTeXPath $\r$\n\
					og undermapper."
${LangFileString} MiKTeXInfo 'LaTeX-distribusjonen $\"MiKTeX$\" vil bli brukt sammen med LyX.$\r$\n\
				Vi anbefaler at du oppdaterer MiKTeX med programmet $\"MiKTeX Update Wizard$\"$\r$\n\
				før du bruker LyX for første gang.$\r$\n\
				Vil du sjå etter MiKTeX oppdateringer nå?'

${LangFileString} ModifyingConfigureFailed "Forsøket på å stille inn 'path_prefix' i konfigurasjonsscriptet mislyktes"
${LangFileString} RunConfigureFailed "Fikk ikke kjørt konfigurasjonsscriptet"
${LangFileString} NotAdmin "Du trenger administratorrettigheter for å installere LyX!"
${LangFileString} InstallRunning "Installasjonsprogrammet er allerede i gang!"
${LangFileString} AlreadyInstalled "LyX ${APP_SERIES_KEY2} er allerede installert!$\r$\n\
				Installing over existing installations is not recommended if the installed version$\r$\n\
				is a test release or if you have problems with your existing LyX installation.$\r$\n\
				In these cases better reinstall LyX.$\r$\n\
				Dou you nevertheles want to install LyX over the existing version?"
${LangFileString} NewerInstalled "Du prøver å installere en eldre versjon av Lyx enn den du har installert fra før.$\r$\n\
				  Dersom du ønsker dette må du avinstallere LyX $OldVersionNumber først."

${LangFileString} MultipleIndexesNotAvailable "Støtte for flere indekser vil ikke være tilgjengelig fordi$\r$\n\
						MiKTeX er installert med administratorrettigheter og du installerer LyX uten."
${LangFileString} MetafileNotAvailable "Støtte i LyX for bilder i formatene EMF eller WMF vil ikke være$\r$\n\
					tilgjengelig fordi man da må installere programvareskriver i $\r$\n\
					Windows som bare er mulig med administratorrettigheter."

${LangFileString} FinishPageMessage "Gratulerer!! LyX er installert.$\r$\n\
					$\r$\n\
					(Første gangs oppstart av LyX kan ta noen sekunder.)"
${LangFileString} FinishPageRun "Start LyX"

${LangFileString} UnNotInRegistryLabel "Fant ikke LyX i registeret.$\r$\n\
					Snarveier på skrivebordet og i startmenyen fjernes ikke."
${LangFileString} UnInstallRunning "Du må avslutte LyX først!"
${LangFileString} UnNotAdminLabel "Du må ha administratorrettigheter for å fjerne LyX!"
${LangFileString} UnReallyRemoveLabel "Er du sikker på at du vil fjerne LyX og alle tilhørende komponenter?"
${LangFileString} UnLyXPreferencesTitle 'LyX sine bruker innstillinger'

${LangFileString} SecUnMiKTeXDescription "Avinstallerer LaTeX-distribusjonen MiKTeX."
${LangFileString} SecUnJabRefDescription "Avinstallerer bibliografiprogrammet JabRef."
${LangFileString} SecUnPreferencesDescription 'Sletter LyX sine konfigurasjonsmapper$\r$\n\
						$\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						for alle brukere.'
${LangFileString} SecUnProgramFilesDescription "Avinstallerer LyX og alle delkomponenter."

