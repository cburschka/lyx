/*
LyX Installer Language File
Language: Dutch
Author: Niko Strijbol, others
*/

!insertmacro LANGFILE_EXT "Dutch"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Geïnstalleerd voor Huidige Gebruiker)"

${LangFileString} TEXT_WELCOME "Deze wizard zal $(^NameDA) op uw systeem installeren.$\r$\n\
				$\r$\n\
				$_CLICK"

#${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "MiKTeX-bestandsdatabank bijwerken..."
${LangFileString} TEXT_CONFIGURE_LYX "LyX configureren ($LaTeXInstalled kan ontbrekende pakketten downloaden, dit kan even duren) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Python-scripts compileren..."

${LangFileString} TEXT_FINISH_DESKTOP "Maak een snelkoppeling op het bureaublad"
${LangFileString} TEXT_FINISH_WEBSITE "Bezoek lyx.org voor het laatste nieuws, ondersteuning en tips"

#${LangFileString} FileTypeTitle "LyX-document"

#${LangFileString} SecInstJabRefTitle "JabRef"
#${LangFileString} SecAllUsersTitle "Installeren voor alle gebruikers?"
${LangFileString} SecFileAssocTitle "Bestandsassociaties"
${LangFileString} SecDesktopTitle "Bureaubladpictogram"

${LangFileString} SecCoreDescription "De LyX-bestanden."
#${LangFileString} SecInstJabRefDescription "Beheer en bewerk bibliografische referenties in BibTeX-bestanden."
#${LangFileString} SecAllUsersDescription "LyX voor alle gebruikers of uitsluitend de huidige gebruiker installeren?"
${LangFileString} SecFileAssocDescription "Associeer het LyX-programma met de extensie .lyx."
${LangFileString} SecDesktopDescription "Een LyX-pictogram op het bureaublad."
${LangFileString} SecDictionaries "Woordenboeken"
${LangFileString} SecDictionariesDescription "Woordenboeken voor spellingcontrole die gedownload en geïnstalleerd kunnen worden."
${LangFileString} SecThesaurus "Thesaurus"
${LangFileString} SecThesaurusDescription "Thesauruswoordenboeken die gedownload en geïnstalleerd kunnen worden."

${LangFileString} EnterLaTeXHeader1 'LaTeX-distributie'
${LangFileString} EnterLaTeXHeader2 'Geef aan welke LaTeX-distributie LyX moet gebruiken.'
${LangFileString} EnterLaTeXFolder 'U kunt hier opgeven in welke map het programma $\"latex.exe$\" zich bevindt en op deze manier bepalen \
					welke LaTeX-distributie gebruikt wordt door LyX.\r\n\
					Zonder LaTeX kan LyX geen documenten generen!\r\n\
					\r\n\
					Het installatie programma heeft de LaTeX software \
					$\"$LaTeXName$\" op uw systeem gevonden in de volgende map.'
${LangFileString} EnterLaTeXFolderNone 'U kunt hier opgeven in welke map het programma $\"latex.exe$\" zich bevindt en op deze manier bepalen \
					welke LaTeX-distributie gebruikt wordt door LyX.\r\n\
					Zonder LaTeX kan LyX geen documenten generen!\r\n\
					\r\n\
					Het installatieprogramma heeft geen LaTeX-distributie op uw systeem gevonden.'
${LangFileString} PathName 'Map met het programma $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Gebruik geen LaTeX"
${LangFileString} InvalidLaTeXFolder '$\"latex.exe$\" is niet gevonden.'

${LangFileString} LatexInfo 'Het installatieprogramma van $\"MiKTeX$\" word gestart.$\r$\n\
				Om het programma te installeren klik op de $\"Volgende$\"-knop in het installatievenster totdat de installatie begint.$\r$\n\
				$\r$\n\
				!!! Gebruik de standaardopties tijdens de installatie van MiKTeX !!!'
${LangFileString} LatexError1 'Er is geen LaTeX-distributie gevonden!$\r$\n\
				LyX kan niet worden gebruikt zonder LaTeX-distributie zoals $\"MiKTeX$\"!$\r$\n\
				De installatie wordt daarom afgebroken.'

${LangFileString} HunspellFailed 'Download van woordenboek voor taal $\"$R3$\" mislukt.'
${LangFileString} ThesaurusFailed 'Download van thesaurus voor taal $\"$R3$\" mislukt.'

#${LangFileString} JabRefInfo 'Het installatieprogramma van $\"JabRef$\" wordt nu gestart.$\r$\n\
#				U kunt de standaardopties gebruiken.'
#${LangFileString} JabRefError 'Het programma $\"JabRef$\" kon niet worden geïnstalleerd!$\r$\n\
#				De LyX-installatie gaat desondanks verder.$\r$\n\
#				Probeer JabRef later te installeren.'

