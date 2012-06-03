!insertmacro LANGFILE_EXT "Danish"

${LangFileString} WelcomePageText "Denne guide vil installere LyX på din computer.$\r$\n\
                                        $\r$\n\
                                        Du skal have administratorrettigheder for at installerer LyX.$\r$\n\
                                        $\r$\n\
                                        Det anbefales at du lukker alle kørende programmer inden start af setup-guiden. Dette vil tillade guiden at opdatere de nødvendiget systemfiler uden at skulle genstarte computeren.$\r$\n\
                                        $\r$\n\
                                        $_CLICK"

${LangFileString} FileTypeTitle "LyX-Dokument"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Installer til alle brugere?"
${LangFileString} SecFileAssocTitle "Fil-associationer"
${LangFileString} SecDesktopTitle "Skrivebordsikon"

${LangFileString} SecCoreDescription "Filerne til LyX."
${LangFileString} SecInstJabRefDescription "Bibliography reference manager and editor for BibTeX files."
${LangFileString} SecAllUsersDescription "Installer LyX til alle brugere, eller kun den aktuelle bruger."
${LangFileString} SecFileAssocDescription "Opret association mellem LyX og .lyx filer."
${LangFileString} SecDesktopDescription "Et LyX ikon på skrivebordet"

${LangFileString} LangSelectHeader "Valg af sprog i LyX's menuer"
${LangFileString} AvailableLang " Tilgængelige Sprog "

${LangFileString} EnterLaTeXHeader1 'LaTeX-distribution'
${LangFileString} EnterLaTeXHeader2 'Vælg den LaTeX-distribution som LyX skal bruge.'
${LangFileString} EnterLaTeXFolder 'Du kan her vælge stien til filen $\"latex.exe$\" og derved vælge hvilken \
                                          LaTeX-distribution som bruges LyX.$\r$\n\
                                          Hvis du ikke vælger LaTeX, kan LyX ikke generere dokumenter!$\r$\n\
                                          $\r$\n\
                                          Installationsprogrammet har detekteret LaTeX-distributionen \
                                          $\"$LaTeXName$\" på din computer i nedenstående sti:'
${LangFileString} EnterLaTeXFolderNone 'Vælg nedenfor stien til filen $\"latex.exe$\". Herved kan du vælge hvilken \
                                              LaTeX-distribution LyX skal bruge.$\r$\n\
                                              Hvis du ikke vælger LaTeX, kan LyX ikke generere dokumenter!$\r$\n\
                                              $\r$\n\
                                              Installationsprogrammet kunne ikke finde nogen LaTeX-distribution.'
${LangFileString} PathName 'Sti til filen $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Brug ikke LaTeX"
${LangFileString} InvalidLaTeXFolder 'Kunne ikke finde $\"latex.exe$\".'

${LangFileString} LatexInfo 'Installationen af LaTeX-distributionen $\"MiKTeX$\" startes.$\r$\n\
                                  For at installere programmet tryk på $\"Next$\"-knappen i installationsvinduet indtil installationen begynder.$\r$\n\
                                  $\r$\n\
                                  !!! Brug alle MiKTeX installationsprogrammets foreslåde valg !!!'
${LangFileString} LatexError1 'Ingen LaTeX-distribution fundet!$\r$\n\
                                    LyX kan ikke bruges uden en LaTeX-distribution sasom $\"MiKTeX$\"!$\r$\n\
                                    Installationen afbrydes.'

${LangFileString} HunspellFailed 'Download of dictionary for language $\"$R3$\" failed.'
${LangFileString} ThesaurusFailed 'Download of thesaurus for language $\"$R3$\" failed.'

${LangFileString} JabRefInfo 'Installationen af programmet $\"JabRef$\" startes.$\r$\n\
                                    Du kan roligt bruge alle $\"default$\" valgmuligheder ved JabRef-installationen.'
${LangFileString} JabRefError 'Programmet $\"JabRef$\" kunne ikke installeres!$\r$\n\
                                     Installationen vil dog fortsætte.$\r$\n\
                                     Prøv at installere JabRef igen senere.'

${LangFileString} LatexConfigInfo "Den følgende konfiguration af LyX vil tage et stykke tid."

${LangFileString} MiKTeXPathInfo "For at alle brugere senere kan konfigurere  MiKTeX$\r$\n\
                                        er det nødvendigt at give skriverettigheder for alle MiKTeX brugere i installations mappen$\r$\n\
                                        $MiKTeXPath $\r$\n\
                                        og dennes undermapper."
${LangFileString} MiKTeXInfo 'LaTeX-distributionen $\"MiKTeX$\" vil blive brugt sammen med LyX.$\r$\n\
                                    Det anbefales at installere tilgængelige MiKTeX-opdateringer ved hjælp af programmet $\"MiKTeX Update Wizard$\"$\r$\n\
                                    før LyX startes for første gang.$\r$\n\
                                    Vil du tjekke om der findes MiKTeX opdateringer?'

${LangFileString} UpdateNotAllowed "This update package can only update ${PRODUCT_VERSION_OLD}!"
${LangFileString} ModifyingConfigureFailed "Forsoget på at indstille 'path_prefix' i konfigurationen mislykkedes"
${LangFileString} CreateCmdFilesFailed "Kunne ikke oprette lyx.bat"
${LangFileString} RunConfigureFailed "Mislykket forsog på at afvikle konfigurations-scriptet"
${LangFileString} NotAdmin "Du skal have administrator-rettigheder for at installere LyX!"
${LangFileString} InstallRunning "Installationsprogrammet kører allerede!"
${LangFileString} StillInstalled "LyX er allerede installeret! Afinstaller LyX først."

${LangFileString} FinishPageMessage "Tillykke!! LyX er installeret.$\r$\n\
                                          $\r$\n\
                                          (Når LyX startes første gang, kan det tage noget tid.)"
${LangFileString} FinishPageRun "Start LyX"

${LangFileString} UnNotInRegistryLabel "Kunne ikke finde LyX i registreringsdatabsen.$\r$\n\
                                             Genvejene på skrivebordet og i Start-menuen bliver ikke fjernet"
${LangFileString} UnInstallRunning "Du ma afslutte LyX forst!"
${LangFileString} UnNotAdminLabel "Du skal have administrator-rettigheder for at afinstallere LyX!"
${LangFileString} UnReallyRemoveLabel "Er du sikker på, at du vil slette LyX og alle tilhørende komponenter?"
${LangFileString} UnLyXPreferencesTitle 'LyX$\'s user preferences'

${LangFileString} SecUnMiKTeXDescription 'Afinstallerer LaTeX-distributionen $\"MiKTeX$\".'
${LangFileString} SecUnJabRefDescription 'Afinstallerer bibliografi programmet $\"JabRef$\".'
${LangFileString} SecUnPreferencesDescription 'Sletter LyX$\'s konfigurations mappe$\r$\n\
                                          $\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
                                          for alle brugere.'
${LangFileString} SecUnProgramFilesDescription "Afinstallerer LyX og alle dets komponenter."

