/*

LyX Installer Language File
Language: English
Author: Joost Verburg, Uwe Stöhr

*/

!insertmacro LANGFILE_EXT "English"

${LangFileString} TEXT_NO_PRIVILEDGES "You don't have Administrator or Power User privileges.$\r$\nThe utilities for image conversion cannot be installed without these priviledges."

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installed for Current User)"

${LangFileString} TEXT_WELCOME "This wizard will guide you through the installation of $(^NameDA), $\r$\n$\r$\n$_CLICK"

${LangFileString} TEXT_REINSTALL_TITLE "Reinstall"
${LangFileString} TEXT_REINSTALL_SUBTITLE "Choose whether you want to reinstall the $(^NameDA) program files."
${LangFileString} TEXT_REINSTALL_INFO "$(^NameDA) is already installed. This installer will allow you to change the location of your LaTeX system or download additional dictionaries. If you also want to reinstall the program files, check the box below. $(^ClickNext)"
${LangFileString} TEXT_REINSTALL_ENABLE "Reinstall $(^NameDA) program files"

${LangFileString} TEXT_EXTERNAL_NOPRIVILEDGES "(Administrator or Power User privileges required)"

${LangFileString} TEXT_EXTERNAL_LATEX_TITLE "LaTeX Typesetting System"
${LangFileString} TEXT_EXTERNAL_LATEX_SUBTITLE "A typesetting system is required to be able to create documents for printing or publishing."
${LangFileString} TEXT_EXTERNAL_LATEX_INFO_DOWNLOAD "Please specify whether you want to download MiKTeX or want to use an existing LaTeX system. $(^ClickNext)"
${LangFileString} TEXT_EXTERNAL_LATEX_INFO_INSTALL "Please specify whether you want to install MiKTeX or want to use an existing LaTeX system. $(^ClickNext)"
${LangFileString} TEXT_EXTERNAL_LATEX_DOWNLOAD "Download MiKTeX"
${LangFileString} TEXT_EXTERNAL_LATEX_INSTALL "Install MiKTeX"
${LangFileString} TEXT_EXTERNAL_LATEX_FOLDER "Use an existing LaTeX system in the following folder:"
${LangFileString} TEXT_EXTERNAL_LATEX_FOLDER_INFO "This folder should contain ${BIN_LATEX}."
${LangFileString} TEXT_EXTERNAL_LATEX_NONE "Install without a LaTeX typesetting system (not recommended)"
${LangFileString} TEXT_EXTERNAL_LATEX_NOTFOUND "${BIN_LATEX} does not exist in the folder you specified."

${LangFileString} TEXT_EXTERNAL_IMAGEMAGICK_TITLE "ImageMagick"
${LangFileString} TEXT_EXTERNAL_IMAGEMAGICK_SUBTITLE "ImageMagick is required to support various image formats."
${LangFileString} TEXT_EXTERNAL_IMAGEMAGICK_INFO_DOWNLOAD "Please specify whether you want to download ImageMagick or have it already installed. $(^ClickNext)"
${LangFileString} TEXT_EXTERNAL_IMAGEMAGICK_INFO_INSTALL "Please specify whether you want to install ImageMagick or have it already installed. $(^ClickNext)"
${LangFileString} TEXT_EXTERNAL_IMAGEMAGICK_DOWNLOAD "Download ImageMagick"
${LangFileString} TEXT_EXTERNAL_IMAGEMAGICK_INSTALL "Install ImageMagick"
${LangFileString} TEXT_EXTERNAL_IMAGEMAGICK_FOLDER "Use an existing ImageMagick installation in the following folder:"
${LangFileString} TEXT_EXTERNAL_IMAGEMAGICK_FOLDER_INFO "This folder should contain ${BIN_IMAGEMAGICK}."
${LangFileString} TEXT_EXTERNAL_IMAGEMAGICK_NONE "Install without ImageMagick (not recommended)"
${LangFileString} TEXT_EXTERNAL_IMAGEMAGICK_NOTFOUND "${BIN_IMAGEMAGICK} does not exist in the folder you specified."

