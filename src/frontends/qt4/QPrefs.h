// -*- C++ -*-
/**
 * \file QPrefs.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QPREFS_H
#define QPREFS_H

#include "QDialogView.h"

#include "Color.h"
#include "LyXRC.h"

#include "ui_PrefsUi.h"

#include <vector>

#include <QDialog>
#include <QCloseEvent>

#include "ui_PrefPlaintextUi.h"
#include "ui_PrefDateUi.h"
#include "ui_PrefKeyboardUi.h"
#include "ui_PrefLatexUi.h"
#include "ui_PrefScreenFontsUi.h"
#include "ui_PrefColorsUi.h"
#include "ui_PrefCygwinPathUi.h"
#include "ui_PrefDisplayUi.h"
#include "ui_PrefPathsUi.h"
#include "ui_PrefSpellcheckerUi.h"
#include "ui_PrefConvertersUi.h"
#include "ui_PrefCopiersUi.h"
#include "ui_PrefFileformatsUi.h"
#include "ui_PrefLanguageUi.h"
#include "ui_PrefPrinterUi.h"
#include "ui_PrefUi.h"
#include "ui_PrefIdentityUi.h"

namespace lyx {

class Converters;
class Formats;
class Movers;

namespace frontend {

class QPrefs;

class PrefModule : public QWidget
{
	Q_OBJECT
public:
	PrefModule(docstring const & t,
			QPrefs * form = 0, QWidget * parent = 0)
		: QWidget(parent), title_(t), form_(form)
	{}

	virtual void apply(LyXRC & rc) const = 0;
	virtual void update(LyXRC const & rc) = 0;

	docstring const & title() const { return title_; }

protected:
	docstring title_;
	QPrefs * form_;

Q_SIGNALS:
	void changed();
};


class PrefPlaintext :  public PrefModule, public Ui::QPrefPlaintextUi
{
	Q_OBJECT
public:
	PrefPlaintext(QWidget * parent = 0);

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);
};


class PrefDate :  public PrefModule, public Ui::QPrefDateUi
{
	Q_OBJECT
public:
	PrefDate(QWidget * parent = 0);

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);
};


class PrefKeyboard :  public PrefModule, public Ui::QPrefKeyboardUi
{
	Q_OBJECT
public:
	PrefKeyboard(QPrefs * form, QWidget * parent = 0);

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

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);
};


class PrefScreenFonts :  public PrefModule, public Ui::QPrefScreenFontsUi
{
	Q_OBJECT
public:
	PrefScreenFonts(QPrefs * form, QWidget * parent = 0);

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

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

private Q_SLOTS:
	void change_color();
	void change_lyxObjects_selection();

private:
	std::vector<Color_color> lcolors_;
	// FIXME the use of mutable here is required due to the
	// fact that initialization is not done in the controller
	// but in the constructor.
	std::vector<QString> curcolors_;
	std::vector<QString> newcolors_;

};


class PrefCygwinPath :  public PrefModule, public Ui::QPrefCygwinPathUi
{
	Q_OBJECT
public:
	PrefCygwinPath(QWidget * parent = 0);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);
};


class PrefDisplay :  public PrefModule, public Ui::QPrefDisplayUi
{
	Q_OBJECT
public:
	PrefDisplay(QWidget * parent = 0);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);
};


class PrefPaths :  public PrefModule, public Ui::QPrefPathsUi
{
	Q_OBJECT
public:
	PrefPaths(QPrefs * form, QWidget * parent = 0);

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

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

public Q_SLOTS:
	void updateGui();

private Q_SLOTS:
	void update_converter();
	void switch_converter();
	void converter_changed();
	void remove_converter();
	void on_cacheCB_stateChanged(int state);

private:
	void updateButtons();
};


class PrefCopiers :  public PrefModule, public Ui::QPrefCopiersUi
{
	Q_OBJECT
public:
	PrefCopiers(QPrefs * form, QWidget * parent = 0);

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

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);
};


class PrefUserInterface :  public PrefModule, public Ui::QPrefUi
{
	Q_OBJECT
public:
	PrefUserInterface(QPrefs * form, QWidget * parent = 0);

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

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);
};

///
class QPrefsDialog : public QDialog, public Ui::QPrefsUi
{
	Q_OBJECT
public:
	QPrefsDialog(QPrefs *);

	void apply(LyXRC & rc) const;
	void updateRc(LyXRC const & rc);

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


class ControlPrefs;

class QPrefs
	: public QController<ControlPrefs, QView<QPrefsDialog> >
{
public:
	QPrefs(Dialog &);

	Converters & converters();
	Formats & formats();
	Movers & movers();

private:
	/// Apply changes
	virtual void apply();

	/// update (do we need this?)
	virtual void update_contents();

	/// build the dialog
	virtual void build_dialog();

};

} // namespace frontend
} // namespace lyx

#endif // QPREFS_H
