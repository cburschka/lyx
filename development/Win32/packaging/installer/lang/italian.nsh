﻿/*

LyX Installer Language File
Language: Italian
Author: Enrico Forestieri

*/

!insertmacro LANGFILE_EXT "Italian"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installed for Current User)"

${LangFileString} TEXT_WELCOME "Verrete guidati nell'installazione di $(^NameDA)$\r$\n\
				$\r$\n\
				$_CLICK"

#${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Aggiornamento dei file del database MiKTeX in corso..."
${LangFileString} TEXT_CONFIGURE_LYX "Configurazione di LyX in corso ($LaTeXInstalled potrebbe installare i pacchetti mancanti, l'azione potrebbe richiedere qualche minuto) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compilazione degli script Python in corso..."

${LangFileString} TEXT_FINISH_DESKTOP "Crea icona sul desktop"
${LangFileString} TEXT_FINISH_WEBSITE "Visitate lyx.org per ultime novità, aiuto e suggerimenti"

#${LangFileString} FileTypeTitle "Documento di LyX"

#${LangFileString} SecInstJabRefTitle "JabRef"
#${LangFileString} SecAllUsersTitle "Installare per tutti gli utenti?"
${LangFileString} SecFileAssocTitle "Associazioni dei file"
${LangFileString} SecDesktopTitle "Icona sul Desktop"

${LangFileString} SecCoreDescription "I file di LyX."
#${LangFileString} SecInstJabRefDescription "Gestore dei riferimenti bibliografici ed editor per i files di BibTeX."
#${LangFileString} SecAllUsersDescription "Installazione LyX per tutti gli utenti o solo per l'utente attuale."
${LangFileString} SecFileAssocDescription "Associa i files con estensione .lyx al programma LyX."
${LangFileString} SecDesktopDescription "Icona LyX sul desktop."
${LangFileString} SecDictionaries "Dizionari"
${LangFileString} SecDictionariesDescription "Dizionari per il controllo ortografico che possono essere scaricati e installati."
${LangFileString} SecThesaurus "Thesaurus"
${LangFileString} SecThesaurusDescription "Dizionari dei sinonimi che possono essere scaricati e installati."

${LangFileString} EnterLaTeXHeader1 'Distribuzione di LaTeX'
${LangFileString} EnterLaTeXHeader2 'Scegliere la distribuzione di LaTeX che LyX dovrà usare.'
${LangFileString} EnterLaTeXFolder 'Puoi impostare qui il percorso del file $\"latex.exe$\" e impostare di conseguenza \
					la distribuzione di LaTeX che LyX dovrà usare.\r\n\
					Senza LaTeX, LyX non può generare documenti!\r\n\
					\r\n\
					Il programma di installazione ha rilevato la distribuzione di LaTeX \
					$\"$LaTeXName$\" sul tuo sistema. Il suo percorso è riportato sotto.'
${LangFileString} EnterLaTeXFolderNone 'Imposta qui sotto il percorso del file $\"latex.exe$\". Con questo imposti quale \
					distribuzione di LaTeX dovrà essere usata da LyX.\r\n\
					Senza LaTeX, LyX non può generare documenti!\r\n\
					\r\n\
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

${LangFileString} HunspellFailed 'Lo scaricamento del dizionario per la lingua  $\"$R3$\" non e$\' andato a buon fine.'
${LangFileString} ThesaurusFailed 'Lo scaricamento dei sinonimi per la lingua $\"$R3$\" non e$\' andato a buon fine.'

#${LangFileString} JabRefInfo 'Verrà adesso lanciato il programma di installazione di $\"JabRef$\".$\r$\n\
#				Puoi usare tutte le opzioni predefinite per il programma di installazione di JabRef.'
#${LangFileString} JabRefError 'Il programma $\"JabRef$\" non è stato installato correttamente!$\r$\n\
#				L$\'installazione può comunque proseguire.$\r$\n\
#				Si consiglia di tentare nuovamente l$\'installazione di JabRef al termine.'

#${LangFileString} LatexConfigInfo "La seguente configurazione di LyX richiederà un po' di tempo."

#${LangFileString} MiKTeXPathInfo "Per permettere a tutti gli utenti di personalizzare successivamente MiKTeX in base alle loro esigenze$\r$\n\
#					è necessario impostare i permessi di scrittura per tutti gli utenti alla cartella di installazione di MiKTeX$\r$\n\
#					$MiKTeXPath $\r$\n\
#					e alle sue sottocartelle."
${LangFileString} MiKTeXInfo 'La distribuzione di LaTeX $\"MiKTeX$\" verrà usata insieme a LyX.$\r$\n\
				Si raccomanda di installare gli aggiornamenti disponibili per MiKTeX utilizzando il programma $\"MiKTeX Update Wizard$\"$\r$\n\
				prima di usare LyX per la prima volta.$\r$\n\
				Vuoi controllare ora la presenza di aggiornamenti per MiKTeX?'

${LangFileString} ModifyingConfigureFailed "Fallito tentativo di aggiornare 'path_prefix' nello script di configurazione"
#${LangFileString} RunConfigureFailed "Fallito tentativo di eseguire lo script di configurazione"
${LangFileString} NotAdmin "Occorrono i privilegi da amministratore per installare LyX!"
${LangFileString} InstallRunning "Il programma di installazione è già in esecuzione!"
${LangFileString} AlreadyInstalled "LyX ${APP_SERIES_KEY2} è già installato!$\r$\n\
				Procedere con l'installazione su quella esistente non è raccomandabile se la versione version$\r$\n\
				è una release di test o se avete problemi con la vostra installazione corrente di Lyx.$\r$\n\
				In questi casi è preferibile installare nuovamente LyX.$\r$\n\
				Volete procedere comunque con l'installazione di LyX su quella esistente?"
${LangFileString} NewerInstalled "Si sta procedendo ad installare una versione di LyX precedente a quella in uso.$\r$\n\
				  Se si vuole procedere, è necessario prima disinstallare la versione LyX $OldVersionNumber."

${LangFileString} MultipleIndexesNotAvailable "Il supporto di più indici per documento non sarà disponibile perche'$\r$\n\
						MiKTeX è stato installato con privilegi di amministratore, ma si sta procedendo a installare LyX senza questi privilegi."

#${LangFileString} FinishPageMessage "Congratulazioni! LyX è stato installato con successo.$\r$\n\
#					$\r$\n\
#					(Il primo avvio di LyX potrebbe richiedere qualche secondo in più.)"
${LangFileString} FinishPageRun "Lancia LyX"

${LangFileString} UnNotInRegistryLabel "Non riesco a trovare LyX nel registro.$\r$\n\
					I collegamenti sul desktop e nel menu Start non saranno rimossi."
${LangFileString} UnInstallRunning "È necessario chiudere LyX!"
${LangFileString} UnNotAdminLabel "Occorrono i privilegi da amministratore per rimuovere LyX!"
${LangFileString} UnReallyRemoveLabel "Siete sicuri di voler rimuovere completamente LyX e tutti i suoi componenti?"
${LangFileString} UnLyXPreferencesTitle 'Impostazioni personali di LyX'

${LangFileString} SecUnMiKTeXDescription 'Rimuove la distribuzione di LaTeX $\"MiKTeX$\".'
#${LangFileString} SecUnJabRefDescription "Rimuove il gestore della bibliografia JabRef."
${LangFileString} SecUnPreferencesDescription 'Elimina la cartella con la configurazione di LyX$\r$\n\
						$\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						per tutti gli utenti.'
${LangFileString} SecUnProgramFilesDescription "Rimuove LyX e tutti i suoi componenti."
