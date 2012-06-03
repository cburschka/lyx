/*

LyX Installer Language File
Language: Italian
Author: Enrico Forestieri

*/

!insertmacro LANGFILE_EXT "Italian"

${LangFileString} TEXT_NO_PRIVILEDGES "Non si hanno privilegi da amministratore o utente avanzato.$\r$\nLe utilità per la conversione delle immagini non possono essere installate senza tali privilegi."

${LangFileString} TEXT_WELCOME "Verrete guidati nell'installazione di $(^NameDA)$\r$\n$\r$\n$_CLICK"

${LangFileString} TEXT_REINSTALL_TITLE "Reinstallazione"
${LangFileString} TEXT_REINSTALL_SUBTITLE "Scegliete se volete reinstallare i file di $(^NameDA)."
${LangFileString} TEXT_REINSTALL_INFO "$(^NameDA) è già installato. Questo installer vi consentirà di cambiare l'ubicazione di applicazioni esterne o di scaricare dalla rete dizionari aggiuntivi. Se volete anche reinstallare i file del programma, selezionate la casella sottostante. $(^ClickNext)"
${LangFileString} TEXT_REINSTALL_ENABLE "Reinstallazione dei file di $(^NameDA)"

${LangFileString} TEXT_EXTERNAL_NOPRIVILEDGES "(sono richiesti privilegi da amministratore o utente avanzato)"

${LangFileString} TEXT_EXTERNAL_LATEX_TITLE "Sistema di Composizione Tipografica LaTeX"
${LangFileString} TEXT_EXTERNAL_LATEX_SUBTITLE "È necessario il sistema di composizione tipografica LaTeX per potere creare documenti adatti per la stampa o la pubblicazione."
${LangFileString} TEXT_EXTERNAL_LATEX_INFO_DOWNLOAD "Specificate se volete scaricare dalla rete MiKTeX o volete usare un sistema LaTeX preesistente. $(^ClickNext)"
${LangFileString} TEXT_EXTERNAL_LATEX_INFO_INSTALL "Specificate se volete installare MiKTeX o volete usare un sistema LaTeX preesistente. $(^ClickNext)"
${LangFileString} TEXT_EXTERNAL_LATEX_DOWNLOAD "Scarica MiKTeX"
${LangFileString} TEXT_EXTERNAL_LATEX_INSTALL "Installa MiKTeX"
${LangFileString} TEXT_EXTERNAL_LATEX_FOLDER "Usa il sistema LaTeX preesistente nella seguente cartella:"
${LangFileString} TEXT_EXTERNAL_LATEX_FOLDER_INFO "Questa cartella deve contenere ${BIN_LATEX}."
${LangFileString} TEXT_EXTERNAL_LATEX_NONE "Installa senza un sistema di composizione tipografica LaTeX (sconsigliato)"
${LangFileString} TEXT_EXTERNAL_LATEX_NOTFOUND "${BIN_LATEX} non è presente nella cartella specificata."

${LangFileString} TEXT_EXTERNAL_IMAGEMAGICK_TITLE "ImageMagick"
${LangFileString} TEXT_EXTERNAL_IMAGEMAGICK_SUBTITLE "ImageMagick è necessario per il supporto di vari formati grafici."
${LangFileString} TEXT_EXTERNAL_IMAGEMAGICK_INFO_DOWNLOAD "Specificate se volete scaricare dalla rete ImageMagick o se è già installato. $(^ClickNext)"
${LangFileString} TEXT_EXTERNAL_IMAGEMAGICK_INFO_INSTALL "Specificate se volete installare ImageMagick o se è già installato. $(^ClickNext)"
${LangFileString} TEXT_EXTERNAL_IMAGEMAGICK_DOWNLOAD "Scarica ImageMagick"
${LangFileString} TEXT_EXTERNAL_IMAGEMAGICK_INSTALL "Installa ImageMagick"
${LangFileString} TEXT_EXTERNAL_IMAGEMAGICK_FOLDER "Usa l'installazione di ImageMagick preesistente nella seguente cartella:"
${LangFileString} TEXT_EXTERNAL_IMAGEMAGICK_FOLDER_INFO "Questa cartella deve contenere ${BIN_IMAGEMAGICK}."
${LangFileString} TEXT_EXTERNAL_IMAGEMAGICK_NONE "Installa senza ImageMagick (sconsigliato)"
${LangFileString} TEXT_EXTERNAL_IMAGEMAGICK_NOTFOUND "${BIN_IMAGEMAGICK} non è presente nella cartella specificata."

