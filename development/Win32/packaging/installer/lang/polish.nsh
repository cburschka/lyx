!insertmacro LANGFILE_EXT "Polish"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Zainstalowane dla bieżącego użytkownika)"

${LangFileString} TEXT_WELCOME "Kreator przeprowadzi Ciebie przez proces instalacji LyX$\'a.$\r$\n\
				$\r$\n\
				$_CLICK"

#${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Aktualizacja bazy danych nazw plików MikTeX..."
${LangFileString} TEXT_CONFIGURE_LYX "Konfigurowanie LyX ($LaTeXInstalled może pobrać brakujące pakiety, co może zająć trochę czasu)..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Kompilowanie skryptów Python..."

${LangFileString} TEXT_FINISH_DESKTOP "Utwórz skrót na pulpicie"
${LangFileString} TEXT_FINISH_WEBSITE "Odwiedź lyx.org by poznać wiadomości i wskazówki lub skorzystać ze wsparcia"

#${LangFileString} FileTypeTitle "Dokument LyX"

#${LangFileString} SecInstJabRefTitle "JabRef"
#${LangFileString} SecAllUsersTitle "Instalacja dla wszystkich użytkowników?"
${LangFileString} SecFileAssocTitle "Skojarzenie plików .lyx"
${LangFileString} SecDesktopTitle "Ikona na pulpicie"

${LangFileString} SecCoreDescription "Pliki LyX$\'a."
#${LangFileString} SecInstJabRefDescription "Aplikacja do zarządzania i edycji odnośników bibliografii w plikach BibTeX."
#${LangFileString} SecAllUsersDescription "Instalacja dla wszystkich użytkowników lub tylko dla bieżącego użytkownika."
${LangFileString} SecFileAssocDescription "Skojarzenie LyX-a z plikami o rozszerzeniu .lyx."
${LangFileString} SecDesktopDescription "Ikona LyX$\'a na pulpicie."
${LangFileString} SecDictionaries "Słowniki"
${LangFileString} SecDictionariesDescription "Słowniki sprawdzania pisowni, które mogą zostać pobrane i zainstalowane."
${LangFileString} SecThesaurus "Tezaurus"
${LangFileString} SecThesaurusDescription "Słowniki tezaurusa, które mogą zostać pobrane i zainstalowane."

${LangFileString} EnterLaTeXHeader1 'Dystrybucja LaTeX'
${LangFileString} EnterLaTeXHeader2 'Wybierz dystrybucję LaTeX, którą chcesz używać z LyX.'
${LangFileString} EnterLaTeXFolder 'Możesz opcjonalnie określić ścieżkę do pliku $\"latex.exe$\" i w związku z tym ustawić, \
					która dystrybucja LaTeX ma być używana przez LyX.\r\n\
					Jeżeli nie użyjesz LaTeX, LyX będzie niezdolny do wyprodukowania dokumentów!\r\n\
					\r\n\
					Instalator wykrył dystrybucję LaTeX \
					$\"$LaTeXName$\" w twoim systemie. Poniżej wyświetlono jego ścieżkę.'
${LangFileString} EnterLaTeXFolderNone 'Poniżej podaj ścieżkę do pliku $\"latex.exe$\". W związku z tym od Ciebie \
					zależy, która dystrybucja LaTeX zostanie użyta przez LyX.\r\n\
					Jeżeli nie użyjesz LaTeX, LyX będzie niezdolny do wyprodukowania dokumentów!\r\n\
					\r\n\
					Instalator nie może odnaleźć dystrybucji LaTeX w twoim systemie.'
${LangFileString} PathName 'Ścieżka do pliku $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Nie używaj LaTeX-a"
${LangFileString} InvalidLaTeXFolder 'Plik $\"latex.exe$\" nie znajduje się w podanej ścieżce.'

${LangFileString} LatexInfo 'Teraz zostanie uruchomiony instalator dystrybucji LaTeX $\"MiKTeX$\".$\r$\n\
				Aby zainstalować ten program naciśnij przycisk $\"Dalej$\" w oknie instalatora.$\r$\n\
				$\r$\n\
				!!! Proszę użyć wszystkich domyślnych opcji instalatora MiKTeX-a !!!'
${LangFileString} LatexError1 'Nie znaleziono dystrybucji LaTeX$\'a!$\r$\n\
				LyX nie może być użyty bez dystrybucji LaTeX-a takiej jak $\"MiKTeX$\"!$\r$\n\
				Z tego powodu instalacja zostanie przerwana.'

${LangFileString} HunspellFailed 'Pobranie słownika dla języka $\"$R3$\" nie powiodło się.'
${LangFileString} ThesaurusFailed 'Pobranie tezaurusa dla języka $\"$R3$\" nie powiodło się.'

