/*
LyX Installer Language File
Language: French
Authors: Jean-Pierre Chrétien and Siegfried Meunier-Guttin-Cluzel
*/

!insertmacro LANGFILE_EXT "French"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installation pour l'utilisateur courant)"

${LangFileString} TEXT_WELCOME "Cet assistant va vous guider tout au long de l'installation de $(^NameDA).$\r$\n\
				$\r$\n\
				$_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Mise à jour de la base de données MiKTeX..."
${LangFileString} TEXT_CONFIGURE_LYX "Configuration de LyX ($LaTeXInstalled peut télécharger les paquetages manquants, ceci peut prendre du temps) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compilation des scripts Python..."

${LangFileString} TEXT_FINISH_DESKTOP "Créer un raccourci sur le bureau"
${LangFileString} TEXT_FINISH_WEBSITE "Consulter les dernières nouvelles, trucs et astuces sur le site lyx.org"

${LangFileString} FileTypeTitle "Document LyX"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Installer pour tous les utilisateurs ?"
${LangFileString} SecFileAssocTitle "Associations de fichiers"
${LangFileString} SecDesktopTitle "Icône du bureau"

${LangFileString} SecCoreDescription "Les fichiers LyX"
${LangFileString} SecInstJabRefDescription "Gestionnaire de références bibliographiques et éditeur de fichiers BibTeX."
${LangFileString} SecAllUsersDescription "Installer LyX pour tous les utilisateurs, ou seulement pour l$\'utilisateur courant ?"
${LangFileString} SecFileAssocDescription "Les fichiers de suffixe .lyx seront automatiquement ouverts dans LyX."
${LangFileString} SecDesktopDescription "Une icône LyX sur le bureau."
${LangFileString} SecDictionariesDescription "Les dictionnaires pour correcteur orthographique qui peuvent être téléchargés et installés."
${LangFileString} SecThesaurusDescription "Les dictionnaires de synonymes qui peuvent être téléchargés et installés."

${LangFileString} EnterLaTeXHeader1 'Distribution LaTeX'
${LangFileString} EnterLaTeXHeader2 'Choisir la distribution LaTeX que LyX devra utiliser.'
${LangFileString} EnterLaTeXFolder 'Éventuellement, vous pouvez fixer ici le chemin d$\'accès au fichier $\"latex.exe$\" et de ce fait fixer \
					la distribution LaTeX utilisée par Lyx.\r\n\
					Si vous n$\'utilisez pas LaTeX, LyX ne peut créer de documents !\r\n\
					\r\n\
					L$\'assistant d$\'installation a détecté la distribution LaTeX \
					$\"$LaTeXName$\" sur votre système. Le chemin d$\'accès est affiché ci-dessous.'
${LangFileString} EnterLaTeXFolderNone 'Fixez ci-dessous le chemin d$\'accès au fichier $\"latex.exe$\". De ce fait vous fixez la \
					distribution LaTeX utilisée par Lyx.\r\n\
					Si vous n$\'utilisez pas LaTeX, LyX ne peut créer de documents !\r\n\
					\r\n\
					L$\'assistant d$\'installation n$\'a pas pu trouver de distribution LaTeX sur votre système.'
${LangFileString} PathName 'Chemin vers le fichier $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "N'utilisez pas LaTeX"
${LangFileString} InvalidLaTeXFolder '$\"latex.exe$\" introuvable dans le chemin d$\'accès spécifié.'

${LangFileString} LatexInfo 'Lancement de l$\'installation de la distribution LaTeX $\"MiKTeX$\".$\r$\n\
				Pour installer l$\'application, appuyer sur le bouton $\"Next$\" de la fenêtre d$\'installation jusqu$\'à ce que celle-ci commence.$\r$\n\
				$\r$\n\
				!!! Prenez garde à utiliser toutes les options implicites du programme d$\'installation MikTeX !!!'
${LangFileString} LatexError1 'Distribution LaTeX introuvable !$\r$\n\
				LyX ne peut être utilisé sans distribution LaTeX, comme par exemple $\"MiKTeX$\" !$\r$\n\
				L$\'installation de LyX va donc être abandonnée.'

${LangFileString} HunspellFailed 'Le chargement du dictionnaire pour la langue $\"$R3$\" a échoué.'
${LangFileString} ThesaurusFailed 'Le chargement du dictionnaire de synonymes pour la langue $\"$R3$\" a échoué.'

