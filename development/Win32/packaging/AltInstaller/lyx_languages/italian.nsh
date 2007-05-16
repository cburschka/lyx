!ifndef _LYX_LANGUAGES_ITALIAN_NSH_
!define _LYX_LANGUAGES_ITALIAN_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_ITALIAN}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "Questo programma installerà LyX nel vostro computer.\r\n\
                      \r\n\
                      Per installare LyX sono necessari i privilegi di amministratore.\r\n\
                      \r\n\
                      Si raccomanda di chiudere tutte le altre applicazioni prima di iniziare l'installazione. Questo permetterà al programma di installazione di aggiornare i file di sistema senza dover riavviare il computer.\r\n\
                      \r\n\
                      $_CLICK"

LangString FileTypeTitle "${LYX_LANG}" "Documento di LyX"

LangString SecInstGSviewTitle "${LYX_LANG}" "GSview"
LangString SecInstJabRefTitle "${LYX_LANG}" "JabRef"
LangString SecAllUsersTitle "${LYX_LANG}" "Installare per tutti gli utenti?"
LangString SecFileAssocTitle "${LYX_LANG}" "Associazioni dei file"
LangString SecDesktopTitle "${LYX_LANG}" "Icona sul Desktop"

LangString SecCoreDescription "${LYX_LANG}" "I file di LyX."
LangString SecInstGSviewDescription "${LYX_LANG}" "Programma per visualizzare documenti in formato Postscript e PDF."
LangString SecInstJabRefDescription "${LYX_LANG}" "Gestore dei riferimenti bibliografici ed editor per i files di BibTeX."
LangString SecAllUsersDescription "${LYX_LANG}" "Installazione LyX per tutti gli utenti o solo per l'utente attuale."
LangString SecFileAssocDescription "${LYX_LANG}" "Associa i files con estensione .lyx al programma LyX."
LangString SecDesktopDescription "${LYX_LANG}" "Icona LyX sul desktop."

LangString LangSelectHeader "${LYX_LANG}" "Scelta della lingua del menu di LyX"
LangString AvailableLang "${LYX_LANG}" " Lingue disponibili "

LangString MissProgHeader "${LYX_LANG}" "Controllo dei programmi da installare"
LangString MissProgCap "${LYX_LANG}" "I seguenti programmi aggiuntivi sono necessari e verranno installati"
LangString MissProgLatex "${LYX_LANG}" "MiKTeX, una distribuzione di LaTeX"
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript, un interprete per documenti PostScript PDF"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick, un convertitore di immagini"
LangString MissProgAspell "${LYX_LANG}" "Aspell, un correttore ortografico"
LangString MissProgMessage "${LYX_LANG}" 'Nessun programma aggiuntivo deve essere installato.'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'Distribuzione di LaTeX'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'Scegliere la distribuzione di LaTeX che LyX dovrà usare.'
LangString EnterLaTeXFolder "${LYX_LANG}" 'Puoi impostare qui il percorso del file "latex.exe" e impostare di conseguenza \
			                   la distribuzione di LaTeX che LyX dovrà usare.\r\n\
					   Senza LaTeX, LyX non può generare documenti!\r\n\
					   \r\n\
					   Il programma di installazione ha rilevato la distribuzione di LaTeX \
					   "$LaTeXName" sul tuo sistema. Il suo percorso è riportato sotto.'
LangString EnterLaTeXFolderNone "${LYX_LANG}" 'Imposta qui sotto il percorso del file "latex.exe". Con questo imposti quale \
					       distribuzione di LaTeX dovrà essere usata da LyX.\r\n\
					       Senza LaTeX, LyX non può generare documenti!\r\n\
					       \r\n\
					       Il programma di installazione non ha trovato una distribuzione di LaTeX sul tuo sistema.'
