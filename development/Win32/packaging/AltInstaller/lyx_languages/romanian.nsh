!ifndef _LYX_LANGUAGES_ROMANIAN_NSH_
!define _LYX_LANGUAGES_ROMANIAN_NSH_

!ifdef LYX_LANG
 !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_ROMANIAN}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "Acest asistent vã va ghida în procesul de instalare a programului LyX. \r\n\
                                         \r\n\
                                         Trebuie sã aveþi drepturi de administrator ca sã puteþi instala LyX. \r\n\
                                         \r\n\
                                         Este recomandat ca sã închideþi celelalte aplicaþii înainte de începerea instalãrii. Acest lucru va permite upgradarea fiºierelor de sistem relevante fãrã sã fiþi nevoiþi sã reporniþi calculatorul.  \r\n\
                                         \r\n\
                                         $_CLICK"

LangString FileTypeTitle "${LYX_LANG}" "Document LyX"

LangString SecInstGSviewTitle "${LYX_LANG}" "GSview"
LangString SecInstJabRefTitle "${LYX_LANG}" "JabRef"
LangString SecAllUsersTitle "${LYX_LANG}" "Doriþi sã instalaþi pentru toþi utilizatorii?"
LangString SecFileAssocTitle "${LYX_LANG}" "Asocierea fiºierelor"
LangString SecDesktopTitle "${LYX_LANG}" "Iconiþã pe desktop"

LangString SecCoreDescription "${LYX_LANG}" "Fiºierele LyX."
LangString SecInstGSviewDescription "${LYX_LANG}" "Program  de vizualizat documente tip Postscript ºi PDF."
LangString SecInstJabRefDescription "${LYX_LANG}" "Gestionarul de Bibliografie ºi editorul de fiºiere tip BibTeX."
LangString SecAllUsersDescription "${LYX_LANG}" "Instaleazã LyX pentru toþi utilizatorii sau doar pentru utilizatorul curent."
LangString SecFileAssocDescription "${LYX_LANG}" "Fiºierele cu extensia .lyx vor fi deschise automat cu LyX."
LangString SecDesktopDescription "${LYX_LANG}" "A iconiþã LyX pe desktop."

LangString LangSelectHeader "${LYX_LANG}" "Selecþia de limbaj pentru meniurile LyX."
LangString AvailableLang "${LYX_LANG}" "Limbajele disponibile."

LangString MissProgHeader "${LYX_LANG}" "Verificã pentru programele necesare."
LangString MissProgCap "${LYX_LANG}" "Urmãtoarele programe necesare vor fi instalate adiþional."
LangString MissProgLatex "${LYX_LANG}" "MiKTeX, o distribuþie LaTeX"
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript, un vizualizator de documente tip PostScript ºi PDF"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick, un convertor de imagini"
LangString MissProgAspell "${LYX_LANG}" "Aspell, un corector ortografic"
LangString MissProgMessage "${LYX_LANG}" 'Toate programele necesare sunt deja instalate.'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'Distribuþia LaTeX'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'Seteazã distribuþia LaTeX pe care sã o foloseascã LyX.'
LangString EnterLaTeXFolder "${LYX_LANG}" 'Opþional puteþi seta calea cãtre fiºierul "latex.exe" ºi, implicit, seta distribuþia \ LaTeX pe care sã o foloseascã LyX. \r\n\
                                          Dacã nu folosiþi LaTeX, LyX nu va putea sã producã fiºiere!\r\n\
                                          \r\n\
                                          Programul de instalare a detectat distribuþia LaTeX \
                                          "$LaTeXName" instalatã pe calculatorul dumneavoastrã. Dedesupt este afiºatã calea acestei distribuþii.'
LangString EnterLaTeXFolderNone "${LYX_LANG}" 'Setaþi calea cãtre fiºierul "latex.exe" ºi, implicit, seta distribuþia \ LaTeX pe care sã o foloseascã LyX. \r\n\
                                          Dacã nu folosiþi LaTeX, LyX nu va putea sã producã fiºiere!\r\n\
                                          \r\n\
                                          Programul de instalare nu a detectat nici o distribuþia LaTeX instalatã pe calculatorul dumneavoastrã.'