${LangFileString} TEXT_EXTERNAL_GHOSTSCRIPT_TITLE "Ghostscript"
${LangFileString} TEXT_EXTERNAL_GHOSTSCRIPT_SUBTITLE "Ghostscript è necessario per l'anteprima di immagini in formato PostScript."
${LangFileString} TEXT_EXTERNAL_GHOSTSCRIPT_INFO_DOWNLOAD "Specificate se volete scaricare dalla rete Ghostscript o se è già installato. $(^ClickNext)"
${LangFileString} TEXT_EXTERNAL_GHOSTSCRIPT_INFO_INSTALL "Specificate se volete installare Ghostscript o se è già installato. $(^ClickNext)"
${LangFileString} TEXT_EXTERNAL_GHOSTSCRIPT_DOWNLOAD "Scarica Ghostscript"
${LangFileString} TEXT_EXTERNAL_GHOSTSCRIPT_INSTALL "Installa Ghostscript"
${LangFileString} TEXT_EXTERNAL_GHOSTSCRIPT_FOLDER "Usa l'installazione di Ghostscript preesistente nella seguente cartella:"
${LangFileString} TEXT_EXTERNAL_GHOSTSCRIPT_FOLDER_INFO "Questa cartella deve contenere ${BIN_GHOSTSCRIPT}."
${LangFileString} TEXT_EXTERNAL_GHOSTSCRIPT_NONE "Installa senza GhostScript (sconsigliato)"
${LangFileString} TEXT_EXTERNAL_GHOSTSCRIPT_NOTFOUND "${BIN_GHOSTSCRIPT} non è presente nella cartella specificata."

${LangFileString} TEXT_VIEWER_TITLE "Visualizzatore di Documenti"
${LangFileString} TEXT_VIEWER_SUBTITLE "Un visualizzatore è necessario per mostrare sullo schermo i documenti compilati."
${LangFileString} TEXT_VIEWER_INFO_DOWNLOAD "Non è installato nessun visualizzatore in grado di mostrare file sia PDF che Postscript, ovvero i formati comuni per i documenti compilati. Selezionando la casella sottostante, verrà scaricato dalla rete il visualizzatore PDF/Postscript GSView. $(^ClickNext)"
${LangFileString} TEXT_VIEWER_INFO_INSTALL "Non è installato nessun visualizzatore in grado di mostrare file sia PDF che Postscript, ovvero i formati comuni per i documenti compilati. Selezionando la casella sottostante, verrà installato il visualizzatore PDF/Postscript GSView. $(^ClickNext)"
${LangFileString} TEXT_VIEWER_DOWNLOAD "Scarica GSView"
${LangFileString} TEXT_VIEWER_INSTALL "Installa GSView"

${LangFileString} TEXT_DICT_TITLE "Dizionari per il Correttore Ortografico"
${LangFileString} TEXT_DICT_SUBTITLE "Scegliete le lingue che volete usare per il correttore ortografico."
${LangFileString} TEXT_DICT_TOP "Per verificare l'ortografia dei documenti in una data lingua, è necessario scaricare dalla rete un dizionario di parole valide. Selezionate i dizionari da scaricare. $_CLICK"
${LangFileString} TEXT_DICT_LIST "Selezionate i dizionari da scaricare:"

${LangFileString} TEXT_LANGUAGE_TITLE "Scelta della Lingua"
${LangFileString} TEXT_LANGUAGE_SUBTITLE "Scegliete la lingua nella quale volete usare $(^NameDA)."
${LangFileString} TEXT_LANGUAGE_INFO "Scegliete la lingua da usare per l'interfaccia di ${APP_NAME}. $(^ClickNext)"

${LangFileString} TEXT_DOWNLOAD_FAILED_LATEX "Non sono riuscito a scaricare MiKTeX. Volete provare ancora?"
${LangFileString} TEXT_DOWNLOAD_FAILED_IMAGEMAGICK "Non sono riuscito a scaricare ImageMagick. Volete provare ancora?"
${LangFileString} TEXT_DOWNLOAD_FAILED_GHOSTSCRIPT "Non sono riuscito a scaricare Ghostscript. Volete provare ancora?"
${LangFileString} TEXT_DOWNLOAD_FAILED_VIEWER "Non sono riuscito a scaricare GSView. Volete provare ancora?"
${LangFileString} TEXT_DOWNLOAD_FAILED_DICT "Non sono riuscito a scaricare il dizionario ortografico $DictionaryLangName. Volete provare ancora?"