LangString PathName "${LYX_LANG}" 'Percorso del file "latex.exe"'
LangString DontUseLaTeX "${LYX_LANG}" "Non usare LaTeX"
LangString InvalidLaTeXFolder "${LYX_LANG}" 'Il file "latex.exe" non è nel percorso indicato.'

LangString LatexInfo "${LYX_LANG}" 'Verrà adesso lanciato il programma di installazione della distribuzione di LateX "MiKTeX".$\r$\n\
                        	    Per installare il programma premere "Next" nelle finestre di installazione fino all$\'avvio dell$\'installazione.$\r$\n\
				    $\r$\n\
				    !!! Lasciare le impostazioni di default suggerite dal programma di installazione di "MiKTeX" !!!'
LangString LatexError1 "${LYX_LANG}" 'Il programma non ha rilevato la presenza di alcuna distribuzione valida di LaTeX!$\r$\n\
                                      LyX non può funzionare senza una distribuzione LaTeX (ad es. "MiKTeX")!$\r$\n\
				      Il processo di installazione verrà quindi terminato.'
                
LangString GSviewInfo "${LYX_LANG}" 'Verrà adesso lanciato il programma di installazione di "GSview".$\r$\n\
                     		     Per proseguire premere il pulsante "Setup" che appare nella prima finestra di installazione,$\r$\n\
				     scegliere la lingua e premere il pulsante "Next" nella finestra di installazione successiva.$\r$\n\
				     Per comodità si consiglia di accettare le impostazioni di default suggerite dal programma di installazione di GSview.'
LangString GSviewError "${LYX_LANG}" 'Il programma "GSview" non è stato installato correttamente!$\r$\n\
                      		      L$\'installazione può comunque proseguire.$\r$\n\
				      Si consiglia di tentare nuovamente l$\'installazione di GSView al termine.'

LangString JabRefInfo "${LYX_LANG}" 'Verrà adesso lanciato il programma di installazione di "JabRef".$\r$\n\
                     		     Puoi usare tutte le opzioni predefinite per il programma di installazione di JabRef.'
LangString JabRefError "${LYX_LANG}" 'Il programma "JabRef" non è stato installato correttamente!$\r$\n\
     		               	      L$\'installazione può comunque proseguire.$\r$\n\
				      Si consiglia di tentare nuovamente l$\'installazione di JabRef al termine.'

LangString LatexConfigInfo "${LYX_LANG}" "La seguente configurazione di LyX richiederà un po' di tempo."

LangString AspellInfo "${LYX_LANG}" 'Adesso verranno scaricati ed installati i dizionari per il correttore ortografico "Aspell".$\r$\n\
                     		     Ogni dizionario ha la propria licenza che verrà mostrata prima dell$\'installazione.'
LangString AspellDownloadFailed "${LYX_LANG}" "Impossibile scaricare i dizionari di Aspell!"
LangString AspellInstallFailed "${LYX_LANG}" "Impossibile installare i dizionari di Aspell!"
LangString AspellPartAnd "${LYX_LANG}" " e "
LangString AspellPartSep "${LYX_LANG}" ", "
LangString AspellAll "${LYX_LANG}" 'Un dizionario è già installato per la lingua'
LangString AspellNone "${LYX_LANG}" 'Nessun dizionario per il correttore ortografico "Aspell" è stato installato.$\r$\n\
                     		     I dizionari possono essere installati da$\r$\n\
				     ${AspellLocation}$\r$\n\
				     Vuoi scaricare i dizionari ora?'
LangString AspellPartStart "${LYX_LANG}" "È stato installato con successo "
LangString AspellPart1 "${LYX_LANG}" "un dizionario inglese"
LangString AspellPart2 "${LYX_LANG}" "un dizionario per la lingua $LangName"
LangString AspellPart3 "${LYX_LANG}" "un$\r$\n\
                      		      dizionario per la lingua $LangNameSys"
LangString AspellPart4 "${LYX_LANG}" ' per il correttore ortografico "Aspell".$\r$\n\
                      		      Ulteriori dizionari possono essere installati da$\r$\n\
				      ${AspellLocation}$\r$\n\
				      Vuoi scaricare altri dizionari ora?'

