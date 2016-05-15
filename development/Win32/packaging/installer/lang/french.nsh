/*
LyX Installer Language File
Language: French
Authors: Jean-Pierre Chr�tien and Siegfried Meunier-Guttin-Cluzel
*/

!insertmacro LANGFILE_EXT "French"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installation pour l'utilisateur courant)"

${LangFileString} TEXT_WELCOME "Cet assistant va vous guider tout au long de l'installation de $(^NameDA).$\r$\n\
				$\r$\n\
				$_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Mise � jour de la base de donn�es MiKTeX..."
${LangFileString} TEXT_CONFIGURE_LYX "Configuration de LyX ($LaTeXInstalled peut t�l�charger les paquetages manquants, ceci peut prendre du temps) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compilation des scripts Python..."

${LangFileString} TEXT_FINISH_DESKTOP "Cr�er un raccourci sur le bureau"
${LangFileString} TEXT_FINISH_WEBSITE "Consulter les derni�res nouvelles, trucs et astuces sur le site lyx.org"

${LangFileString} FileTypeTitle "Document LyX"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Installer pour tous les utilisateurs ?"
${LangFileString} SecFileAssocTitle "Associations de fichiers"
${LangFileString} SecDesktopTitle "Ic�ne du bureau"

${LangFileString} SecCoreDescription "Les fichiers LyX"
${LangFileString} SecInstJabRefDescription "Gestionnaire de r�f�rences bibliographiques et �diteur de fichiers BibTeX."
${LangFileString} SecAllUsersDescription "Installer LyX pour tous les utilisateurs, ou seulement pour l$\'utilisateur courant ?"
${LangFileString} SecFileAssocDescription "Les fichiers de suffixe .lyx seront automatiquement ouverts dans LyX."
${LangFileString} SecDesktopDescription "Une ic�ne LyX sur le bureau."
${LangFileString} SecDictionariesDescription "Les dictionnaires pour correcteur orthographique qui peuvent �tre t�l�charg�s et install�s."
${LangFileString} SecThesaurusDescription "Les dictionnaires de synonymes qui peuvent �tre t�l�charg�s et install�s."

${LangFileString} EnterLaTeXHeader1 'Distribution LaTeX'
${LangFileString} EnterLaTeXHeader2 'Choisir la distribution LaTeX que LyX devra utiliser.'
${LangFileString} EnterLaTeXFolder '�ventuellement, vous pouvez fixer ici le chemin d$\'acc�s au fichier $\"latex.exe$\" et de ce fait fixer \
					la distribution LaTeX utilis�e par Lyx.\r\n\
					Si vous n$\'utilisez pas LaTeX, LyX ne peut cr�er de documents !\r\n\
					\r\n\
					L$\'assistant d$\'installation a d�tect� la distribution LaTeX \
					$\"$LaTeXName$\" sur votre syst�me. Le chemin d$\'acc�s est affich� ci-dessous.'
${LangFileString} EnterLaTeXFolderNone 'Fixez ci-dessous le chemin d$\'acc�s au fichier $\"latex.exe$\". De ce fait vous fixez la \
					distribution LaTeX utilis�e par Lyx.\r\n\
					Si vous n$\'utilisez pas LaTeX, LyX ne peut cr�er de documents !\r\n\
					\r\n\
					L$\'assistant d$\'installation n$\'a pas pu trouver de distribution LaTeX sur votre syst�me.'
${LangFileString} PathName 'Chemin vers le fichier $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "N'utilisez pas LaTeX"
${LangFileString} InvalidLaTeXFolder '$\"latex.exe$\" introuvable dans le chemin d$\'acc�s sp�cifi�.'

${LangFileString} LatexInfo 'Lancement de l$\'installation de la distribution LaTeX $\"MiKTeX$\".$\r$\n\
				Pour installer l$\'application, appuyer sur le bouton $\"Next$\" de la fen�tre d$\'installation jusqu$\'� ce que celle-ci commence.$\r$\n\
				$\r$\n\
				!!! Prenez garde � utiliser toutes les options implicites du programme d$\'installation MikTeX !!!'
${LangFileString} LatexError1 'Distribution LaTeX introuvable !$\r$\n\
				LyX ne peut �tre utilis� sans distribution LaTeX, comme par exemple $\"MiKTeX$\" !$\r$\n\
				L$\'installation de LyX va donc �tre abandonn�e.'

${LangFileString} HunspellFailed 'Le chargement du dictionnaire pour la langue $\"$R3$\" a �chou�.'
${LangFileString} ThesaurusFailed 'Le chargement du dictionnaire de synonymes pour la langue $\"$R3$\" a �chou�.'

