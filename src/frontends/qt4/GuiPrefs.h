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

#include "Converter.h"
#include "Format.h"
#include "KeyMap.h"
#include "LyXRC.h"
#include "Mover.h"

#include "ui_PrefsUi.h"

#include "ui_PrefOutputUi.h"
#include "ui_PrefInputUi.h"
#include "ui_PrefLatexUi.h"
#include "ui_PrefScreenFontsUi.h"
#include "ui_PrefCompletionUi.h"
#include "ui_PrefColorsUi.h"
#include "ui_PrefDisplayUi.h"
#include "ui_PrefDocHandlingUi.h"
#include "ui_PrefEditUi.h"
#include "ui_PrefPathsUi.h"
#include "ui_PrefShortcutsUi.h"
#include "ui_PrefSpellcheckerUi.h"
#include "ui_PrefConvertersUi.h"
#include "ui_PrefFileformatsUi.h"
#include "ui_PrefLanguageUi.h"
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

	void applyRC(LyXRC & rc) const;
	void updateRC(LyXRC const & rc);

public Q_SLOTS:
	void change_adaptor();

Q_SIGNALS:
	void prefsApplied(LyXRC const & rc);

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

	/// general browse
	QString browse(QString const & file, QString const & title) const;

	/// set a color
	void setColor(ColorCode col, QString const & hex);

	/// update the screen fonts after change
	void updateScreenFonts();

	/// update the previews after change
	void updatePreviews();

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

	bool update_screen_font_;
	bool update_previews_;
};


class PrefModule : public QWidget
{
	Q_OBJECT
public:
	PrefModule(QString const & cat, QString const & t,
			GuiPreferences * form)
		: QWidget(form), category_(cat), title_(t), form_(form)
	{}

	virtual void applyRC(LyXRC & rc) const = 0;
	virtual void updateRC(LyXRC const & rc) = 0;

	QString const & category() const { return category_; }
	QString const & title() const { return title_; }

protected:
	QString category_;
	QString title_;
	GuiPreferences * form_;

Q_SIGNALS:
	void changed();
};


class PrefOutput : public PrefModule, public Ui::PrefOutputUi
{
	Q_OBJECT
public:
	PrefOutput(GuiPreferences * form);

	virtual void applyRC(LyXRC & rc) const;
	virtual void updateRC(LyXRC const & rc);

private Q_SLOTS:
	void on_DateED_textChanged(const QString &);
};


class PrefInput : public PrefModule, public Ui::PrefInputUi
{
	Q_OBJECT
public:
	PrefInput(GuiPreferences * form);

	virtual void applyRC(LyXRC & rc) const;
	virtual void updateRC(LyXRC const & rc);

private Q_SLOTS:
	void on_firstKeymapPB_clicked(bool);
	void on_secondKeymapPB_clicked(bool);
	void on_keymapCB_toggled(bool);
	void on_scrollzoomEnableCB_toggled(bool);

private:
	QString testKeymap(QString const & keymap);
};


class PrefCompletion : public PrefModule, public Ui::PrefCompletionUi
{
	Q_OBJECT
public:
	PrefCompletion(GuiPreferences * form);

	virtual void applyRC(LyXRC & rc) const;
	virtual void updateRC(LyXRC const & rc);
	virtual void enableCB();
private Q_SLOTS:
	void on_popupTextCB_clicked();
	void on_inlineTextCB_clicked();
};


class PrefLatex : public PrefModule, public Ui::PrefLatexUi
{
	Q_OBJECT
public:
	PrefLatex(GuiPreferences * form);

	virtual void applyRC(LyXRC & rc) const;
	virtual void updateRC(LyXRC const & rc);

private Q_SLOTS:
	void on_latexEncodingCB_stateChanged(int state);
	void on_latexBibtexCO_activated(int n);
	void on_latexIndexCO_activated(int n);

private:
	///
	std::set<std::string> bibtex_alternatives;
	///
	std::set<std::string> index_alternatives;
};


class PrefScreenFonts : public PrefModule, public Ui::PrefScreenFontsUi
{
	Q_OBJECT
public:
	PrefScreenFonts(GuiPreferences * form);

	virtual void applyRC(LyXRC & rc) const;
	virtual void updateRC(LyXRC const & rc);

private Q_SLOTS:
	void selectRoman(const QString&);
	void selectSans(const QString&);
	void selectTypewriter(const QString&);

public Q_SLOTS:
	void updateScreenFontSizes(LyXRC const & rc);
};


class PrefColors : public PrefModule, public Ui::PrefColorsUi
{
	Q_OBJECT
public:
	PrefColors(GuiPreferences * form);

	void applyRC(LyXRC & rc) const;
	void updateRC(LyXRC const & rc);

private Q_SLOTS:
	void changeColor();
	void changeSysColor();
	void changeLyxObjectsSelection();

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

	void applyRC(LyXRC & rc) const;
	void updateRC(LyXRC const & rc);

private Q_SLOTS:
	void on_instantPreviewCO_currentIndexChanged(int);
};


class PrefPaths : public PrefModule, public Ui::PrefPathsUi
{
	Q_OBJECT
public:
	PrefPaths(GuiPreferences * form);

	void applyRC(LyXRC & rc) const;
	void updateRC(LyXRC const & rc);

private Q_SLOTS:
	void selectExampledir();
	void selectTemplatedir();
	void selectTempdir();
	void selectBackupdir();
	void selectWorkingdir();
	void selectThesaurusdir();
	void selectHunspelldir();
	void selectLyxPipe();

};


class PrefSpellchecker : public PrefModule, public Ui::PrefSpellcheckerUi
{
	Q_OBJECT
public:
	PrefSpellchecker(GuiPreferences * form);

