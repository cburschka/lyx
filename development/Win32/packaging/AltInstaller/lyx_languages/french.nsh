!ifndef _LYX_LANGUAGES_FRENCH_NSH_
!define _LYX_LANGUAGES_FRENCH_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_FRENCH}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "Cet assistant vous guidera durant l'installation de LyX.\r\n\
					  \r\n\
					  Vous devez être administrateur pour installer LyX.\r\n\
					  \r\n\
					  Avant de débuter l'installation, il est recommandé de fermer toutes les autres applications. Cela permettra de mettre à jour certains fichiers système sans redémarrer votre ordinateur.\r\n\
					  \r\n\
					  $_CLICK"

LangString FileTypeTitle "${LYX_LANG}" "Document LyX"

LangString SecInstGSviewTitle "${LYX_LANG}" "GSview"
LangString SecInstJabRefTitle "${LYX_LANG}" "JabRef"
LangString SecAllUsersTitle "${LYX_LANG}" "Installer pour tous les utilisateurs ?"
LangString SecFileAssocTitle "${LYX_LANG}" "Associations de fichiers"
LangString SecDesktopTitle "${LYX_LANG}" "Icône du bureau"

LangString SecCoreDescription "${LYX_LANG}" "Les fichiers LyX"
LangString SecInstGSviewDescription "${LYX_LANG}" "Application permettant d'afficher les documents PostScript - et PDF."
LangString SecInstJabRefDescription "${LYX_LANG}" "Gestionnaire de références bibliographiques et éditeur de ficjiers BibTeX."
LangString SecAllUsersDescription "${LYX_LANG}" "Installer LyX pour tous les utilisateurs, ou seulement pour l'utilisateur courant ?."
LangString SecFileAssocDescription "${LYX_LANG}" "Les fichiers de suffixe .lyx seront automatiquement ouverts dans LyX."
LangString SecDesktopDescription "${LYX_LANG}" "Une icône LyX sur le bureau."

LangString LangSelectHeader "${LYX_LANG}" "Sélection de la langue pour les menus de LyX"
LangString AvailableLang "${LYX_LANG}" " Langues disponibles"

LangString MissProgHeader "${LYX_LANG}" "Vérification de la présence des applications requises."
LangString MissProgCap "${LYX_LANG}" "Le(s) application(s) requise(s) suivante(s) seront également installées."
LangString MissProgLatex "${LYX_LANG}" "MiKTeX, une distribution LaTeX"
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript, un interpréteur pour PostScript and PDF"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick, un convertisseur graphique"
LangString MissProgAspell "${LYX_LANG}" "Aspell, un correcteur orthographique"
LangString MissProgMessage "${LYX_LANG}" 'Aucune application supplémentaire ne doit être installée.'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'Distribution LaTeX'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'Choisir la distribution LaTeX que LyX devra utiliser.'
LangString EnterLaTeXFolder "${LYX_LANG}" 'Éventuellement, vous pouvez fixer ici le chemin d$\'accès au fichier "latex.exe" et de ce fait fixer \
					   la distribution LaTeX utilisée par Lyx.\r\n\
					   Si vous n$\'utilisez pas LaTeX, LyX ne peut créer de documents !\r\n\
					   \r\n\
					   L$\'assistant d$\'installation a détecté la distribution LaTeX \
					   "$LaTeXName" sur votre système. Le chemin d$\'accès est affiché ci-dessous.'
LangString EnterLaTeXFolderNone "${LYX_LANG}" 'Fixez ci-dessous le chemin d$\'accès au fichier "latex.exe". De ce fait vous fixez la \
					       distribution LaTeX utilisée par Lyx.\r\n\
					       Si vous n$\'utilisez pas LaTeX, LyX ne peut créer de documents !\r\n\
					       \r\n\
					       L$\'assistant d$\'installation n$\'a pas pu trouver de distribution LaTeX sur votre système.'