${LangFileString} TEXT_NOTINSTALLED_LATEX "L'installazione di MiKTeX non è stata completata. Volete rilanciare il programma di installazione di MiKTeX?"
${LangFileString} TEXT_NOTINSTALLED_IMAGEMAGICK "L'installazione di ImageMagick non è stata completata. Volete rilanciare il programma di installazione di ImageMagick?"
${LangFileString} TEXT_NOTINSTALLED_GHOSTSCRIPT "L'installazione di Ghostscript non è stata completata. Volete rilanciare il programma di installazione di Ghostscript?"
${LangFileString} TEXT_NOTINSTALLED_VIEWER "Non avete associato GSView ai file di tipo PDF/Postscript. Volete rilanciare il programma di installazione di GSView?"
${LangFileString} TEXT_NOTINSTALLED_DICT "L'installazione del dizionario ortografico $DictionaryLangName non è stata completata. Volete riprovarla?"

${LangFileString} TEXT_FINISH_DESKTOP "Crea icona sul desktop"
${LangFileString} TEXT_FINISH_WEBSITE "Visitate lyx.org per ultime novità, aiuto e suggerimenti"

${LangFileString} WelcomePageText "Questo programma installerà LyX nel vostro computer.$\r$\n\
                      $\r$\n\
                      Per installare LyX sono necessari i privilegi di amministratore.$\r$\n\
                      $\r$\n\
                      Si raccomanda di chiudere tutte le altre applicazioni prima di iniziare l'installazione. Questo permetterà al programma di installazione di aggiornare i file di sistema senza dover riavviare il computer.$\r$\n\
                      $\r$\n\
                      $_CLICK"

${LangFileString} FileTypeTitle "Documento di LyX"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Installare per tutti gli utenti?"
${LangFileString} SecFileAssocTitle "Associazioni dei file"
${LangFileString} SecDesktopTitle "Icona sul Desktop"

${LangFileString} SecCoreDescription "I file di LyX."
${LangFileString} SecInstJabRefDescription "Gestore dei riferimenti bibliografici ed editor per i files di BibTeX."
${LangFileString} SecAllUsersDescription "Installazione LyX per tutti gli utenti o solo per l'utente attuale."
${LangFileString} SecFileAssocDescription "Associa i files con estensione .lyx al programma LyX."
${LangFileString} SecDesktopDescription "Icona LyX sul desktop."

${LangFileString} LangSelectHeader "Scelta della lingua del menu di LyX"
${LangFileString} AvailableLang " Lingue disponibili "

${LangFileString} EnterLaTeXHeader1 'Distribuzione di LaTeX'
${LangFileString} EnterLaTeXHeader2 'Scegliere la distribuzione di LaTeX che LyX dovrà usare.'
${LangFileString} EnterLaTeXFolder 'Puoi impostare qui il percorso del file $\"latex.exe$\" e impostare di conseguenza \
			                   la distribuzione di LaTeX che LyX dovrà usare.$\r$\n\
					   Senza LaTeX, LyX non può generare documenti!$\r$\n\
					   $\r$\n\
					   Il programma di installazione ha rilevato la distribuzione di LaTeX \
					   $\"$LaTeXName$\" sul tuo sistema. Il suo percorso è riportato sotto.'
${LangFileString} EnterLaTeXFolderNone 'Imposta qui sotto il percorso del file $\"latex.exe$\". Con questo imposti quale \
					       distribuzione di LaTeX dovrà essere usata da LyX.$\r$\n\
					       Senza LaTeX, LyX non può generare documenti!$\r$\n\
					       $\r$\n\
					       Il programma di installazione non ha trovato una distribuzione di LaTeX sul tuo sistema.'
${LangFileString} PathName 'Percorso del file $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Non usare LaTeX"
${LangFileString} InvalidLaTeXFolder 'Il file $\"latex.exe$\" non è nel percorso indicato.'