#${LangFileString} LatexConfigInfo "De volgende configuratie van LyX zal enige tijd duren."

#${LangFileString} MiKTeXPathInfo "Opdat elke gebruiker zijn MiKTeX-configuratie kan aanpassen$\r$\n\
#					is het noodzakelijk alle gebruikers schrijftoegang te geven in MiKTeX's installatiemap$\r$\n\
#					$MiKTeXPath $\r$\n\
#					en submappen."
${LangFileString} MiKTeXInfo 'De LaTeX-distributie $\"MiKTeX$\" zal gebruikt worden door LyX.$\r$\n\
				Het is aanbevolen MiKTeX-updates te installeren via de $\"MiKTeX Update Wizard$\"$\r$\n\
				voordat u LyX voor de eerste keer gebruikt.$\r$\n\
				Wilt u controleren of er updates voor MiKTeX beschikbaar zijn?'

${LangFileString} ModifyingConfigureFailed "Kon 'path_prefix' niet instellen in het configuratiescript"
#${LangFileString} RunConfigureFailed "Kon configuratiescript niet uitvoeren"
${LangFileString} InstallRunning "Het installatieprogramma is al gestart!"
${LangFileString} AlreadyInstalled "LyX ${APP_SERIES_KEY2} is reeds geïnstalleerd!$\r$\n\
				Het installeren over een bestaande installatie wordt afgeraden als de geïnstalleerde versie$\r$\n\
				een testversie is of als u problemen hebt met uw huidige LyX-installatie.$\r$\n\
				In deze gevallen is het beter om LyX opnieuw te installeren.$\r$\n\
				Wilt u toch LyX installeren over de bestaande versie?"
${LangFileString} NewerInstalled "U probeert een oudere versie van LyX te installeren dan wat er al geïnstalleerd is.$\r$\n\
				  Als u dit echt wilt, moet u eerste de bestaande versie van LyX $OldVersionNumber verwijderen."

${LangFileString} MultipleIndexesNotAvailable "Ondersteuning voor meerdere indices in een document zal niet beschikbaar zijn omdat$\r$\n\
						MiKTeX geïnstalleerd is met administratorbevoegdheden, maar u installeert LyX zonder."

#${LangFileString} FinishPageMessage "Gefeliciteerd! LyX is succesvol geinstalleerd.$\r$\n\
#					$\r$\n\
#					(De eerste keer dat u LyX start kan dit enige seconden duren.)"
${LangFileString} FinishPageRun "LyX starten"

${LangFileString} UnNotInRegistryLabel "LyX is niet gevonden in het Windows-register.$\r$\n\
					Snelkoppelingen op het bureaublad en in het Startmenu worden niet verwijderd."
${LangFileString} UnInstallRunning "U moet LyX eerst afsluiten!"
${LangFileString} UnNotAdminLabel "U heeft administratorbevoegdheden nodig om LyX te verwijderen!"
${LangFileString} UnReallyRemoveLabel "Weet u zeker dat u LyX en alle componenten volledig wil verwijderen van deze computer?"
${LangFileString} UnLyXPreferencesTitle 'Gebruikersvoorkeuren van LyX'

${LangFileString} SecUnMiKTeXDescription "Verwijderd de LaTeX-distributie MiKTeX."
#${LangFileString} SecUnJabRefDescription "Verwijderd de bibliografiebeheerder JabRef."
${LangFileString} SecUnPreferencesDescription 'Verwijderd de configuratie van LyX$\r$\n\
						(map $\"$AppPre\username\$\r$\n\
						$AppSuff\$\r$\n\
						${APP_DIR_USERDATA}$\")$\r$\n\
						voor u of voor alle gebruikers (als u admin bent).'
${LangFileString} SecUnProgramFilesDescription "Verwijder LyX en alle bijbehorende onderdelen."

# MikTeX Update Stuff
${LangFileString} MiKTeXUpdateMsg "Als gevolg van wijzigingen in MiKTeX kan de installatie van versies van LyX later dan 2.2.3 een gedeeltelijke update veroorzaken waardoor MiKTeX onbruikbaar wordt.$\r$\n\
$\r$\n\
Als u dit nog niet heeft gedaan, raden we u aan MiKTeX bij te werken naar de nieuwste versie voordat u een nieuwe versie van LyX installeert.$\r$\n\
$\r$\n\
U kunt nu doorgaan met de installatie of deze annuleren, zodat u MiKTeX kunt bijwerken."

${LangFileString} MiKTeXUpdateHeader "WAARSCHUWING: MiKTeX-wijzigingen"
