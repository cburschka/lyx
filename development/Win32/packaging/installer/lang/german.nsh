/*

LyX Installer Language File
Language: German
Author: Andreas Deininger

*/

!insertmacro LanguageString TEXT_NO_PRIVILEDGES "Sie gehören weder der Gruppe der Administratoren noch der Gruppe der Hauptbenutzer an.$\r$\n$\r$\nSie können zwar ${APP_NAME} installieren und ebenso auch das Satzprogramm. Die Werkzeuge zur Grafikkonvertierung und für die Vorschau können jedoch ohne diese Rechte nicht installiert werden."

!insertmacro LanguageString TEXT_WELCOME_DOWNLOAD "Dieses Installationsprogramm wird Sie durch die Installation von $(^NameDA) leiten, des Dokumentenverarbeitungsprogramms welches das Verfassen von Texten basierend auf ihrer Struktur unterstützt, nicht das Verfassen von Texten ausgerichtet auf ihr Erscheinungsbild.\r\n\r\nDieses Installationsprogramm wird ein Komplettsystem zur Nutzung von ${APP_NAME} auf Ihrem Rechner erzeugen, dabei können fehlende Komponenten automatisch heruntergeladen werden.\r\n\r\n$(^ClickNext)"
!insertmacro LanguageString TEXT_WELCOME_INSTALL "Dieses Installationsprogramm wird Sie durch die Installation von $(^NameDA) leiten, des Dokumentenverarbeitungsprogramms welches das Verfassen von Texten basierend auf ihrer Struktur unterstützt, nicht das Verfassen von Texten ausgerichtet auf ihr Erscheinungsbild.\r\n\r\nDieses Installationsprogramm wird ein Komplettsystem zur Nutzung von ${APP_NAME} auf Ihrem Rechner erzeugen, dabei können fehlende Komponenten automatisch heruntergeladen werden.\r\n\r\n$(^ClickNext)"

!insertmacro LanguageString TEXT_USER_TITLE "Benutzer auswählen"
!insertmacro LanguageString TEXT_USER_SUBTITLE "Wählen Sie die Benutzer aus, für die Sie $(^NameDA) installieren wollen."
!insertmacro LanguageString TEXT_USER_INFO "Wählen Sie aus, ob Sie $(^NameDA) nur für den eigenen Gebrauch oder für die Nutzung durch alle Benutzer dieses Systems installieren möchten. $(^ClickNext)"
!insertmacro LanguageString TEXT_USER_CURRENT "Installation nur für mich"
!insertmacro LanguageString TEXT_USER_ALL "Installation für alle Benutzer dieses Computers"

!insertmacro LanguageString TEXT_REINSTALL_TITLE "Erneute Installation"
!insertmacro LanguageString TEXT_REINSTALL_SUBTITLE "Wählen Sie aus, ob Sie die $(^NameDA) Programmdateien erneut installieren möchten."
!insertmacro LanguageString TEXT_REINSTALL_INFO "$(^NameDA) ist bereits installiert. Das Installationsprogramm gestattet Ihnen die Spracheinstellungen der Benutzeroberfläche oder den Ort externer Anwendungen abzuändern, ferner können Sie zusätzliche Wörterbücher herunterladen. Falls Sie die Programmdateien erneut installieren möchten, markieren Sie das untenstehende Kästchen. $(^ClickNext)"
!insertmacro LanguageString TEXT_REINSTALL_ENABLE "$(^NameDA) Programmdateien erneut installieren"

!insertmacro LanguageString TEXT_EXTERNAL_NOPRIVILEDGES "(Rechte als Administrator oder Hauptbenutzer erforderlich)"

!insertmacro LanguageString TEXT_EXTERNAL_LATEX_TITLE "LaTeX Satzsystem"
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_SUBTITLE "Es wird ein Satzsystem benötigt, um Dokumente zum Druck oder zur Publikation erzeugen zu können."
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_INFO_DOWNLOAD "Bitte geben Sie an, ob sie MiKTeX herunterladen wollen oder ob Sie eine bereits existierendes LaTeX-System nutzen wollen. $(^ClickNext)"
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_INFO_INSTALL "Bitte geben Sie an, ob sie MiKTeX installieren wollen oder ob Sie eine bereits existierendes LaTeX -System nutzen wollen. $(^ClickNext)"
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_DOWNLOAD "MiKTeX herunterladen"
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_INSTALL "MiKTeX installieren"
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_FOLDER "Ein bereits existierendes LaTeX-System im folgenden Verzeichnis nutzen:"
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_FOLDER_INFO "Dieses Verzeichnis sollte ${BIN_LATEX} enthalten."
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_NONE "Installation ohne das LaTeX Satzsystem (nicht empfohlen)"
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_NOTFOUND "${BIN_LATEX} existiert nicht in dem von Ihnen angegebenen Verzeichnis."