LangString PathName "${LYX_LANG}" 'Calea cãtre fiºierul "latex.exe"'
LangString DontUseLaTeX "${LYX_LANG}" "Nu folosi LaTeX"
LangString InvalidLaTeXFolder "${LYX_LANG}" 'Fiºierul "latex.exe" nu se aflã în calea specificatã.'

LangString LatexInfo "${LYX_LANG}" 'Programul de instalare al distribuþiei LaTeX  "MiKTeX" va fi lansat. $\r$\n\
                                   Pentru instalare apãsaþi butonul "Next" în fereastra prgramului de instalare pînã cînd instalarea înncepe. $\r$\n\
                                   $\r$\n\
                                   !!! Vã rugãm sã folosiþi setãrile implicite ale programului de instalat MiKTex !!!'
LangString LatexError1 "${LYX_LANG}" 'Nu am detectat nici o distribuþia LaTeX instalatã pe calculatorul dumneavoastrã!$\r$\n\
                                     LyX nu poate fi folosit fãrã o distribuþia LaTeX ca  "MiKTeX"!$\r$\n\
                                     Instalarea va fi abandonatã.'

LangString GSviewInfo "${LYX_LANG}" 'Acum va fi lansat programul de instalare al aplicaþiei "GSview".$\r$\n\
                                    Pentru instalare apãsaþi butonul "Setup" în fereastra prgramului de instalare,$\r$\n\
                                    alegeþi un limbaj ºi apãsaþi butonul "Next" în urmatoarea fereastrã$\r$\n\
                                    Este indicat sã folosiþi setãrile implicite ale programului de instalat GSview.'
LangString GSviewError "${LYX_LANG}" 'Programul "GSview" nu a putut fi instalat!$\r$\n\
                                     Programul de instalare LyX va continua totuºi.$\r$\n\
                                     Încercaþi sã instalaþi GSview mai tîrziu.'

LangString JabRefInfo "${LYX_LANG}" 'Acum va fi lansat programul de instalare al aplicaþiei "JabRef".$\r$\n\
                                     Este indicat sã folosiþi setãrile implicite ale programului de instalat  JabRef.'
LangString JabRefError "${LYX_LANG}" 'Programul "JabRef" nu a putut fi instalat!$\r$\n\
                                     Programul de instalare LyX va continua totuºi.$\r$\n\
                                     Încercaþi sã instalaþi JabRef mai tîrziu.'

LangString LatexConfigInfo "${LYX_LANG}" "Configurarea programului LyX va dura o perioadã de timp."

LangString AspellInfo "${LYX_LANG}" 'Acum vor fi descãrcate ºi instalate dicþionarele corectorului ortografic  "Aspell".$\r$\n\
                                    Fiecare dicþionar are o licenþã diferitã ce va fi afiºatã înaintea instalãrii.'
LangString AspellDownloadFailed "${LYX_LANG}" 'Nu au putut fi descãrcate dicþionarele corectorului ortografic   "Aspell"!'
LangString AspellInstallFailed "${LYX_LANG}" 'Nu au putut fi instalate dicþionarele corectorului ortografic  "Aspell"!'
LangString AspellPartAnd "${LYX_LANG}" " ºi "
LangString AspellPartSep "${LYX_LANG}" ", "
LangString AspellAll "${LYX_LANG}" 'Existã deja un dicþionar instalat.'
LangString AspellNone "${LYX_LANG}" 'Nu au putut fi instalate dicþionarele corectorului ortografic  "Aspell".$\r$\n\
                                    Dicþionarele pot fi descãrcate de la$\r$\n\
                                    ${AspellLocation}$\r$\n\
                                    Doriþi sã descãrcaþi dicþionarele acum?'
LangString AspellPartStart "${LYX_LANG}" "A fost instalat cu succes "
LangString AspellPart1 "${LYX_LANG}" "Un dicþionar englezesc"
LangString AspellPart2 "${LYX_LANG}" "Un dicþionar pentru limba $LangName"
LangString AspellPart3 "${LYX_LANG}" "a$\r$\n\
                                       dicþionarul pentru limba $LangNameSys"
