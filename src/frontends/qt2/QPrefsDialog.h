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

	void updateConverters();

	void updateFormats();

public slots:
	virtual void switchPane(QListViewItem * i);

	void change_adaptor();

	void switch_format(int);
	void new_format();
	void modify_format();
	void remove_format();
 
	void switch_converter(int);
	void new_converter();
	void modify_converter();
	void remove_converter();
 
protected:
	void closeEvent(QCloseEvent * e);
 
private:
	typedef std::map<QListViewItem *, QWidget *> PaneMap;

	PaneMap pane_map_;

	QPrefAsciiModule * asciiModule;
	QPrefDateModule * dateModule;
	QPrefKeyboardModule * keyboardModule;
	QPrefLatexModule * latexModule;
	QPrefScreenFontsModule * screenfontsModule;
	QPrefColorsModule * colorsModule;
	QPrefDisplayModule * displayModule;
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