${LangFileString} JabRefInfo 'Lancement de l$\'installation de l$\'application $\"JabRef$\".$\r$\n\
				Vous pouvez utiliser les options implicites du programme d$\'installation JabRef.'
${LangFileString} JabRefError 'L$\'application $\"JabRef$\" n$\'a pas pu être installée correctement !$\r$\n\
				L$\'installation de LyX va continuer.$\r$\n\
				Essayer d$\'installer JabRef de nouveau plus tard.'

${LangFileString} LatexConfigInfo "La configuration de LyX qui va suivre prendra un moment."

${LangFileString} MiKTeXPathInfo "Pour que chaque utilisateur soit capable de personnaliser MiKTeX pour ses besoins$\r$\n\
					il est nécessaire de positionner les permissions en écriture dans le répertoire d$\'installation de MiKTeX pour tous les utilisateurs$\r$\n\
					($MiKTeXPath) $\r$\n\
					et pour ses sous-répertoires."
${LangFileString} MiKTeXInfo 'La distribution LaTeX $\"MiKTeX$\" sera utilisée par LyX.$\r$\n\
				Il est recommandé d$\'installer les mises à jour en utilisant l$\'application $\"MiKTeX Update Wizard$\"$\r$\n\
				avant d$\'utiliser LyX pour la première fois.$\r$\n\
				Voulez-vous vérifier maintenant les mises à jour de MiKTeX ?'

${LangFileString} ModifyingConfigureFailed "Échec de l$\'allocation 'path_prefix' lors de la configuration."
${LangFileString} RunConfigureFailed "Échec de la tentative de configuration initiale de LyX."
${LangFileString} NotAdmin "Vous devez avoir les droits d$\'administration pour installer LyX !"
${LangFileString} InstallRunning "Le programme d$\'installation est toujours en cours !"
${LangFileString} AlreadyInstalled "LyX ${APP_SERIES_KEY2} est déjà installé !$\r$\n\
				L'installation par dessus les installations existantes n'est pas recommandée si la version à installer$\r$\n\
				est une version de test ou si vous avez des problèmes avec votre installation LyX existante.$\r$\n\
				Dans ces situations il vaut mieux réinstaller LyX.$\r$\n\
				Voulez-vous néanmoins installer LyX par dessus la version existante ?"
${LangFileString} NewerInstalled "Vous essayez d$\'installer une version de LyX plus ancienne que celle qui est déjà installée.$\r$\n\
				  Si c$\'est ce qu vous voulez, vous devez d$\'abord désinstaller LyX $OldVersionNumber."

${LangFileString} MultipleIndexesNotAvailable "La possibilité d$\'utiliser plusieurs indexes dans un même document ne sera pas disponible car$\r$\n\
						MiKTeX a été installé avec des privilèges d$\'administrateur et LyX a été istallé sans."

${LangFileString} FinishPageMessage "Félicitations ! LyX est installé avec succès.$\r$\n\
					$\r$\n\
					(Le premier démarrage de LyX peut demander quelques secondes.)"
${LangFileString} FinishPageRun "Démarrer LyX"

${LangFileString} UnNotInRegistryLabel "LyX introuvable dans la base des registres.$\r$\n\
					Les raccourcis sur le bureau et dans le menu de démarrage ne seront pas supprimés."
${LangFileString} UnInstallRunning "Vous devez fermer LyX d$\'abord !"
${LangFileString} UnNotAdminLabel "Vous devez avoir les droits d$\'administration pour désinstaller LyX !"
${LangFileString} UnReallyRemoveLabel "Êtes vous sûr(e) de vouloir supprimer complètement LyX et tous ses composants ?"
${LangFileString} UnLyXPreferencesTitle 'Préférences utilisateurs de LyX'

${LangFileString} SecUnMiKTeXDescription "Désinstalle la distribution LaTeX MiKTeX."
${LangFileString} SecUnJabRefDescription "Désinstalle le gestionnaire de bibliographie JabRef."
${LangFileString} SecUnPreferencesDescription 'Supprime le répertoire de configuration de LyX$\r$\n\
						$\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						pour tous les utilisateurs.'
${LangFileString} SecUnProgramFilesDescription "Désinstaller LyX et tous ses composants."