LangString AspellPart4 "${LYX_LANG}" 'pentru corectorul ortografic "Aspell".$\r$\n\
                                     Mai multe dicþionare pot fi descãrcate de la$\r$\n\
                                     ${AspellLocation}$\r$\n\
                                     Doriþi sã descãrcaþi dicþionarele acum?'

LangString MiKTeXPathInfo "${LYX_LANG}" "Pentru ca utilizatorii sã poatã modifica setãrile MiKTeX$\r$\n\
                                        este necesar sã setaþi dreptul de scriere la directorul$\r$\n\
                                        $MiKTeXPath $\r$\n\
                                         unde a fost instalat MiKTex ºi toate subdirectoarele pentru toþi utilizatorii."
LangString MiKTeXInfo "${LYX_LANG}" 'Distribuþia LaTeX "MiKTeX" va fi folositã cu LyX.$\r$\n\
                                    Este recomandat sã instalaþi actualizãri ale programului MiKTex folosind programul "MiKTeX Update Wizard"$\r$\n\
                                    înainte de a folosi LyX pentru prima oarã.$\r$\n\
                                    Doriþi sã verificaþi dacã sunt disponibile actualizãri pentru MiKTeX?'

LangString UpdateNotAllowed "${LYX_LANG}" "Acest program de actualizare poate actualiza numai ${PRODUCT_VERSION_OLD}!"
LangString ModifyingConfigureFailed "${LYX_LANG}" "Nu am putut seta 'path_prefix' în procesul de configurare"
LangString CreateCmdFilesFailed "${LYX_LANG}" "Nu am putut crea lyx.bat"
LangString RunConfigureFailed "${LYX_LANG}" "Nu am putut executa scriptul de configurare"
LangString NotAdmin "${LYX_LANG}" "Trebuie sã aveþi drepturi de administrator pentru instalarea programului LyX!"
LangString InstallRunning "${LYX_LANG}" "Programul de instalare este deja pornit!"
LangString StillInstalled "${LYX_LANG}" "LyX este deja instalat! Dezinstalaþi LyX prima oarã."

LangString FinishPageMessage "${LYX_LANG}" "Felicitãri! LyX a fost instalat cu succes.\r\n\
                                           \r\n\
                                           (Prima oarã cînd porniþi LyX s-ar putea sã dureze cîteva secunde.)"
LangString FinishPageRun "${LYX_LANG}" "Lanseazã LyX"

LangString UnNotInRegistryLabel "${LYX_LANG}" "Nu am gãsit LyX în registri.$\r$\n\
                                              Scurtãturile de pe desktop ºi Start Menu nu vor fi ºterse."
LangString UnInstallRunning "${LYX_LANG}" "Trebuie sã inchideþi LyX prima oarã!"
LangString UnNotAdminLabel "${LYX_LANG}" "Trebuie sã aveþi drepturi de administrator pentru dezinstalarea programului LyX!"
LangString UnReallyRemoveLabel "${LYX_LANG}" "Sunteþi sigur cã doriþi sã dezinstalaþi programul LyX ºi toate componentele lui?"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'Preferinþele utilizatorului pentru LyX'
LangString UnGSview "${LYX_LANG}" 'Apãsaþi butonul "Uninstall" în urmatoarea fereastrã pentru a dezinstala  $\r$\n\
                                  vizualizatorul de documente tip Postscript ºi PDF  "GSview".'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "LyX a fost dezinstalat cu succes de pe calculatorul dumneavoastrã."

LangString SecUnAspellDescription "${LYX_LANG}" 'Dezinstaleazã  corectorul ortografic "Aspell" ºi toate componentele lui.'
LangString SecUnMiKTeXDescription "${LYX_LANG}" 'Dezinstaleazã Distribuþia LaTeX "MiKTeX".'
LangString SecUnJabRefDescription "${LYX_LANG}" "Dezinstaleazã gestionarul de bibliografie JabRef."
LangString SecUnPreferencesDescription "${LYX_LANG}" 'ªterge directorul cu setãrile LyX$\r$\n\
					  "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
                                          pentru toþi utilizatorii.'
LangString SecUnProgramFilesDescription "${LYX_LANG}" " Dezinstalaþi programul LyX ºi toate componentele lui."

!undef LYX_LANG

!endif # _LYX_LANGUAGES_ROMANIAN_NSH_

