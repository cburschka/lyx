!ifndef _LYX_LANGUAGES_HUNGARIAN_NSH_
!define _LYX_LANGUAGES_HUNGARIAN_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_HUNGARIAN}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "A varázsló segítségével tudja telepíteni a LyX-et.\r\n\
					  \r\n\
					  A LyX telepítéséhez rendszergazdai jogra van szüksége.\r\n\
					  \r\n\
					  A telepítés megkezdése elõtt, javasolt kilépni minden futó alkalmazásból. Ez a lépés teszi lehetõvé, hogy frissítsünk fontos rendszerfájlokat újraindítás szükségessége nélkül.\r\n\
					  \r\n\
					  $_CLICK"

LangString FileTypeTitle "${LYX_LANG}" "LyX-dokumentum"

LangString SecInstGSviewTitle "${LYX_LANG}" "GSview"
LangString SecInstJabRefTitle "${LYX_LANG}" "JabRef"
LangString SecAllUsersTitle "${LYX_LANG}" "Telepítés minden felhasználónak"
LangString SecFileAssocTitle "${LYX_LANG}" "Fájltársítások"
LangString SecDesktopTitle "${LYX_LANG}" "Parancsikon asztalra"

LangString SecCoreDescription "${LYX_LANG}" "A LyX futtatásához szükséges fájlok."
LangString SecInstGSviewDescription "${LYX_LANG}" "Alkalmazás Postscript- és PDF-fájlok megjelenítéséhez."
LangString SecInstJabRefDescription "${LYX_LANG}" "Irodalomjegyzék-hivatkozás szerkesztõ és kezelõ BibTeX fájlokhoz."
LangString SecAllUsersDescription "${LYX_LANG}" "Minden felhasználónak telepítsem vagy csak az aktuálisnak?"
LangString SecFileAssocDescription "${LYX_LANG}" "A .lyx kiterjesztéssel rendelkezõ fájlok megnyitása automatikusan a LyX-el történjen."
LangString SecDesktopDescription "${LYX_LANG}" "LyX-ikon elhelyezése az asztalon."

LangString LangSelectHeader "${LYX_LANG}" "A LyX felhasználói felület nyelvének kiválasztása"
LangString AvailableLang "${LYX_LANG}" " Választható nyelvek "

LangString MissProgHeader "${LYX_LANG}" "Mûködéshez szükséges programok ellenõrzése"
LangString MissProgCap "${LYX_LANG}" "A következõ program(ok) telepítése fog még megtörténni"
LangString MissProgLatex "${LYX_LANG}" "MiKTeX, LaTeX-disztribúció"
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript, ami egy PostScript és PDF értelmezõ"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick, ami egy képátalakító"
LangString MissProgAspell "${LYX_LANG}" "Aspell, ami egy helyesírás ellenõrzõ"
LangString MissProgMessage "${LYX_LANG}" 'Nem kell további programokat telepítenie.'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'LaTeX-disztribúció'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'Adja meg a LaTeX-disztribúciót, amit a LyX-nek használnia kell.'
LangString EnterLaTeXFolder "${LYX_LANG}" 'Itt megadhatja az elérési útvonalat a "latex.exe" fájlhoz és ezzel meg is adja \
					   melyik LaTeX disztribúciót fogja használni a LyX.\r\n\
					   Amennyiben nem használ LaTeX-et, a LyX nem tud kimenetet készíteni!\r\n\
					   \r\n\
					   A telepítõ megtalálta az ön számítógépén a \
            "$LaTeXName" disztribúciót. Lent láthatja az elérési útvonalát.'
LangString EnterLaTeXFolderNone "${LYX_LANG}" 'Adja meg a használandó "latex.exe" fájl elérési útvonalát. Ezzel azt is megadja \
					       melyik LaTeX disztribúciót fogja használni a LyX.\r\n\
					       Amennyiben nem használ LaTeX-et, a LyX nem tud kimenetet készíteni!\r\n\
					       \r\n\
					       A telepítõ nem talált a számítógépén LaTeX disztribúciót!'
