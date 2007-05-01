!ifndef _LYX_LANGUAGES_ENGLISH_NSH_
!define _LYX_LANGUAGES_ENGLISH_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_ENGLISH}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "This wizard will guide you through the installation of LyX.\r\n\
					  \r\n\
					  You need administrator privileges to install LyX.\r\n\
					  \r\n\
					  It is recommended that you close all other applications before starting Setup. This will make it possible to update relevant system files without having to reboot your computer.\r\n\
					  \r\n\
					  $_CLICK"

LangString FileTypeTitle "${LYX_LANG}" "LyX-Document"

LangString SecInstGSviewTitle "${LYX_LANG}" "GSview"
LangString SecInstJabRefTitle "${LYX_LANG}" "JabRef"
LangString SecAllUsersTitle "${LYX_LANG}" "Install for all users?"
LangString SecFileAssocTitle "${LYX_LANG}" "File associations"
LangString SecDesktopTitle "${LYX_LANG}" "Desktop icon"

LangString SecCoreDescription "${LYX_LANG}" "The LyX files."
LangString SecInstGSviewDescription "${LYX_LANG}" "Program to view Postscript- and PDF-documents."
LangString SecInstJabRefDescription "${LYX_LANG}" "Bibliography reference manager and editor for BibTeX files."
LangString SecAllUsersDescription "${LYX_LANG}" "Install LyX for all users or just the current user."
LangString SecFileAssocDescription "${LYX_LANG}" "Files with a .lyx extension will automatically open in LyX."
LangString SecDesktopDescription "${LYX_LANG}" "A LyX icon on the desktop."

LangString LangSelectHeader "${LYX_LANG}" "Selection of LyX's menu language"
LangString AvailableLang "${LYX_LANG}" " Available Languages "

LangString MissProgHeader "${LYX_LANG}" "Check for required programs"
LangString MissProgCap "${LYX_LANG}" "The following required program(s) will be installed additionally"
LangString MissProgLatex "${LYX_LANG}" "MiKTeX, a LaTeX distribution"
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript, an interpreter for PostScript and PDF"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick, an image converter"
LangString MissProgAspell "${LYX_LANG}" "Aspell, a spellchecker"
LangString MissProgMessage "${LYX_LANG}" 'No additional programs need to be installed.'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'LaTeX-distribution'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'Set the LaTeX-distribution that LyX should use.'
LangString EnterLaTeXFolder "${LYX_LANG}" 'You can optionally set here the path to the file "latex.exe" and therewith set the \
					   LaTeX-distribution that should be used by LyX.\r\n\
					   If you don$\'t use LaTeX, LyX cannot output documents!\r\n\
					   \r\n\
					   The installer has detected the LaTeX-distribution \
					   "$LaTeXName" on your system. Displayed below is its path.'
LangString EnterLaTeXFolderNone "${LYX_LANG}" 'Set below the path to the file "latex.exe". Therewith you set which \
					       LaTeX-distribution should be used by LyX.\r\n\
					       If you don$\'t use LaTeX, LyX cannot output documents!\r\n\
					       \r\n\
					       The installer couldn$\'t find a LaTeX-distribution on your system.'
LangString PathName "${LYX_LANG}" 'Path to the file "latex.exe"'
LangString DontUseLaTeX "${LYX_LANG}" "Don't use LaTeX"
LangString InvalidLaTeXFolder "${LYX_LANG}" 'The file "latex.exe" is not in the specified path.'

LangString LatexInfo "${LYX_LANG}" 'Now the installer of the LaTeX-distribution "MiKTeX" will be launched.$\r$\n\
			            To install the program press the "Next"-button in the installer windows until the installation begins.$\r$\n\
				    $\r$\n\
				    !!! Please use all default options of the MiKTeX-installer !!!'
LangString LatexError1 "${LYX_LANG}" 'No LaTeX-distribution could be found!$\r$\n\
                      		      LyX cannot be used without a LaTeX-distribution like "MiKTeX"!$\r$\n\
				      The installation will therefore be aborted.'
			    
LangString GSviewInfo "${LYX_LANG}" 'Now the installer of the program "GSview" will be launched.$\r$\n\
			             To install the program press the "Setup"-button in the first installer window,$\r$\n\
				     choose a language and then press the "Next"-button in the following installer windows.$\r$\n\
				     You can use all default options of the GSview-installer.'
LangString GSviewError "${LYX_LANG}" 'The program "GSview" could not successfully be installed!$\r$\n\
		                      The installer will continue anyway.$\r$\n\
				      Try to install GSview again later.'
				      
LangString JabRefInfo "${LYX_LANG}" 'Now the installer of the program "JabRef" will be launched.$\r$\n\
				     You can use all default options of the JabRef-installer.'
LangString JabRefError "${LYX_LANG}" 'The program "JabRef" could not successfully be installed!$\r$\n\
		                      The installer will continue anyway.$\r$\n\
				      Try to install JabRef again later.'
				      