LangString PathName "${LYX_LANG}" 'Chemin vers le fichier "latex.exe"'
LangString DontUseLaTeX "${LYX_LANG}" "N'utilisez pas LaTeX"
LangString InvalidLaTeXFolder "${LYX_LANG}" '"latex.exe" introuvable dans le chemin d$\'accès spécifié.'

LangString LatexInfo "${LYX_LANG}" 'Lancement de l$\'installation de la distribution LaTeX "MiKTeX".$\r$\n\
			            Pour installer l$\'application, appuyer sur le bouton "Next" de la fenêtre d$\'installation jusqu$\'à ce que celle-ci commence.$\r$\n\
				    $\r$\n\
				    !!! Prenez garde à utiliser toutes les options implicites du programme d$\'installation MikTeX !!!'
LangString LatexError1 "${LYX_LANG}" 'Distribution LaTeX introuvable !$\r$\n\
                      		      LyX ne peut être utilisé sans distribution LaTeX, comme par exemple "MiKTeX" !$\r$\n\
				      L$\'installation de LyX va donc être abandonnée.'
			    
LangString GSviewInfo "${LYX_LANG}" 'Lancement de l$\'installation de l$\'application "GSview".$\r$\n\
			             Pour installer l$\'application, appuyer sur le bouton "Setup" de la première fenêtre d$\'installation$\r$\n\
				     choisir une langue, puis appuyer sur le bouton "Next" de la fenêtre d$\'installation suivante.$\r$\n\
				     Vous pouvez utiliser les options implicites du programme d$\'installation GSview.'
LangString GSviewError "${LYX_LANG}" 'L$\'application "GSview" n$\'a pas pu être installée correctement !$\r$\n\
		                      L$\'installation de LyX va continuer.$\r$\n\
				      Essayer d$\'installer GSview de nouveau plus tard.'

LangString JabRefInfo "${LYX_LANG}" 'Lancement de l$\'installation de l$\'application "JabRef".$\r$\n\
				     Vous pouvez utiliser les options implicites du programme d$\'installation JabRef.'
LangString JabRefError "${LYX_LANG}" 'L$\'application "JabRef" n$\'a pas pu être installée correctement !$\r$\n\
		                      L$\'installation de LyX va continuer.$\r$\n\
				      Essayer d$\'installer JabRef de nouveau plus tard.'
				      
LangString LatexConfigInfo "${LYX_LANG}" "La configuration de LyX qui va suivre prendra un moment."

LangString AspellInfo "${LYX_LANG}" 'Maintenant les dictionnaires du correcteur orthographique "Aspell" vont être téléchargés et installés.$\r$\n\
				     Chaque dictionnaire est doté d$\'une licence différente qui  sera affichée avant l$\'installation.'
LangString AspellDownloadFailed "${LYX_LANG}" "Aucun dictionnaire Aspell n$\'a pu être téléchargé !"
LangString AspellInstallFailed "${LYX_LANG}" "Aucun dictionnaire Aspell n$\'a pu être installé !"
LangString AspellPartAnd "${LYX_LANG}" " et "
LangString AspellPartSep "${LYX_LANG}" ", "
LangString AspellAll "${LYX_LANG}" 'Un dictionnaire est déjà installé pour la langue'
LangString AspellNone "${LYX_LANG}" 'Aucun dictionnaire pour le correcteur orthographique Aspell n$\'a pu être installé.$\r$\n\
				     Les dictionnaires peuvent être téléchargés depuis$\r$\n\
				     ${AspellLocation}$\r$\n\
				     Voulez-vous télécharger les dictionnaires maintenant ?'
LangString AspellPartStart "${LYX_LANG}" "Ont été installés avec succès "
LangString AspellPart1 "${LYX_LANG}" "un dictionnaire anglophone"
LangString AspellPart2 "${LYX_LANG}" "un dictionnaire pour la langue $LangName"
LangString AspellPart3 "${LYX_LANG}" "un$\r$\n\
				      dictionnaire pour la langue $LangNameSys"
