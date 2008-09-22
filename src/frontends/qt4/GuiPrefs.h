// -*- C++ -*-
/**
 * \file GuiPrefs.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Bo Peng
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIPREFS_H
#define GUIPREFS_H

#include "GuiDialog.h"

#include "ColorCode.h"
#include "Converter.h"
#include "Format.h"
#include "FuncCode.h"
#include "KeyMap.h"
#include "LyXRC.h"
#include "Mover.h"

#include "ui_PrefsUi.h"

#include "ui_PrefPlaintextUi.h"
#include "ui_PrefDateUi.h"
#include "ui_PrefInputUi.h"
#include "ui_PrefLatexUi.h"
#include "ui_PrefScreenFontsUi.h"
#include "ui_PrefCompletionUi.h"
#include "ui_PrefColorsUi.h"
#include "ui_PrefDisplayUi.h"
#include "ui_PrefEditUi.h"
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

#include <string>
#include <vector>


namespace lyx {

class Converters;
class Formats;
class Movers;

namespace frontend {

class PrefModule;

class GuiPreferences : public GuiDialog, public Ui::PrefsUi
{
	Q_OBJECT
public:
	GuiPreferences(GuiView & lv);

	void apply(LyXRC & rc) const;
	void updateRc(LyXRC const & rc);

public Q_SLOTS:
	void change_adaptor();

public:
	/// Apply changes
	void applyView();

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
	QString browsebind(QString const & file) const;
	QString browseUI(QString const & file) const;
	QString browsekbmap(QString const & file) const;
	QString browsedict(QString const & file) const;

	/// general browse
	QString browse(QString const & file, QString const & title) const;

	/// set a color
	void setColor(ColorCode col, QString const & hex);

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
	///
	void addModule(PrefModule * module);

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


class PrefModule : public QWidget
{
	Q_OBJECT
public:
	PrefModule(QString const & cat, QString const & t,
			GuiPreferences * form)
		: QWidget(form), category_(cat), title_(t), form_(form)
	{}

	virtual void apply(LyXRC & rc) const = 0;
	virtual void update(LyXRC const & rc) = 0;

	QString const & category() const { return category_; }
	QString const & title() const { return title_; }

protected:
	QString category_;
	QString title_;
	GuiPreferences * form_;

Q_SIGNALS:
	void changed();
};


class PrefPlaintext : public PrefModule, public Ui::PrefPlaintextUi
{
	Q_OBJECT
public:
	PrefPlaintext(GuiPreferences * form);

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);
};


class PrefDate : public PrefModule, public Ui::PrefDateUi
{
	Q_OBJECT
public:
	PrefDate(GuiPreferences * form);

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);
};


class PrefInput : public PrefModule, public Ui::PrefInputUi
{
	Q_OBJECT
public:
	PrefInput(GuiPreferences * form);

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);

private Q_SLOTS:
	void on_firstKeymapPB_clicked(bool);
	void on_secondKeymapPB_clicked(bool);
	void on_keymapCB_toggled(bool);

private:
	QString testKeymap(QString const & keymap);
};


class PrefCompletion : public PrefModule, public Ui::PrefCompletionUi
{
	Q_OBJECT
public:
	PrefCompletion(GuiPreferences * form);

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);
};


class PrefLatex : public PrefModule, public Ui::PrefLatexUi
{
	Q_OBJECT
public:
	PrefLatex(GuiPreferences * form);

	virtual void apply(LyXRC & rc) const;
	virtual void update(LyXRC const & rc);
};


class PrefScreenFonts : public PrefModule, public Ui::PrefScreenFontsUi
{
	Q_OBJECT
public:
	PrefScreenFonts(GuiPreferences * form);

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
	PrefColors(GuiPreferences * form);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

private Q_SLOTS:
	void change_color();
	void change_lyxObjects_selection();

private:
	std::vector<ColorCode> lcolors_;
	std::vector<QString> curcolors_;
	std::vector<QString> newcolors_;
};


class PrefDisplay : public PrefModule, public Ui::PrefDisplayUi
{
	Q_OBJECT
public:
	PrefDisplay(GuiPreferences * form);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);
};


class PrefPaths : public PrefModule, public Ui::PrefPathsUi
{
	Q_OBJECT
public:
	PrefPaths(GuiPreferences * form);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

private Q_SLOTS:
	void select_exampledir();
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
	PrefSpellchecker(GuiPreferences * form);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

private Q_SLOTS:
	void select_dict();
};


class PrefConverters : public PrefModule, public Ui::PrefConvertersUi
{
	Q_OBJECT
public:
	PrefConverters(GuiPreferences * form);

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


class PrefFileformats : public PrefModule, public Ui::PrefFileformatsUi
{
	Q_OBJECT
public:
	PrefFileformats(GuiPreferences * form);

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
	PrefLanguage(GuiPreferences * form);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

private Q_SLOTS:
	void on_uiLanguageCO_currentIndexChanged(int);
};


class PrefPrinter : public PrefModule, public Ui::PrefPrinterUi
{
	Q_OBJECT
public:
	PrefPrinter(GuiPreferences * form);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);
};


class PrefUserInterface : public PrefModule, public Ui::PrefUi
{
	Q_OBJECT
public:
	PrefUserInterface(GuiPreferences * form);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);

public Q_SLOTS:
	void select_ui();
	void on_clearSessionPB_clicked();
};


class PrefEdit : public PrefModule, public Ui::PrefEditUi
{
	Q_OBJECT
public:
	PrefEdit(GuiPreferences * form);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);
};



class GuiShortcutDialog : public QDialog, public Ui::shortcutUi
{
public:
	GuiShortcutDialog(QWidget * parent);
};


class PrefShortcuts : public PrefModule, public Ui::PrefShortcuts
{
	Q_OBJECT
private:
	enum ItemType {
		System,         //< loaded from a bind file
		UserBind,       //< \bind loaded from user.bind
		UserUnbind,     //< \unbind loaded from user.bind, with corresponding
		                //<    entry in system bind file
		UserExtraUnbind	//< \unbind loaded from user.bind, without
		                //<    corresponding entry in system bind file.
	};
public:
	PrefShortcuts(GuiPreferences * form);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);
	void updateShortcutsTW();
	void modifyShortcut();
	void removeShortcut();
	///
	void setItemType(QTreeWidgetItem * item, ItemType tag);
	QTreeWidgetItem * insertShortcutItem(FuncRequest const & lfun, 
		KeySequence const & shortcut, ItemType tag);

public Q_SLOTS:
	void select_bind();
	void on_modifyPB_pressed();
	void on_newPB_pressed();
	void on_removePB_pressed();
	void on_searchLE_textEdited();
	///
	void on_shortcutsTW_itemSelectionChanged();
	void shortcut_okPB_pressed();
	void shortcut_cancelPB_pressed();
	void shortcut_clearPB_pressed();
	void shortcut_removePB_pressed();
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
	///
	QString save_lfun_;
};


class PrefIdentity : public PrefModule, public Ui::PrefIdentityUi
{
	Q_OBJECT
public:
	PrefIdentity(GuiPreferences * form);

	void apply(LyXRC & rc) const;
	void update(LyXRC const & rc);
};


} // namespace frontend
} // namespace lyx

#endif // GUIPREFS_H
