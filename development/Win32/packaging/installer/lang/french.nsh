/*

LyX Installer Language File
Language: French
Author: Jean-Pierre Chrétien

*/

!insertmacro LanguageString TEXT_NO_PRIVILEDGES "Vous n'avez pas les privilèges d'administrateur.$\r$\n$\r$\nBien que vous puissiez installer ${APP_NAME} et le système typographique, les utilitaires de conversion de format graphique et de prévisualisation ne peuvent être installés sans ces privilèges."

!insertmacro LanguageString TEXT_WELCOME_DOWNLOAD "Cet assistant va vous guider tout au long de l'installation de $(^NameDA), le constructeur de documents qui encourage une saisie fondée sur la structure du document plutôt que sur son apparence.\r\n\r\nCet assistant d'installation va paramétrer un environnement complet d'utilisation de ${APP_NAME} et peut télécharger automatiquement les composants manquants.\r\n\r\n$(^ClickNext)"
!insertmacro LanguageString TEXT_WELCOME_INSTALL "Cet assistant va vous guider tout au long de l'installation de $(^NameDA), le constructeur de documents qui encourage une saisie fondée sur la structure du document plutôt que sur son apparence.\r\n\r\nCet assistant d'installation va paramétrer un environnement complet d'utilisation de ${APP_NAME} et peut installer automatiquement les composants manquants.\r\n\r\n$(^ClickNext)"

!insertmacro LanguageString TEXT_USER_TITLE "Choix des utilisateurs"
!insertmacro LanguageString TEXT_USER_SUBTITLE "Sélection des utilisateurs désirant utiliser $(^NameDA)."
!insertmacro LanguageString TEXT_USER_INFO "Choix entre installer $(^NameDA) seulement pour vous-même  ou bien pour tous les utilisateurs du système. $(^ClickNext)"
!insertmacro LanguageString TEXT_USER_CURRENT "Installer seulement pour moi."
!insertmacro LanguageString TEXT_USER_ALL "Installer pour tous les utilisateurs."

!insertmacro LanguageString TEXT_REINSTALL_TITLE "Réinstallation"
!insertmacro LanguageString TEXT_REINSTALL_SUBTITLE "Réinstallation éventuelle des fichiers programmes de $(^NameDA) ?"
!insertmacro LanguageString TEXT_REINSTALL_INFO "$(^NameDA) est déjà installé. Cet assistant vous permettra de modifier le langage de l'interface ou l'emplacement des applications externes, ou de télécharger de nouveaux dictionnaires. Si vous souhaitez réinstaller aussi les fichiers programmes, cochez la case ci-dessous. $(^ClickNext)"
!insertmacro LanguageString TEXT_REINSTALL_ENABLE "Réinstaller les fichiers programmes $(^NameDA)"

!insertmacro LanguageString TEXT_EXTERNAL_NOPRIVILEDGES "(Privilèges d'administration requis)"

!insertmacro LanguageString TEXT_EXTERNAL_LATEX_TITLE "Système typographique LaTeX"
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_SUBTITLE "Un système typographique est requis pour pouvoir créer des documents imprimables ou publiables."
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_INFO_DOWNLOAD "Merci de préciser si vous désirez télécharger MikTeX ou si vous voulez utiliser une distribution LaTeX existante. $(^ClickNext)"
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_INFO_INSTALL "Merci de préciser si vous désirez installer MikTeX ou si vous voulez utiliser une distribution LaTeX existante. $(^ClickNext)"
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_DOWNLOAD "Télécharger MikTeX"
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_INSTALL "Installer MikTeX"
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_FOLDER "Utiliser une distribution LaTeX existante dans le répertoire suivant :"
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_FOLDER_INFO "Ce répertoire doit contenir ${BIN_LATEX}."
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_NONE "Installer sans distribution LaTeX (non recommandé)"
!insertmacro LanguageString TEXT_EXTERNAL_LATEX_NOTFOUND "${BIN_LATEX} n'existe pas dans le répertoire spécifié."

!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_TITLE "ImageMagick"
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_SUBTITLE "ImageMagick est requis pour les conversions de formats graphiques variés."
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_INFO_DOWNLOAD "Merci de préciser si vous désirez télécharger ImageMagick ou si vous en disposez déjà. $(^ClickNext)"
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_INFO_INSTALL "Merci de préciser si vous désirez installer ImageMagick ou si vous en disposez déjà. $(^ClickNext)"
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_DOWNLOAD "Télécharger ImageMagick"
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_INSTALL "Installer ImageMagick"
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_FOLDER "Utiliser une installation ImageMagick existante dans le répertoire suivant :"
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_FOLDER_INFO "Ce répertoire doit contenir ${BIN_IMAGEMAGICK}."
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_NONE "Installer sans ImageMagick (non recommandé)"
!insertmacro LanguageString TEXT_EXTERNAL_IMAGEMAGICK_NOTFOUND "${BIN_IMAGEMAGICK} n'existe pas dans le répertoire spécifié."