${LangFileString} TEXT_EXTERNAL_GHOSTSCRIPT_TITLE "Ghostscript"
${LangFileString} TEXT_EXTERNAL_GHOSTSCRIPT_SUBTITLE "Ghostscript is required to preview images in PostScript format."
${LangFileString} TEXT_EXTERNAL_GHOSTSCRIPT_INFO_DOWNLOAD "Please specify whether you want to download Ghostscript or have it already installed. $(^ClickNext)"
${LangFileString} TEXT_EXTERNAL_GHOSTSCRIPT_INFO_INSTALL "Please specify whether you want to install Ghostscript or have it already installed. $(^ClickNext)"
${LangFileString} TEXT_EXTERNAL_GHOSTSCRIPT_DOWNLOAD "Download Ghostscript"
${LangFileString} TEXT_EXTERNAL_GHOSTSCRIPT_INSTALL "Install Ghostscript"
${LangFileString} TEXT_EXTERNAL_GHOSTSCRIPT_FOLDER "Use an existing Ghostscript installation in the following folder:"
${LangFileString} TEXT_EXTERNAL_GHOSTSCRIPT_FOLDER_INFO "This folder should contain ${BIN_GHOSTSCRIPT}."
${LangFileString} TEXT_EXTERNAL_GHOSTSCRIPT_NONE "Install without GhostScript (not recommended)"
${LangFileString} TEXT_EXTERNAL_GHOSTSCRIPT_NOTFOUND "${BIN_GHOSTSCRIPT} does not exist in the folder you specified."

${LangFileString} TEXT_VIEWER_TITLE "Document Viewer"
${LangFileString} TEXT_VIEWER_SUBTITLE "A viewer is required to be able to display typeset documents on your screen."
${LangFileString} TEXT_VIEWER_INFO_DOWNLOAD "There is no viewer installed on your system to preview both PDF and Postscript files, which are common formats for typeset documents. If you check the box below, the PDF/Postscript viewer GSView will be downloaded. $(^ClickNext)"
${LangFileString} TEXT_VIEWER_INFO_INSTALL "There is no viewer installed on your system to preview both PDF and Postscript files, which are common formats for typeset documents. If you check the box below, the PDF/Postscript viewer GSView will be installed. $(^ClickNext)"
${LangFileString} TEXT_VIEWER_DOWNLOAD "Download GSView"
${LangFileString} TEXT_VIEWER_INSTALL "Install GSView"

${LangFileString} TEXT_DICT_TITLE "Spell Checker Dictionaries"
${LangFileString} TEXT_DICT_SUBTITLE "Choose the languages you want the spell checker to support."
${LangFileString} TEXT_DICT_TOP "In order to spell check documents in a certain language, a dictionary with valid words has to be downloaded. $_CLICK"
${LangFileString} TEXT_DICT_LIST "Select dictionaries to download:"

${LangFileString} TEXT_LANGUAGE_TITLE "Choose Language"
${LangFileString} TEXT_LANGUAGE_SUBTITLE "Choose the language in which you want to use $(^NameDA)."
${LangFileString} TEXT_LANGUAGE_INFO "Select the language to use for the ${APP_NAME} interface. $(^ClickNext)"

${LangFileString} TEXT_DOWNLOAD_FAILED_LATEX "Downloading MiKTeX failed. Would you like to try again?"
${LangFileString} TEXT_DOWNLOAD_FAILED_IMAGEMAGICK "Downloading ImageMagick failed. Would you like to try again?"
${LangFileString} TEXT_DOWNLOAD_FAILED_GHOSTSCRIPT "Downloading Ghostscript failed. Would you like to try again?"
${LangFileString} TEXT_DOWNLOAD_FAILED_VIEWER "Downloading GSView failed. Would you like to try again?"
${LangFileString} TEXT_DOWNLOAD_FAILED_DICT "Failed to download the $DictionaryLangName spell checker dictionary. Would you like to try again?"

${LangFileString} TEXT_NOTINSTALLED_LATEX "The MiKTeX setup wizard was not completed. Do you want to run MiKTeX setup again?"
${LangFileString} TEXT_NOTINSTALLED_IMAGEMAGICK "The ImageMagick was not completed. Do you want to run ImageMagick setup again?"
${LangFileString} TEXT_NOTINSTALLED_GHOSTSCRIPT "The Ghostscript setup wizard was not completed. Do you want to run Ghostscript setup again?"
${LangFileString} TEXT_NOTINSTALLED_VIEWER "You did not associate GSView with the PDF/Postscript file types. Do you want to run GSView setup again?"
${LangFileString} TEXT_NOTINSTALLED_DICT "The setup wizard of the $DictionaryLangName spell checker dictionary was not completed. Do you want to run the setup again?"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Updating MiKTeX filename database..."
${LangFileString} TEXT_CONFIGURE_LYX "Configuring LyX (MiKTeX may download missing packages, this can take some time) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compiling Python scripts..."