LangString LatexConfigInfo "${LYX_LANG}" "The following configuration of LyX will take a while."

LangString AspellInfo "${LYX_LANG}" 'Now dictionaries for the spellchecker "Aspell" will be downloaded and installed.$\r$\n\
				     Every dictionary has a different license that will be displayed before the installation.'
LangString AspellDownloadFailed "${LYX_LANG}" "No Aspell spellchecker dictionary could be downloaded!"
LangString AspellInstallFailed "${LYX_LANG}" "No Aspell spellchecker dictionary could be installed!"
LangString AspellPartAnd "${LYX_LANG}" " and "
LangString AspellPartSep "${LYX_LANG}" ", "
LangString AspellAll "${LYX_LANG}" 'There is already installed a dictionary of the language'
LangString AspellNone "${LYX_LANG}" 'No dictionary for the spellchecker "Aspell" has been installed.$\r$\n\
				     Dictionaries can be downloaded from$\r$\n\
				     ${AspellLocation}$\r$\n\
				     Do you want to download dictionaries now?'
LangString AspellPartStart "${LYX_LANG}" "There was successfully installed "
LangString AspellPart1 "${LYX_LANG}" "an english dictionary"
LangString AspellPart2 "${LYX_LANG}" "a dictionary of the language $LangName"
LangString AspellPart3 "${LYX_LANG}" "a$\r$\n\
				      dictionary of the language $LangNameSys"
LangString AspellPart4 "${LYX_LANG}" ' for the spellchecker "Aspell".$\r$\n\
				      More dictionaries can be downloaded from$\r$\n\
				      ${AspellLocation}$\r$\n\
				      Do you want to download other dictionaries now?'

LangString MiKTeXPathInfo "${LYX_LANG}" "In order that every user is later able to customize MiKTeX for his needs$\r$\n\
					 it is necessary to set write permissions for all users to MiKTeX's installation folder$\r$\n\
					 $MiKTeXPath $\r$\n\
					 and its subfolders."
LangString MiKTeXInfo "${LYX_LANG}" 'The LaTeX-distribution "MiKTeX" will be used together with LyX.$\r$\n\
				     It is recommended to install available MiKTeX-updates using the program "MiKTeX Update Wizard"$\r$\n\
				     before you use LyX for the first time.$\r$\n\
				     Would you now check for MiKTeX updates?'

LangString UpdateNotAllowed "${LYX_LANG}" "This update package can only update ${PRODUCT_VERSION_OLD}!"
LangString ModifyingConfigureFailed "${LYX_LANG}" "Could not set 'path_prefix' in the configure script"
LangString CreateCmdFilesFailed "${LYX_LANG}" "Could not create lyx.bat"
LangString RunConfigureFailed "${LYX_LANG}" "Could not execute the configure script"
LangString NotAdmin "${LYX_LANG}" "You must have administrator privileges to install LyX!"
LangString InstallRunning "${LYX_LANG}" "The installer is already running!"
LangString StillInstalled "${LYX_LANG}" "LyX is already installed! Deinstall LyX first."

LangString FinishPageMessage "${LYX_LANG}" "Congratulations! LyX has been installed successfully.\r\n\
					    \r\n\
					    (The first start of LyX might take some seconds.)"
LangString FinishPageRun "${LYX_LANG}" "Launch LyX"

LangString UnNotInRegistryLabel "${LYX_LANG}" "Unable to find LyX in the registry.$\r$\n\
					       Shortcuts on the desktop and in the Start Menu will not be removed."
LangString UnInstallRunning "${LYX_LANG}" "You must close LyX at first!"
LangString UnNotAdminLabel "${LYX_LANG}" "You must have administrator privileges to uninstall LyX!"
LangString UnReallyRemoveLabel "${LYX_LANG}" "Are you sure you want to completely remove LyX and all of its components?"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'LyX$\'s user preferences'
LangString UnGSview "${LYX_LANG}" 'Please press the "Uninstall" button in the next window to uninstall$\r$\n\
				   the Postscript and PDF-viewer "GSview".'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "LyX was successfully removed from your computer."

LangString SecUnAspellDescription "${LYX_LANG}" 'Uninstalls the spellchecker Aspell and all of its dictionaries.'
LangString SecUnMiKTeXDescription "${LYX_LANG}" "Uninstalls the LaTeX-distribution MiKTeX."
LangString SecUnJabRefDescription "${LYX_LANG}" "Uninstalls the bibliography manager JabRef."
LangString SecUnPreferencesDescription "${LYX_LANG}" 'Deletes LyX$\'s configuration folder$\r$\n\
					   "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
					   for all users.'
LangString SecUnProgramFilesDescription "${LYX_LANG}" "Uninstall LyX and all of its components."

!undef LYX_LANG

!endif # _LYX_LANGUAGES_ENGLISH_NSH_
