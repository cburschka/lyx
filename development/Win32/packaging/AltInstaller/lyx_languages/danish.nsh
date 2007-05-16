!ifndef _LYX_LANGUAGES_DANISH_NSH_
!define _LYX_LANGUAGES_DANISH_NSH_

!ifdef LYX_LANG
 !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_DANISH}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "Denne guide vil installere LyX på din computer.\r\n\
                                        \r\n\
                                        Du skal have administratorrettigheder for at installerer LyX.\r\n\
                                        \r\n\
                                        Det anbefales at du lukker alle kørende programmer inden start af setup-guiden. Dette vil tillade guiden at opdatere de nødvendiget systemfiler uden at skulle genstarte computeren.\r\n\
                                        \r\n\
                                        $_CLICK"

LangString FileTypeTitle "${LYX_LANG}" "LyX-Dokument"

LangString SecInstGSviewTitle "${LYX_LANG}" "GSview"
LangString SecInstJabRefTitle "${LYX_LANG}" "JabRef"
LangString SecAllUsersTitle "${LYX_LANG}" "Installer til alle brugere?"
LangString SecFileAssocTitle "${LYX_LANG}" "Fil-associationer"
LangString SecDesktopTitle "${LYX_LANG}" "Skrivebordsikon"

LangString SecCoreDescription "${LYX_LANG}" "Filerne til LyX."
LangString SecInstGSviewDescription "${LYX_LANG}" "Program til gennemsyn af Postscript- og PDF-dokumenter."
LangString SecInstJabRefDescription "${LYX_LANG}" "Bibliography reference manager and editor for BibTeX files."
LangString SecAllUsersDescription "${LYX_LANG}" "Installer LyX til alle brugere, eller kun den aktuelle bruger."
LangString SecFileAssocDescription "${LYX_LANG}" "Opret association mellem LyX og .lyx filer."
LangString SecDesktopDescription "${LYX_LANG}" "Et LyX ikon på skrivebordet"

LangString LangSelectHeader "${LYX_LANG}" "Valg af sprog i LyX's menuer"
LangString AvailableLang "${LYX_LANG}" " Tilgængelige Sprog "

LangString MissProgHeader "${LYX_LANG}" "Tjek for nødvendige programmer"
LangString MissProgCap "${LYX_LANG}" "Følgende nødvendige program(mer) vil også blive installeret"
LangString MissProgLatex "${LYX_LANG}" "MiKTeX, en LaTeX distribution"
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript, en PostScript og PDF oversætter"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick, en billed-konverter"
LangString MissProgAspell "${LYX_LANG}" "Aspell, en stavekontrol"
LangString MissProgMessage "${LYX_LANG}" 'Ingen andre programmer behøves.'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'LaTeX-distribution'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'Vælg den LaTeX-distribution som LyX skal bruge.'
LangString EnterLaTeXFolder "${LYX_LANG}" 'Du kan her vælge stien til filen "latex.exe" og derved vælge hvilken \
                                          LaTeX-distribution som bruges LyX.\r\n\
                                          Hvis du ikke vælger LaTeX, kan LyX ikke generere dokumenter!\r\n\
                                          \r\n\
                                          Installationsprogrammet har detekteret LaTeX-distributionen \
                                          "$LaTeXName" på din computer i nedenstående sti:'
LangString EnterLaTeXFolderNone "${LYX_LANG}" 'Vælg nedenfor stien til filen "latex.exe". Herved kan du vælge hvilken \
                                              LaTeX-distribution LyX skal bruge.\r\n\
                                              Hvis du ikke vælger LaTeX, kan LyX ikke generere dokumenter!\r\n\
                                              \r\n\
                                              Installationsprogrammet kunne ikke finde nogen LaTeX-distribution.'
LangString PathName "${LYX_LANG}" 'Sti til filen "latex.exe"'
LangString DontUseLaTeX "${LYX_LANG}" "Brug ikke LaTeX"
LangString InvalidLaTeXFolder "${LYX_LANG}" 'Kunne ikke finde "latex.exe".'

LangString LatexInfo "${LYX_LANG}" 'Installationen af LaTeX-distributionen "MiKTeX" startes.$\r$\n\
                                  For at installere programmet tryk på "Next"-knappen i installationsvinduet indtil installationen begynder.$\r$\n\
                                  $\r$\n\
                                  !!! Brug alle MiKTeX installationsprogrammets foreslåde valg !!!'
LangString LatexError1 "${LYX_LANG}" 'Ingen LaTeX-distribution fundet!$\r$\n\
                                    LyX kan ikke bruges uden en LaTeX-distribution sasom "MiKTeX"!$\r$\n\
                                    Installationen afbrydes.'

LangString GSviewInfo "${LYX_LANG}" 'Installationen af programmet "GSview" startes.$\r$\n\
                                   For at installere programmet tryk på "Setup"-knappen i det forste installationsvindue$\r$\n\
                                   vælg sprog og tryk på "Next"-knappen i det efterfolgende installationsvindue.$\r$\n\
                                   Du kan bruge alle GSview installationsprogrammets foreslåede valg.'
LangString GSviewError "${LYX_LANG}" 'Programmet "GSview" kunne ikke installeres!$\r$\n\
                                    Installationen af LyX fortsættes alligevel.$\r$\n\
                                    Prov at installere GSview på et senere tidspunkt.'

LangString JabRefInfo "${LYX_LANG}" 'Installationen af programmet "JabRef" startes.$\r$\n\
                                    Du kan roligt bruge alle "default" valgmuligheder ved JabRef-installationen.'