${LangFileString} TEXT_FINISH_DESKTOP "Create desktop shortcut"
${LangFileString} TEXT_FINISH_WEBSITE "Visit lyx.org for the latest news, support and tips"

${LangFileString} FileTypeTitle "LyX-Document"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Install for all users?"
${LangFileString} SecFileAssocTitle "File associations"
${LangFileString} SecDesktopTitle "Desktop icon"

${LangFileString} SecCoreDescription "The LyX files."
${LangFileString} SecInstJabRefDescription "Bibliography reference manager and editor for BibTeX files."
${LangFileString} SecAllUsersDescription "Install LyX for all users or just the current user."
${LangFileString} SecFileAssocDescription "Files with a .lyx extension will automatically open in LyX."
${LangFileString} SecDesktopDescription "A LyX icon on the desktop."

${LangFileString} LangSelectHeader "Selection of LyX's menu language"
${LangFileString} AvailableLang " Available Languages "

${LangFileString} EnterLaTeXHeader1 'LaTeX-distribution'
${LangFileString} EnterLaTeXHeader2 'Set the LaTeX-distribution that LyX should use.'
${LangFileString} EnterLaTeXFolder 'You can optionally set here the path to the file $\"latex.exe$\" and therewith set the \
					   LaTeX-distribution that should be used by LyX.$\r$\n\
					   If you don$\'t use LaTeX, LyX cannot output documents!$\r$\n\
					   $\r$\n\
					   The installer has detected the LaTeX-distribution \
					   $\"$LaTeXName$\" on your system. Displayed below is its path.'
${LangFileString} EnterLaTeXFolderNone 'Set below the path to the file $\"latex.exe$\". Therewith you set which \
					       LaTeX-distribution should be used by LyX.$\r$\n\
					       If you don$\'t use LaTeX, LyX cannot output documents!$\r$\n\
					       $\r$\n\
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

${LangFileString} LatexConfigInfo "The following configuration of LyX will take a while."

${LangFileString} MiKTeXPathInfo "In order that every user is later able to customize MiKTeX for his needs$\r$\n\
					 it is necessary to set write permissions for all users to MiKTeX$\'s installation folder$\r$\n\
					 $MiKTeXPath $\r$\n\
					 and its subfolders."
${LangFileString} MiKTeXInfo 'The LaTeX-distribution $\"MiKTeX$\" will be used together with LyX.$\r$\n\
				     It is recommended to install available MiKTeX-updates using the program $\"MiKTeX Update Wizard$\"$\r$\n\
				     before you use LyX for the first time.$\r$\n\
				     Would you now check for MiKTeX updates?'

${LangFileString} ModifyingConfigureFailed "Could not set 'path_prefix' in the configure script"
${LangFileString} CreateCmdFilesFailed "Could not create lyx.bat"
${LangFileString} RunConfigureFailed "Could not execute the configure script"
${LangFileString} InstallRunning "The installer is already running!"
${LangFileString} StillInstalled "LyX is already installed! Deinstall LyX first."

${LangFileString} FinishPageMessage "Congratulations! LyX has been installed successfully.$\r$\n\
					    $\r$\n\
					    (The first start of LyX might take some seconds.)"
${LangFileString} FinishPageRun "Launch LyX"

${LangFileString} UnNotInRegistryLabel "Unable to find LyX in the registry.$\r$\n\
					       Shortcuts on the desktop and in the Start Menu will not be removed."
${LangFileString} UnInstallRunning "You must close LyX at first!"
${LangFileString} UnNotAdminLabel "You must have administrator privileges to uninstall LyX!"
${LangFileString} UnReallyRemoveLabel "Are you sure you want to completely remove LyX and all of its components?"
${LangFileString} UnLyXPreferencesTitle 'LyX$\'s user preferences'

${LangFileString} SecUnMiKTeXDescription "Uninstalls the LaTeX-distribution MiKTeX."
${LangFileString} SecUnJabRefDescription "Uninstalls the bibliography manager JabRef."
${LangFileString} SecUnPreferencesDescription 'Deletes LyX$\'s configuration folder$\r$\n\
					   "$AppPre\username\$AppSuff\${APP_DIR_USERDATA}"$\r$\n\
					   for all users.'
${LangFileString} SecUnProgramFilesDescription "Uninstall LyX and all of its components."
