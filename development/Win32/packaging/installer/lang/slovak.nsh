/*
LyX Installer Language File
Language: Slovak
Author: Kornel Benko
*/

!insertmacro LANGFILE_EXT "Slovak"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(In�talovan� pre s��asn�ho u��vate�a)"

${LangFileString} TEXT_WELCOME "Tento sprievodca V�m pom�ha in�talova� LyX.$\r$\n\
				$\r$\n\
				$_CLICK"

#${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Aktualiz�cia MiKTeX-datab�zy s�borov�ch mien..."
${LangFileString} TEXT_CONFIGURE_LYX "Konfigur�cia LyXu ($LaTeXInstalled m��e na��ta� ch�baj�ce bal�ky, to m��e trochu trva�) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Kompil�cia Python skriptov..."

${LangFileString} TEXT_FINISH_DESKTOP "Vytvori� skratku pre pracovn� plochu"
${LangFileString} TEXT_FINISH_WEBSITE "Nav�t�vte lyx.org pre posledn� novinky, podporu a tipy"

${LangFileString} FileTypeTitle "LyX dokument"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "In�talova� pre v�etk�ch u��vate�ov?"
${LangFileString} SecFileAssocTitle "Asoci�cie s�borov"
${LangFileString} SecDesktopTitle "Ikona pracovnej plochy"

${LangFileString} SecCoreDescription "S�bory LyXu."
${LangFileString} SecInstJabRefDescription "Mana��r pre bibliografick� odkazy a editor pre BibTeX s�bory."
#${LangFileString} SecAllUsersDescription "In�talova� LyX pre v�etk�ch u��vate�ov alebo len pre s��asn�ho u��vate�a."
${LangFileString} SecFileAssocDescription "S�bory s roz��ren�m .lyx sa automaticky otv�raj� v LyXe."
${LangFileString} SecDesktopDescription "Ikona LyXa na pracovnej ploche."
${LangFileString} SecDictionariesDescription "Slovn�ky pre kontrolu pravopisu ktor� mo�no na��ta� a in�talova�."
${LangFileString} SecThesaurusDescription "Slovn�ky tezaura ktor� mo�no na��ta� a in�talova�."

${LangFileString} EnterLaTeXHeader1 'LaTeX distrib�cia'
${LangFileString} EnterLaTeXHeader2 'Ur�ite LaTeX distrib�ciu, ktor� m� LyX pou��va�.'
${LangFileString} EnterLaTeXFolder 'Tu m��te nastavi� cestu k $\"latex.exe$\" a t�m nastavi� \
					LaTeX-distrib�ciu, ktor� by mala by� pou��van� LyXom.\r\n\
					Bez LaTeXu, LyX nedok�e produkova� dokumenty!\r\n\
					\r\n\
					In�talat�r detegoval LaTeX-distrib�ciu \
					$\"$LaTeXName$\" na va�om syst�me. Dolu je zobrazen� jeho cesta.'
${LangFileString} EnterLaTeXFolderNone 'Nastavte dolu cestu k $\"latex.exe$\". T�m nastav�te, ktor� \
					LaTeX-distrib�cia m� by� pou��van� LyXom.\r\n\
					Bez LaTeXu LyX nedok�e produkova� dokumenty!\r\n\
					\r\n\
					In�talat�r nemohol n�js� LaTeX-distrib�ciu na va�om syst�me.'
${LangFileString} PathName 'Cesta na s�bor $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Nepou�i� LaTeX"
${LangFileString} InvalidLaTeXFolder 'S�bor $\"latex.exe$\" nie je na �pecifikovanej ceste.'

${LangFileString} LatexInfo 'Teraz �tartuje in�talat�r LaTeX distrib�cie $\"MiKTeX$\".$\r$\n\
				Na in�talovanie programu tla�te $\"Next$\" v okne in�tal�tora a� in�tal�cia za�ne.$\r$\n\
				$\r$\n\
				!!! Pros�m pou�ite v�etky �tandardn� mo�nosti in�talat�ra MiKTeX !!!'

${LangFileString} LatexError1 'Nena�la sa �iadna LaTeX-distrib�cia!$\r$\n\
				LyX sa nem��e u��va� bez LaTeX distrib�cie ako napr�klad $\"MiKTeX$\"!$\r$\n\
				In�tal�cia sa preto zru�uje.'

${LangFileString} HunspellFailed 'Na��tanie slovn�ka pre jazyk $\"$R3$\" zlyhalo.'
${LangFileString} ThesaurusFailed 'Na��tanie tezaura pre jazyk $\"$R3$\" zlyhalo.'

