!ifndef _LYX_LANGUAGES_ITALIAN_NSH_
!define _LYX_LANGUAGES_ITALIAN_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_ITALIAN}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString SecAllUsersTitle "${LYX_LANG}" "Installazione per tutti gli utenti?"
LangString SecFileAssocTitle "${LYX_LANG}" "Associazioni dei file"
LangString SecDesktopTitle "${LYX_LANG}" "Icona sul Desktop"

LangString SecCoreDescription "${LYX_LANG}" "I file di ${PRODUCT_NAME}."
LangString SecAllUsersDescription "${LYX_LANG}" "Installazione per tutti gli utenti o solo per l'utente attuale. (Sono richiesti privilegi da amministratore.)"
LangString SecFileAssocDescription "${LYX_LANG}" "Crea le associazioni tra il programma e le estensioni .lyx."
LangString SecDesktopDescription "${LYX_LANG}" "Icona ${PRODUCT_NAME} sul desktop."

LangString ModifyingConfigureFailed "${LYX_LANG}" "Fallito tentativo di aggiornare 'path_prefix' nello script di configurazione"
LangString RunConfigureFailed "${LYX_LANG}" "Fallito tentativo di eseguire lo script di configurazione"

LangString FinishPageMessage "${LYX_LANG}" "Congratulazioni! LyX è stato installato con successo."
LangString FinishPageRun "${LYX_LANG}" "Lancia LyX"

LangString DownloadPageField2 "${LYX_LANG}" "&Non installare"

LangString MinSYSHeader "${LYX_LANG}" "MinSYS"
LangString MinSYSDescription "${LYX_LANG}" "MinSYS fornisce un ambiente unix minimale (www.mingw.org/msys.shtml) richiesto da ${PRODUCT_NAME} per far girare un certo numero di file script."
LangString EnterMinSYSFolder "${LYX_LANG}" "Inserite il percorso alla cartella contenente sh.exe"
LangString InvalidMinSYSFolder "${LYX_LANG}" "Non riesco a trovare sh.exe"
LangString MinSYSDownloadLabel "${LYX_LANG}" "&Scarica MinSYS"
LangString MinSYSFolderLabel "${LYX_LANG}" "&Cartella contenente sh.exe"

LangString PythonHeader "${LYX_LANG}" "Python"
LangString PythonDescription "${LYX_LANG}" "L'interprete Python (www.python.org) deve essere installato, altrimenti ${PRODUCT_NAME} non sarà in grado di far girare un certo numero di file script."
LangString EnterPythonFolder "${LYX_LANG}" "Inserite il percorso alla cartella contenente Python.exe"
LangString InvalidPythonFolder "${LYX_LANG}" "Non riesco a trovare Python.exe"
LangString PythonDownloadLabel "${LYX_LANG}" "&carica Python"
LangString PythonFolderLabel "${LYX_LANG}" "&Cartella contenente Python.exe"

LangString MiKTeXHeader "${LYX_LANG}" "MiKTeX"
LangString MiKTeXDescription "${LYX_LANG}" "MiKTeX (www.miktex.org) è un'aggiornata implementazione di TeX per Windows."
LangString EnterMiKTeXFolder "${LYX_LANG}" "Inserite il percorso alla cartella contenente latex.exe"
LangString InvalidMiKTeXFolder "${LYX_LANG}" "Non riesco a trovare latex.exe"
LangString MiKTeXDownloadLabel "${LYX_LANG}" "&Scarica MiKTeX"
LangString MiKTeXFolderLabel "${LYX_LANG}" "&Cartella contenente latex.exe"

LangString PerlHeader "${LYX_LANG}" "Perl"
LangString PerlDescription "${LYX_LANG}" "Se volete usare reLyX per convertire documenti LaTeX nel formato LyX, dovete installare Perl (www.perl.com)."
LangString EnterPerlFolder "${LYX_LANG}" "Inserite il percorso alla cartella contenente Perl.exe"
LangString InvalidPerlFolder "${LYX_LANG}" "Non riesco a trovare Perl.exe"
LangString PerlDownloadLabel "${LYX_LANG}" "&Scarica Perl"
LangString PerlFolderLabel "${LYX_LANG}" "&Cartella contenente perl.exe"

LangString ImageMagickHeader "${LYX_LANG}" "ImageMagick"
LangString ImageMagickDescription "${LYX_LANG}" "I programmi forniti da ImageMagick (www.imagemagick.org/script/index.php) vengono usati per convertire file grafici da un certo formato ad un qualsiasi altro formato."
LangString EnterImageMagickFolder "${LYX_LANG}" "Inserite il percorso alla cartella contenente convert.exe"
LangString InvalidImageMagickFolder "${LYX_LANG}" "Non riesco a trovare convert.exe"
LangString ImageMagickDownloadLabel "${LYX_LANG}" "&Scarica ImageMagick"
LangString ImageMagickFolderLabel "${LYX_LANG}" "&Cartella contenente convert.exe"

LangString GhostscriptHeader "${LYX_LANG}" "Ghostscript"
LangString GhostscriptDescription "${LYX_LANG}" "Ghostscript (http://www.cs.wisc.edu/~ghost/) viene usato per convertire immagini in/da PostScript."
LangString EnterGhostscriptFolder "${LYX_LANG}" "Inserite il percorso alla cartella contenente gswin32c.exe"
LangString InvalidGhostscriptFolder "${LYX_LANG}" "Non riesco a trovare gswin32c.exe"
LangString GhostscriptDownloadLabel "${LYX_LANG}" "&Scarica Ghostscript"
LangString GhostscriptFolderLabel "${LYX_LANG}" "&Cartella contenente gswin32c.exe"

LangString SummaryTitle "${LYX_LANG}" "Sommario del software"
LangString SummaryPleaseInstall "${LYX_LANG}" "Siete pregati di installare i pacchetti scaricati e quindi lanciare nuovamente l'installazione di LyX."
LangString SummaryPathPrefix "${LYX_LANG}" "Verrà aggiunta una stringa 'path_prefix' al file 'lyxrc.defaults' contenente:"

LangString UnNotInRegistryLabel "${LYX_LANG}" "Non riesco a trovare $(^Name) nel registro$\r$\nI collegamenti sul desktop e nel menu Start non saranno rimossi."
LangString UnNotAdminLabel "${LYX_LANG}" "Spiacente! Occorrono privilegi da amministratore$\r$\nper disinstallare $(^Name)."
LangString UnReallyRemoveLabel "${LYX_LANG}" "Siete sicuri di voler rimuovere completamente $(^Name) e tutti i suoi componenti?"
LangString UnRemoveSuccessLabel "${LYX_LANG}" "$(^Name) è stato rimosso con successo dal computer."

!undef LYX_LANG

!endif ; _LYX_LANGUAGES_ITALIAN_NSH_
