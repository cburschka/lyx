﻿/*
LyX Installer Language File
Language: Hungarian
Author: Szőke Sándor
*/

!insertmacro LANGFILE_EXT "Hungarian"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Telepítve az aktuális felhasználónak)"

${LangFileString} TEXT_WELCOME "A varázsló segítségével tudja telepíteni a LyX-et.$\r$\n\
				$\r$\n\
				$_CLICK"

#${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "MiKTeX filenév adatbázis frissítése..."
${LangFileString} TEXT_CONFIGURE_LYX "LyX beállítása ($LaTeXInstalled letöltheti a hiányzó csomagokat, ez eltarthat egy ideig) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Python parancsfájlok fordítása..."

${LangFileString} TEXT_FINISH_DESKTOP "Indítóikon létrehozása Asztalon"
${LangFileString} TEXT_FINISH_WEBSITE "Látogasson el a lyx.org oldalra az aktuális hírekért, támogatásért és tippekért"

#${LangFileString} FileTypeTitle "LyX-dokumentum"

#${LangFileString} SecInstJabRefTitle "JabRef"
#${LangFileString} SecAllUsersTitle "Telepítés minden felhasználónak"
${LangFileString} SecFileAssocTitle "Fájltársítások"
${LangFileString} SecDesktopTitle "Parancsikon Asztalra"

${LangFileString} SecCoreDescription "A LyX futtatásához szükséges fájlok."
#${LangFileString} SecInstJabRefDescription "Irodalomjegyzék-hivatkozás szerkesztő és kezelő BibTeX fájlokhoz."
#${LangFileString} SecAllUsersDescription "Minden felhasználónak telepítsem vagy csak az aktuálisnak?"
${LangFileString} SecFileAssocDescription "A .lyx kiterjesztéssel rendelkező fájlok megnyitása automatikusan a LyX-el történjen."
${LangFileString} SecDesktopDescription "LyX-ikon elhelyezése az Asztalon."
${LangFileString} SecDictionaries "Szótárak"
${LangFileString} SecDictionariesDescription "Helyesírás-ellenőrző szótárak, amiket letölthet és telepíthet."
${LangFileString} SecThesaurus "Tezaurusz"
${LangFileString} SecThesaurusDescription "Szinoníma-szótárak, amiket letölthet és telepíthet."

${LangFileString} EnterLaTeXHeader1 'LaTeX-disztribúció'
${LangFileString} EnterLaTeXHeader2 'Adja meg a LaTeX-disztribúciót, amit a LyX-nek használnia kell.'
${LangFileString} EnterLaTeXFolder 'Itt megadhatja az elérési útvonalat a $\"latex.exe$\" fájlhoz és ezzel meg is adja \
					melyik LaTeX disztribúciót fogja használni a LyX.\r\n\
					Amennyiben nem használ LaTeX-et, a LyX nem tud kimenetet készíteni!\r\n\
					\r\n\
					A telepítő megtalálta az ön számítógépén a \
					$\"$LaTeXName$\" disztribúciót. Lent láthatja az elérési útvonalát.'
${LangFileString} EnterLaTeXFolderNone 'Adja meg a használandó $\"latex.exe$\" fájl elérési útvonalát. Ezzel azt is megadja \
					melyik LaTeX disztribúciót fogja használni a LyX.\r\n\
					Amennyiben nem használ LaTeX-et, a LyX nem tud kimenetet készíteni!\r\n\
					\r\n\
					A telepítő nem talált a számítógépén LaTeX disztribúciót!'
${LangFileString} PathName 'A $\"latex.exe$\" fájl elérési útja'
${LangFileString} DontUseLaTeX "Ne használja a LaTeX-et"
${LangFileString} InvalidLaTeXFolder 'Nem találom a $\"latex.exe$\" fájlt, a megadott helyen.'

${LangFileString} LatexInfo 'A $\"MiKteX$\" LaTeX-disztribúció telepítőjének indítása következik.$\r$\n\
				A program telepítéséhez addig nyomkodja a $\"Következő$\" gombot a telepítő ablakában, amíg a telepítés el nem kezdődik.$\r$\n\
				$\r$\n\
				!!! Kérem használja a MikTeX-telepítő alapértelmezett opcióit!!!'
${LangFileString} LatexError1 'Nem találtam LaTeX-disztribúciót!$\r$\n\
				A LyX nem használható egy LaTeX-disztibúció nélkül,(mint például a $\"MiKTeX$\")!$\r$\n\
				Ezért a telepítés nem folytatható!'