	void applyRC(LyXRC & rc) const;
	void updateRC(LyXRC const & rc);

private Q_SLOTS:
	void on_spellcheckerCB_currentIndexChanged(int);
};


class PrefConverters : public PrefModule, public Ui::PrefConvertersUi
{
	Q_OBJECT
public:
	PrefConverters(GuiPreferences * form);

	void applyRC(LyXRC & rc) const;
	void updateRC(LyXRC const & rc);

public Q_SLOTS:
	void updateGui();

private Q_SLOTS:
	void updateConverter();
	void switchConverter();
	void removeConverter();
	void changeConverter();
	void on_cacheCB_stateChanged(int state);

private:
	void updateButtons();
};


class PrefFileformats : public PrefModule, public Ui::PrefFileformatsUi
{
	Q_OBJECT
public:
	PrefFileformats(GuiPreferences * form);

	void applyRC(LyXRC & rc) const;
	void updateRC(LyXRC const & rc);
	void updateView();

Q_SIGNALS:
	void formatsChanged();

private Q_SLOTS:
	void on_copierED_textEdited(const QString & s);
	void on_extensionsED_textEdited(const QString &);
	void on_viewerED_textEdited(const QString &);
	void on_editorED_textEdited(const QString &);
	void on_mimeED_textEdited(const QString &);
	void on_shortcutED_textEdited(const QString &);
	void on_formatED_editingFinished();
	void on_formatED_textChanged(const QString &);
	void on_formatsCB_currentIndexChanged(int);
	void on_formatsCB_editTextChanged(const QString &);
	void on_formatNewPB_clicked();
	void on_formatRemovePB_clicked();
	void on_viewerCO_currentIndexChanged(int i);
	void on_editorCO_currentIndexChanged(int i);
	void setFlags();
	void updatePrettyname();

private:
	Format & currentFormat();
	///
	void updateViewers();
	///
	void updateEditors();
	///
	LyXRC::Alternatives viewer_alternatives;
	///
	LyXRC::Alternatives editor_alternatives;
};


class PrefLanguage : public PrefModule, public Ui::PrefLanguageUi
{
	Q_OBJECT
public:
	PrefLanguage(GuiPreferences * form);

	void applyRC(LyXRC & rc) const;
	void updateRC(LyXRC const & rc);

private Q_SLOTS:
	void on_uiLanguageCO_currentIndexChanged(int);
	void on_languagePackageCO_currentIndexChanged(int);
};


class PrefUserInterface : public PrefModule, public Ui::PrefUi
{
	Q_OBJECT
public:
	PrefUserInterface(GuiPreferences * form);

	void applyRC(LyXRC & rc) const;
	void updateRC(LyXRC const & rc);

public Q_SLOTS:
	void selectUi();
};


class PrefDocHandling : public PrefModule, public Ui::PrefDocHandlingUi
{
	Q_OBJECT
public:
	PrefDocHandling(GuiPreferences * form);

	void applyRC(LyXRC & rc) const;
	void updateRC(LyXRC const & rc);

public Q_SLOTS:
	void on_clearSessionPB_clicked();
};



class PrefEdit : public PrefModule, public Ui::PrefEditUi
{
	Q_OBJECT
public:
	PrefEdit(GuiPreferences * form);

	void applyRC(LyXRC & rc) const;
	void updateRC(LyXRC const & rc);
};



class GuiShortcutDialog : public QDialog, public Ui::shortcutUi
{
public:
	GuiShortcutDialog(QWidget * parent);
};


class PrefShortcuts : public PrefModule, public Ui::PrefShortcuts
{
	Q_OBJECT
public:
	PrefShortcuts(GuiPreferences * form);

	void applyRC(LyXRC & rc) const;
	void updateRC(LyXRC const & rc);
	void updateShortcutsTW();

public Q_SLOTS:
	void selectBind();
	void on_modifyPB_pressed();
	void on_newPB_pressed();
	void on_removePB_pressed();
	void on_searchLE_textEdited();
	///
	void on_shortcutsTW_itemSelectionChanged();
	void on_shortcutsTW_itemDoubleClicked();
	///
	void shortcutOkPressed();
	void shortcutCancelPressed();
	void shortcutClearPressed();
	void shortcutRemovePressed();

private:
	void modifyShortcut();
	/// remove selected binding, restore default value
	void removeShortcut();
	/// remove bindings, do not restore default values
	void deactivateShortcuts(QList<QTreeWidgetItem*> const & items);
	/// check the new binding k->func, and remove existing bindings to k after
	/// asking the user. We exclude lfun_to_modify from this test: we assume
	/// that if the user clicked "modify" then they agreed to modify the
	/// binding. Returns false if the shortcut is invalid or the user cancels.
	bool validateNewShortcut(FuncRequest const & func,
	                         KeySequence const & k,
	                         QString const & lfun_to_modify);
	/// compute current active shortcut
	FuncRequest currentBinding(KeySequence const & k);
	///
	void setItemType(QTreeWidgetItem * item, KeyMap::ItemType tag);
	///
	static KeyMap::ItemType itemType(QTreeWidgetItem & item);
	/// some items need to be always hidden, for instance empty rebound
	/// system keys
	static bool isAlwaysHidden(QTreeWidgetItem & item);
	/// unhide an empty system binding that may have been hidden
	/// returns either null or the unhidden shortcut
	void unhideEmpty(QString const & lfun, bool select);
	///
	QTreeWidgetItem * insertShortcutItem(FuncRequest const & lfun,
		KeySequence const & shortcut, KeyMap::ItemType tag);
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

	void applyRC(LyXRC & rc) const;
	void updateRC(LyXRC const & rc);
};


} // namespace frontend
} // namespace lyx

#endif // GUIPREFS_H