LangString MiKTeXPathInfo "${LYX_LANG}" "Per permettere a tutti gli utenti di personalizzare successivamente MiKTeX in base alle loro esigenze$\r$\n\
                     			 è necessario impostare i permessi di scrittura per tutti gli utenti alla cartella di installazione di MiKTeX$\r$\n\
					 $MiKTeXPath $\r$\n\
					 e alle sue sottocartelle."
LangString MiKTeXInfo "${LYX_LANG}" 'La distribuzione di LaTeX "MiKTeX" verrà usata insieme a LyX.$\r$\n\
                     		     Si raccomanda di installare gli aggiornamenti disponibili per MiKTeX utilizzando il programma "MiKTeX Update Wizard"$\r$\n\
				     prima di usare LyX per la prima volta.$\r$\n\
				     Vuoi controllare ora la presenza di aggiornamenti per MiKTeX?'

LangString UpdateNotAllowed "${LYX_LANG}" "This update package can only update ${PRODUCT_VERSION_OLD}!"
LangString ModifyingConfigureFailed "${LYX_LANG}" "Fallito tentativo di aggiornare 'path_prefix' nello script di configurazione"
LangString CreateCmdFilesFailed "${LYX_LANG}" "Non è possibile creare lyx.bat"
LangString RunConfigureFailed "${LYX_LANG}" "Fallito tentativo di eseguire lo script di configurazione"
LangString NotAdmin "${LYX_LANG}" "Occorrono i privilegi da amministratore per installare LyX!"
LangString InstallRunning "${LYX_LANG}" "Il programma di installazione è già in esecuzione!"
LangString StillInstalled "${LYX_LANG}" "LyX è già installato! Occorre rimuoverlo per poter procedere."

LangString FinishPageMessage "${LYX_LANG}" "Congratulazioni! LyX è stato installato con successo.\r\n\
                        		    \r\n\
					    (Il primo avvio di LyX potrebbe richiedere qualche secondo in più.)"
LangString FinishPageRun "${LYX_LANG}" "Lancia LyX"

LangString UnNotInRegistryLabel "${LYX_LANG}" "Non riesco a trovare LyX nel registro.$\r$\n\
                           		       I collegamenti sul desktop e nel menu Start non saranno rimossi."
LangString UnInstallRunning "${LYX_LANG}" "È necessario chiudere LyX!"
LangString UnNotAdminLabel "${LYX_LANG}" "Occorrono i privilegi da amministratore per rimuovere LyX!"
LangString UnReallyRemoveLabel "${LYX_LANG}" "Siete sicuri di voler rimuovere completamente LyX e tutti i suoi componenti?"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'Impostazioni personali di LyX'
LangString UnGSview "${LYX_LANG}" 'Per rimuovere "GSview" (un visualizzatore di file Postscript e PDF) è$\r$\n\ 
                                   necessario premere il pulsante "Uninstall" che apparirà nella finestra successiva.'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "LyX è stato rimosso dal sistema con successo."

LangString SecUnAspellDescription "${LYX_LANG}" 'Rimuove il correttore ortografico "Aspell" e tutti i suoi dizionari.'
LangString SecUnMiKTeXDescription "${LYX_LANG}" 'Rimuove la distribuzione di LaTeX "MiKTeX".'
LangString SecUnJabRefDescription "${LYX_LANG}" "Rimuove il gestore della bibliografia JabRef."
LangString SecUnPreferencesDescription "${LYX_LANG}" 'Elimina la cartella con la configurazione di LyX$\r$\n\
                      				      "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
						      per tutti gli utenti.'
LangString SecUnProgramFilesDescription "${LYX_LANG}" "Rimuove LyX e tutti i suoi componenti."

!undef LYX_LANG

!endif # _LYX_LANGUAGES_ITALIAN_NSH_
