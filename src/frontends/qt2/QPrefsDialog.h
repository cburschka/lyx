// -*- C++ -*-
/**
 * \file QPrefsDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QPREFSDIALOG_H
#define QPREFSDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QPrefsDialogBase.h"

#include <map>
 
class QPrefs;
class QListViewItem;
class QPrefAsciiModule;
class QPrefDateModule;
class QPrefKeyboardModule;
class QPrefLatexModule;
class QPrefScreenFontsModule;
class QPrefColorsModule;
class QPrefDisplayModule;
class QPrefLNFMiscModule;
class QPrefPathsModule;
class QPrefSpellcheckerModule;
class QPrefConvertersModule;
class QPrefFileformatsModule;
class QPrefLanguageModule;
class QPrefPrinterModule;
class QPrefUIModule;

class QPrefsDialog : public QPrefsDialogBase {
	Q_OBJECT
public:
	friend class QPrefs;

	QPrefsDialog(QPrefs *);
 
	~QPrefsDialog();

public slots:
	virtual void switchPane(QListViewItem * i);

protected:
	void closeEvent(QCloseEvent * e);
 
private:
	/*enum Panes {
		ASCII,
		DATE,
		KEYBOARD,
		LATEX,
		SCREENFONTS,
		COLORS,
		DISPLAY,
		LNFMISC,
		PATHS,
		SPELLCHECKER,
		CONVERTERS,
		FILEFORMATS,
		LANGUAGE,
		PRINTER,
		UI
	};*/
 
	typedef std::map<QListViewItem *, QWidget *> PaneMap;

	PaneMap pane_map_;

/*
QPrefAsciiModule
QPrefDateModule
QPrefKeyboardModule
QPrefLatexModule
QPrefScreenFontsModule
QPrefColorsModule
QPrefDisplayModule
QPrefLNFMiscModule
QPrefPathsModule
QPrefSpellcheckerModule
QPrefConvertersModule
QPrefFileformatsModule
QPrefLanguageModule
QPrefPrinterModule
QPrefUIModule
*/ 
	QPrefAsciiModule * asciiModule;
	QPrefDateModule * dateModule;
	QPrefKeyboardModule * keyboardModule;
	QPrefLatexModule * latexModule;
	QPrefScreenFontsModule * screenfontsModule;
	QPrefColorsModule * colorsModule;
	QPrefDisplayModule * displayModule;
	QPrefLNFMiscModule * lnfmiscModule;
	QPrefPathsModule * pathsModule;
	QPrefSpellcheckerModule * spellcheckerModule;
	QPrefConvertersModule * convertersModule;
	QPrefFileformatsModule * fileformatsModule;
	QPrefLanguageModule * languageModule;
	QPrefPrinterModule * printerModule;
	QPrefUIModule * uiModule;

	QPrefs * form_;
};

#endif // PREFSDIALOG_H
