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
#include "lyxrc.h"

#include "ui/QPrefsUi.h"

#include <vector>

#include <QDialog>
#include <QCloseEvent>

#include "ui/QPrefPlaintextUi.h"
#include "ui/QPrefDateUi.h"
#include "ui/QPrefKeyboardUi.h"
#include "ui/QPrefLatexUi.h"
#include "ui/QPrefScreenFontsUi.h"
#include "ui/QPrefColorsUi.h"
#include "ui/QPrefCygwinPathUi.h"
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

namespace lyx {
namespace frontend {

class QPrefs;

class PrefModule : public QWidget
{
	Q_OBJECT
public:
	PrefModule(lyx::docstring const & cat, lyx::docstring const & t, QPrefs * form = 0, QWidget * parent = 0)
		: QWidget(parent), category_(cat), title_(t), form_(form)
	{
	}
	virtual ~PrefModule() {}

	virtual void apply(LyXRC & rc) const = 0;
	virtual void update(LyXRC const & rc) = 0;

	lyx::docstring const & category() {
		return category_;
	}

	lyx::docstring const & title() {
		return title_;
	}

protected:
	lyx::docstring category_;
	lyx::docstring title_;
	QPrefs * form_;

Q_SIGNALS:
	void changed();
};


class PrefPlaintext :  public PrefModule, public Ui::QPrefPlaintextUi
{
	Q_OBJECT
public:
	PrefPlaintext(QWidget * parent = 0);
	~PrefPlaintext() {}

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);
};


class PrefDate :  public PrefModule, public Ui::QPrefDateUi
{
	Q_OBJECT
public:
	PrefDate(QWidget * parent = 0);
	~PrefDate() {}

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);
};


class PrefKeyboard :  public PrefModule, public Ui::QPrefKeyboardUi
{
	Q_OBJECT
public:
	PrefKeyboard(QPrefs * form, QWidget * parent = 0);
	~PrefKeyboard() {}

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);

private Q_SLOTS:
	void on_firstKeymapPB_clicked(bool);
	void on_secondKeymapPB_clicked(bool);
	void on_keymapCB_toggled(bool);

private:
	QString testKeymap(QString keymap);
};


class PrefLatex :  public PrefModule, public Ui::QPrefLatexUi
{
	Q_OBJECT
public:
	PrefLatex(QPrefs * form, QWidget * parent = 0);
	~PrefLatex() {}

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);
};


class PrefScreenFonts :  public PrefModule, public Ui::QPrefScreenFontsUi
{
	Q_OBJECT
public:
	PrefScreenFonts(QPrefs * form, QWidget * parent = 0);
	~PrefScreenFonts() {}

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);

private Q_SLOTS:
	void select_roman(const QString&);
	void select_sans(const QString&);
	void select_typewriter(const QString&);
};


class PrefColors :  public PrefModule, public Ui::QPrefColorsUi
{
	Q_OBJECT
public:
	PrefColors(QPrefs * form, QWidget * parent = 0);
	~PrefColors() {}

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

private Q_SLOTS:
	void change_color();

private:
	std::vector<LColor_color> lcolors_;
	std::vector<QString> prefcolors_;
	std::vector<QString> newcolors_;

};


class PrefCygwinPath :  public PrefModule, public Ui::QPrefCygwinPathUi
{
	Q_OBJECT
public:
	PrefCygwinPath(QWidget * parent = 0);
	~PrefCygwinPath() {}

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);
};


class PrefDisplay :  public PrefModule, public Ui::QPrefDisplayUi
{
	Q_OBJECT
public:
	PrefDisplay(QWidget * parent = 0);
	~PrefDisplay() {}

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);
};


class PrefPaths :  public PrefModule, public Ui::QPrefPathsUi
{
	Q_OBJECT
public:
	PrefPaths(QPrefs * form, QWidget * parent = 0);
	~PrefPaths() {}

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

private Q_SLOTS:
	void select_templatedir();
	void select_tempdir();
	void select_backupdir();
	void select_workingdir();
	void select_lyxpipe();

};


class PrefSpellchecker :  public PrefModule, public Ui::QPrefSpellcheckerUi
{
	Q_OBJECT
public:
	PrefSpellchecker(QPrefs * form, QWidget * parent = 0);
	~PrefSpellchecker() {}

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

private Q_SLOTS:
	void select_dict();
};


class PrefConverters :  public PrefModule, public Ui::QPrefConvertersUi
{
	Q_OBJECT
public:
	PrefConverters(QPrefs * form, QWidget * parent = 0);
	~PrefConverters() {}

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

public Q_SLOTS:
	void updateGui();

private Q_SLOTS:
	void update_converter();
	void switch_converter();
	void converter_changed();
	void remove_converter();

private:
	void updateButtons();
};


class PrefCopiers :  public PrefModule, public Ui::QPrefCopiersUi
{
	Q_OBJECT
public:
	PrefCopiers(QPrefs * form, QWidget * parent = 0);
	~PrefCopiers() {}

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

	void update();

private Q_SLOTS:
	void switch_copierLB(int nr);
	void switch_copierCO(int nr);
	void copiers_changed();
	void new_copier();
	void modify_copier();
	void remove_copier();

private:
	void updateButtons();
};


class PrefFileformats :  public PrefModule, public Ui::QPrefFileformatsUi
{
	Q_OBJECT
public:
	PrefFileformats(QPrefs * form, QWidget * parent = 0);
	~PrefFileformats() {}

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

	void update();
Q_SIGNALS:
	void formatsChanged();
private:
	void updateButtons();

private Q_SLOTS:
	void switch_format(int);
	void fileformat_changed();
	void new_format();
	void modify_format();
	void remove_format();
};


class PrefLanguage :  public PrefModule, public Ui::QPrefLanguageUi
{
	Q_OBJECT
public:
	PrefLanguage(QWidget * parent = 0);
	~PrefLanguage() {}

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

private:
	std::vector<std::string> lang_;

};


class PrefPrinter :  public PrefModule, public Ui::QPrefPrinterUi
{
	Q_OBJECT
public:
	PrefPrinter(QWidget * parent = 0);
	~PrefPrinter() {}

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);
};


class PrefUserInterface :  public PrefModule, public Ui::QPrefUi
{
	Q_OBJECT
public:
	PrefUserInterface(QPrefs * form, QWidget * parent = 0);
	~PrefUserInterface() {}

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

public Q_SLOTS:
	void select_ui();
	void select_bind();
	void on_loadWindowSizeCB_toggled(bool);

};


class PrefIdentity :  public PrefModule, public Ui::QPrefIdentityUi
{
	Q_OBJECT
public:
	PrefIdentity(QWidget * parent = 0);
	~PrefIdentity() {}

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);
};

///
class QPrefsDialog : public QDialog, public Ui::QPrefsUi
{
	Q_OBJECT
public:
	QPrefsDialog(QPrefs *);

	~QPrefsDialog();

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

public Q_SLOTS:
	void change_adaptor();

protected:
	void closeEvent(QCloseEvent * e);

private:
	void add(PrefModule * module);

private:
	QPrefs * form_;
	std::vector<PrefModule *> modules_;
};

} // namespace frontend
} // namespace lyx

#endif // PREFSDIALOG_H