!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_TITLE "ImageMagick"
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_SUBTITLE "ImageMagick wird benötigt, damit verschiedene Bildformate unterstützt werden können."
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_INFO_DOWNLOAD "Bitte geben Sie an, ob sie ImageMagick herunterladen wollen oder ob Sie es bereits installiert haben. $(^ClickNext)"
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_INFO_INSTALL "Bitte geben Sie an, ob sie ImageMagick installieren wollen oder ob Sie es bereits installiert haben. $(^ClickNext)"
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_DOWNLOAD "ImageMagick herunterladen"
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_INSTALL "ImageMagick installieren"
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_FOLDER "Eine bereits bestehende ImageMagick-Installation aus dem folgenden Verzeichnis nutzen:"
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_FOLDER_INFO "Dieses Verzeichnis sollte ${BIN_IMAGEMAGICK} enthalten."
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_NONE "Installation ohne ImageMagick (nicht empfohlen)"
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_NOTFOUND "${BIN_IMAGEMAGICK} existiert nicht in dem von Ihnen angegebenen Verzeichnis."

!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_TITLE "Ghostscript"
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_SUBTITLE "Ghostscript wird für die Vorschauansicht von Bildern im PostScript-Format benötigt."
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_INFO_DOWNLOAD "Bitte geben Sie an, ob sie Ghostscript herunterladen wollen oder ob Sie es bereits installiert haben. $(^ClickNext)"
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_INFO_INSTALL "Bitte geben Sie an, ob sie Ghostscript installieren wollen oder ob Sie es bereits installiert haben. $(^ClickNext)"
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_DOWNLOAD "Ghostscript herunterladen"
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_INSTALL "Ghostscript installieren"
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_FOLDER "Eine bereits bestehende Ghostscript-Installation aus dem folgenden Verzeichnis nutzen:"
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_FOLDER_INFO "Dieses Verzeichnis sollte ${BIN_GHOSTSCRIPT} enthalten."
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_NONE "Installation ohne Ghostscript (nicht empfohlen)"
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_NOTFOUND "${BIN_GHOSTSCRIPT} existiert nicht in dem von Ihnen angegebenen Verzeichnis."

!insertmacro LanguageString TEXT_VIEWER_TITLE "Dokumentenbetrachter"
!insertmacro LanguageString TEXT_VIEWER_SUBTITLE "Um von Ihnen verfasste Dokumente auf Ihrem Bildschirm anzuzeigen, wird ein Anzeigeprogramm benötigt."
!insertmacro LanguageString TEXT_VIEWER_INFO_DOWNLOAD "Auf Ihrem System ist kein Anzeigeprogramm für die Vorschauansicht von sowohl PDF- als auch Postscript-Dateien installiert, beides sind gebräuchliche Formate beim Dokumentensatz. Falls Sie das untenstehende Kästchen markieren, wird GSView heruntergeladen, ein Anzeigeprogramm für PDF- und Postscript-Dateien. $(^ClickNext)"
!insertmacro LanguageString TEXT_VIEWER_INFO_INSTALL "Auf Ihrem System ist kein Anzeigeprogramm für die Vorschauansicht von sowohl PDF- als auch Postscript-Dateien installiert, beides sind gebräuchliche Formate beim Dokumentensatz. Falls Sie das untenstehende Kästchen markieren, wird GSView installiert, ein Anzeigeprogramm für PDF- und Postscript-Dateien. $(^ClickNext)"
!insertmacro LanguageString TEXT_VIEWER_DOWNLOAD "GSView herunterladen"
!insertmacro LanguageString TEXT_VIEWER_INSTALL "GSView installieren"