LangString AspellPart4 "${LYX_LANG}" ' pour le correcteur orthographique "Aspell".$\r$\n\
				      D$\'autres dictionnaires peuvent être téléchargés depuis$\r$\n\
				      ${AspellLocation}$\r$\n\
				      Voulez-vous télécharger d$\'autres dictionnaires maintenant ?'

LangString MiKTeXPathInfo "${LYX_LANG}" "Pour que chaque utilisateur soit capable de personnaliser MiKTeX pour ses besoins$\r$\n\
					 il est nécessaire de positionner les permissions en écriture dans le répertoire d$\'installation de MiKTeX pour tous les utilisateurs$\r$\n\
					 ($MiKTeXPath) $\r$\n\
					 et pour ses sous-répertoires."
LangString MiKTeXInfo "${LYX_LANG}" 'La distribution LaTeX "MiKTeX" sera utilisée par LyX.$\r$\n\
				     Il est recommandé d$\'installer les mises à jour en utilisant l$\'application "MiKTeX Update Wizard"$\r$\n\
				     avant d$\'utiliser LyX pour la première fois.$\r$\n\
				     Voulez-vous vérifier maintenant les mises à jour de MiKTeX ?'

LangString UpdateNotAllowed "${LYX_LANG}" "Ce paquetage ne peut mettre à jour que ${PRODUCT_VERSION_OLD}!"
LangString ModifyingConfigureFailed "${LYX_LANG}" "Échec de l'allocation 'path_prefix' lors de la configuration."
LangString CreateCmdFilesFailed "${LYX_LANG}" "Création de lyx.bat impossible."
LangString RunConfigureFailed "${LYX_LANG}" "Échec de la tentative de configuration initiale de LyX."
LangString NotAdmin "${LYX_LANG}" "Vous devez avoir les droits d'administration pour installer LyX !"
LangString InstallRunning "${LYX_LANG}" "Le programme d'installation est toujours en cours !"
LangString StillInstalled "${LYX_LANG}" "LyX est déjà installé ! Le désinstaller d'abord."

LangString FinishPageMessage "${LYX_LANG}" "Félicitations ! LyX est installé avec succès.\r\n\
					    \r\n\
					    (Le premier démarrage de LyX peut demander quelques secondes.)"
LangString FinishPageRun "${LYX_LANG}" "Démarrer LyX"

LangString UnNotInRegistryLabel "${LYX_LANG}" "LyX introuvable dans le base des registres.$\r$\n\
					       Les raccourcis sur le bureau et dans le menu de démarrage ne seront pas supprimés."
LangString UnInstallRunning "${LYX_LANG}" "Vous devez fermer LyX d'abord !"
LangString UnNotAdminLabel "${LYX_LANG}" "Vous devez avoir les droits d'administration pour désinstaller LyX !"
LangString UnReallyRemoveLabel "${LYX_LANG}" "Êtes vous sûr(e) de vouloir supprimer complètement LyX et tous ses composants ?"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'Préférences utilisateurs de LyX'
LangString UnGSview "${LYX_LANG}" 'Merci d$\'appuyer sur le bouton "Désinstaller" de la prochaine fenêtre pour désinstaller.$\r$\n\
				   l$\'afficheur Postscript/PDF "GSview".'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "LyX supprimé de votre ordinateur avec succès."

LangString SecUnAspellDescription "${LYX_LANG}" "Désinstalle le correcteur orthographique Aspell et tous ses dictionnaires."
LangString SecUnMiKTeXDescription "${LYX_LANG}" "Désinstalle la distribution LaTeX MiKTeX."
LangString SecUnJabRefDescription "${LYX_LANG}" "Uninstalls the bibliography manager JabRef."
LangString SecUnPreferencesDescription "${LYX_LANG}" 'Supprime le répertoire de configuration de LyX$\r$\n\
					   "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
					   pour tous les utilisateurs.'
LangString SecUnProgramFilesDescription "${LYX_LANG}" "Désinstaller LyX et tous ses composants."

!undef LYX_LANG

!endif # _LYX_LANGUAGES_FRENCH_NSH_
