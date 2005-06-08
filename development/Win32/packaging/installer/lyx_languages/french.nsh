
!ifndef _LYX_LANGUAGES_FRENCH_NSH_
!define _LYX_LANGUAGES_FRENCH_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_FRENCH}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString SecAllUsersTitle "${LYX_LANG}" "Installer pour tous les utilisateurs ?"
LangString SecFileAssocTitle "${LYX_LANG}" "Associations de fichiers"
LangString SecDesktopTitle "${LYX_LANG}" "Icône du bureau"

LangString SecCoreDescription "${LYX_LANG}" "Les fichiers ${PRODUCT_NAME}"
LangString SecAllUsersDescription "${LYX_LANG}" "Installer pour tous les utilisateurs (nécessite les droits d'administrateur), ou seulement pour l'utilisateur courant ?."
LangString SecFileAssocDescription "${LYX_LANG}" "Crée les associations entre l'exécutable et le suffixe .lyx."
LangString SecDesktopDescription "${LYX_LANG}" "Une icône ${PRODUCT_NAME} sur le bureau."

LangString ModifyingConfigureFailed "${LYX_LANG}" "Échec de l'allocation 'path_prefix' lors de la configuration."
LangString RunConfigureFailed "${LYX_LANG}" "Échec de la tentative de configuration initiale de LyX."

LangString FinishPageMessage "${LYX_LANG}" "Félicitations ! LyX est installé avec succès."
LangString FinishPageRun "${LYX_LANG}" "Démarrer LyX"

LangString DownloadPageField2 "${LYX_LANG}" "&Ne pas installer"

LangString MinSYSHeader "${LYX_LANG}" "MinSYS"
LangString MinSYSDescription "${LYX_LANG}" "MinSYS est un environnement minimal pour gérer des commandes (www.mingw.org/msys.shtml) dont ${PRODUCT_NAME} a besoin pour ses traitements."
LangString EnterMinSYSFolder "${LYX_LANG}" "Merci de saisir le chemin conduisant au répertoire où se trouve sh.exe"
LangString InvalidMinSYSFolder "${LYX_LANG}" "sh.exe introuvable"
LangString MinSYSDownloadLabel "${LYX_LANG}" "&Télécharger MinSYS"
LangString MinSYSFolderLabel "${LYX_LANG}" "&Répertoire contenant sh.exe"

LangString PythonHeader "${LYX_LANG}" "Python"
LangString PythonDescription "${LYX_LANG}" "Le langage de commandes Python (www.python.org) doit être installé ou ${PRODUCT_NAME} ne pourra pas exécuter un certain nombre de commandes."
LangString EnterPythonFolder "${LYX_LANG}" "Merci de saisir le chemin conduisant au répertoire où se trouve Python.exe"
LangString InvalidPythonFolder "${LYX_LANG}" "Python.exe introuvable"
LangString PythonDownloadLabel "${LYX_LANG}" "&Télécharger Python"
LangString PythonFolderLabel "${LYX_LANG}" "&Répertoire contenant Python.exe"

LangString MiKTeXHeader "${LYX_LANG}" "MiKTeX"
LangString MiKTeXDescription "${LYX_LANG}" "MiKTeX (www.miktex.org) est une distribution moderne de TeX pour Windows."
LangString EnterMiKTeXFolder "${LYX_LANG}" "Merci de saisir le chemin conduisant au répertoire où se trouve latex.exe"
LangString InvalidMiKTeXFolder "${LYX_LANG}" "latex.exe introuvable"
LangString MiKTeXDownloadLabel "${LYX_LANG}" "&Télécharger MiKTeX"
LangString MiKTeXFolderLabel "${LYX_LANG}" "&Répertoire contenant latex.exe"

LangString PerlHeader "${LYX_LANG}" "Perl"
LangString PerlDescription "${LYX_LANG}" "Si vous avez l'intention d'utiliser reLyX pour convertir des documents LaTeX en LyX, vous devez installer Perl (www.perl.com)."
LangString EnterPerlFolder "${LYX_LANG}" "Merci de saisir le chemin conduisant au répertoire où se trouve Perl.exe"
LangString InvalidPerlFolder "${LYX_LANG}" "Perl.exe introuvable"
LangString PerlDownloadLabel "${LYX_LANG}" "&Télécharger Perl"
LangString PerlFolderLabel "${LYX_LANG}" "&Répertoire contenant perl.exe"

LangString ImageMagickHeader "${LYX_LANG}" "ImageMagick"
LangString ImageMagickDescription "${LYX_LANG}" "Les outils graphiques de ImageMagick (www.imagemagick.org/script/index.php) peuvent être utilisés pour convertir les fichiers graphiques dans n'importe quel format nécessaire."
LangString EnterImageMagickFolder "${LYX_LANG}" "Merci de saisir le chemin conduisant au répertoire où se trouve convert.exe"
LangString InvalidImageMagickFolder "${LYX_LANG}" "convert.exe introuvable"
LangString ImageMagickDownloadLabel "${LYX_LANG}" "&Télécharger ImageMagick"
LangString ImageMagickFolderLabel "${LYX_LANG}" "&Répertoire contenant convert.exe"

LangString GhostscriptHeader "${LYX_LANG}" "Ghostscript"
LangString GhostscriptDescription "${LYX_LANG}" "Ghostscript (http://www.cs.wisc.edu/~ghost/) est utilisé pour convertir les graphiques depuis/vers PostScript."
LangString EnterGhostscriptFolder "${LYX_LANG}" "Merci de saisir le chemin conduisant au répertoire où se trouve gswin32c.exe"
LangString InvalidGhostscriptFolder "${LYX_LANG}" "gswin32c.exe introuvable"
LangString GhostscriptDownloadLabel "${LYX_LANG}" "&Télécharger Ghostscript"
LangString GhostscriptFolderLabel "${LYX_LANG}" "&Répertoire contenant gswin32c.exe"

LangString SummaryTitle "${LYX_LANG}" "Résumé d'installation"
LangString SummaryPleaseInstall "${LYX_LANG}" "Merci d'installer les fichiers téléchargés, puis d'exécuter de nouveau l'installation de LyX."
LangString SummaryPathPrefix "${LYX_LANG}" "Durant l'installation, une chaîne de caractères 'path_prefix'  sera ajoutée à 'lyxrc.defaults', elle contiendra :$\r$\n$PathPrefix"

LangString UnNotInRegistryLabel "${LYX_LANG}" "$(^Name) introuvable dans le base des registres. $\r$\nLes raccourcis sur le bureau et dans le menu de démarrage ne seront pas supprimés."
LangString UnNotAdminLabel "${LYX_LANG}" "Désolé ! Vous devez avoir les droits d'administration$\r$\npour installer$(^Name)."
LangString UnReallyRemoveLabel "${LYX_LANG}" "Êtes vous sûr(e) de vouloir supprimer complètement $(^Name) et tous ses composants ?"
LangString UnRemoveSuccessLabel "${LYX_LANG}" "$(^Name) supprimé de votre ordinateur avec succès."

!undef LYX_LANG

!endif ; _LYX_LANGUAGES_FRENCH_NSH_