${LangFileString} LatexInfo 'Verrà adesso lanciato il programma di installazione della distribuzione di LateX $\"MiKTeX$\".$\r$\n\
                        	    Per installare il programma premere $\"Next$\" nelle finestre di installazione fino all$\'avvio dell$\'installazione.$\r$\n\
				    $\r$\n\
				    !!! Lasciare le impostazioni di default suggerite dal programma di installazione di $\"MiKTeX$\" !!!'
${LangFileString} LatexError1 'Il programma non ha rilevato la presenza di alcuna distribuzione valida di LaTeX!$\r$\n\
                                      LyX non può funzionare senza una distribuzione LaTeX (ad es. $\"MiKTeX$\")!$\r$\n\
				      Il processo di installazione verrà quindi terminato.'

${LangFileString} HunspellFailed 'Download of dictionary for language $\"$R3$\" failed.'
${LangFileString} ThesaurusFailed 'Download of thesaurus for language $\"$R3$\" failed.'

${LangFileString} JabRefInfo 'Verrà adesso lanciato il programma di installazione di $\"JabRef$\".$\r$\n\
                     		     Puoi usare tutte le opzioni predefinite per il programma di installazione di JabRef.'
${LangFileString} JabRefError 'Il programma $\"JabRef$\" non è stato installato correttamente!$\r$\n\
     		               	      L$\'installazione può comunque proseguire.$\r$\n\
				      Si consiglia di tentare nuovamente l$\'installazione di JabRef al termine.'

${LangFileString} LatexConfigInfo "La seguente configurazione di LyX richiederà un po' di tempo."

${LangFileString} MiKTeXPathInfo "Per permettere a tutti gli utenti di personalizzare successivamente MiKTeX in base alle loro esigenze$\r$\n\
                     			 è necessario impostare i permessi di scrittura per tutti gli utenti alla cartella di installazione di MiKTeX$\r$\n\
					 $MiKTeXPath $\r$\n\
					 e alle sue sottocartelle."
${LangFileString} MiKTeXInfo 'La distribuzione di LaTeX $\"MiKTeX$\" verrà usata insieme a LyX.$\r$\n\
                     		     Si raccomanda di installare gli aggiornamenti disponibili per MiKTeX utilizzando il programma $\"MiKTeX Update Wizard$\"$\r$\n\
				     prima di usare LyX per la prima volta.$\r$\n\
				     Vuoi controllare ora la presenza di aggiornamenti per MiKTeX?'

${LangFileString} UpdateNotAllowed "This update package can only update ${PRODUCT_VERSION_OLD}!"
${LangFileString} ModifyingConfigureFailed "Fallito tentativo di aggiornare 'path_prefix' nello script di configurazione"
${LangFileString} CreateCmdFilesFailed "Non è possibile creare lyx.bat"
${LangFileString} RunConfigureFailed "Fallito tentativo di eseguire lo script di configurazione"
${LangFileString} NotAdmin "Occorrono i privilegi da amministratore per installare LyX!"
${LangFileString} InstallRunning "Il programma di installazione è già in esecuzione!"
${LangFileString} StillInstalled "LyX è già installato! Occorre rimuoverlo per poter procedere."

${LangFileString} FinishPageMessage "Congratulazioni! LyX è stato installato con successo.$\r$\n\
                        		    $\r$\n\
					    (Il primo avvio di LyX potrebbe richiedere qualche secondo in più.)"
${LangFileString} FinishPageRun "Lancia LyX"

${LangFileString} UnNotInRegistryLabel "Non riesco a trovare LyX nel registro.$\r$\n\
                           		       I collegamenti sul desktop e nel menu Start non saranno rimossi."
${LangFileString} UnInstallRunning "È necessario chiudere LyX!"
${LangFileString} UnNotAdminLabel "Occorrono i privilegi da amministratore per rimuovere LyX!"
${LangFileString} UnReallyRemoveLabel "Siete sicuri di voler rimuovere completamente LyX e tutti i suoi componenti?"
${LangFileString} UnLyXPreferencesTitle 'Impostazioni personali di LyX'

${LangFileString} SecUnMiKTeXDescription 'Rimuove la distribuzione di LaTeX $\"MiKTeX$\".'
${LangFileString} SecUnJabRefDescription "Rimuove il gestore della bibliografia JabRef."
${LangFileString} SecUnPreferencesDescription 'Elimina la cartella con la configurazione di LyX$\r$\n\
                      				      $\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						      per tutti gli utenti.'
${LangFileString} SecUnProgramFilesDescription "Rimuove LyX e tutti i suoi componenti."

