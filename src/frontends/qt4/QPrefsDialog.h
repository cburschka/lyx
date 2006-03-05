// -*- C++ -*-
/**
 * \file QPrefsDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QPREFSDIALOG_H
#define QPREFSDIALOG_H

#include "LColor.h"

#include "ui/QPrefsUi.h"

#include <vector>

#include <QDialog>
#include <QCloseEvent>

#include "ui/QPrefAsciiUi.h"
#include "ui/QPrefDateUi.h"
#include "ui/QPrefKeyboardUi.h"
#include "ui/QPrefLatexUi.h"
#include "ui/QPrefScreenFontsUi.h"
#include "ui/QPrefColorsUi.h"
#if defined(__CYGWIN__) || defined(__CYGWIN32__)
#include "ui/QPrefCygwinPathUi.h"
#endif
#include "ui/QPrefDisplayUi.h"
#include "ui/QPrefPathsUi.h"
#include "ui/QPrefSpellcheckerUi.h"
#include "ui/QPrefConvertersUi.h"
#include "ui/QPrefCopiersUi.h"
#include "ui/QPrefFileformatsUi.h"
#include "ui/QPrefLanguageUi.h"
#include "ui/QPrefPrinterUi.h"
#include "ui/QPrefUi.h"
#include "ui/QPrefIdentityUi.h"

template<class UI>
	class UiWidget: public QWidget, public UI
	{
	public:
		UiWidget(QWidget * Parent): QWidget(Parent)
		{
			UI::setupUi(this);
		}
	};


namespace lyx {
namespace frontend {

class QPrefs;

class QPrefsDialog : public QDialog, public Ui::QPrefsUi {
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

	UiWidget<Ui::QPrefAsciiUi> * asciiModule;
	UiWidget<Ui::QPrefDateUi> * dateModule;
	UiWidget<Ui::QPrefKeyboardUi> * keyboardModule;
	UiWidget<Ui::QPrefLatexUi> * latexModule;
	UiWidget<Ui::QPrefScreenFontsUi> * screenfontsModule;
	UiWidget<Ui::QPrefColorsUi> * colorsModule;
#if defined(__CYGWIN__) || defined(__CYGWIN32__)
	UiWidget<Ui::QPrefCygwinPathUi> * cygwinpathModule;
#endif
	UiWidget<Ui::QPrefDisplayUi> * displayModule;
	UiWidget<Ui::QPrefPathsUi> * pathsModule;
	UiWidget<Ui::QPrefSpellcheckerUi> * spellcheckerModule;
	UiWidget<Ui::QPrefConvertersUi> * convertersModule;
	UiWidget<Ui::QPrefCopiersUi> * copiersModule;
	UiWidget<Ui::QPrefFileformatsUi> * fileformatsModule;
	UiWidget<Ui::QPrefLanguageUi> * languageModule;
	UiWidget<Ui::QPrefPrinterUi> * printerModule;
	UiWidget<Ui::QPrefUi> * uiModule;
	UiWidget<Ui::QPrefIdentityUi> * identityModule;

	QPrefs * form_;
};

} // namespace frontend
} // namespace lyx

#endif // PREFSDIALOG_H