${LangFileString} JabRefInfo 'Teraz sa �tartuje in�talat�r programu $\"JabRef$\".$\r$\n\
				M��te pou�i� v�etky �tandardn� mo�nosti in�talat�ra JabRef.'
${LangFileString} JabRefError 'Program $\"JabRef$\" nebol �spe�ne in�talovan�!$\r$\n\
				Predsa in�tal�cia pokra�uje.$\r$\n\
				Sk�ste in�talova� JabRef nesk�r e�te raz.'

${LangFileString} LatexConfigInfo "Nasleduj�ca konfigur�cia LyXu trochu potrv�."

${LangFileString} MiKTeXPathInfo "Aby ka�d� pou��vate� bol nesk�r schopn� pou��va� MiKTeX pre svoje potreby$\r$\n\
					je treba nastavi� povolenia pre v�etk�ch pou��vate�ov do in�tala�n�ho adres�ra MiKTeX $\r$\n\
					$MiKTeXPath $\r$\n\
					i podriaden�ch adres�rov."
${LangFileString} MiKTeXInfo 'LaTeX-distrib�cia $\"MiKTeX$\" sa bude pou��va� spolu s LyXom.$\r$\n\
				Doporu�uje sa in�talova� dostupn� MiKTeX aktualiz�cie pomocou programu $\"MiKTeX Update Wizard$\"$\r$\n\
				pred prv�m pou�it�m LyXu.$\r$\n\
				Chcete teraz skontrolova� MiKTeX aktualiz�cie?'

${LangFileString} ModifyingConfigureFailed "Nedal sa nastavi� 'path_prefix' v konfigura�nom skripte"
#${LangFileString} RunConfigureFailed "Nedal sa spusti� konfigura�n� skript"
${LangFileString} NotAdmin "Pre in�tal�ciu LyXu potrebujete administr�torsk� pr�va!"
${LangFileString} InstallRunning "In�tala�n� program u� be��!"
${LangFileString} StillInstalled "LyX ${APP_SERIES_KEY2} je u� in�talovan�! LyX treba najprv odin�talova�."
${LangFileString} NewerInstalled "Pok��ate sa in�talova� verziu LyXu ktor� je star�ia ako t� ktor� je in�talovan�.$\r$\n\
				  Ke� to naozaj chcete, odin�talujte najprv existuj�ci LyX $OldVersionNumber."

${LangFileString} MultipleIndexesNotAvailable "Podpora pou��vania viacer�ch indexov v dokumente nebude dostupn�, lebo\r$\n\
						MiKTeX bol in�talovan� pod administr�torsk�mi pr�vami a vy in�talujete LyX bez nich."
${LangFileString} MetafileNotAvailable "Podpora obr�zkov vo form�te EMF alebo WMF nebude dostupn�$\r$\n\
					lebo to vy�aduje in�tal�ciu softv�rovej tla�iarne pre Windows$\r$\n\
					�o je mo�n� len pod administr�torsk�mi pr�vami."

${LangFileString} FinishPageMessage "Gratul�cia! LyX bol �spe�ne in�talovan�.$\r$\n\
					$\r$\n\
					(Prv� LyX �tart m��e trva� nieko�ko sek�nd.)"
${LangFileString} FinishPageRun "Spusti� LyX"

${LangFileString} UnNotInRegistryLabel "Nem��em n�js� LyX v registre.$\r$\n\
					Skratky na pracovnej ploche a v �tartovacom Menu sa nedaj� odstr�ni�."
${LangFileString} UnInstallRunning "Najprv treba zavrie� LyX!"
${LangFileString} UnNotAdminLabel "Pre odin�tal�ciu LyX potrebujete administr�torsk� pr�va!"
${LangFileString} UnReallyRemoveLabel "Ste si ist�, �e chcete kompletne odin�talova� LyX a v�etky jeho s��iastky?"
${LangFileString} UnLyXPreferencesTitle 'LyXove u��vate�sk� nastavenia'

${LangFileString} SecUnMiKTeXDescription "Odin�taluje LaTeX-distrib�ciu MiKTeX."
${LangFileString} SecUnJabRefDescription "Odin�taluje mana��ra bibliografie JabRef."
${LangFileString} SecUnPreferencesDescription 'Odstr�ni konfigura�n� adres�r LyXu $\r$\n\
						$\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						pre v�etk�ch u��vate�ov (ke� m�te administr�torsk� pr�va).'
${LangFileString} SecUnProgramFilesDescription "Odin�taluj LyX a v�etky jeho s��iastky."

