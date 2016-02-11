/*
LyX Installer Language File
Language: English
Author: Joost Verburg, Uwe Stöhr
*/

!insertmacro LANGFILE_EXT "English"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installed for Current User)"

${LangFileString} TEXT_WELCOME "This wizard will guide you through the installation of $(^NameDA), $\r$\n\
				$\r$\n\
				$_CLICK"

#${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Updating MiKTeX filename database..."
${LangFileString} TEXT_CONFIGURE_LYX "Configuring LyX ($LaTeXInstalled may download missing packages, this can take some time) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compiling Python scripts..."

${LangFileString} TEXT_FINISH_DESKTOP "Create desktop shortcut"
${LangFileString} TEXT_FINISH_WEBSITE "Visit lyx.org for the latest news, support and tips"

#${LangFileString} FileTypeTitle "LyX-Document"

${LangFileString} SecInstJabRefTitle "JabRef"
#${LangFileString} SecAllUsersTitle "Install for all users?"
${LangFileString} SecFileAssocTitle "File associations"
${LangFileString} SecDesktopTitle "Desktop icon"

${LangFileString} SecCoreDescription "The LyX files."
${LangFileString} SecInstJabRefDescription "Bibliography reference manager and editor for BibTeX files."
#${LangFileString} SecAllUsersDescription "Install LyX for all users or just the current user."
${LangFileString} SecFileAssocDescription "Files with a .lyx extension will automatically open in LyX."
${LangFileString} SecDesktopDescription "A LyX icon on the desktop."
${LangFileString} SecDictionariesDescription "Spell-checker dictionaries that can be downloaded and installed."
${LangFileString} SecThesaurusDescription "Thesaurus dictionaries that can be downloaded and installed."

${LangFileString} EnterLaTeXHeader1 'LaTeX-distribution'
${LangFileString} EnterLaTeXHeader2 'Set the LaTeX-distribution that LyX should use.'
${LangFileString} EnterLaTeXFolder 'You can optionally set here the path to the file $\"latex.exe$\" and therewith set the \
					LaTeX-distribution that should be used by LyX.\r\n\
					If you don$\'t use LaTeX, LyX cannot output documents!\r\n\
					\r\n\
					The installer has detected the LaTeX-distribution \
					$\"$LaTeXName$\" on your system. Displayed below is its path.'
${LangFileString} EnterLaTeXFolderNone 'Set below the path to the file $\"latex.exe$\". Therewith you set which \
					LaTeX-distribution should be used by LyX.\r\n\
					If you don$\'t use LaTeX, LyX cannot output documents!\r\n\
					\r\n\
					The installer couldn$\'t find a LaTeX-distribution on your system.'
${LangFileString} PathName 'Path to the file $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Don$\'t use LaTeX"
${LangFileString} InvalidLaTeXFolder 'The file $\"latex.exe$\" is not in the specified path.'

${LangFileString} LatexInfo 'Now the installer of the LaTeX-distribution $\"MiKTeX$\" will be launched.$\r$\n\
				To install the program press the $\"Next$\"-button in the installer windows until the installation begins.$\r$\n\
				$\r$\n\
				!!! Please use all default options of the MiKTeX-installer !!!'
${LangFileString} LatexError1 'No LaTeX-distribution could be found!$\r$\n\
				LyX cannot be used without a LaTeX-distribution like $\"MiKTeX$\"!$\r$\n\
				The installation will therefore be aborted.'

${LangFileString} HunspellFailed 'Download of dictionary for language $\"$R3$\" failed.'
${LangFileString} ThesaurusFailed 'Download of thesaurus for language $\"$R3$\" failed.'

${LangFileString} JabRefInfo 'Now the installer of the program $\"JabRef$\" will be launched.$\r$\n\
				You can use all default options of the JabRef-installer.'
${LangFileString} JabRefError 'The program $\"JabRef$\" could not successfully be installed!$\r$\n\
				The installer will continue anyway.$\r$\n\
				Try to install JabRef again later.'

#${LangFileString} LatexConfigInfo "The following configuration of LyX will take a while."

#${LangFileString} MiKTeXPathInfo "In order that every user is later able to customize MiKTeX for his needs$\r$\n\
#					it is necessary to set write permissions for all users to MiKTeX$\'s installation folder$\r$\n\
#					$MiKTeXPath $\r$\n\
#					and its subfolders."
${LangFileString} MiKTeXInfo 'The LaTeX-distribution $\"MiKTeX$\" will be used together with LyX.$\r$\n\
				It is recommended to install available MiKTeX-updates using the program $\"MiKTeX Update Wizard$\"$\r$\n\
				before you use LyX for the first time.$\r$\n\
				Would you now check for MiKTeX updates?'

${LangFileString} ModifyingConfigureFailed "Could not set 'path_prefix' in the configure script"
#${LangFileString} RunConfigureFailed "Could not execute the configure script"
${LangFileString} InstallRunning "The installer is already running!"
${LangFileString} AlreadyInstalled "LyX ${APP_SERIES_KEY2} is already installed!$\r$\n\
				Installing over existing installations is not recommended if the installed version$\r$\n\
				is a test release or if you have problems with your existing LyX installation.$\r$\n\
				In these cases better reinstall LyX.$\r$\n\
				Do you nevertheless want to install LyX over the existing version?"
${LangFileString} NewerInstalled "You are trying to install an older version of LyX than what you have installed.$\r$\n\
				  If you really want this, you must uninstall the existing LyX $OldVersionNumber before."

${LangFileString} MultipleIndexesNotAvailable "The support for using several indexes in a document will not be available because$\r$\n\
						MiKTeX was installed with administrator privileges but you are installing LyX without them."

#${LangFileString} FinishPageMessage "Congratulations! LyX has been installed successfully.$\r$\n\
#					$\r$\n\
#					(The first start of LyX might take some seconds.)"
${LangFileString} FinishPageRun "Launch LyX"

${LangFileString} UnNotInRegistryLabel "Unable to find LyX in the registry.$\r$\n\
					Shortcuts on the desktop and in the Start Menu will not be removed."
${LangFileString} UnInstallRunning "You must close LyX at first!"
${LangFileString} UnNotAdminLabel "You must have administrator privileges to uninstall LyX!"
${LangFileString} UnReallyRemoveLabel "Are you sure you want to completely remove LyX and all of its components?"
${LangFileString} UnLyXPreferencesTitle 'LyX$\'s user preferences'

${LangFileString} SecUnMiKTeXDescription "Uninstalls the LaTeX-distribution MiKTeX."
${LangFileString} SecUnJabRefDescription "Uninstalls the bibliography manager JabRef."
${LangFileString} SecUnPreferencesDescription 'Deletes LyX$\'s configuration$\r$\n\
						(folder $\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						for you or for all users (if you are admin).'
${LangFileString} SecUnProgramFilesDescription "Uninstall LyX and all of its components."