#${LangFileString} JabRefInfo 'Teraz zostanie uruchomiony instalator aplikacji $\"JabRef$\".$\r$\n\
#				Możesz użyć wszystkich domyślnych opcji instalatora JabRef.'
#${LangFileString} JabRefError 'Aplikacja $\"JabRef$\" nie może być z powodzeniem zainstalowana!$\r$\n\
#				Instalator jednakże będzie kontynuował dalsze działanie..$\r$\n\
#				Spróbuj ponownie później zainstalować JabRef.'

#${LangFileString} LatexConfigInfo "Dalsza konfiguracja LyX$\'a chwilę potrwa."

#${LangFileString} MiKTeXPathInfo "Aby umożliwić każdemu z użytkowników późniejsze dostosowanie MikTeX do ich potrzeb$\r$\n\
#					niezbędne jest nadanie praw zapisu wszystkim użytkownikom od folderu instalacji MikTeX$\r$\n\
#					$MiKTeXPath $\r$\n\
#					i jego podfolderom."
${LangFileString} MiKTeXInfo 'Dystrybucja LaTeX $\"MikTeX$\" będzie używana razem z LyX.$\r$\n\
				Zaleca się instalację dostępnych aktualizacji MikTeX z użyciem aplikacji $\"MikTeX update wizard$\"$\r$\n\
				przed użyciem LyX po raz pierwszy.$\r$\n\
				Czy życzysz sobie sprawdzenia tych aktualizacji dla MikTeX teraz?'

${LangFileString} ModifyingConfigureFailed "Nieudana próba ustawienia zmiennej 'path_prefix' w pliku skryptu konfiguracyjnego"
#${LangFileString} RunConfigureFailed "Niedana próba wykonania skryptu konfiguracyjnego"
${LangFileString} NotAdmin "Musisz mieć prawa administratora aby zainstalować LyX$\'a!"
${LangFileString} InstallRunning "Instalator jest już uruchomiony!"
${LangFileString} AlreadyInstalled "LyX ${APP_SERIES_KEY2} jest już zainstalowany!$\r$\n\
				Installing over existing installations is not recommended if the installed version$\r$\n\
				is a test release or if you have problems with your existing LyX installation.$\r$\n\
				In these cases better reinstall LyX.$\r$\n\
				Dou you nevertheles want to install LyX over the existing version?"
${LangFileString} NewerInstalled "Próbujesz zainstalować starszą wersję LyX, niż ta która jest już zainstalowana.$\r$\n\
				  Jeżeli naprawdę chcesz tego dokonać, musisz wpierw odinstalować LyX $OldVersionNumber."

${LangFileString} MultipleIndexesNotAvailable "Wsparcie dla używania wielu indeksów w dokumencie nie będzie dostępne, ponieważ$\r$\n\
						MiKTeX został zainstalowany z uprawnieniami administratora, natomiast instalujesz LyX bez nich."

#${LangFileString} FinishPageMessage "Gratulacje! LyX został pomyślnie zainstalowany.$\r$\n\
#					$\r$\n\
#					(Pierwsze uruchomienie może potrwać kilka sekund.)"
${LangFileString} FinishPageRun "Uruchom LyX"

${LangFileString} UnNotInRegistryLabel "Nie można znaleźć LyX$\'a w rejestrze.$\r$\n\
					Skróty na pulpicie i w menu Start nie zostaną usunięte."
${LangFileString} UnInstallRunning "Musisz najpierw zamknąć LyX$\'a!"
${LangFileString} UnNotAdminLabel "Musisz posiadać prawa administratora do deinstalacji programu LyX."
${LangFileString} UnReallyRemoveLabel "Czy na pewno chcesz usunąć LyX$\'a i wszystkie jego komponenty?"
${LangFileString} UnLyXPreferencesTitle 'Preferencje użytkownika LyX$\'a'

${LangFileString} SecUnMiKTeXDescription "Deinstalacja MikTeX, dystrybucji LaTeX."
#${LangFileString} SecUnJabRefDescription "Deinstalacja zarządcy bibliografii JabRef."
${LangFileString} SecUnPreferencesDescription 'Usuwa folder konfiguracji LyX$\'a$\r$\n\
						$\"$AppPre\username\$\r$\n\
						$AppSuff\$\r$\n\
						${APP_DIR_USERDATA}$\")$\r$\n\
						wszystkim użytkownikom.'
${LangFileString} SecUnProgramFilesDescription "Deinstalacja LyX i wszystkich jego komponentów."

# MikTeX Update Stuff
${LangFileString} MiKTeXUpdateMsg "W efekcie zmian w MiKTeX, instalacja wersji LyX starszej niż 2.2.3 może powodować częściową aktualizację, która uczyni MiKTeX niesprawnym.$\r$\n\
$\r$\n\
Jeżeli jeszcze tego nie uczyniłeś, zalecamy aktualizację MiKTeX do najnowszej wersji przed instalacją nowej wersji LyX.$\r$\n\
$\r$\n\
Teraz albo możesz kontynuować instalację lub przerwać, aby móc zaktualizować MiKTeX."

${LangFileString} MiKTeXUpdateHeader "UWAGA: Zmiany w MiKTeX"