${LangFileString} JabRefInfo 'Lancement de l$\'installation de l$\'application $\"JabRef$\".$\r$\n\
				Vous pouvez utiliser les options implicites du programme d$\'installation JabRef.'
${LangFileString} JabRefError 'L$\'application $\"JabRef$\" n$\'a pas pu �tre install�e correctement !$\r$\n\
				L$\'installation de LyX va continuer.$\r$\n\
				Essayer d$\'installer JabRef de nouveau plus tard.'

${LangFileString} LatexConfigInfo "La configuration de LyX qui va suivre prendra un moment."

${LangFileString} MiKTeXPathInfo "Pour que chaque utilisateur soit capable de personnaliser MiKTeX pour ses besoins$\r$\n\
					il est n�cessaire de positionner les permissions en �criture dans le r�pertoire d$\'installation de MiKTeX pour tous les utilisateurs$\r$\n\
					($MiKTeXPath) $\r$\n\
					et pour ses sous-r�pertoires."
${LangFileString} MiKTeXInfo 'La distribution LaTeX $\"MiKTeX$\" sera utilis�e par LyX.$\r$\n\
				Il est recommand� d$\'installer les mises � jour en utilisant l$\'application $\"MiKTeX Update Wizard$\"$\r$\n\
				avant d$\'utiliser LyX pour la premi�re fois.$\r$\n\
				Voulez-vous v�rifier maintenant les mises � jour de MiKTeX ?'

${LangFileString} ModifyingConfigureFailed "�chec de l$\'allocation 'path_prefix' lors de la configuration."
${LangFileString} RunConfigureFailed "�chec de la tentative de configuration initiale de LyX."
${LangFileString} NotAdmin "Vous devez avoir les droits d$\'administration pour installer LyX !"
${LangFileString} InstallRunning "Le programme d$\'installation est toujours en cours !"
${LangFileString} AlreadyInstalled "LyX ${APP_SERIES_KEY2} est d�j� install� !$\r$\n\
				Installing over existing installations is not recommended if the installed version$\r$\n\
				is a test release or if you have problems with your existing LyX installation.$\r$\n\
				In these cases better reinstall LyX.$\r$\n\
				Dou you nevertheles want to install LyX over the existing version ?"
${LangFileString} NewerInstalled "Vous essayez d$\'installer une version de LyX plus ancienne que celle qui est d�j� install�e.$\r$\n\
				  Si c$\'est ce qu vous voulez, vous devez d$\'abord d�sinstaller LyX $OldVersionNumber."

${LangFileString} MultipleIndexesNotAvailable "La possibilit� d$\'utiliser plusieurs indexes dans un m�me document ne sera pas disponible car$\r$\n\
						MiKTeX a �t� install� avec des privil�ges d$\'administrateur et LyX a �t� istall� sans."
${LangFileString} MetafileNotAvailable "La prise en compte par LyX d$\'images au format EMF ou WMF ne sera pas disponible$\r$\n\
					car elle necessite l$\'installation dans Windows d$\'une imprimante logicielle, $\r$\n\
					ce qui n$\'est possible qu$\'avec des privil�ges d$\'administrateur."

${LangFileString} FinishPageMessage "F�licitations ! LyX est install� avec succ�s.$\r$\n\
					$\r$\n\
					(Le premier d�marrage de LyX peut demander quelques secondes.)"
${LangFileString} FinishPageRun "D�marrer LyX"

${LangFileString} UnNotInRegistryLabel "LyX introuvable dans la base des registres.$\r$\n\
					Les raccourcis sur le bureau et dans le menu de d�marrage ne seront pas supprim�s."
${LangFileString} UnInstallRunning "Vous devez fermer LyX d$\'abord !"
${LangFileString} UnNotAdminLabel "Vous devez avoir les droits d$\'administration pour d�sinstaller LyX !"
${LangFileString} UnReallyRemoveLabel "�tes vous s�r(e) de vouloir supprimer compl�tement LyX et tous ses composants ?"
${LangFileString} UnLyXPreferencesTitle 'Pr�f�rences utilisateurs de LyX'

${LangFileString} SecUnMiKTeXDescription "D�sinstalle la distribution LaTeX MiKTeX."
${LangFileString} SecUnJabRefDescription "D�sinstalle le gestionnaire de bibliographie JabRef."
${LangFileString} SecUnPreferencesDescription 'Supprime le r�pertoire de configuration de LyX$\r$\n\
						$\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						pour tous les utilisateurs.'
${LangFileString} SecUnProgramFilesDescription "D�sinstaller LyX et tous ses composants."

