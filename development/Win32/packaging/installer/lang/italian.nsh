/*

LyX Installer Language File
Language: Italian
Author: Enrico Forestieri

*/

!insertmacro LanguageString TEXT_NO_PRIVILEDGES "Non si hanno privilegi da amministratore o utente avanzato.$\r$\n$\r$\nSebbene sia possibile installare ${APP_NAME} ed il sistema di composizione tipografica, le utilità per la conversione delle immagini e per l'anteprima non possono essere installate senza tali privilegi."

!insertmacro LanguageString TEXT_WELCOME_DOWNLOAD "Verrete guidati nell'installazione di $(^NameDA), il compilatore di documenti che incoraggia un approccio alla scrittura basato sulla struttura dei vostri testi, non sulla loro apparenza.\r\n\r\nQuesto installer preparerà un ambiente completo per usare ${APP_NAME} ed è in grado di scaricare automaticamente dalla rete i componenti mancanti.\r\n\r\n$(^ClickNext)"
!insertmacro LanguageString TEXT_WELCOME_INSTALL "Verrete guidati nell'installazione di $(^NameDA), il compilatore di documenti che incoraggia un approccio alla scrittura basato sulla struttura dei vostri testi, non sulla loro apparenza.\r\n\r\nQuesto installer preparerà un ambiente completo per usare ${APP_NAME} ed è in grado di installare automaticamente i componenti mancanti.\r\n\r\n$(^ClickNext)"

!insertmacro LanguageString TEXT_USER_TITLE "Scelta degli Utenti"
!insertmacro LanguageString TEXT_USER_SUBTITLE "Scegliete per quali utenti volete installare $(^NameDA)."
!insertmacro LanguageString TEXT_USER_INFO "Scegliete se volete installare $(^NameDA) solo per voi o per tutti gli utenti di questo sistema. $(^ClickNext)"
!insertmacro LanguageString TEXT_USER_CURRENT "Installazione personale"
!insertmacro LanguageString TEXT_USER_ALL "Installazione per tutti gli utenti"

!insertmacro LanguageString TEXT_REINSTALL_TITLE "Reinstallazione"
!insertmacro LanguageString TEXT_REINSTALL_SUBTITLE "Scegliete se volete reinstallare i file di $(^NameDA)."
!insertmacro LanguageString TEXT_REINSTALL_INFO "$(^NameDA) è già installato. Questo installer vi consentirà di cambiare la lingua dell'interfaccia, l'ubicazione di applicazioni esterne o di scaricare dalla rete dizionari aggiuntivi. Se volete anche reinstallare i file del programma, selezionate la casella sottostante. $(^ClickNext)"
!insertmacro LanguageString TEXT_REINSTALL_ENABLE "Reinstallazione dei file di $(^NameDA)"

!insertmacro LanguageString TEXT_EXTERNAL_NOPRIVILEDGES "(sono richiesti privilegi da amministratore o utente avanzato)"

!insertmacro LanguageString TEXT_EXTERNAL_LATEX_TITLE "Sistema di Composizione Tipografica LaTeX"
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_SUBTITLE "È necessario il sistema di composizione tipografica LaTeX per potere creare documenti adatti per la stampa o la pubblicazione."
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_INFO_DOWNLOAD "Specificate se volete scaricare dalla rete MiKTeX o volete usare un sistema LaTeX preesistente. $(^ClickNext)"
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_INFO_INSTALL "Specificate se volete installare MiKTeX o volete usare un sistema LaTeX preesistente. $(^ClickNext)"
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_DOWNLOAD "Scarica MiKTeX"
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_INSTALL "Installa MiKTeX"
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_FOLDER "Usa il sistema LaTeX preesistente nella seguente cartella:"
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_FOLDER_INFO "Questa cartella deve contenere ${BIN_LATEX}."
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_NONE "Installa senza un sistema di composizione tipografica LaTeX (sconsigliato)"
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_NOTFOUND "${BIN_LATEX} non è presente nella cartella specificata."

!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_TITLE "ImageMagick"
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_SUBTITLE "ImageMagick è necessario per il supporto di vari formati grafici."
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_INFO_DOWNLOAD "Specificate se volete scaricare dalla rete ImageMagick o se è già installato. $(^ClickNext)"
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_INFO_INSTALL "Specificate se volete installare ImageMagick o se è già installato. $(^ClickNext)"
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_DOWNLOAD "Scarica ImageMagick"
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_INSTALL "Installa ImageMagick"
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_FOLDER "Usa l'installazione di ImageMagick preesistente nella seguente cartella:"
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_FOLDER_INFO "Questa cartella deve contenere ${BIN_IMAGEMAGICK}."
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_NONE "Installa senza ImageMagick (sconsigliato)"
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_NOTFOUND "${BIN_IMAGEMAGICK} non è presente nella cartella specificata."