LangString PathName "${LYX_LANG}" 'A "latex.exe" fájl elérési útja'
LangString DontUseLaTeX "${LYX_LANG}" "Ne használja a LaTeX-et"
LangString InvalidLaTeXFolder "${LYX_LANG}" 'Nem találom a "latex.exe" fájlt, a megadott helyen.'

LangString LatexInfo "${LYX_LANG}" 'A "MiKteX" LaTeX-disztribúció telepítõjének indítása következik.$\r$\n\
			            A program telepítéséhez addig nyomkodja a "Következõ" gombot a telepítõ ablakában, amíg a telepítés el nem kezd?dik.$\r$\n\
				    $\r$\n\
				    !!! Kérem használja a MikTeX-telepítõ alapértelmezett opcióit!!!'
LangString LatexError1 "${LYX_LANG}" 'Nem találtam LaTeX-disztribúciót!$\r$\n\
                      		      A LyX nem használható egy LaTeX-disztibúció nélkül,(mint például a "MiKTeX")!$\r$\n\
				      Ezért a telepítés nem folytatható!'

LangString GSviewInfo "${LYX_LANG}" 'A "GSview" program telepítõjének indítása következik.$\r$\n\
			             A program telepítéséhez, nyomja meg a "Telepít"-gombot a telepítõ elsõ ablakában,$\r$\n\
				     azután válasszon nyelvet, majd nyomja meg a "Következõ"-gombot a következõ telepítõ ablakokban.$\r$\n\
				     Használhatja a program által javasolt beállításokat.'
LangString GSviewError "${LYX_LANG}" 'A "GSview" programot nem sikerült telepíteni!$\r$\n\
		                      Azonban a telepítés tovább fog folytatódni.$\r$\n\
				      Késõbb próbálja meg újra a GSview telepítését.'

LangString JabRefInfo "${LYX_LANG}" 'A "JabRef" telepítõjének elindítása következik.$\r$\n\
				     Használhatja az alapértelmezett opciókat.'
LangString JabRefError "${LYX_LANG}" 'Nem tudom telepíteni a "JabRef" programot!$\r$\n\
		                      Azonban a telepítés tovább fog folytatódni.$\r$\n\
				      Késõbb próbálja meg újra telepíteni a JabRef-et.'

LangString LatexConfigInfo "${LYX_LANG}" "A LyX telepítés utáni beállítása hosszú idõt vehet igénybe."

LangString AspellInfo "${LYX_LANG}" 'Az "Aspell" helyesírás-ellenõrzõhöz használható szótárak letöltése és telepítése következik.$\r$\n\
				     Minden egyes szótárnak különbözõ licence lehet, ezért a licenc minden telepítés elõtt meg lesz jelenítve.'
LangString AspellDownloadFailed "${LYX_LANG}" "Nem tudtam Aspell szótárat letölteni!"
LangString AspellInstallFailed "${LYX_LANG}" "Nincs telepítendõ Aspell szótár!"
LangString AspellPartAnd "${LYX_LANG}" " és "
LangString AspellPartSep "${LYX_LANG}" ", "
LangString AspellAll "${LYX_LANG}" 'Már van telepítve helyesírás-ellenõrzõ szótár a következõ nyelvhez'
LangString AspellNone "${LYX_LANG}" 'Nem lett szótár telepítve az "Aspell" helyesírás-ellenõrzõhöz.$\r$\n\
				     A szükséges szótárakat letöltheti a következõ címrõl:$\r$\n\
				     ${AspellLocation}$\r$\n\
				     Szeretne most szótárakat letölteni?'
LangString AspellPartStart "${LYX_LANG}" "Sikeresen települt "
LangString AspellPart1 "${LYX_LANG}" "az angol szótár"
LangString AspellPart2 "${LYX_LANG}" "a(z) $LangName nyelv szótára"
LangString AspellPart3 "${LYX_LANG}" "a(z)$\r$\n\
				      $LangNameSys nyelv szótára"