${LangFileString} HunspellFailed 'Szótár letöltése a(z) $\"$R3$\" nyelvhez sikertelen.'
${LangFileString} ThesaurusFailed 'Szinoníma szótár letöltése a(z) $\"$R3$\" nyelvhez sikertelen.'

#${LangFileString} JabRefInfo 'A $\"JabRef$\" telepítőjének elindítása következik.$\r$\n\
#				Használhatja az alapértelmezett opciókat.'
#${LangFileString} JabRefError 'Nem tudom telepíteni a $\"JabRef$\" programot!$\r$\n\
#				Azonban a telepítés tovább fog folytatódni.$\r$\n\
#				Később próbálja meg újra telepíteni a JabRef-et.'

#${LangFileString} LatexConfigInfo "A LyX telepítés utáni beállítása hosszú időt vehet igénybe."

#${LangFileString} MiKTeXPathInfo "Ahhoz, hogy a többi felhasználó is testre tudja használni a MiKTeX-et$\r$\n\
#					írás jogot kell adni minden felhasználó részére a MiKTeX alkönyvtárhoz.$\r$\n\
#					A $MiKTeXPath $\r$\n\
#					valamint alkönyvtáraihoz."
${LangFileString} MiKTeXInfo 'A $\"MiKTeX$\" LaTeX-disztibúció lesz használva a LyX által.$\r$\n\
				Javasolt telepíteni a MiKTeX frissítéseket a $\"MiKTeX Update Wizard$\" segítségével,$\r$\n\
				a LyX első indítása előtt.$\r$\n\
				Szeretné ellenőrizni a frissítéseket most?'

${LangFileString} ModifyingConfigureFailed "Nem tudom beállítani a 'path_prefix'-et a configure parancsfájlban!"
#${LangFileString} RunConfigureFailed "Nem tudom végrehajtani a configure parancsfájlt!"
${LangFileString} NotAdmin "A LyX telepítéséhez rendszergazdai jogok szükségesek!"
${LangFileString} InstallRunning "A telepítő már fut!"
${LangFileString} AlreadyInstalled "A LyX ${APP_SERIES_KEY2} már teleptve van!$\r$\n\
				Installing over existing installations is not recommended if the installed version$\r$\n\
				is a test release or if you have problems with your existing LyX installation.$\r$\n\
				In these cases better reinstall LyX.$\r$\n\
				Dou you nevertheles want to install LyX over the existing version?"
${LangFileString} NewerInstalled "A jelenleg telepítettnél régebbi LyX verziót próbál telepíteni.$\r$\n\
				  Ha valóban ezt akarja, először el kell távolítania a meglévő LyX $OldVersionNumber változatot."

${LangFileString} MultipleIndexesNotAvailable "Bizonyos listák támogatása nem lesz elérhető a dokumentumokban,\r$\n\
						mert a MiKTeX rendszergazda joggal lett telepítve, azonban a LyX-et ezen jog nélkül telepíti."
						
#${LangFileString} FinishPageMessage "Gratulálok! Sikeresen telepítette a LyX-et.$\r$\n\
#					$\r$\n\
#					(A program első indítása egy kis időt vehet igénybe...)"
${LangFileString} FinishPageRun "LyX indítása"

${LangFileString} UnNotInRegistryLabel "Nem találom a LyX-et a regisztriben.$\r$\n\
					Az Asztalon és a Start Menüben található parancsikonok nem lesznek eltávolítva!."
${LangFileString} UnInstallRunning "Először be kell zárnia a LyX-et!"
${LangFileString} UnNotAdminLabel "A LyX eltávolításhoz rendszergazdai jogokkal kell rendelkeznie!"
${LangFileString} UnReallyRemoveLabel "Biztosan abban, hogy el akarja távolítani a LyX-t, minden tartozékával együtt?"
${LangFileString} UnLyXPreferencesTitle 'LyX felhasználói beállítások'

${LangFileString} SecUnMiKTeXDescription "MikTeX LaTeX-disztibúció eltávolítása."
#${LangFileString} SecUnJabRefDescription "JabRef irodalomjegyzék kezelő eltávolítása."
${LangFileString} SecUnPreferencesDescription 'A  LyX beállítások mappa törlése$\r$\n\
						$\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						minden felhasználónál.'
${LangFileString} SecUnProgramFilesDescription "A LyX és minden komponensének eltávolítása."