!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_TITLE "Ghostscript"
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_SUBTITLE "Ghostscript est requis pour prévisualiser les documents au format PostScript."
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_INFO_DOWNLOAD "Merci de préciser si vous désirez télécharger Ghostscript ou si vous en disposez déjà. $(^ClickNext)"
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_INFO_INSTALL "Merci de préciser si vous désirez installer Ghostscript ou si vous en disposez déjà. $(^ClickNext)"
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_DOWNLOAD "Télécharger Ghostscript"
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_INSTALL "Installer Ghostscript"
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_FOLDER "Utiliser une installation Ghostscript existante dans le répertoire suivant :"
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_FOLDER_INFO "Ce répertoire doit contenir ${BIN_GHOSTSCRIPT}."
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_NONE "Installer sans GhostScript (non recommandé)"
!insertmacro LanguageString TEXT_EXTERNAL_GHOSTSCRIPT_NOTFOUND "${BIN_GHOSTSCRIPT} n'existe pas dans le répertoire spécifié."

!insertmacro LanguageString TEXT_VIEWER_TITLE "Visualisation des documents"
!insertmacro LanguageString TEXT_VIEWER_SUBTITLE "Un afficheur est requis pour présenter les résultats des traitements à l'écran."
!insertmacro LanguageString TEXT_VIEWER_INFO_DOWNLOAD "Il n'y a pas d'afficheur installé sur votre système pour prévisualiser les fichiers PDF et PostScript , formats courants de la version typographique des documents. Si vous cochez la case ci-dessous, l'afficheur PDF/PostScript Gaviez sera téléchargé. $(^ClickNext)"
!insertmacro LanguageString TEXT_VIEWER_INFO_INSTALL "Il n'y a pas d'afficheur installé sur votre système pour prévisualiser les fichiers PDF et PostScript , formats courants de la version typographique des documents. Si vous cochez la case ci-dessous, l'afficheur PDF/PostScript GSView sera installé. $(^ClickNext)"
!insertmacro LanguageString TEXT_VIEWER_DOWNLOAD "Télécharger GSView"
!insertmacro LanguageString TEXT_VIEWER_INSTALL "Installer GSView"

!insertmacro LanguageString TEXT_DICT_TITLE "Dictionnaires orthographiques"
!insertmacro LanguageString TEXT_DICT_SUBTITLE "Sélectionner les langues pour lesquelles vous désirez un dictionnaire orthographique."
!insertmacro LanguageString TEXT_DICT_TOP "Pour vérifier l'orthographe des documents dans un langage donné, un dictionnaire contenant les mots admis dot être téléchargé. Sélectionner les dictionnaires à télécharger. $_CLICK"
!insertmacro LanguageString TEXT_DICT_LIST "Sélectionner les dictionnaires à télécharger :"

!insertmacro LanguageString TEXT_LANGUAGE_TITLE "Choix de la langue"
!insertmacro LanguageString TEXT_LANGUAGE_SUBTITLE "Choix de la langue de l'interface $(^NameDA)."
!insertmacro LanguageString TEXT_LANGUAGE_INFO "Sélectionner la langue à utiliser dans l'interface ${APP_NAME}. $(^ClickNext)"

!insertmacro LanguageString TEXT_DOWNLOAD_FAILED_LATEX "Le téléchargement de MikTeX a échoué. Voulez-vous réessayer ?"
!insertmacro LanguageString TEXT_DOWNLOAD_FAILED_IMAGEMAGICK "Le téléchargement de ImageMagick a échoué. Voulez-vous réessayer ?"
!insertmacro LanguageString TEXT_DOWNLOAD_FAILED_GHOSTSCRIPT "Le téléchargement de Ghostscript a échoué. Voulez-vous réessayer ?"
!insertmacro LanguageString TEXT_DOWNLOAD_FAILED_VIEWER "Le téléchargement de GSView a échoué. Voulez-vous réessayer ?"
!insertmacro LanguageString TEXT_DOWNLOAD_FAILED_DICT "Le téléchargement du dictionnaire orthographique ${DICT_NAME} a échoué. Voulez-vous réessayer ?"

!insertmacro LanguageString TEXT_NOTINSTALLED_LATEX "L'installation de MikTeX est restée incomplète. Voulez-vous relancer l'installation de MikTeX ?"
!insertmacro LanguageString TEXT_NOTINSTALLED_IMAGEMAGICK "L'installation de ImageMagick est restée incomplète. Voulez-vous relancer l'installation de ImageMagick ?"
!insertmacro LanguageString TEXT_NOTINSTALLED_GHOSTSCRIPT "L'installation de Ghostscript est restée incomplète. Voulez-vous relancer l'installation de Ghostscript ?"
!insertmacro LanguageString TEXT_NOTINSTALLED_VIEWER "Vous n'avez pas associé GSView aux types de fichier PDF/PostScript. Voulez-vous relancer l'installation de GSView ?"
!insertmacro LanguageString TEXT_NOTINSTALLED_DICT "L'installation du dictionnaire orthographique ${DICT_NAME} est restée incomplète. Voulez-vous relancer l'installation ?"

!insertmacro LanguageString TEXT_FINISH_DESKTOP "Créer un raccourci sur le bureau"
!insertmacro LanguageString TEXT_FINISH_WEBSITE "Consulter les dernières nouvelles, trucs et astuces sur le site lyx.org"

!insertmacro LanguageString UNTEXT_WELCOME "Cet assistant va vous guider tout au long de la désinstallation de $(^NameDA). Merci de quitter $(^NameDA) avant de poursuivre.\r\n\r\nNotez bien que cet assistant ne désinstallera que LyX proprement dit. Si vous désirez désinstaller d'autres applications que vous avez installées pour les besoins de $(^NameDA), effectuez cette désinstallation via la page « Ajout/Suppression de programmes ».\r\n\r\n$_CLICK"
