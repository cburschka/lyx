// -*- C++ -*-
/**
 * \file QPrefsDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QPREFSDIALOG_H
#define QPREFSDIALOG_H

#include "LColor.h"

#include "ui/QPrefsDialogBase.h"

#include <vector>

class LColor_color;
class QPrefAsciiModule;
class QPrefDateModule;
class QPrefKeyboardModule;
class QPrefLatexModule;
class QPrefScreenFontsModule;
class QPrefColorsModule;
class QPrefCygwinPathModule;
class QPrefDisplayModule;
class QPrefPathsModule;
class QPrefSpellcheckerModule;
class QPrefConvertersModule;
class QPrefCopiersModule;
class QPrefFileformatsModule;
class QPrefLanguageModule;
class QPrefPrinterModule;
class QPrefUIModule;
class QPrefIdentityModule;

namespace lyx {
namespace frontend {

class QPrefs;

class QPrefsDialog : public QPrefsDialogBase {
	Q_OBJECT
public:
	friend class QPrefs;

	QPrefsDialog(QPrefs *);

	~QPrefsDialog();

	void updateConverters();
	void updateConverterButtons();
	void updateCopiers();
	void updateCopierButtons();
	void updateFormats();
	void updateFormatsButtons();

public slots:
	void change_adaptor();

	void switch_format(int);
	void fileformat_changed();
	void new_format();
	void modify_format();
	void remove_format();

	void switch_converter(int);
	void converter_changed();
	void new_converter();
	void modify_converter();
	void remove_converter();

	void switch_copierLB(int nr);
	void switch_copierCO(int nr);
	void copiers_changed();
	void new_copier();
	void modify_copier();
	void remove_copier();

	void change_color();

	void select_ui();
	void select_bind();
	void select_keymap1();
	void select_keymap2();
	void select_dict();
	void select_templatedir();
	void select_tempdir();
	void select_backupdir();
	void select_workingdir();
	void select_lyxpipe();

	void select_roman(const QString&);
	void select_sans(const QString&);
	void select_typewriter(const QString&);

protected:
	void closeEvent(QCloseEvent * e);

private:

	std::vector<LColor_color> colors_;

	QPrefAsciiModule * asciiModule;
	QPrefDateModule * dateModule;
	QPrefKeyboardModule * keyboardModule;
	QPrefLatexModule * latexModule;
	QPrefScreenFontsModule * screenfontsModule;
	QPrefColorsModule * colorsModule;
#if defined(__CYGWIN__) || defined(__CYGWIN32__)
	QPrefCygwinPathModule * cygwinpathModule;	
#endif	
	QPrefDisplayModule * displayModule;
	QPrefPathsModule * pathsModule;
	QPrefSpellcheckerModule * spellcheckerModule;
	QPrefConvertersModule * convertersModule;
	QPrefCopiersModule * copiersModule;
	QPrefFileformatsModule * fileformatsModule;
	QPrefLanguageModule * languageModule;
	QPrefPrinterModule * printerModule;
	QPrefUIModule * uiModule;
	QPrefIdentityModule * identityModule;

	QPrefs * form_;
};

} // namespace frontend
} // namespace lyx

#endif // PREFSDIALOG_H
