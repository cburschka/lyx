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

#include "GuiDialog.h"

#include "Color.h"
#include "Converter.h"
#include "Format.h"
#include "KeyMap.h"
#include "lfuns.h"
#include "LyXRC.h"
#include "Mover.h"

#include "ui_PrefsUi.h"

#include "ui_PrefPlaintextUi.h"
#include "ui_PrefDateUi.h"
#include "ui_PrefKeyboardUi.h"
#include "ui_PrefLatexUi.h"
#include "ui_PrefScreenFontsUi.h"
#include "ui_PrefColorsUi.h"
#include "ui_PrefDisplayUi.h"
#include "ui_PrefPathsUi.h"
#include "ui_PrefShortcutsUi.h"
#include "ui_PrefSpellcheckerUi.h"
#include "ui_PrefConvertersUi.h"
#include "ui_PrefFileformatsUi.h"
#include "ui_PrefLanguageUi.h"
#include "ui_PrefPrinterUi.h"
#include "ui_PrefUi.h"
#include "ui_PrefIdentityUi.h"
#include "ui_ShortcutUi.h"

#include <QDialog>
#include <QValidator>

#include <string>
#include <vector>


namespace lyx {

class Color_color;
class Converters;
class Formats;
class Movers;

namespace frontend {

class GuiPreferences;

class PrefModule : public QWidget
{
	Q_OBJECT
public:
	PrefModule(docstring const & t,
			GuiPreferences * form = 0, QWidget * parent = 0)
		: QWidget(parent), title_(t), form_(form)
	{}

	virtual void apply(LyXRC & rc) const = 0;
	virtual void update(LyXRC const & rc) = 0;

	docstring const & title() const { return title_; }

protected:
	docstring title_;
	GuiPreferences * form_;

Q_SIGNALS:
	void changed();
};


class PrefPlaintext : public PrefModule, public Ui::PrefPlaintextUi
{
	Q_OBJECT
public:
	PrefPlaintext(QWidget * parent = 0);

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);
};


class PrefDate : public PrefModule, public Ui::PrefDateUi
{
	Q_OBJECT
public:
	PrefDate(QWidget * parent = 0);

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);
};


class PrefKeyboard : public PrefModule, public Ui::PrefKeyboardUi
{
	Q_OBJECT
public:
	PrefKeyboard(GuiPreferences * form, QWidget * parent = 0);

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);

private Q_SLOTS:
	void on_firstKeymapPB_clicked(bool);
	void on_secondKeymapPB_clicked(bool);
	void on_keymapCB_toggled(bool);

private:
	QString testKeymap(QString keymap);
};


class PrefLatex : public PrefModule, public Ui::PrefLatexUi
{
	Q_OBJECT
public:
	PrefLatex(GuiPreferences * form, QWidget * parent = 0);

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);
};


class PrefScreenFonts : public PrefModule, public Ui::PrefScreenFontsUi
{
	Q_OBJECT
public:
	PrefScreenFonts(GuiPreferences * form, QWidget * parent = 0);

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);

private Q_SLOTS:
	void select_roman(const QString&);
	void select_sans(const QString&);
	void select_typewriter(const QString&);
};


class PrefColors : public PrefModule, public Ui::PrefColorsUi
{
	Q_OBJECT
public:
	PrefColors(GuiPreferences * form, QWidget * parent = 0);

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


class PrefDisplay : public PrefModule, public Ui::PrefDisplayUi
{
	Q_OBJECT
public:
	PrefDisplay(QWidget * parent = 0);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);
};


class PrefPaths : public PrefModule, public Ui::PrefPathsUi
{
	Q_OBJECT
public:
	PrefPaths(GuiPreferences * form, QWidget * parent = 0);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

private Q_SLOTS:
	void select_templatedir();
	void select_tempdir();
	void select_backupdir();
	void select_workingdir();
	void select_lyxpipe();

};


class PrefSpellchecker : public PrefModule, public Ui::PrefSpellcheckerUi
{
	Q_OBJECT
public:
	PrefSpellchecker(GuiPreferences * form, QWidget * parent = 0);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

private Q_SLOTS:
	void select_dict();
};


class PrefConverters : public PrefModule, public Ui::PrefConvertersUi
{
	Q_OBJECT
public:
	PrefConverters(GuiPreferences * form, QWidget * parent = 0);

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


class FormatValidator : public QValidator
{
public:
	FormatValidator(QWidget *, Formats const & f);
	void fixup(QString & input) const;
	QValidator::State validate(QString & input, int & pos) const;
private:
	virtual std::string str(Formats::const_iterator it) const = 0;
	int nr() const;
	Formats const & formats_;
};


class FormatNameValidator : public FormatValidator
{
public:
	FormatNameValidator(QWidget *, Formats const & f);
private:
	std::string str(Formats::const_iterator it) const;
};

class FormatPrettynameValidator : public FormatValidator
{
public:
	FormatPrettynameValidator(QWidget *, Formats const & f);
private:
	std::string str(Formats::const_iterator it) const;
};


class PrefFileformats : public PrefModule, public Ui::PrefFileformatsUi
{
	Q_OBJECT
public:
	PrefFileformats(GuiPreferences * form, QWidget * parent = 0);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);
	void updateView();

Q_SIGNALS:
	void formatsChanged();

private Q_SLOTS:
	void on_copierED_textEdited(const QString & s);
	void on_extensionED_textEdited(const QString &);
	void on_viewerED_textEdited(const QString &);
	void on_editorED_textEdited(const QString &);
	void on_shortcutED_textEdited(const QString &);
	void on_formatED_editingFinished();
	void on_formatED_textChanged(const QString &);
	void on_formatsCB_currentIndexChanged(int);
	void on_formatsCB_editTextChanged(const QString &);
	void on_formatNewPB_clicked();
	void on_formatRemovePB_clicked();
	void setFlags();
	void updatePrettyname();

private:
	Format & currentFormat();
};