!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_TITLE "Ghostscript"
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_SUBTITLE "Ghostscript è necessario per l'anteprima di immagini in formato PostScript."
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_INFO_DOWNLOAD "Specificate se volete scaricare dalla rete Ghostscript o se è già installato. $(^ClickNext)"
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_INFO_INSTALL "Specificate se volete installare Ghostscript o se è già installato. $(^ClickNext)"
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_DOWNLOAD "Scarica Ghostscript"
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_INSTALL "Installa Ghostscript"
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_FOLDER "Usa l'installazione di Ghostscript preesistente nella seguente cartella:"
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_FOLDER_INFO "Questa cartella deve contenere ${BIN_GHOSTSCRIPT}."
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_NONE "Installa senza GhostScript (sconsigliato)"
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_NOTFOUND "${BIN_GHOSTSCRIPT} non è presente nella cartella specificata."

!insertmacro LanguageString TEXT_VIEWER_TITLE "Visualizzatore di Documenti"
!insertmacro LanguageString TEXT_VIEWER_SUBTITLE "Un visualizzatore è necessario per mostrare sullo schermo i documenti compilati."
!insertmacro LanguageString TEXT_VIEWER_INFO_DOWNLOAD "Non è installato nessun visualizzatore in grado di mostrare file sia PDF che Postscript, ovvero i formati comuni per i documenti compilati. Selezionando la casella sottostante, verrà scaricato dalla rete il visualizzatore PDF/Postscript GSView. $_CLICK"
!insertmacro LanguageString TEXT_VIEWER_INFO_INSTALL "Non è installato nessun visualizzatore in grado di mostrare file sia PDF che Postscript, ovvero i formati comuni per i documenti compilati. Selezionando la casella sottostante, verrà installato il visualizzatore PDF/Postscript GSView. $_CLICK"
!insertmacro LanguageString TEXT_VIEWER_DOWNLOAD "Scarica GSView"
!insertmacro LanguageString TEXT_VIEWER_INSTALL "Installa GSView"

!insertmacro LanguageString TEXT_DICT_TITLE "Dizionari per il Correttore Ortografico"
!insertmacro LanguageString TEXT_DICT_SUBTITLE "Scegliete le lingue che volete usare per il correttore ortografico."
!insertmacro LanguageString TEXT_DICT_TOP "Per verificare l'ortografia dei documenti in una data lingua, è necessario scaricare dalla rete un dizionario di parole valide. Selezionate i dizionari da scaricare. $_CLICK"
!insertmacro LanguageString TEXT_DICT_LIST "Selezionate i dizionari da scaricare:"

!insertmacro LanguageString TEXT_LANGUAGE_TITLE "Scelta della Lingua"
!insertmacro LanguageString TEXT_LANGUAGE_SUBTITLE "Scegliete la lingua nella quale volete usare $(^NameDA)."
!insertmacro LanguageString TEXT_LANGUAGE_INFO "Scegliete la lingua da usare per l'interfaccia di ${APP_NAME}. $(^ClickNext)"

!insertmacro LanguageString TEXT_DOWNLOAD_FAILED_LATEX "Non sono riuscito a scaricare MiKTeX. Volete provare ancora?"
!insertmacro LanguageString TEXT_DOWNLOAD_FAILED_IMAGEMAGICK "Non sono riuscito a scaricare ImageMagick. Volete provare ancora?"
!insertmacro LanguageString TEXT_DOWNLOAD_FAILED_GHOSTSCRIPT "Non sono riuscito a scaricare Ghostscript. Volete provare ancora?"
!insertmacro LanguageString TEXT_DOWNLOAD_FAILED_VIEWER "Non sono riuscito a scaricare GSView. Volete provare ancora?"
!insertmacro LanguageString TEXT_DOWNLOAD_FAILED_DICT "Non sono riuscito a scaricare il dizionario ortografico ${DICT_NAME}. Volete provare ancora?"

!insertmacro LanguageString TEXT_NOTINSTALLED_LATEX "L'installazione di MiKTeX non è stata completata. Volete rilanciare il programma di installazione di MiKTeX?"
!insertmacro LanguageString TEXT_NOTINSTALLED_IMAGEMAGICK "L'installazione di ImageMagick non è stata completata. Volete rilanciare il programma di installazione di ImageMagick?"
!insertmacro LanguageString TEXT_NOTINSTALLED_GHOSTSCRIPT "L'installazione di Ghostscript non è stata completata. Volete rilanciare il programma di installazione di Ghostscript?"
!insertmacro LanguageString TEXT_NOTINSTALLED_VIEWER "Non avete associato GSView ai file di tipo PDF/Postscript. Volete rilanciare il programma di installazione di GSView?"
!insertmacro LanguageString TEXT_NOTINSTALLED_DICT "L'installazione del dizionario ortografico ${DICT_NAME} non è stata completata. Volete riprovarla?"

!insertmacro LanguageString TEXT_FINISH_DESKTOP "Crea icona sul desktop"
!insertmacro LanguageString TEXT_FINISH_WEBSITE "Visitate lyx.org per ultime novità, aiuto e suggerimenti"

!insertmacro LanguageString UNTEXT_WELCOME "Verrete guidati nella disinstallazione di $(^NameDA). Per favore, chiudete $(^NameDA) prima di continuare.\r\n\r\nNotate che verrà disinstallato solamente LyX. Se volete disinstallare le altre applicazioni installate insieme a LyX, dovrete farlo mediante la voce corrispondente in 'Installazione applicazioni'.\r\n\r\n$_CLICK"
