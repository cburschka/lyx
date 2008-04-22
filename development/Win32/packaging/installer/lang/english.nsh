/*

LyX Installer Language File
Language: English
Author: Joost Verburg

*/

!insertmacro LANGFILE_EXT "English"

${LangFileString} TEXT_NO_PRIVILEDGES "You don't have Administrator or Power User privileges.$\r$\nThe utilities for image conversion cannot be installed without these priviledges."

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installed for Current User)"

${LangFileString} TEXT_WELCOME_DOWNLOAD "This wizard will guide you through the installation of $(^NameDA), the document processor that encourages an approach to writing based on the structure of your documents, not their appearance.$\r$\n$\r$\nThis installer will setup a complete environment to use ${APP_NAME} and can download missing components automatically.$\r$\n$\r$\n$(^ClickNext)"
${LangFileString} TEXT_WELCOME_INSTALL "This wizard will guide you through the installation of $(^NameDA), the document processor that encourages an approach to writing based on the structure of your documents, not their appearance.$\r$\n$\r$\nThis installer will setup a complete environment to use ${APP_NAME} and can install missing components automatically.$\r$\n$\r$\n$(^ClickNext)"

${LangFileString} TEXT_REINSTALL_TITLE "Reinstall"
${LangFileString} TEXT_REINSTALL_SUBTITLE "Choose whether you want to reinstall the $(^NameDA) program files."
${LangFileString} TEXT_REINSTALL_INFO "$(^NameDA) is already installed. This installer will allow you to change the interface language and locations of external applications or download additional dictionaries. If you also want to reinstall the program files, check the box below. $(^ClickNext)"
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

${LangFileString} TEXT_FINISH_DESKTOP "Create desktop shortcut"
${LangFileString} TEXT_FINISH_WEBSITE "Visit lyx.org for the latest news, support and tips"

${LangFileString} UNTEXT_WELCOME "This wizard will guide you through the uninstallation of $(^NameDA). Please close $(^NameDA) before you continue.$\r$\n$\r$\nNote that this wizard will only uninstall LyX itself. If you want to uninstall other applications you installed for use with LyX, uninstall them using their entry in Add/Remove Programs.$\r$\n$\r$\n$_CLICK"