!insertmacro LanguageString TEXT_DICT_TITLE "Wörterbücher zur Rechtschreibprüfung"
!insertmacro LanguageString TEXT_DICT_SUBTITLE "Wählen Sie die Sprachen aus, für die eine Rechtschreibprüfung möglich sein soll."
!insertmacro LanguageString TEXT_DICT_TOP "Um für Dokumente in einer bestimmten Sprache die Rechtschreibprüfung zu benutzen, muss ein Wörterbuch heruntergeladen werden. $_CLICK"
!insertmacro LanguageString TEXT_DICT_LIST "Wählen Sie die Wörterbucher aus, die Sie herunterladen möchten:"

!insertmacro LanguageString TEXT_LANGUAGE_TITLE "Wählen Sie die Sprache aus"
!insertmacro LanguageString TEXT_LANGUAGE_SUBTITLE "Wählen Sie die Sprache aus, in der Sie $(^NameDA) nutzen wollen."
!insertmacro LanguageString TEXT_LANGUAGE_INFO "Wählen Sie die Sprache aus, die Sie für die ${APP_NAME} Benutzeroberfläche nutzen wollen. $(^ClickNext)"

!insertmacro LanguageString TEXT_DOWNLOAD_FAILED_LATEX "Das Herunterladen von MiKTeX ist fehlgeschlagen. Möchten Sie es erneut versuchen?"
!insertmacro LanguageString TEXT_DOWNLOAD_FAILED_IMAGEMAGICK "Das Herunterladen von ImageMagick ist fehlgeschlagen. Möchten Sie es erneut versuchen?"
!insertmacro LanguageString TEXT_DOWNLOAD_FAILED_GHOSTSCRIPT "Das Herunterladen von Ghostscript ist fehlgeschlagen. Möchten Sie es erneut versuchen?"
!insertmacro LanguageString TEXT_DOWNLOAD_FAILED_VIEWER "Das Herunterladen von GSView ist fehlgeschlagen. Möchten Sie es erneut versuchen?"
!insertmacro LanguageString TEXT_DOWNLOAD_FAILED_DICT "Das Herunterladen von Wöterbüchern für die ${DICT_NAME} Rechtschreibprüfung ist fehlgeschlagen. Möchten Sie es erneut versuchen?"

!insertmacro LanguageString TEXT_NOTINSTALLED_LATEX "Die Installation von MiKTeX konnte nicht erfolgreich abgeschlossen werden. Möchten Sie MiKTeX erneut installieren?"
!insertmacro LanguageString TEXT_NOTINSTALLED_IMAGEMAGICK "Die Installation von ImageMagick konnte nicht erfolgreich abgeschlossen werden. Möchten Sie ImageMagick erneut installieren?"
!insertmacro LanguageString TEXT_NOTINSTALLED_GHOSTSCRIPT "Die Installation von Ghostscript konnte nicht erfolgreich abgeschlossen werden. Möchten Sie Ghostscript erneut installieren?"
!insertmacro LanguageString TEXT_NOTINSTALLED_VIEWER "GSView ist nicht für die Anzeige von PDF- oder Postscript-Dateien konfiguriert. Möchten Sie GSView erneut installieren?"
!insertmacro LanguageString TEXT_NOTINSTALLED_DICT "Die Installation der Wörterbucher für die ${DICT_NAME} Rechtschreibprüfung konnte nicht erfolgreich abgeschlossen werden. Möchten Sie die Installation erneut versuchen?"

!insertmacro LanguageString TEXT_FINISH_DESKTOP "Ein Symbol auf der Arbeitsoberfläche erzeugen"
!insertmacro LanguageString TEXT_FINISH_WEBSITE "Besuchen Sie lyx.org für aktuelle Neuigkeiten"

!insertmacro LanguageString UNTEXT_WELCOME "Dieses Installationsprogramm wird Sie durch die Deinstallation von $(^NameDA) leiten. Bitte schließen Sie $(^NameDA) bevor Sie fortfahren.\r\n\r\nBeachten Sie bitte, dass im folgenden nur LyX selbst deinstalliert wird. Falls Sie andere Anwendungen aus ihrem System entfernen möchten, die zur Nutzung von Lyx installiert wurden, benutzen Sie hierzu bitte den Eintrag Software im Menü Systemsteuerung und wählen Sie dort dann den entsprechenden Eintrag für das Programm aus.\r\n\r\n$_CLICK"