class PrefLanguage : public PrefModule, public Ui::PrefLanguageUi
{
	Q_OBJECT
public:
	PrefLanguage(QWidget * parent = 0);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

private:
	std::vector<std::string> lang_;
};


class PrefPrinter : public PrefModule, public Ui::PrefPrinterUi
{
	Q_OBJECT
public:
	PrefPrinter(QWidget * parent = 0);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);
};


class PrefUserInterface : public PrefModule, public Ui::PrefUi
{
	Q_OBJECT
public:
	PrefUserInterface(GuiPreferences * form, QWidget * parent = 0);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

public Q_SLOTS:
	void select_ui();
	void on_loadWindowSizeCB_toggled(bool);

};


class GuiShortcutDialog : public QDialog, public Ui::ShortcutUi
{
public:
	GuiShortcutDialog(QWidget * parent) : QDialog(parent)
	{
		Ui::ShortcutUi::setupUi(this);
		QDialog::setModal(true);
	}
};


class PrefShortcuts : public PrefModule, public Ui::PrefShortcuts
{
	Q_OBJECT
private:
	enum item_type {
		System,		//< loaded from a bind file
		UserBind,	//< \bind loaded from user.bind
		UserUnbind	//< \unbind loaded from user.bind
	};
public:
	PrefShortcuts(GuiPreferences * form, QWidget * parent = 0);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);
	void updateShortcutsTW();
	///
	void setItemType(QTreeWidgetItem * item, item_type tag);
	QTreeWidgetItem * insertShortcutItem(FuncRequest const & lfun, 
		KeySequence const & shortcut, item_type tag);

public Q_SLOTS:
	void select_bind();
	void on_newPB_pressed();
	void on_removePB_pressed();
	void on_searchPB_pressed();
	void on_searchLE_textChanged();
	///
	void on_shortcutsTW_itemSelectionChanged();
	void shortcut_okPB_pressed();
	void on_shortcutsTW_itemDoubleClicked();

private:
	///
	GuiShortcutDialog * shortcut_;
	///
	ButtonController shortcut_bc_;
	/// category items
	QTreeWidgetItem * editItem_;
	QTreeWidgetItem * mathItem_;
	QTreeWidgetItem * bufferItem_;
	QTreeWidgetItem * layoutItem_;
	QTreeWidgetItem * systemItem_;
	// system_bind_ holds bindings from rc.bind_file
	// user_bind_ holds \bind bindings from user.bind
	// user_unbind_ holds \unbind bindings from user.bind
	// When an item is inserted, it is added to user_bind_
	// When an item from system_bind_ is deleted, it is added to user_unbind_
	// When an item in user_bind_ or user_unbind_ is deleted, it is 
	//	deleted (unbind)
	KeyMap system_bind_;
	KeyMap user_bind_;
	KeyMap user_unbind_;
};


class PrefIdentity : public PrefModule, public Ui::PrefIdentityUi
{
	Q_OBJECT
public:
	PrefIdentity(QWidget * parent = 0);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);
};


class GuiPreferences : public GuiDialog, public Ui::PrefsUi
{
	Q_OBJECT
public:
	GuiPreferences(LyXView & lv);

	void apply(LyXRC & rc) const;
	void updateRc(LyXRC const & rc);

public Q_SLOTS:
	void change_adaptor();

public:
	//
	void closeEvent(QCloseEvent * e);
	///
	void add(PrefModule * module);
	/// Apply changes
	void applyView();
	/// update (do we need this?)
	void updateContents();

	std::vector<PrefModule *> modules_;

	///
	bool initialiseParams(std::string const &);
	///
	void clearParams() {}
	///
	void dispatchParams();
	///
	bool isBufferDependent() const { return false; }

	/// various file pickers
	docstring const browsebind(docstring const & file) const;
	docstring const browseUI(docstring const & file) const;
	docstring const browsekbmap(docstring const & file) const;
	docstring const browsedict(docstring const & file) const;

	/// general browse
	docstring const browse(docstring const & file,
				 docstring const & title) const;

	/// browse directory
	docstring const browsedir(docstring const & path,
				    docstring const & title) const;

	/// set a color
	void setColor(Color_color col, std::string const & hex);

	/// update the screen fonts after change
	void updateScreenFonts();

	/// adjust the prefs paper sizes
	PAPER_SIZE toPaperSize(int i) const;
	/// adjust the prefs paper sizes
	int fromPaperSize(PAPER_SIZE papersize) const;

	LyXRC & rc() { return rc_; }
	Converters & converters() { return converters_; }
	Formats & formats() { return formats_; }
	Movers & movers() { return movers_; }

private:
	/// temporary lyxrc
	LyXRC rc_;
	/// temporary converters
	Converters converters_;
	/// temporary formats
	Formats formats_;
	/// temporary movers
	Movers movers_;

	/// A list of colors to be dispatched
	std::vector<std::string> colors_;

	bool redraw_gui_;
	bool update_screen_font_;
};


} // namespace frontend
} // namespace lyx

#endif // GUIPREFS_H