LangString JabRefError "${LYX_LANG}" 'Programmet "JabRef" kunne ikke installeres!$\r$\n\
                                     Installationen vil dog fortsætte.$\r$\n\
                                     Prøv at installere JabRef igen senere.'

LangString LatexConfigInfo "${LYX_LANG}" "Den følgende konfiguration af LyX vil tage et stykke tid."

LangString AspellInfo "${LYX_LANG}" 'Ordbøger til stavekontrollen "Aspell" vil blive hentet og installeret.$\r$\n\
                                    Hvert sprog har sin egen licens, som vil blive vist før installationen.'
LangString AspellDownloadFailed "${LYX_LANG}" "Ingen Aspell ordbog kunne hentes!"
LangString AspellInstallFailed "${LYX_LANG}" "Ingen Aspell ordbog kunne installeres!"
LangString AspellPartAnd "${LYX_LANG}" " og "
LangString AspellPartSep "${LYX_LANG}" ", "
LangString AspellAll "${LYX_LANG}" 'Der er allerede installeret en ordbog for dette sprog'
LangString AspellNone "${LYX_LANG}" 'Ingen ordbog for stavekontrollen "Aspell" blev installeret.$\r$\n\
                                    Ordbøger kan hentes fra$\r$\n\
                                    ${AspellLocation}$\r$\n\
                                    Vil du hente ordbøgerne nu?'
LangString AspellPartStart "${LYX_LANG}" "Succesfuld installation af "
LangString AspellPart1 "${LYX_LANG}" "en engelsk ordbog"
LangString AspellPart2 "${LYX_LANG}" "en $LangName ordbog"
LangString AspellPart3 "${LYX_LANG}" "en$\r$\n\
                                     $LangNameSys ordbog"
LangString AspellPart4 "${LYX_LANG}" ' for stavekontrollen "Aspell".$\r$\n\
                                     Flere ordbøger kan hentes fra$\r$\n\
                                     ${AspellLocation}$\r$\n\
                                     Vil du hente flere ordbøger nu?'

LangString MiKTeXPathInfo "${LYX_LANG}" "For at alle brugere senere kan konfigurere  MiKTeX$\r$\n\
                                        er det nødvendigt at give skriverettigheder for alle MiKTeX brugere i installations mappen$\r$\n\
                                        $MiKTeXPath $\r$\n\
                                        og dennes undermapper."
LangString MiKTeXInfo "${LYX_LANG}" 'LaTeX-distributionen "MiKTeX" vil blive brugt sammen med LyX.$\r$\n\
                                    Det anbefales at installere tilgængelige MiKTeX-opdateringer ved hjælp af programmet "MiKTeX Update Wizard"$\r$\n\
                                    før LyX startes for første gang.$\r$\n\
                                    Vil du tjekke om der findes MiKTeX opdateringer?'

LangString UpdateNotAllowed "${LYX_LANG}" "This update package can only update ${PRODUCT_VERSION_OLD}!"
LangString ModifyingConfigureFailed "${LYX_LANG}" "Forsoget på at indstille 'path_prefix' i konfigurationen mislykkedes"
LangString CreateCmdFilesFailed "${LYX_LANG}" "Kunne ikke oprette lyx.bat"
LangString RunConfigureFailed "${LYX_LANG}" "Mislykket forsog på at afvikle konfigurations-scriptet"
LangString NotAdmin "${LYX_LANG}" "Du skal have administrator-rettigheder for at installere LyX!"
LangString InstallRunning "${LYX_LANG}" "Installationsprogrammet kører allerede!"
LangString StillInstalled "${LYX_LANG}" "LyX er allerede installeret! Afinstaller LyX først."

LangString FinishPageMessage "${LYX_LANG}" "Tillykke!! LyX er installeret.\r\n\
                                          \r\n\
                                          (Når LyX startes første gang, kan det tage noget tid.)"
LangString FinishPageRun "${LYX_LANG}" "Start LyX"

LangString UnNotInRegistryLabel "${LYX_LANG}" "Kunne ikke finde LyX i registreringsdatabsen.$\r$\n\
                                             Genvejene på skrivebordet og i Start-menuen bliver ikke fjernet"
LangString UnInstallRunning "${LYX_LANG}" "Du ma afslutte LyX forst!"
LangString UnNotAdminLabel "${LYX_LANG}" "Du skal have administrator-rettigheder for at afinstallere LyX!"
LangString UnReallyRemoveLabel "${LYX_LANG}" "Er du sikker på, at du vil slette LyX og alle tilhørende komponenter?"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'LyX$\'s user preferences'
LangString UnGSview "${LYX_LANG}" 'Vær sa god at trykke på "Uninstall"-knappen i det næste vindue for at afinstallere$\r$\n\
                                 Postscript and PDF-programmet "GSview".'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "LyX er blevet fjernet fra din computer."

LangString SecUnAspellDescription "${LYX_LANG}" 'Afinstallerer stavekontrollen "Aspell" og alle dens mapper.'
LangString SecUnMiKTeXDescription "${LYX_LANG}" 'Afinstallerer LaTeX-distributionen "MiKTeX".'
LangString SecUnJabRefDescription "${LYX_LANG}" 'Afinstallerer bibliografi programmet "JabRef".'
LangString SecUnPreferencesDescription "${LYX_LANG}" 'Sletter LyX$\'s konfigurations mappe$\r$\n\
                                          "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
                                          for alle brugere.'
LangString SecUnProgramFilesDescription "${LYX_LANG}" "Afinstallerer LyX og alle dets komponenter."

!undef LYX_LANG

!endif # _LYX_LANGUAGES_DANISH_NSH_