LangString AspellPart4 "${LYX_LANG}" ' az "Aspell" helyesírás-ellenõrzõhöz.$\r$\n\
				      Szótárat egyéb nyelvekhez a ${AspellLocation}$\r$\n\
				      címrõl tölthet le.$\r$\n\
				      Szeretne letölteni egyéb szótárakat?'

LangString MiKTeXPathInfo "${LYX_LANG}" "Ahhoz, hogy a többi felhasználó is testre tudja használni a MiKTeX-et$\r$\n\
					 írás jogot kell adni minden felhasználó részére a MiKTeX alkönyvtárhoz.$\r$\n\
					 A $MiKTeXPath $\r$\n\
					 valamint alkönyvtáraihoz."
LangString MiKTeXInfo "${LYX_LANG}" 'A "MiKTeX" LaTeX-disztibúció lesz használva a LyX által.$\r$\n\
				     Javasolt telepíteni a MiKTeX frissítéseket a "MiKTeX Update Wizard" segítségével,$\r$\n\
				     a LyX elsõ indítása elõtt.$\r$\n\
				     Szeretné ellenõrizni a frissítéseket most?'

LangString UpdateNotAllowed "${LYX_LANG}" "Ez a frissítõ csomag csak a ${PRODUCT_VERSION_OLD} verziót tudja frissíteni!"
LangString ModifyingConfigureFailed "${LYX_LANG}" "Nem tudom beállítani a 'path_prefix'-et a configure parancsfájlban!"
LangString CreateCmdFilesFailed "${LYX_LANG}" "Nem tudom létrehozni a lyx.bat fájlt."
LangString RunConfigureFailed "${LYX_LANG}" "Nem tudom végrehajtani a configure parancsfájlt!"
LangString NotAdmin "${LYX_LANG}" "A LyX telepítéséhez rendszergazdai jogok szükségesek!"
LangString InstallRunning "${LYX_LANG}" "A telepítõ már fut!"
LangString StillInstalled "${LYX_LANG}" "A LyX már teleptve van! Elõször távolítsa el a régit."

LangString FinishPageMessage "${LYX_LANG}" "Gratulálok! Sikeresen telepítette a LyX-et.\r\n\
					    \r\n\
					    (Az program elsõ indítása egy kis idõt vehet igénybe...)"
LangString FinishPageRun "${LYX_LANG}" "LyX indítása"

LangString UnNotInRegistryLabel "${LYX_LANG}" "Nem találom a LyX-et a regisztriben.$\r$\n\
					       Az Asztalon és a Start Menüben található parancsikonok nem lesznek eltávolítva!."
LangString UnInstallRunning "${LYX_LANG}" "Elõször be kell zárnia a LyX-et!"
LangString UnNotAdminLabel "${LYX_LANG}" "A LyX eltávolításhoz rendszergazdai jogokkal kell rendelkeznie!"
LangString UnReallyRemoveLabel "${LYX_LANG}" "Biztosan abban, hogy el akarja távolítani a LyX-t, minden tartozékával együtt?"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'LyX felhasználói beállítások'
LangString UnGSview "${LYX_LANG}" 'Kérem nyomja meg az "Eltávolítás" gombot, a PostScript és a\r$\n\
				   PDF megjelenítõ "GSview" eltávolításához a következõ ablakban.'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "LyX eltávolítása a számítógépérõl zökkenõmentesen befejezõdött."

LangString SecUnAspellDescription "${LYX_LANG}" "Aspell és minden szótárának eltávolítása."
LangString SecUnMiKTeXDescription "${LYX_LANG}" "MikTeX LaTeX-disztibúció eltávolítása."
LangString SecUnJabRefDescription "${LYX_LANG}" "JabRef irodalomjegyzék kezelõ eltávolítása."
LangString SecUnPreferencesDescription "${LYX_LANG}" 'A  LyX beállítások mappa törlése$\r$\n\
					   "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
					   minden felhasználónál.'
LangString SecUnProgramFilesDescription "${LYX_LANG}" "A LyX és minden komponensének eltávolítása."

!undef LYX_LANG

!endif # _LYX_LANGUAGES_HUNGARIAN_NSH_
