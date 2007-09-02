// -*- C++ -*-
/**
 * \file GuiPrefs.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIPREFS_H
#define GUIPREFS_H

#include "GuiDialogView.h"
#include "ControlPrefs.h"
#include "Color.h"
#include "LyXRC.h"

#include "ui_PrefsUi.h"

#include "ui_PrefPlaintextUi.h"
#include "ui_PrefDateUi.h"
#include "ui_PrefKeyboardUi.h"
#include "ui_PrefLatexUi.h"
#include "ui_PrefScreenFontsUi.h"
#include "ui_PrefColorsUi.h"
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

#include <QDialog>

#include <vector>

namespace lyx {

class Converters;
class Formats;
class Movers;

namespace frontend {

class GuiPrefs;

class PrefModule : public QWidget
{
	Q_OBJECT
public:
	PrefModule(docstring const & t,
			GuiPrefs * form = 0, QWidget * parent = 0)
		: QWidget(parent), title_(t), form_(form)
	{}

	virtual void apply(LyXRC & rc) const = 0;
	virtual void update(LyXRC const & rc) = 0;

	docstring const & title() const { return title_; }

protected:
	docstring title_;
	GuiPrefs * form_;

Q_SIGNALS:
	void changed();
};


class PrefPlaintext :  public PrefModule, public Ui::PrefPlaintextUi
{
	Q_OBJECT
public:
	PrefPlaintext(QWidget * parent = 0);

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);
};


class PrefDate :  public PrefModule, public Ui::PrefDateUi
{
	Q_OBJECT
public:
	PrefDate(QWidget * parent = 0);

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);
};


class PrefKeyboard :  public PrefModule, public Ui::PrefKeyboardUi
{
	Q_OBJECT
public:
	PrefKeyboard(GuiPrefs * form, QWidget * parent = 0);

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);

private Q_SLOTS:
	void on_firstKeymapPB_clicked(bool);
	void on_secondKeymapPB_clicked(bool);
	void on_keymapCB_toggled(bool);

private:
	QString testKeymap(QString keymap);
};


class PrefLatex :  public PrefModule, public Ui::PrefLatexUi
{
	Q_OBJECT
public:
	PrefLatex(GuiPrefs * form, QWidget * parent = 0);

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);
};


class PrefScreenFonts :  public PrefModule, public Ui::PrefScreenFontsUi
{
	Q_OBJECT
public:
	PrefScreenFonts(GuiPrefs * form, QWidget * parent = 0);

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);

private Q_SLOTS:
	void select_roman(const QString&);
	void select_sans(const QString&);
	void select_typewriter(const QString&);
};


class PrefColors :  public PrefModule, public Ui::PrefColorsUi
{
	Q_OBJECT
public:
	PrefColors(GuiPrefs * form, QWidget * parent = 0);

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


class PrefDisplay :  public PrefModule, public Ui::PrefDisplayUi
{
	Q_OBJECT
public:
	PrefDisplay(QWidget * parent = 0);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);
};


class PrefPaths :  public PrefModule, public Ui::PrefPathsUi
{
	Q_OBJECT
public:
	PrefPaths(GuiPrefs * form, QWidget * parent = 0);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

private Q_SLOTS:
	void select_templatedir();
	void select_tempdir();
	void select_backupdir();
	void select_workingdir();
	void select_lyxpipe();

};


class PrefSpellchecker :  public PrefModule, public Ui::PrefSpellcheckerUi
{
	Q_OBJECT
public:
	PrefSpellchecker(GuiPrefs * form, QWidget * parent = 0);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

private Q_SLOTS:
	void select_dict();
};


class PrefConverters :  public PrefModule, public Ui::PrefConvertersUi
{
	Q_OBJECT
public:
	PrefConverters(GuiPrefs * form, QWidget * parent = 0);

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


class PrefCopiers :  public PrefModule, public Ui::PrefCopiersUi
{
	Q_OBJECT
public:
	PrefCopiers(GuiPrefs * form, QWidget * parent = 0);

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


class PrefFileformats :  public PrefModule, public Ui::PrefFileformatsUi
{
	Q_OBJECT
public:
	PrefFileformats(GuiPrefs * form, QWidget * parent = 0);

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


class PrefLanguage :  public PrefModule, public Ui::PrefLanguageUi
{
	Q_OBJECT
public:
	PrefLanguage(QWidget * parent = 0);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

private:
	std::vector<std::string> lang_;
};


class PrefPrinter :  public PrefModule, public Ui::PrefPrinterUi
{
	Q_OBJECT
public:
	PrefPrinter(QWidget * parent = 0);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);
};


class PrefUserInterface :  public PrefModule, public Ui::PrefUi
{
	Q_OBJECT
public:
	PrefUserInterface(GuiPrefs * form, QWidget * parent = 0);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

public Q_SLOTS:
	void select_ui();
	void select_bind();
	void on_loadWindowSizeCB_toggled(bool);

};


class PrefIdentity :  public PrefModule, public Ui::PrefIdentityUi
{
	Q_OBJECT
public:
	PrefIdentity(QWidget * parent = 0);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);
};

///
class GuiPrefsDialog : public QDialog, public Ui::PrefsUi
{
	Q_OBJECT
public:
	GuiPrefsDialog(GuiPrefs *);

	void apply(LyXRC & rc) const;
	void updateRc(LyXRC const & rc);

public Q_SLOTS:
	void change_adaptor();

protected:
	void closeEvent(QCloseEvent * e);

private:
	void add(PrefModule * module);

private:
	GuiPrefs * form_;
	std::vector<PrefModule *> modules_;
};


class GuiPrefs : public GuiView<GuiPrefsDialog>
{
public:
	GuiPrefs(Dialog &);

	Converters & converters();
	Formats & formats();
	Movers & movers();
	/// parent controller
	ControlPrefs & controller()
	{ return static_cast<ControlPrefs &>(this->getController()); }
	/// parent controller
	ControlPrefs const & controller() const
	{ return static_cast<ControlPrefs const &>(this->getController()); }
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

#endif // GUIPREFS_H
